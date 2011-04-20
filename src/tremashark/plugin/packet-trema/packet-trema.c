/*
 * Author: Yasunobu Chiba, Yasunori Nakazawa
 *
 * Copyright (C) 2008-2011 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <epan/emem.h>
#include <epan/packet.h>
#include <epan/dissectors/packet-tcp.h>
#include <epan/prefs.h>
#include <epan/ipproto.h>
#include <epan/etypes.h>
#include <epan/addr_resolv.h>
#include <epan/reassemble.h>
#include <string.h>
#include <arpa/inet.h>
#include "trema.h"
#include "openflow_service_interface.h"


#define PROTO_TAG_TREMA  "TREMA"

#define NO_STRINGS NULL
#define NO_MASK 0x0


// Macro for debug use
//#define PRINTF( ... ) printf( __VA_ARGS__ )
#define PRINTF( ... )


// Wireshark ID
static int proto_trema = -1;
//static dissector_handle_t trema_handle;
static void dissect_trema( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree );


// Structure defined in tremashark.c
typedef struct message_pcap_dump_header {
  uint16_t dump_type;
  struct {
    uint32_t sec;
    uint32_t nsec;
  } sent_time;
  uint16_t app_name_len;
  uint16_t service_name_len;
  uint32_t data_len;
}  __attribute__( ( packed ) ) message_pcap_dump_header;

// Structure and enum definitions defined in messenger.c
typedef struct message_header {
  uint8_t version;         // version = 0 (unused)
  uint8_t message_type;    // MESSAGE_TYPE_
  uint16_t tag;            // user defined
  uint32_t message_length; // message length including header
  uint8_t value[ 0 ];
} message_header;

typedef struct stream_id {
  gchar *app_name;
  guint16 app_name_length;
  gchar *service_name;
  guint16 service_name_length;
} stream_id;

// Fragmented stream information
typedef struct fragmented_stream_info {
  stream_id stream_name;      // app name and service name pair
  gint32 unreceived_length;   // unreceived length
  guint32 message_length;     // expected message length
  guint32 reassemble_id;      // unique id for reassembling
  guint32 number;             // fragment count of the stream. start from 0.
  gboolean temporary_length;  // set TRUE until it received message_header.
} fragmented_stream_info;

// Packet status information
typedef struct packet_status_info {
  guint32 packet_number;
  guint32 reassemble_id;
} packet_status_info;


// Try to find the OpenFlow dissector to dissect encapsulated OpenFlow message
static dissector_handle_t data_openflow;

// Fragment tables
static GHashTable *trema_fragment_table = NULL;
static GHashTable *trema_reassembled_table = NULL;

// Fragment status list
static dlist_element *fragments_status = NULL;
static packet_status_info **packets_status = NULL;
static guint32 packets_status_size = 0;   // allocated element count of packets_status;

// Header fields
static gint hf_dump_header = -1;
static gint hf_dump_type = -1;
static gint hf_dump_event_time = -1;
static gint hf_dump_app_name_length = -1;
static gint hf_dump_service_name_length = -1;
static gint hf_dump_data_length = -1;
static gint hf_dump_app_name = -1;
static gint hf_dump_service_name = -1;
static gint hf_message_header = -1;
static gint hf_version = -1;
static gint hf_message_type = -1;
static gint hf_tag = -1;
static gint hf_message_length = -1;
static gint hf_service_header = -1;
static gint hf_context_handle = -1;
static gint hf_datapath_id = -1;
static gint hf_service_name_length = -1;
static gint hf_service_name = -1;
static gint hf_transaction_id = -1;
static gint hf_hex_dump = -1;

// Subtrees
static gint ett_trema = -1;
static gint ett_dump_header = -1;
static gint ett_message_header = -1;
static gint ett_service_header = -1;
static gint ett_context_handle = -1;

// Fragment subtrees
static gint ett_trema_fragment = -1;
static gint ett_trema_fragments = -1;

// Fragment fields
static int hf_trema_fragments = -1;
static int hf_trema_fragment = -1;
static int hf_trema_fragment_overlap = -1;
static int hf_trema_fragment_overlap_conflict = -1;
static int hf_trema_fragment_multiple_tails = -1;
static int hf_trema_fragment_too_long_fragment = -1;
static int hf_trema_fragment_error = -1;

// Reassembled in field
static int hf_trema_reassembled_in = -1;

// Reassembled length field
static int hf_trema_reassembled_length = -1;


enum {
  MESSAGE_TYPE_NOTIFY,
  MESSAGE_TYPE_REQUEST,
  MESSAGE_TYPE_REPLY,
};


// Names to bind to various values in the type field
static const value_string names_dump_type[] = {
  { MESSENGER_DUMP_SENT, "Sent" },
  { MESSENGER_DUMP_RECEIVED, "Received" },
  { MESSENGER_DUMP_RECV_CONNECTED, "Receive Connected" },
  { MESSENGER_DUMP_RECV_OVERFLOW, "Receive Overflow" },
  { MESSENGER_DUMP_RECV_CLOSED, "Receive Closed" },
  { MESSENGER_DUMP_SEND_CONNECTED, "Send Connected" },
  { MESSENGER_DUMP_SEND_REFUSED, "Send Refused" },
  { MESSENGER_DUMP_SEND_OVERFLOW, "Send Overflow" },
  { MESSENGER_DUMP_SEND_CLOSED, "Send Closed" },
  { 0, NULL },
};

static const value_string names_message_type[] = {
  { MESSAGE_TYPE_NOTIFY, "Notify" },
  { MESSAGE_TYPE_REQUEST, "Request" },
  { MESSAGE_TYPE_REPLY, "Reply" },
  { 0, NULL },
};

static const value_string names_service_tag[] = {
  { MESSENGER_OPENFLOW_MESSAGE, "OpenFlow Message" },
  { MESSENGER_OPENFLOW_CONNECTED, "Switch Connected" },
  { MESSENGER_OPENFLOW_READY, "Switch Ready" },
  { MESSENGER_OPENFLOW_DISCONNECTED, "Switch Disconnected" },
  { 0, NULL },
};

static const fragment_items trema_fragment_items = {
  // Fragment subtrees
  &ett_trema_fragment,
  &ett_trema_fragments,
  // Fragment fields
  &hf_trema_fragments,
  &hf_trema_fragment,
  &hf_trema_fragment_overlap,
  &hf_trema_fragment_overlap_conflict,
  &hf_trema_fragment_multiple_tails,
  &hf_trema_fragment_too_long_fragment,
  &hf_trema_fragment_error,
  // Reassembled in field
  &hf_trema_reassembled_in,
  // Reassembled length field
  &hf_trema_reassembled_length,
  // Tag
  "Trema fragments"
};


/** Check the status whether the packet_number exists.
 *
 *  return TRUE when it exists.
 */
static gboolean
check_packet_status( guint32 packet_number ) {
  if ( packet_number == 0 || packet_number > packets_status_size ) {
    return FALSE;
  }
  
  if ( packets_status[ packet_number - 1 ] ) {
    return TRUE;
  }
  return FALSE;
}


/** Update packet_status
 *
 *  This function would overwrite information when any data already exists.
 */
static void
update_packet_status( guint32 packet_number, guint32 reassemble_id ) {
  const guint32 PACKET_STATUS_CHUNK_SIZE = 1000;
  
  guint32 current_size = packets_status_size;
  packet_status_info *p = NULL;

  // This should use '>=' to keep NULL termination for free.
  while ( packet_number >= packets_status_size ) {
    packets_status_size += PACKET_STATUS_CHUNK_SIZE;
  }

  if ( packets_status_size > current_size ) {
    // extend pointer table
    packet_status_info **new_table_buffer = xcalloc( packets_status_size, sizeof( packet_status_info * ) );
    if ( packets_status != NULL ) {
      memcpy( ( void * )new_table_buffer, ( void * )packets_status, current_size * sizeof( packet_status_info * ) );
      xfree( packets_status );
    }
    packets_status = new_table_buffer;
  }

  if ( packets_status[ packet_number - 1 ] == NULL ) {
    packets_status[ packet_number - 1 ] = xmalloc( sizeof( packet_status_info ) );
  }
  p = packets_status[ packet_number - 1 ];
  p->packet_number = packet_number;
  p->reassemble_id = reassemble_id;
}


/** Get packet status information
 */
static packet_status_info *
get_packet_status( guint32 packet_number ) {
  packet_status_info *p = NULL;
  
  if ( check_packet_status( packet_number ) ) {
    p = packets_status[ packet_number - 1 ];
  }

  return p;
}


/** Clear packet status
 */
static void
clear_packet_status() {
  if ( packets_status != NULL ) {
    guint count;
    
    for ( count = 0; count < packets_status_size; count++ ) {
      if ( packets_status[ count ] ) {
        xfree( packets_status[ count ] );
      }
    }
    xfree( packets_status );
    packets_status = NULL;
    packets_status_size = 0;
  }
}


/** Get app name and service name, this pair can be used as stream name.
 */
static void
get_stream_id( tvbuff_t *tvb, stream_id *stream_name ) {
  assert( tvb != NULL );
  assert( stream_name != NULL );

  stream_name->app_name_length = tvb_get_ntohs( tvb, offsetof( message_pcap_dump_header, app_name_len ) );
  stream_name->app_name = tvb_format_text( tvb, sizeof( message_pcap_dump_header ), stream_name->app_name_length - 1 );
  stream_name->service_name_length = tvb_get_ntohs( tvb, offsetof( message_pcap_dump_header, service_name_len ) );
  stream_name->service_name = tvb_format_text( tvb, sizeof( message_pcap_dump_header ) + stream_name->app_name_length,
                                               stream_name->service_name_length - 1 );
}


/** Generate a unique reassemble_id.
 */
static guint32
generate_reassemble_id() {
  guint32 new_id = 1;   // The lowest id is 1, 0 means non fragmentation

  if ( packets_status != NULL ) {
    guint count;

    for ( count = 0; count < packets_status_size; count++ ) {
      if ( packets_status[ count ] ) {
        packet_status_info *p = packets_status[ count ];
        if ( new_id == p->reassemble_id ) {
          new_id++;
          count = 0;
          continue;
        }
      }
    }
  }

  return new_id;
}


/** Confirm status of fragment whether it received enough fragments or not.
 *
 *  return TRUE if the stream information indicates it needs more fragment,
 *  FALSE means the stream has enough fragments, it can be reassembled.
 */
static gboolean
is_last_fragment( fragmented_stream_info* fragment_info ) {
  assert( fragment_info != NULL );

  if ( fragment_info->unreceived_length <= 0 ) {
    return FALSE;
  }
  return TRUE;
}


/** Trim a message from packet payload.
 *
 *  'offset' points at message header.
 *  Return new subset tvbuff of a message.
 *  If given tvb is not enough length, return NULL.
 */
static tvbuff_t *
trim_message( tvbuff_t *tvb, gint offset ) {
  guint32 length = tvb_length( tvb );
  guint32 message_length;

  assert( tvb != NULL );

  if ( length < sizeof( message_header ) ) {
    PRINTF( "trim() failed  length %d\n", length );
    return NULL;
  }
  message_length = tvb_get_letohl( tvb, offset + offsetof( message_header, message_length ) );
  if ( length < message_length ) {
    PRINTF( "trim() failed  length %d  message_length %d\n", length, message_length );
    return NULL;
  }
  return tvb_new_subset( tvb, offset, message_length, message_length );
}


/** Get fragmented_stream_info from the current fragments_status by stream_name.
 */
static fragmented_stream_info *
get_fragmented_stream_info( stream_id *stream_name ) {
  dlist_element *element;

  assert( stream_name != NULL );

  if ( fragments_status == NULL ) {
    return NULL;
  }

  for ( element = get_first_element( fragments_status ); element != NULL; element = element->next ) {
    if ( element->data != NULL ) {
      fragmented_stream_info *fragment_info = element->data;
      if ( strncmp( fragment_info->stream_name.app_name, stream_name->app_name, stream_name->app_name_length - 1 ) == 0 &&
           strncmp( fragment_info->stream_name.service_name, stream_name->service_name, stream_name->service_name_length - 1 ) == 0 ) {
        return fragment_info;
      }
    }
  }

  return NULL;
}


/** Add a new fragmented stream information to fragment status list.
 *
 *  'offset' points at begging of messages.
 *  return updated fragmented streasm info.
 */
static fragmented_stream_info *
add_fragmented_stream_info( tvbuff_t *tvb, gint offset, stream_id *stream_name ) {
  fragmented_stream_info *fragment_info;

  assert( tvb != NULL );
  assert( stream_name != NULL );
  assert( get_fragmented_stream_info( stream_name ) == NULL );

  PRINTF( "  ** add_fragmented_stream_info\n" );
  guint32 received_message_length = tvb_length_remaining( tvb, offset );
  fragment_info = xmalloc( sizeof( fragmented_stream_info ) );
  fragment_info->stream_name.app_name = xmalloc( stream_name->app_name_length );
  memset( fragment_info->stream_name.app_name, ( int )NULL, stream_name->app_name_length );
  strncpy( fragment_info->stream_name.app_name, stream_name->app_name, stream_name->app_name_length - 1 );
  fragment_info->stream_name.app_name_length = stream_name->app_name_length;
  fragment_info->stream_name.service_name = xmalloc( stream_name->service_name_length );
  memset( fragment_info->stream_name.service_name, ( int )NULL, stream_name->service_name_length );
  strncpy( fragment_info->stream_name.service_name, stream_name->service_name, stream_name->service_name_length - 1 );
  fragment_info->stream_name.service_name_length = stream_name->service_name_length;
  if ( received_message_length < sizeof( message_header ) ) {
    /* this case cannot calculate message length until reassembling message_header.
     * we need next fragment to build whole message_header.
     */
    PRINTF( "  - received stream is less than message_header\n" );
    fragment_info->message_length = sizeof( message_header );
    fragment_info->temporary_length = TRUE;
  }
  else {
    fragment_info->message_length = tvb_get_letohl( tvb, offset + offsetof( message_header, message_length ) );
    fragment_info->temporary_length = FALSE;
  }
  
  fragment_info->unreceived_length = fragment_info->message_length - received_message_length;
  fragment_info->reassemble_id = generate_reassemble_id();
  fragment_info->number = 0;

  if ( fragments_status == NULL ) {
    fragments_status = create_dlist();
    fragments_status->data = fragment_info;
  }
  else {
    fragments_status = insert_before_dlist( get_first_element( fragments_status ), fragment_info );
  }

  PRINTF( "  - unreceived_length %d  message_length %u  reassemble_id %d\n",
          fragment_info->unreceived_length, fragment_info->message_length, fragment_info->reassemble_id );

  return fragment_info;
}


/** Reset fragmented stream information with reassembled message which includes message_header.
 *  This function should be invoked when message_header is reassembled.
 *
 *  return updated fragmented streasm info.
 */
static fragmented_stream_info *
reset_fragmented_stream_info( tvbuff_t *reassembled_tvb, fragmented_stream_info *fragment_info ) {
  guint32 received_message_length;

  assert( reassembled_tvb != NULL );
  assert( fragment_info != NULL );
  assert( fragment_info->temporary_length == TRUE );

  PRINTF( "  ** reset_fragmented_stream_info  reassemble_id %d\n", fragment_info->reassemble_id );
  received_message_length = tvb_reported_length( reassembled_tvb );
  assert( received_message_length >= sizeof( message_header ) );

  fragment_info->message_length = tvb_get_letohl( reassembled_tvb, offsetof( message_header, message_length ) );
  fragment_info->unreceived_length = fragment_info->message_length - received_message_length;
  fragment_info->temporary_length = FALSE;

  PRINTF( "  - unreceived_length %d  message_length %u  reassemble_id %d\n",
          fragment_info->unreceived_length, fragment_info->message_length, fragment_info->reassemble_id );
  return fragment_info;
}


/** Update fragment status list with new fragmented stream information.
 *
 *  'offset' points at beginning of inner message.
 *  return updated fragmented streasm info.
 */
static fragmented_stream_info *
update_fragmented_stream_info( tvbuff_t *tvb, gint offset, fragmented_stream_info *fragment_info ) {
  assert( tvb != NULL );
  assert( fragment_info != NULL );

  PRINTF( "  ** update_fragmented_stream_info  reassemble_id %d\n", fragment_info->reassemble_id );
  fragment_info->unreceived_length -= tvb_reported_length_remaining( tvb, offset );
  fragment_info->number++;
  PRINTF( "  - unreceived_length %d  message_length %u  reassemble_id %d\n",
          fragment_info->unreceived_length, fragment_info->message_length, fragment_info->reassemble_id );
  return fragment_info;
}


/** Inner function to remove fragmented stream information in dlist
 */
static void
delete_fragmented_stream_info( dlist_element *element ) {
  fragmented_stream_info *fragment_info;

  assert( element != NULL );

  fragment_info = element->data;
  xfree( fragment_info->stream_name.app_name );
  xfree( fragment_info->stream_name.service_name );
  xfree( fragment_info );

  // get first element for just to be safe
  fragments_status = get_first_element( fragments_status );

  if ( element == fragments_status ) {
    if ( fragments_status->next != NULL ) {
      fragments_status = fragments_status->next;
    }
    else {
      // no element in the list.
      fragments_status = NULL;
    }
  }
  delete_dlist_element( element );
}


/** Remove a fragment stream information from fragment status list.
 */
static gboolean
remove_fragmented_stream_info( fragmented_stream_info *fragment_info ) {
  dlist_element *element;
  
  assert( fragment_info != NULL );

  PRINTF( "  ** remove_fragmented_stream_info  reassemble_id %d\n", fragment_info->reassemble_id );
  if ( fragments_status == NULL ) {
    return FALSE;
  }

  // get first element for just to be safe
  fragments_status = get_first_element( fragments_status );
  element = find_element( fragments_status, fragment_info );
  if ( element == NULL ) {
    return FALSE;
  }
  delete_fragmented_stream_info( element );
  
  return TRUE;
}


/** Clear all fragment status
 */
static void
clear_fragmented_stream_info() {
  PRINTF( "  ** clear_fragmented_stream_info\n" );

  if ( fragments_status != NULL ) {
    dlist_element *element;
    for ( element = get_first_element( fragments_status ); element != NULL; element = element->next ) {
      delete_fragmented_stream_info( element );
    }
    assert( fragments_status == NULL );
  }
}


/** Add a new fragmented message to hash table and reassemble the fragments if possible.
 *
 *  'offset' which point at beginning of message.
 *  'fragment_info' has fragment information of target stream.
 *
 *  return NULL if there is not enough fragments yet.
 */
static tvbuff_t *
reassemble_message( tvbuff_t *tvb, gint offset, packet_info *pinfo, proto_tree *tree, fragmented_stream_info *fragment_info ) {
  packet_status_info *packet_status;
  gboolean save_fragmented;
  gboolean more_fragment;
  fragment_data *fragment_message;
  tvbuff_t *new_tvb;

  assert( tvb != NULL );
  assert( pinfo != NULL );
  assert( tree != NULL );
  assert( fragment_info != NULL );

  packet_status = get_packet_status( pinfo->fd->num );
  assert( packet_status != NULL );

  save_fragmented = pinfo->fragmented;
  more_fragment = is_last_fragment( fragment_info );

  pinfo->fragmented = TRUE;
  fragment_message = fragment_add_seq_next( tvb, offset, pinfo, packet_status->reassemble_id,
                                            trema_fragment_table, trema_reassembled_table, 
                                            tvb_length_remaining( tvb, offset ), more_fragment );

  new_tvb = process_reassembled_data( tvb, offset, pinfo, "Reassembled Trema",
                                      fragment_message, &trema_fragment_items, NULL, tree );

  if ( fragment_message ) {
    // Reassembled
    col_append_str( pinfo->cinfo, COL_INFO, " (Message reassembled)" );
    PRINTF( "   reassemble_message() reassembled\n" );
  }
  else {
    // Not last packet of reassembled
    col_append_fstr( pinfo->cinfo, COL_INFO, " (Message fragment %u)", fragment_info->number );
    PRINTF( "   reassemble_message() not last packet\n" );
  }

  pinfo->fragmented = save_fragmented;
  return new_tvb;
}


/** Get reassembled message
 */
static tvbuff_t *
get_reassembled_message( tvbuff_t *tvb, gint offset, packet_info *pinfo, proto_tree *tree, stream_id *stream_name ) {
  unsigned int save_visited;
  gboolean save_fragmented;
  gboolean more_fragment;
  fragmented_stream_info *fragment_info;
  packet_status_info *packet_status = NULL;
  fragment_data *fragment_message = NULL;
  tvbuff_t *reassembled_tvb = NULL;

  assert( tvb != NULL );
  assert( pinfo != NULL );
  assert( tree != NULL );
  assert( stream_name != NULL );

  packet_status = get_packet_status( pinfo->fd->num );
  assert( packet_status != NULL );
  
  more_fragment = FALSE;
  fragment_info = get_fragmented_stream_info( stream_name );
  if ( fragment_info != NULL ) {
    if ( fragment_info->reassemble_id == packet_status->reassemble_id ) {
      // this fragment has not closed yet, it still needs more packets
      more_fragment = TRUE;
    }
  }
  save_visited = pinfo->fd->flags.visited;
  pinfo->fd->flags.visited = 1;
  save_fragmented = pinfo->fragmented;
  pinfo->fragmented = TRUE;
  
  fragment_message = fragment_add_seq_next( tvb, offset, pinfo, packet_status->reassemble_id,
                                            trema_fragment_table, trema_reassembled_table,
                                            tvb_length_remaining( tvb, offset ), more_fragment );
  reassembled_tvb = process_reassembled_data( tvb, offset, pinfo, "Reassembled Trema",
                                              fragment_message, &trema_fragment_items, NULL, tree );

  pinfo->fragmented = save_fragmented;
  pinfo->fd->flags.visited = save_visited;

  return reassembled_tvb;
}


/** Update fragment status and store fragmented stream in hash table.
 *  if it can reassemble entire message, do it and removes the fragment information.
 *
 *  return reassembled stream if possible, otherwise return NULL.
 */
static tvbuff_t *
update_fragment_status( tvbuff_t *tvb, gint offset, packet_info *pinfo, proto_tree *tree,
                        fragmented_stream_info *fragment_info, stream_id *stream_name ) {
  tvbuff_t *reassembled_tvb;

  assert( tvb != NULL );
  assert( pinfo != NULL );
  assert( tree != NULL );
  assert( ( fragment_info != NULL && stream_name == NULL ) ||
          ( fragment_info == NULL && stream_name != NULL ) );

  if ( fragment_info == NULL ) {
    fragment_info = add_fragmented_stream_info( tvb, offset, stream_name );
  }
  else {
    fragment_info = update_fragmented_stream_info( tvb, offset, fragment_info );
  }
  update_packet_status( pinfo->fd->num, fragment_info->reassemble_id );
  reassembled_tvb = reassemble_message( tvb, offset, pinfo, tree, fragment_info );

  if ( reassembled_tvb != NULL ) {
    // reassembled done
    if ( fragment_info->unreceived_length <= 0 &&
         fragment_info->temporary_length == TRUE ) {
      // reassembled one includes at least message_header
      fragment_info = reset_fragmented_stream_info( reassembled_tvb, fragment_info );
    }

    if ( fragment_info->unreceived_length <= 0 ) {
      // reassembled entire message
      remove_fragmented_stream_info( fragment_info );
    }
    else {
      // reassembled a part of message which includes header

      // store again the reassembled message as a first fragment of whole message
      reassemble_message( reassembled_tvb, 0, pinfo, tree, fragment_info );
    }
  }

  return reassembled_tvb;
}


static void
init_trema_fragment() {
  // Initialize tables for reassembling
  fragment_table_init( &trema_fragment_table );
  reassembled_table_init( &trema_reassembled_table );

  // Clear fragment status information
  clear_fragmented_stream_info();

  // Clear packet status information
  clear_packet_status();
}


static gint
dissect_pcap_dump_header( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree ) {
  /*
    +------------------------+--------+------------+----+
    |message_pcap_dump_header|app_name|service_name|data|
    +------------------------+--------+------------+----+

    typedef struct message_pcap_dump_header {
      uint16_t dump_type;
      struct {
        uint32_t sec;
        uint32_t nsec;
      } sent_time;
      uint16_t app_name_len;
      uint16_t service_name_len;
      uint32_t data_len;
    }  __attribute__( ( packed ) ) message_pcap_dump_header;
  */

  gchar *src, *dst;
  gint offset = 0;
  guint16 dump_type = tvb_get_ntohs( tvb, 0 );
  guint16 app_name_len = tvb_get_ntohs( tvb, offsetof( message_pcap_dump_header, app_name_len ) );
  guint16 service_name_len = tvb_get_ntohs( tvb, offsetof( message_pcap_dump_header, service_name_len ) );

  if ( check_col( pinfo->cinfo, COL_DEF_SRC ) ) {
    src = tvb_format_text( tvb, sizeof( message_pcap_dump_header ), app_name_len - 1 );
    if ( dump_type != MESSENGER_DUMP_RECEIVED ) {
      col_add_str( pinfo->cinfo, COL_DEF_SRC, src );
    }
  }

  if ( check_col( pinfo->cinfo, COL_DEF_DST ) ) {
    dst = tvb_format_text( tvb, sizeof( message_pcap_dump_header ) + app_name_len, service_name_len - 1 );
    col_add_str( pinfo->cinfo, COL_DEF_DST, dst );
  }

  if( check_col( pinfo->cinfo, COL_INFO ) ) {
    if ( g_strcasecmp( src, dst ) == 0 ) {
      col_add_fstr( pinfo->cinfo, COL_INFO, "%s (%s)",
                    src, names_dump_type[ dump_type ].strptr );
    }
    else {
      col_add_fstr( pinfo->cinfo, COL_INFO, "%s > %s (%s)",
                    src, dst, names_dump_type[ dump_type ].strptr );
    }
  }

  if ( trema_tree != NULL ) {
    proto_tree *dump_header_tree = NULL;
    proto_item *ti = NULL;

    offset = 0;

    ti = proto_tree_add_item( trema_tree, hf_dump_header, tvb, offset, 18, FALSE );
    dump_header_tree = proto_item_add_subtree( ti, ett_dump_header );

    proto_tree_add_item( dump_header_tree, hf_dump_type, tvb, offset, 2, FALSE );
    offset += 2;
    nstime_t sent_time;
    sent_time.secs = tvb_get_ntohl( tvb, offset );
    sent_time.nsecs = tvb_get_ntohl( tvb, offset + 4 );
    proto_tree_add_time( dump_header_tree, hf_dump_event_time, tvb, offset, 8, &sent_time );
    offset += 8;
    proto_tree_add_item( dump_header_tree, hf_dump_app_name_length, tvb, offset, 2, FALSE );
    offset += 2;
    proto_tree_add_item( dump_header_tree, hf_dump_service_name_length, tvb, offset, 2, FALSE );
    offset += 2;
    proto_tree_add_item( dump_header_tree, hf_dump_data_length, tvb, offset, 4, FALSE );
    offset += 4;

    proto_tree_add_item( trema_tree, hf_dump_app_name, tvb, offset, app_name_len, FALSE );
    offset += app_name_len;
    proto_tree_add_item( trema_tree, hf_dump_service_name, tvb, offset, service_name_len, FALSE );
    offset += service_name_len;
  }

  return offset;
}


static gint
dissect_openflow_service_header( tvbuff_t *tvb, gint offset, proto_tree *trema_tree ) {
  gint head = offset;

  guint16 service_name_len = tvb_get_ntohs( tvb, offset + 8 );

  if ( trema_tree != NULL ) {
    proto_tree *service_header_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_service_header, tvb, offset,
                              sizeof( openflow_service_header_t ), FALSE );
    service_header_tree = proto_item_add_subtree( ti, ett_service_header );

    proto_tree_add_item( service_header_tree, hf_datapath_id, tvb, offset, 8, FALSE );
    offset += 8;
    proto_tree_add_item( service_header_tree, hf_service_name_length, tvb, offset, 2, FALSE );
    offset += 2;
    if ( service_name_len > 0 ) {
      proto_tree_add_item( service_header_tree, hf_service_name, tvb, offset, service_name_len, FALSE );
      offset += service_name_len;
    }
  }

  return ( offset - head );  
}


static void
dissect_openflow( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree ) {
  if ( data_openflow ) {
    if ( check_col( pinfo->cinfo, COL_PROTOCOL ) ) {
      col_append_str( pinfo->cinfo, COL_PROTOCOL, "+" );
    }

    if( check_col( pinfo->cinfo, COL_INFO ) ) {
        col_append_str( pinfo->cinfo, COL_INFO, " => " );
    }

    col_set_fence( pinfo->cinfo, COL_PROTOCOL );
    col_set_fence( pinfo->cinfo, COL_INFO );

    call_dissector( data_openflow, tvb, pinfo, trema_tree );
  }
  else {
    // FIXME: do something here...
  }
}


static gint
dissect_context_handle( tvbuff_t *tvb, gint offset, proto_tree *trema_tree ) {
  gint head = offset;

  guint16 service_name_len = tvb_get_ntohs( tvb, offset + 4 );

  if ( trema_tree != NULL ) {
    proto_tree *context_handle_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_context_handle, tvb, offset,
                              sizeof( messenger_context_handle ), FALSE );
    context_handle_tree = proto_item_add_subtree( ti, ett_context_handle );

    proto_tree_add_item( context_handle_tree, hf_transaction_id, tvb, offset, 4, FALSE );
    offset += 4;
    proto_tree_add_item( context_handle_tree, hf_service_name_length, tvb, offset, 2, FALSE );
    offset += 2;
    offset += 2; // padding
    if ( service_name_len > 0 ) {
      proto_tree_add_item( context_handle_tree, hf_service_name, tvb, offset, service_name_len, FALSE );
      offset += service_name_len;
    }
  }

  return ( offset - head );  
}


static gint
dissect_message_dump( tvbuff_t *tvb, gint offset, proto_tree *trema_tree ) {
  gint length = tvb_length_remaining( tvb, offset );

  assert( tvb != NULL );
  assert( trema_tree != NULL );

  PRINTF( "dissect_message_dump()\n" );

  proto_tree_add_item( trema_tree, hf_hex_dump, tvb, offset, length, FALSE );

  return length;
}


static gint
dissect_message_dump_with_length( tvbuff_t *tvb, gint offset, gint length, proto_tree *trema_tree ) {
  gint remaining_length = tvb_length_remaining( tvb, offset );

  assert( tvb != NULL );
  assert( trema_tree != NULL );

  PRINTF( "dissect_message_dump()\n" );

  if ( remaining_length < length ) {
    length = remaining_length;
  }

  proto_tree_add_item( trema_tree, hf_hex_dump, tvb, offset, length, FALSE );

  return length;
}


static gint
dissect_message_header( tvbuff_t *tvb, packet_info *pinfo, gint offset, proto_tree *trema_tree ) {
  /*
    typedef struct message_header {
      uint8_t version;         // version = 0 (unused)
      uint8_t message_type;    // MESSAGE_TYPE_
      uint16_t tag;            // user defined
      uint32_t message_length; // message length including header
      uint8_t value[ 0 ];
    } message_header;
  */

  gint head = offset;
  guint8 message_type = tvb_get_guint8( tvb, offset + 1 );
  guint16 tag = htons( tvb_get_ntohs( tvb, offset + 2 ) );
  guint32 message_length = htonl( tvb_get_ntohl( tvb, offset + 4 ) );

  PRINTF( "dissect_message_header()\n" );

  if ( trema_tree != NULL ) {
    proto_tree *message_header_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_message_header, tvb, offset,
                              sizeof( message_header ), FALSE );
    message_header_tree = proto_item_add_subtree( ti, ett_message_header );

    proto_tree_add_item( message_header_tree, hf_version, tvb, offset, 1, FALSE );
    offset += 1;
    proto_tree_add_item( message_header_tree, hf_message_type, tvb, offset, 1, FALSE );
    offset += 1;
    proto_tree_add_item( message_header_tree, hf_tag, tvb, offset, 2, TRUE ); // FIXME: little endian
    offset += 2;
    proto_tree_add_item( message_header_tree, hf_message_length, tvb, offset, 4, TRUE ); // FIXME: little endian
    offset += 4;

    if ( message_type == MESSAGE_TYPE_NOTIFY &&
         tag >= MESSENGER_OPENFLOW_MESSAGE &&
         tag <= MESSENGER_OPENFLOW_DISCONNECTED ) {
      offset += dissect_openflow_service_header( tvb, offset, trema_tree );

      if ( tag == MESSENGER_OPENFLOW_MESSAGE && data_openflow ) {
        guint16 total_len = tvb_reported_length_remaining( tvb, offset );
        if ( total_len > 0 ) {
          tvbuff_t *next_tvb = tvb_new_subset( tvb, offset, -1, total_len );
          dissect_openflow( next_tvb, pinfo, trema_tree );
        }
        offset += total_len;
      }
    }
    else if ( message_type == MESSAGE_TYPE_REQUEST ||
              message_type == MESSAGE_TYPE_REPLY ) {
      offset += dissect_context_handle( tvb, offset, trema_tree );
    }

    guint remaining_len = message_length - ( offset - head );
    if ( remaining_len > 0 ) {
      offset += dissect_message_dump_with_length( tvb, offset, remaining_len, trema_tree );
    }
  }

  return ( offset - head );
}


static void
dissect_trema( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree ) {
  gboolean visited = TRUE;
  gint offset = 0;
  gint remaining_length = 0;
  proto_tree *trema_tree = NULL;
  proto_item *ti = NULL;
  tvbuff_t *messages_tvb;
  tvbuff_t *next_tvb;
  stream_id names;
  stream_id *stream_name = &names;

  PRINTF( "----- start %u\n", pinfo->fd->num );
  
  if ( tree == NULL ) {   // TODO: is this OK?
    PRINTF( "----- end %u  (tree is NULL)\n", pinfo->fd->num );
    return;
  }
  
  if ( check_packet_status( pinfo->fd->num ) == FALSE ) {
    visited = FALSE;
    update_packet_status( pinfo->fd->num, 0 );
  }

  if ( check_col( pinfo->cinfo, COL_PROTOCOL ) ) {
    col_set_str( pinfo->cinfo, COL_PROTOCOL, PROTO_TAG_TREMA );
  }

  ti = proto_tree_add_item( tree, proto_trema, tvb, 0, -1, FALSE );
  trema_tree = proto_item_add_subtree( ti, ett_trema );

  get_stream_id( tvb, stream_name );
  PRINTF( " app [%s]  service [%s]\n", stream_name->app_name, stream_name->service_name );

  offset = dissect_pcap_dump_header( tvb, pinfo, trema_tree );
  if ( tvb_length_remaining( tvb, offset ) <= 0 ) {
    PRINTF( "----- end %u  (message_pcap_dump_header only)\n", pinfo->fd->num );
    return;
  }

  if ( visited ) {
    packet_status_info *packet_status = NULL;
    
    PRINTF( " Known packet\n" );
    // We have seen this packet
    packet_status = get_packet_status( pinfo->fd->num );
    assert( packet_status != NULL );
    
    if ( packet_status->reassemble_id != 0 ) {
      // This is a part of fragment.
      messages_tvb = get_reassembled_message( tvb, offset, pinfo, tree, stream_name );
      if ( messages_tvb == NULL ) {
        // We have not had all packets yet to reassemble this
        dissect_message_dump( tvb, offset, trema_tree );
        PRINTF( "----- end %u  (known packet)\n", pinfo->fd->num );
        return;
      }
    }
    else {
      messages_tvb = tvb_new_subset( tvb, offset, -1, -1 );
    }

    offset = 0;
    remaining_length = tvb_length_remaining( messages_tvb, offset );
    while ( remaining_length > 0 ) {
      next_tvb = trim_message( messages_tvb, offset );
      if ( next_tvb == NULL ) {
        dissect_message_dump( messages_tvb, offset, trema_tree );
        break;
      }
      offset += tvb_length( next_tvb );
      dissect_message_header( next_tvb, pinfo, 0, trema_tree );
      remaining_length = tvb_length_remaining( messages_tvb, offset );
    } // end of while

    PRINTF( "----- end %u  (known packet, reassembled)\n", pinfo->fd->num );
    return;
  }

  messages_tvb = tvb;
  remaining_length = tvb_length_remaining( messages_tvb, offset );
  PRINTF( " received payload length %d\n", remaining_length );
  while ( remaining_length > 0 ) {
    fragmented_stream_info *fragment_info = get_fragmented_stream_info( stream_name );
    if ( fragment_info != NULL ) {
      // this stream is in fragmentated status
      PRINTF( " In fragmented status\n" );

      dissect_message_dump( messages_tvb, offset, trema_tree );

      messages_tvb = update_fragment_status( messages_tvb, offset, pinfo, tree, fragment_info, NULL );
      if ( messages_tvb != NULL ) {
        offset = 0;
        remaining_length = tvb_length_remaining( messages_tvb, offset );
      }

      PRINTF( "  unreceived_len %d  remaining_len %d\n", fragment_info->unreceived_length, remaining_length );
      if ( messages_tvb == NULL || fragment_info->unreceived_length > 0 ) {
        // need more packet to reassemble
        PRINTF( "----- end %u  need more packet to reassemble\n", pinfo->fd->num );
        return;
      }
      else {
        // reassembled message
        PRINTF( "  reassembled message\n" );
        next_tvb = trim_message( messages_tvb, offset );
        assert( next_tvb != NULL );
        offset += tvb_length( next_tvb );
      }
    }
    else {
      // Not in fragmented status
      PRINTF( " Not in fragmented status\n" );

      if ( remaining_length < sizeof( message_header ) ) {
        // Not enough data received to get message length
        PRINTF( "  Not enough data for message header  remaining_length %d\n", remaining_length );
        dissect_message_dump( messages_tvb, offset, trema_tree );
        messages_tvb = update_fragment_status( messages_tvb, offset, pinfo, tree, NULL, stream_name );
        offset += remaining_length;
        PRINTF( "----- end %u  (Not enough data for message header)\n", pinfo->fd->num );
        return;
      }
      else {
        guint32 message_length = tvb_get_letohl( messages_tvb, offset + offsetof( message_header, message_length ) );
        
        if ( remaining_length < message_length ) {
          // new fragment
          dissect_message_dump( messages_tvb, offset, trema_tree );
          messages_tvb = update_fragment_status( messages_tvb, offset, pinfo, tree, NULL, stream_name );
          offset += remaining_length;
          PRINTF( "----- end %u  (new fragment)\n", pinfo->fd->num );
          return;
        }
        else {
          next_tvb = trim_message( messages_tvb, offset );
          assert( next_tvb != NULL );
          offset += tvb_length( next_tvb );
        }
      }
    }

    dissect_message_header( next_tvb, pinfo, 0, trema_tree );

    remaining_length = tvb_length_remaining( messages_tvb, offset );
  } // end of while
  
  PRINTF( "----- end %u\n", pinfo->fd->num );
}


void
proto_register_trema() {
  data_openflow = find_dissector( "openflow" );

  static hf_register_info hf[] = {
    { &hf_dump_header,
      { "Dump header", "trema.dump_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Dump header", HFILL }},
    { &hf_dump_type,
      { "Type", "trema.dump_type",
        FT_UINT16, BASE_DEC, VALS( names_dump_type ), NO_MASK, "Type", HFILL }},
    { &hf_dump_event_time,
      { "Event Occurred At", "trema.dump_event_time",
        FT_ABSOLUTE_TIME, ABSOLUTE_TIME_LOCAL, NO_STRINGS, NO_MASK, "Event Occurred At", HFILL }},
    { &hf_dump_app_name_length,
      { "Application name length", "trema.dump_app_name_length",
        FT_UINT16, BASE_DEC, NO_STRINGS, NO_MASK, "Application name length", HFILL }},
    { &hf_dump_service_name_length,
      { "Service name length", "trema.dump_service_name_length",
        FT_UINT16, BASE_DEC, NO_STRINGS, NO_MASK, "Service name length", HFILL }},
    { &hf_dump_data_length,
      { "Data length", "trema.dump_data_length",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, "Data length", HFILL }},
    { &hf_dump_app_name,
      { "Application name", "trema.dump_app_name",
        FT_STRING, BASE_NONE, NO_STRINGS, NO_MASK, "Application name", HFILL }},
    { &hf_dump_service_name,
      { "Service name", "trema.dump_service_name",
        FT_STRING, BASE_NONE, NO_STRINGS, NO_MASK, "Service name", HFILL }},
    { &hf_message_header,
      { "Message header", "trema.message_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Mesasge header", HFILL }},
    { &hf_version,
      { "Version", "trema.version",
        FT_UINT8, BASE_DEC, NO_STRINGS, NO_MASK, "Version", HFILL }},
    { &hf_message_type,
      { "Type", "trema.type",
        FT_UINT8, BASE_DEC, VALS( names_message_type ), NO_MASK, "Type", HFILL }},
    { &hf_tag,
      { "Tag", "trema.tag",
        FT_UINT16, BASE_DEC, VALS( names_service_tag ), NO_MASK, "Tag", HFILL }},
    { &hf_message_length,
      { "Length", "trema.length",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, "Length", HFILL }},
    { &hf_service_header,
      { "OpenFlow service header", "trema.service_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "OpenFlow service header", HFILL }},
    { &hf_datapath_id,
      { "Datapath ID", "trema.datapath_id",
        FT_UINT64, BASE_HEX, NO_STRINGS, NO_MASK, "Datapath ID", HFILL }},
    { &hf_service_name_length,
      { "Service name length", "trema.service_name_length",
        FT_UINT16, BASE_DEC, NO_STRINGS, NO_MASK, "Service name length", HFILL }},
    { &hf_service_name,
      { "Service name", "trema.service_name",
        FT_STRING, BASE_NONE, NO_STRINGS, NO_MASK, "Service name", HFILL }},
    { &hf_context_handle,
      { "Request/Reply context handle", "trema.context_handle",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Request/Reply context handle", HFILL }},
    { &hf_transaction_id,
      { "Transaction ID", "trema.transaction_id",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, "Transaction ID", HFILL }},
    { &hf_hex_dump,
      { "[Unknown data]", "trema.hex_dump",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Hex dump", HFILL }},

    // Fragment fields
    { &hf_trema_fragments,
      { "Trema fragments", "trema.fragments",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_fragment,
      { "Trema fragment", "trema.fragment",
        FT_FRAMENUM, BASE_NONE, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_fragment_overlap,
      { "Trema fragment overlap", "trema.fragment.overlap",
        FT_BOOLEAN, 0, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_fragment_overlap_conflict,
      { "Trema fragment overlapping with conflicting data", "trema.fragment.overlap.conflicts",
        FT_BOOLEAN, 0, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_fragment_multiple_tails,
      { "Trema has multiple tail fragments", "trema.fragment.multiple_tails",
        FT_BOOLEAN, 0, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_fragment_too_long_fragment,
      { "Trema fragment too long", "trema.fragment.too_long_fragment",
        FT_BOOLEAN, 0, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_fragment_error,
      { "Trema defragmentation error", "trema.fragment.error",
        FT_FRAMENUM, BASE_NONE, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_reassembled_in,
      { "Reassembled in", "trema.reassembled.in",
        FT_FRAMENUM, BASE_NONE, NO_STRINGS, NO_MASK, NULL, HFILL }},
    { &hf_trema_reassembled_length,
      { "Reassembled length", "trema.reassembled.length",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, NULL, HFILL }},
  };

  static gint *ett[] = {
    &ett_trema,
    &ett_dump_header,
    &ett_message_header,
    &ett_service_header,
    &ett_context_handle,
    &ett_trema_fragment,
    &ett_trema_fragments,
  };

  proto_trema = proto_register_protocol( "Trema IPC message", "TREMA", "trema" );
  proto_register_field_array( proto_trema, hf, array_length( hf ) );
  proto_register_subtree_array( ett, array_length( ett ) );
  register_dissector( "trema", dissect_trema, proto_trema );

  register_init_routine( init_trema_fragment );
}


void
proto_reg_handoff_trema() {
  /*
  trema_handle = create_dissector_handle( dissect_trema, proto_trema );
  dissector_add( wtap_encap, WTAP_ENCAP_USER0, trema_handle );
  */
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
