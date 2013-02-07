/*
 * Copyright (C) 2008-2013 NEC Corporation
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
#include <pcap/pcap.h>
#include <string.h>
#include <arpa/inet.h>
#include "messenger.h"
#include "openflow_service_interface.h"


#define PROTO_TAG_TREMA  "TREMA"

#define NO_STRINGS NULL
#define NO_MASK 0x0

#define UDP_PORT_SYSLOG 514

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
} __attribute__( ( packed ) ) message_pcap_dump_header;

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


// OpenFlow dissector
static dissector_handle_t openflow_handle = NULL;

// Ethernet dissector
static dissector_handle_t ethernet_handle = NULL;

// PPP dissector
static dissector_handle_t ppp_handle = NULL;

// IPv4 dissector
static dissector_handle_t ip_handle = NULL;

// Linux SLL dissector table
static dissector_table_t sll_dissector_table = NULL;

// UDP dissector table
static dissector_table_t udp_dissector_table = NULL;

// Fragment tables
static GHashTable *trema_fragment_table = NULL;
static GHashTable *trema_reassembled_table = NULL;

// Fragment status list
static GList *fragments_status = NULL;
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
static gint hf_pcap_dump_header = -1;
static gint hf_pcap_dump_datalink = -1;
static gint hf_pcap_dump_interface = -1;
static gint hf_pcap_pkthdr = -1;
static gint hf_pcap_pkthdr_ts = -1;
static gint hf_pcap_pkthdr_caplen = -1;
static gint hf_pcap_pkthdr_len = -1;
static gint hf_syslog_dump_header = -1;
static gint hf_syslog_dump_receive_time = -1;
static gint hf_syslog_dump_message = -1;
static gint hf_text_dump_header = -1;
static gint hf_text_dump_receive_time = -1;
static gint hf_text_dump_string = -1;

// Subtrees
static gint ett_trema = -1;
static gint ett_dump_header = -1;
static gint ett_message_header = -1;
static gint ett_service_header = -1;
static gint ett_context_handle = -1;
static gint ett_pcap_dump_header = -1;
static gint ett_pcap_pkthdr = -1;
static gint ett_syslog_dump_header = -1;
static gint ett_text_dump_header = -1;

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
#ifndef WIRESHARK_VERSION_OLDER_THAN_160
static int hf_trema_fragment_count = -1;
#endif

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
  { MESSENGER_DUMP_LOGGER, "Log Message" },
  { MESSENGER_DUMP_PCAP, "Packet Capture" },
  { MESSENGER_DUMP_SYSLOG, "Syslog" },
  { MESSENGER_DUMP_TEXT, "Text" },
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
  { MESSENGER_OPENFLOW_FAILD_TO_CONNECT, "Switch Failed to connect" },
  { 0, NULL },
};

// Names to bind to various datalink type values
static const value_string names_datalink_type[] = {
  { DLT_EN10MB, "Ethernet (10Mb)" },
  { DLT_PPP, "Point-to-point protocol" },
  { DLT_RAW, "Raw IP" },
  { DLT_LINUX_SLL, "Linux cooked sockets" },
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
#ifndef WIRESHARK_VERSION_OLDER_THAN_160
  &hf_trema_fragment_count,
#endif
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
    packet_status_info **new_table_buffer = g_malloc( packets_status_size * sizeof( packet_status_info * ) );
    memset( new_table_buffer, 0, packets_status_size * sizeof( packet_status_info * ) );
    if ( packets_status != NULL ) {
      memcpy( ( void * ) new_table_buffer, ( void * ) packets_status, current_size * sizeof( packet_status_info * ) );
      g_free( packets_status );
    }
    packets_status = new_table_buffer;
  }

  if ( packets_status[ packet_number - 1 ] == NULL ) {
    packets_status[ packet_number - 1 ] = g_malloc( sizeof( packet_status_info ) );
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
        g_free( packets_status[ count ] );
      }
    }
    g_free( packets_status );
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
is_last_fragment( fragmented_stream_info *fragment_info ) {
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
  message_length = tvb_get_ntohl( tvb, offset + offsetof( message_header, message_length ) );
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
  GList *element;

  assert( stream_name != NULL );

  if ( fragments_status == NULL ) {
    return NULL;
  }

  for ( element = g_list_first( fragments_status ); element != NULL; element = element->next ) {
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
  fragment_info = g_malloc( sizeof( fragmented_stream_info ) );
  fragment_info->stream_name.app_name = g_malloc( stream_name->app_name_length );
  memset( fragment_info->stream_name.app_name, 0, stream_name->app_name_length );
  strncpy( fragment_info->stream_name.app_name, stream_name->app_name, stream_name->app_name_length - 1 );
  fragment_info->stream_name.app_name_length = stream_name->app_name_length;
  fragment_info->stream_name.service_name = g_malloc( stream_name->service_name_length );
  memset( fragment_info->stream_name.service_name, 0, stream_name->service_name_length );
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
    fragment_info->message_length = tvb_get_ntohl( tvb, offset + offsetof( message_header, message_length ) );
    fragment_info->temporary_length = FALSE;
  }

  fragment_info->unreceived_length = fragment_info->message_length - received_message_length;
  fragment_info->reassemble_id = generate_reassemble_id();
  fragment_info->number = 0;

  fragments_status = g_list_prepend( fragments_status, fragment_info );

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

  fragment_info->message_length = tvb_get_ntohl( reassembled_tvb, offsetof( message_header, message_length ) );
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
free_fragmented_stream_info( fragmented_stream_info *info ) {
  if ( info != NULL ) {
    g_free( info->stream_name.app_name );
    g_free( info->stream_name.service_name );
    g_free( info );
  }
}


/** Inner function to remove fragmented stream information in dlist
 */
static void
delete_fragmented_stream_info( fragmented_stream_info *fragment_info ) {
  if ( fragment_info != NULL ) {
    fragments_status = g_list_remove( fragments_status, fragment_info );
    free_fragmented_stream_info( fragment_info );
  }
}


/** Remove a fragment stream information from fragment status list.
 */
static gboolean
remove_fragmented_stream_info( fragmented_stream_info *fragment_info ) {
  GList *element;

  assert( fragment_info != NULL );

  PRINTF( "  ** remove_fragmented_stream_info  reassemble_id %d\n", fragment_info->reassemble_id );
  if ( fragments_status == NULL ) {
    return FALSE;
  }

  element = g_list_find( fragments_status, fragment_info );
  if ( element == NULL ) {
    return FALSE;
  }
  delete_fragmented_stream_info( element->data );

  return TRUE;
}


static void
clear_fragmented_stream_info_walker( gpointer fragment_info, gpointer user_data ) {
  if ( fragment_info != NULL ) {
    free_fragmented_stream_info( fragment_info );
  }
}


/** Clear all fragment status
 */
static void
clear_fragmented_stream_info() {
  PRINTF( "  ** clear_fragmented_stream_info\n" );

  if ( fragments_status != NULL ) {
    g_list_foreach( fragments_status, clear_fragmented_stream_info_walker, NULL );
    g_list_free( fragments_status );
    fragments_status = NULL;
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
dissect_message_pcap_dump_header( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree, guint16 *dump_type ) {
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
    } __attribute__( ( packed ) ) message_pcap_dump_header;
  */

  gchar *src, *dst;
  gint offset = 0;
  *dump_type = tvb_get_ntohs( tvb, 0 );
  guint16 app_name_len = tvb_get_ntohs( tvb, offsetof( message_pcap_dump_header, app_name_len ) );
  guint16 service_name_len = tvb_get_ntohs( tvb, offsetof( message_pcap_dump_header, service_name_len ) );

  if ( check_col( pinfo->cinfo, COL_DEF_SRC ) ) {
    src = tvb_format_text( tvb, sizeof( message_pcap_dump_header ), app_name_len - 1 );
    if ( *dump_type != MESSENGER_DUMP_RECEIVED ) {
      col_add_str( pinfo->cinfo, COL_DEF_SRC, src );
    }
  }

  if ( check_col( pinfo->cinfo, COL_DEF_DST ) ) {
    dst = tvb_format_text( tvb, sizeof( message_pcap_dump_header ) + app_name_len, service_name_len - 1 );
    col_add_str( pinfo->cinfo, COL_DEF_DST, dst );
  }

  if ( check_col( pinfo->cinfo, COL_INFO ) ) {
    switch ( *dump_type ) {
      case MESSENGER_DUMP_SENT:
      case MESSENGER_DUMP_RECEIVED:
      case MESSENGER_DUMP_RECV_CONNECTED:
      case MESSENGER_DUMP_RECV_OVERFLOW:
      case MESSENGER_DUMP_RECV_CLOSED:
      case MESSENGER_DUMP_SEND_CONNECTED:
      case MESSENGER_DUMP_SEND_REFUSED:
      case MESSENGER_DUMP_SEND_OVERFLOW:
      case MESSENGER_DUMP_SEND_CLOSED:
      {
        if ( g_ascii_strcasecmp( src, dst ) == 0 ) {
          col_add_fstr( pinfo->cinfo, COL_INFO, "%s (%s)",
                        src, names_dump_type[ *dump_type ].strptr );
        }
        else {
          col_add_fstr( pinfo->cinfo, COL_INFO, "%s > %s (%s)",
                        src, dst, names_dump_type[ *dump_type ].strptr );
        }
      }
      break;

      case MESSENGER_DUMP_PCAP:
      {
        col_add_fstr( pinfo->cinfo, COL_INFO, "%s (%s)",
                      src, names_dump_type[ *dump_type ].strptr );
      }
      break;

      case MESSENGER_DUMP_LOGGER:
      case MESSENGER_DUMP_SYSLOG:
      case MESSENGER_DUMP_TEXT:
      {
        col_add_fstr( pinfo->cinfo, COL_INFO, "(%s)", names_dump_type[ *dump_type ].strptr );
      }
      break;
      default:
      break;
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
    guint16 app_name_len = tvb_get_ntohs( tvb, offset );
    offset += 2;
    proto_tree_add_item( dump_header_tree, hf_dump_service_name_length, tvb, offset, 2, FALSE );
    guint16 service_name_len = tvb_get_ntohs( tvb, offset );
    offset += 2;
    proto_tree_add_item( dump_header_tree, hf_dump_data_length, tvb, offset, 4, FALSE );
    offset += 4;

    if ( app_name_len > 0 ) {
      proto_tree_add_item( trema_tree, hf_dump_app_name, tvb, offset, app_name_len, FALSE );
      offset += app_name_len;
    }
    if ( service_name_len > 0 ) {
      proto_tree_add_item( trema_tree, hf_dump_service_name, tvb, offset, service_name_len, FALSE );
      offset += service_name_len;
    }
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
  if ( openflow_handle != NULL ) {
    if ( check_col( pinfo->cinfo, COL_PROTOCOL ) ) {
      col_append_str( pinfo->cinfo, COL_PROTOCOL, "+" );
    }

    if ( check_col( pinfo->cinfo, COL_INFO ) ) {
        col_append_str( pinfo->cinfo, COL_INFO, " => " );
    }

    col_set_fence( pinfo->cinfo, COL_PROTOCOL );
    col_set_fence( pinfo->cinfo, COL_INFO );

    call_dissector( openflow_handle, tvb, pinfo, trema_tree );
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

  if ( length > 0 ) {
    proto_tree_add_item( trema_tree, hf_hex_dump, tvb, offset, length, FALSE );
  }

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
  guint16 tag = tvb_get_ntohs( tvb, offset + 2 );
  guint32 message_length = tvb_get_ntohl( tvb, offset + 4 );

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
    proto_tree_add_item( message_header_tree, hf_tag, tvb, offset, 2, FALSE );
    offset += 2;
    proto_tree_add_item( message_header_tree, hf_message_length, tvb, offset, 4, FALSE );
    offset += 4;

    if ( message_type == MESSAGE_TYPE_NOTIFY &&
         tag >= MESSENGER_OPENFLOW_MESSAGE &&
         tag <= MESSENGER_OPENFLOW_DISCONNECTED ) {
      offset += dissect_openflow_service_header( tvb, offset, trema_tree );

      if ( tag == MESSENGER_OPENFLOW_MESSAGE && openflow_handle != NULL ) {
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
dissect_trema_ipc( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, proto_tree *trema_tree,
                   gint offset, gboolean visited, stream_id *stream_name ) {
  gint remaining_length = 0;
  tvbuff_t *messages_tvb;
  tvbuff_t *next_tvb;

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
        guint32 message_length = tvb_get_ntohl( messages_tvb, offset + offsetof( message_header, message_length ) );

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
}


static gint
dissect_pcap_dump_header( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree, gint offset, guint32 *datalink ) {
  /*
    typedef struct pcap_dump_header {
      uint32_t datalink;
      uint8_t interface[ IF_NAMESIZE ];
    } pcap_dump_header;
  */
  gint head = offset;

  *datalink = tvb_get_ntohl( tvb, offset );

  if ( trema_tree != NULL ) {
    proto_tree *pcap_dump_header_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_pcap_dump_header, tvb, offset, sizeof( pcap_dump_header ), FALSE );
    pcap_dump_header_tree = proto_item_add_subtree( ti, ett_pcap_dump_header );

    proto_tree_add_item( pcap_dump_header_tree, hf_pcap_dump_datalink, tvb, offset, 4, FALSE );
    offset += 4;
    proto_tree_add_item( pcap_dump_header_tree, hf_pcap_dump_interface, tvb, offset, IF_NAMESIZE, FALSE );
    offset += IF_NAMESIZE;
  }

  return ( offset - head );
}


static gint
dissect_pcap_pkthdr( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree, gint offset ) {
  /*
    struct pcap_pkthdr {
      struct timeval ts;
      bpf_u_int32 caplen;
      bpf_u_int32 len;
    };
  */
  gint head = offset;

  if ( trema_tree != NULL ) {
    proto_tree *pcap_pkthdr_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_pcap_pkthdr, tvb, offset, sizeof( struct pcap_pkthdr ), FALSE );
    pcap_pkthdr_tree = proto_item_add_subtree( ti, ett_pcap_pkthdr );

    nstime_t ts;
    ts.secs = tvb_get_ntohl( tvb, offset );
    ts.nsecs = tvb_get_ntohl( tvb, offset + 4 ) * 1000;
    proto_tree_add_time( pcap_pkthdr_tree, hf_pcap_pkthdr_ts, tvb, offset, sizeof( nstime_t ), &ts );
    offset += sizeof( nstime_t );
    proto_tree_add_item( pcap_pkthdr_tree, hf_pcap_pkthdr_caplen, tvb, offset, 4, FALSE );
    offset += 4;
    proto_tree_add_item( pcap_pkthdr_tree, hf_pcap_pkthdr_len, tvb, offset, 4, FALSE );
    offset += 4;
  }

  return ( offset - head );
}


static void
dissect_ethernet( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree ) {
  if ( ethernet_handle != NULL ) {
    if ( check_col( pinfo->cinfo, COL_PROTOCOL ) ) {
      col_append_str( pinfo->cinfo, COL_PROTOCOL, "+" );
    }

    if ( check_col( pinfo->cinfo, COL_INFO ) ) {
        col_append_str( pinfo->cinfo, COL_INFO, " => " );
    }

    col_set_fence( pinfo->cinfo, COL_PROTOCOL );
    col_set_fence( pinfo->cinfo, COL_INFO );

    call_dissector( ethernet_handle, tvb, pinfo, trema_tree );
  }
  else {
    // FIXME: do something here
  }
}


static void
dissect_pcap_dump( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, proto_tree *trema_tree,
                   gint offset, gboolean visited, stream_id *stream_name ) {
  guint32 datalink = 0;

  offset += dissect_pcap_dump_header( tvb, pinfo, trema_tree, offset, &datalink );
  offset += dissect_pcap_pkthdr( tvb, pinfo, trema_tree, offset );
  if ( datalink == DLT_EN10MB ) {
    guint16 remaining_len = tvb_reported_length_remaining( tvb, offset );
    if ( remaining_len > 0 ) {
      tvbuff_t *next_tvb = tvb_new_subset( tvb, offset, -1, remaining_len );
      dissect_ethernet( next_tvb, pinfo, trema_tree );
    }
    offset += remaining_len;
  }
  dissect_message_dump( tvb, offset, trema_tree );
}


static gint
dissect_syslog_dump_header( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree, gint offset ) {
  /*
    typedef struct syslog_dump_header {
      struct {
        uint32_t sec;
        uint32_t nsec;
      } sent_time;
    } syslog_dump_header;
  */
  gint head = offset;

  if ( trema_tree != NULL ) {
    proto_tree *syslog_dump_header_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_syslog_dump_header, tvb, offset, sizeof( syslog_dump_header ), FALSE );
    syslog_dump_header_tree = proto_item_add_subtree( ti, ett_syslog_dump_header );

    nstime_t receive_time;
    receive_time.secs = tvb_get_ntohl( tvb, offset );
    receive_time.nsecs = tvb_get_ntohl( tvb, offset + 4 );
    proto_tree_add_time( syslog_dump_header_tree, hf_syslog_dump_receive_time, tvb, offset, 8, &receive_time );
    offset += sizeof( syslog_dump_header );
  }

  return ( offset - head );
}


static void
dissect_syslog( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree ) {
  if ( udp_dissector_table != NULL ) {
    if ( check_col( pinfo->cinfo, COL_PROTOCOL ) ) {
      col_append_str( pinfo->cinfo, COL_PROTOCOL, "+" );
    }

    if ( check_col( pinfo->cinfo, COL_INFO ) ) {
        col_append_str( pinfo->cinfo, COL_INFO, " => " );
    }

    col_set_fence( pinfo->cinfo, COL_PROTOCOL );
    col_set_fence( pinfo->cinfo, COL_INFO );
#ifndef WIRESHARK_VERSION_OLDER_THAN_160
    dissector_try_uint( udp_dissector_table, UDP_PORT_SYSLOG, tvb, pinfo, trema_tree );
#else
    dissector_try_port( udp_dissector_table, UDP_PORT_SYSLOG, tvb, pinfo, trema_tree );
#endif
  }
  else {
    gint length = tvb_length_remaining( tvb, 0 );
    if ( length > 0 ) {
      proto_tree_add_item( trema_tree, hf_syslog_dump_message, tvb, 0, length, FALSE );
    }
  }
}


static void
dissect_syslog_dump( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, proto_tree *trema_tree,
                     gint offset, gboolean visited, stream_id *stream_name ) {

  offset += dissect_syslog_dump_header( tvb, pinfo, trema_tree, offset );
  guint16 remaining_len = tvb_reported_length_remaining( tvb, offset );
  if ( remaining_len > 0 ) {
    tvbuff_t *next_tvb = tvb_new_subset( tvb, offset, -1, remaining_len );
    dissect_syslog( next_tvb, pinfo, trema_tree );
  }
}


static gint
dissect_text_dump_header( tvbuff_t *tvb, packet_info *pinfo, proto_tree *trema_tree, gint offset ) {
  /*
    typedef struct text_dump_header {
      struct {
        uint32_t sec;
        uint32_t nsec;
      } sent_time;
    } text_dump_header;
  */
  gint head = offset;

  if ( trema_tree != NULL ) {
    proto_tree *text_dump_header_tree = NULL;
    proto_item *ti = NULL;

    ti = proto_tree_add_item( trema_tree, hf_text_dump_header, tvb, offset, sizeof( text_dump_header ), FALSE );
    text_dump_header_tree = proto_item_add_subtree( ti, ett_text_dump_header );

    nstime_t receive_time;
    receive_time.secs = tvb_get_ntohl( tvb, offset );
    receive_time.nsecs = tvb_get_ntohl( tvb, offset + 4 );
    proto_tree_add_time( text_dump_header_tree, hf_text_dump_receive_time, tvb, offset, 8, &receive_time );
    offset += sizeof( text_dump_header );
  }

  return ( offset - head );
}


static void
dissect_text_dump( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, proto_tree *trema_tree,
                   gint offset, gboolean visited, stream_id *stream_name ) {

  offset += dissect_text_dump_header( tvb, pinfo, trema_tree, offset );
  guint16 remaining_len = tvb_reported_length_remaining( tvb, offset );
  if ( remaining_len > 0 ) {
    proto_tree_add_item( trema_tree, hf_text_dump_string, tvb, offset, -1, FALSE );

    gchar *string = tvb_format_text( tvb, offset, remaining_len - 1 );

    if ( check_col( pinfo->cinfo, COL_PROTOCOL ) ) {
      col_append_str( pinfo->cinfo, COL_PROTOCOL, "+TEXT" );
    }

    if ( check_col( pinfo->cinfo, COL_INFO ) && string != NULL ) {
      col_append_str( pinfo->cinfo, COL_INFO, " => " );
      col_append_str( pinfo->cinfo, COL_INFO, string );
    }

    col_set_fence( pinfo->cinfo, COL_PROTOCOL );
    col_set_fence( pinfo->cinfo, COL_INFO );
  }
}


static void
dissect_trema( tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree ) {
  gboolean visited = TRUE;
  gint offset = 0;
  guint16 dump_type = 0;
  proto_tree *trema_tree = NULL;
  proto_item *ti = NULL;
  stream_id stream_name;

  PRINTF( "----- start %u\n", pinfo->fd->num );

  if ( tree == NULL ) {   // FIXME: is it okay to return here?
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

  get_stream_id( tvb, &stream_name );
  PRINTF( " app [%s]  service [%s]\n", stream_name.app_name, stream_name.service_name );

  offset = dissect_message_pcap_dump_header( tvb, pinfo, trema_tree, &dump_type );
  if ( tvb_length_remaining( tvb, offset ) <= 0 ) {
    PRINTF( "----- end %u  (message_pcap_dump_header only)\n", pinfo->fd->num );
    return;
  }

  if ( dump_type >= MESSENGER_DUMP_SENT && dump_type <= MESSENGER_DUMP_SEND_CLOSED ) {
    dissect_trema_ipc( tvb, pinfo, tree, trema_tree, offset, visited, &stream_name );
  }
  else if ( dump_type == MESSENGER_DUMP_PCAP ) {
    dissect_pcap_dump( tvb, pinfo, tree, trema_tree, offset, visited, &stream_name );
  }
  else if ( dump_type == MESSENGER_DUMP_SYSLOG ) {
    dissect_syslog_dump( tvb, pinfo, tree, trema_tree, offset, visited, &stream_name );
  }
  else if ( dump_type == MESSENGER_DUMP_TEXT ) {
    dissect_text_dump( tvb, pinfo, tree, trema_tree, offset, visited, &stream_name );
  }
  else {
    // FIXME
    dissect_message_dump( tvb, offset, trema_tree );
  }

  PRINTF( "----- end %u\n", pinfo->fd->num );
}


void
proto_register_trema() {
  static hf_register_info hf[] = {
    { &hf_dump_header,
      { "Dump header", "trema.dump_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Dump header", HFILL }},
    { &hf_dump_type,
      { "Type", "trema.dump_type",
        FT_UINT16, BASE_DEC, VALS( names_dump_type ), NO_MASK, "Type", HFILL }},
    { &hf_dump_event_time,
      { "Event occurred at", "trema.dump_event_time",
        FT_ABSOLUTE_TIME, ABSOLUTE_TIME_LOCAL, NO_STRINGS, NO_MASK, "Event Occurred at", HFILL }},
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

    // pcap_dump_header
    { &hf_pcap_dump_header,
      { "PCAP dump header", "trema.pcap_dump_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "PCAP dump header", HFILL }},
    { &hf_pcap_dump_datalink,
      { "Datalink", "trema.pcap_dump_datalink",
        FT_UINT32, BASE_DEC, VALS( names_datalink_type ), NO_MASK, "Datalink", HFILL }},
    { &hf_pcap_dump_interface,
      { "Interface", "trema.pcap_dump_interface",
        FT_STRING, BASE_NONE, NO_STRINGS, NO_MASK, "Inerface", HFILL }},

    // pcap_pkthdr
    { &hf_pcap_pkthdr,
      { "PCAP packet header", "trema.pcap_pkthdr",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "PCAP packet header", HFILL }},
    { &hf_pcap_pkthdr_ts,
      { "Captured at", "trema.pcap_pkthdr_ts",
        FT_ABSOLUTE_TIME, ABSOLUTE_TIME_LOCAL, NO_STRINGS, NO_MASK, "Captured at", HFILL }},
    { &hf_pcap_pkthdr_caplen,
      { "Capture length", "trema.pcap_pkthdr_caplen",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, "Capture length", HFILL }},
    { &hf_pcap_pkthdr_len,
      { "Frame length", "trema.pcap_pkthdr_len",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, "Frame length", HFILL }},

    // syslog_dump_header
    { &hf_syslog_dump_header,
      { "Syslog dump header", "trema.syslog_dump_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Syslog dump header", HFILL }},
    { &hf_syslog_dump_receive_time,
      { "Received at", "trema.syslog_dump_receive_time",
        FT_ABSOLUTE_TIME, ABSOLUTE_TIME_LOCAL, NO_STRINGS, NO_MASK, "Received at", HFILL }},

    // text_dump_header
    { &hf_text_dump_header,
      { "Text dump header", "trema.text_dump_header",
        FT_NONE, BASE_NONE, NO_STRINGS, NO_MASK, "Text dump header", HFILL }},
    { &hf_text_dump_receive_time,
      { "Received at", "trema.text_dump_receive_time",
        FT_ABSOLUTE_TIME, ABSOLUTE_TIME_LOCAL, NO_STRINGS, NO_MASK, "Received at", HFILL }},
    { &hf_text_dump_string,
      { "Text string", "trema.text_dump_string",
        FT_STRING, BASE_NONE, NO_STRINGS, NO_MASK, "Text string", HFILL }},

    // raw syslog message
    { &hf_syslog_dump_message,
      { "Raw syslog message", "trema.syslog_message",
        FT_STRING, BASE_NONE, NO_STRINGS, NO_MASK, "Raw syslog message", HFILL }},

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
#ifndef WIRESHARK_VERSION_OLDER_THAN_160
    { &hf_trema_fragment_count,
      { "Trema defragmentation count", "trema.fragment.count",
        FT_UINT32, BASE_DEC, NO_STRINGS, NO_MASK, NULL, HFILL }},
#endif
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
    &ett_pcap_dump_header,
    &ett_pcap_pkthdr,
    &ett_syslog_dump_header,
    &ett_text_dump_header,
  };

  proto_trema = proto_register_protocol( "Trema Universal Event Dumper", "TREMA", "trema" );
  proto_register_field_array( proto_trema, hf, array_length( hf ) );
  proto_register_subtree_array( ett, array_length( ett ) );
  register_dissector( "trema", dissect_trema, proto_trema );

  register_init_routine( init_trema_fragment );
}


void
proto_reg_handoff_trema() {
  ethernet_handle = find_dissector( "eth" );
  ppp_handle = find_dissector( "ppp" );
  ip_handle = find_dissector( "ip" );
  openflow_handle = find_dissector( "openflow" );
  sll_dissector_table = find_dissector_table( "sll.ltype" );
  udp_dissector_table = find_dissector_table( "udp.port" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
