/*
 * OpenFlow Packet_in message filter
 *
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


#include <arpa/inet.h>
#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"


#ifdef UNIT_TESTING

#define static
#define main packetin_filter_main

#ifdef printf
#undef printf
#endif
#define printf( fmt, args... )  mock_printf2( fmt, ##args )
int mock_printf2( const char *format, ... );

#ifdef error
#undef error
#endif
#define error( fmt, args... ) mock_error( fmt, ##args )
void mock_error( const char *format, ... );

#ifdef set_match_from_packet
#undef set_match_from_packet
#endif
#define set_match_from_packet mock_set_match_from_packet
void mock_set_match_from_packet( struct ofp_match *match, const uint16_t in_port,
                                 const uint32_t wildcards, const buffer *packet );

#ifdef create_packet_in
#undef create_packet_in
#endif
#define create_packet_in mock_create_packet_in
buffer *mock_create_packet_in( const uint32_t transaction_id, const uint32_t buffer_id,
                               const uint16_t total_len, uint16_t in_port,
                               const uint8_t reason, const buffer *data );

#ifdef insert_match_entry
#undef insert_match_entry
#endif
#define insert_match_entry mock_insert_match_entry
void mock_insert_match_entry( struct ofp_match *ofp_match, uint16_t priority,
                              const char *service_name );

#ifdef lookup_match_entry
#undef lookup_match_entry
#endif
#define lookup_match_entry mock_lookup_match_entry
match_entry *mock_lookup_match_entry( struct ofp_match *match );

#ifdef send_message
#undef send_message
#endif
#define send_message mock_send_message
bool mock_send_message( const char *service_name, const uint16_t tag, const void *data,
                        size_t len );

#ifdef init_trema
#undef init_trema
#endif
#define init_trema mock_init_trema
void mock_init_trema( int *argc, char ***argv );

#ifdef set_packet_in_handler
#undef set_packet_in_handler
#endif
#define set_packet_in_handler mock_set_packet_in_handler
bool mock_set_packet_in_handler( packet_in_handler callback, void *user_data );

#ifdef start_trema
#undef start_trema
#endif
#define start_trema mock_start_trema
void mock_start_trema( void );

#ifdef get_executable_name
#undef get_executable_name
#endif
#define get_executable_name mock_get_executable_name
const char *mock_get_executable_name( void );

#endif // UNIT_TESTING


void
usage() {
  printf(
    "OpenFlow Packet in Filter.\n"
    "Usage: %s [OPTION]... [PACKETIN-FILTER-RULE]...\n"
    "\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
    "\n"
    "PACKETIN-FILTER-RULE:\n"
    "  match-type::destination-service-name\n"
    "\n"
    "match-type:\n"
    "  lldp                            LLDP ethernet frame type and priority is 0x8000\n"
    "  packet_in                       any packet and priority is zero\n"
    "\n"
    "destination-service-name          destination service name\n"
    , get_executable_name()
  );
}


static buffer *
parse_etherip( const buffer *data ) {
  packet_info *packet_info = data->user_data;
  if ( packet_info->etherip_version != ETHERIP_VERSION ) {
    error( "invalid etherip version 0x%04x.", packet_info->etherip_version );
    return NULL;
  }
  if ( packet_info->etherip_offset == 0 ) {
    debug( "too short etherip message" );
    return NULL;
  }

  buffer *copy = duplicate_buffer( data );
  if ( copy == NULL ) {
    error( "duplicate_buffer failed." );
    return NULL;
  }
  copy->user_data = NULL;
  remove_front_buffer( copy, packet_info->etherip_offset );

  if ( !parse_packet( copy ) ) {
    error( "parse_packet failed." );
    free_buffer( copy );
    return NULL;
  }

  debug( "Receive EtherIP packet." );

  return copy;
}


static void
handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                  uint32_t buffer_id, uint16_t total_len,
                  uint16_t in_port, uint8_t reason, const buffer *data,
                  void *user_data ) {
  UNUSED( user_data );

  char match_str[ 1024 ];
  struct ofp_match ofp_match;   // host order

  buffer *copy = NULL;
  packet_info *packet_info = data->user_data;
  debug( "Receive packet. ethertype=0x%04x, ipproto=0x%x", packet_info->eth_type, packet_info->ipv4_protocol );
  if ( packet_type_ipv4_etherip( data ) ) {
    copy = parse_etherip( data );
  }
  set_match_from_packet( &ofp_match, in_port, 0, copy != NULL ? copy : data );
  if ( copy != NULL ) {
    free_buffer( copy );
    copy = NULL;
  }
  match_to_string( &ofp_match, match_str, sizeof( match_str ) );

  list_element *services = lookup_match_entry( ofp_match );
  if ( services == NULL ) {
    debug( "match entry not found" );
    return;
  }

  buffer *buf = create_packet_in( transaction_id, buffer_id, total_len, in_port,
                                  reason, data );

  openflow_service_header_t *message;
  message = append_front_buffer( buf, sizeof( openflow_service_header_t ) );
  message->datapath_id = htonll( datapath_id );
  message->service_name_length = htons( 0 );
  list_element *element;
  for ( element = services; element != NULL; element = element->next ) {
    const char *service_name = element->data;
    if ( !send_message( service_name, MESSENGER_OPENFLOW_MESSAGE,
                        buf->data, buf->length ) ) {
      error( "Failed to send a message to %s ( match = %s ).", service_name, match_str );
      free_buffer( buf );
      return;
    }

    debug( "Sending a message to %s ( match = %s ).", service_name, match_str );
  }

  free_buffer( buf );
}


static void
init_packetin_match_table( void ) {
  init_match_table();
}


static void
free_user_data_entry( struct ofp_match match, uint16_t priority, void *services, void *user_data ) {
  UNUSED( match );
  UNUSED( priority );
  UNUSED( user_data );

  list_element *element;
  for ( element = services; element != NULL; element = element->next ) {
    xfree( element->data );
    element->data = NULL;
  }
  delete_list( services );
}


static void
finalize_packetin_match_table( void ) {
  foreach_match_table( free_user_data_entry, NULL );
  finalize_match_table();
}


static bool
add_packetin_match_entry( struct ofp_match match, uint16_t priority, const char *service_name ) {
  bool ( *insert_or_update_match_entry )( struct ofp_match, uint16_t, void * ) = update_match_entry;
  list_element *services = lookup_match_strict_entry( match, priority );
  if ( services == NULL ) {
    insert_or_update_match_entry = insert_match_entry;
    create_list( &services );
  }
  else {
    list_element *element;
    for ( element = services; element != NULL; element = element->next ) {
      if ( strcmp( element->data, service_name ) == 0 ) {
        char match_string[ 256 ];
        match_to_string( &match, match_string, sizeof( match_string ) );
        warn( "match entry already exists ( match = [%s], service_name = [%s] )", match_string, service_name );
        return false;
      }
    }
  }
  append_to_tail( &services, xstrdup( service_name ) );
  insert_or_update_match_entry( match, priority, services );

  return true;
}


static int
delete_packetin_match_entry( struct ofp_match match, uint16_t priority, const char *service_name ) {
  list_element *head = delete_match_strict_entry( match, priority );
  if ( head == NULL ) {
    return 0;
  }

  int n_deleted = 0;
  int n_remaining_services = 0;
  list_element *services = head;
  while ( services != NULL ) {
    char *service = services->data;
    services = services->next;
    if ( strcmp( service, service_name ) == 0 ) {
      delete_element( &head, service );
      xfree( service );
      n_deleted++;
    }
    else {
      n_remaining_services++;
    }
  }

  if ( n_remaining_services == 0 ) {
    if ( head != NULL ) {
      delete_list( head );
    }
  }
  else {
    insert_match_entry( match, priority, head );
  }

  return n_deleted;
}


static void
register_dl_type_filter( uint16_t dl_type, uint16_t priority, const char *service_name ) {
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL & ~OFPFW_DL_TYPE;
  match.dl_type = dl_type;

  add_packetin_match_entry( match, priority, service_name );
}


static void
register_any_filter( uint16_t priority, const char *service_name ) {
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  add_packetin_match_entry( match, priority, service_name );
}


static const char *
match_type( const char *type, char *name ) {
  size_t len = strlen( type );
  if ( strncmp( name, type, len ) != 0 ) {
    return NULL;
  }

  return name + len;
}


// built-in packetin-filter-rule
static const char LLDP_PACKET_IN[] = "lldp::";
static const char ANY_PACKET_IN[] = "packet_in::";

static bool
set_match_type( int argc, char *argv[] ) {
  int i;
  const char *service_name;
  for ( i = 1; i < argc; i++ ) {
    if ( ( service_name = match_type( LLDP_PACKET_IN, argv[ i ] ) ) != NULL ) {
      register_dl_type_filter( ETH_ETHTYPE_LLDP, OFP_DEFAULT_PRIORITY, service_name );
    }
    else if ( ( service_name = match_type( ANY_PACKET_IN, argv[ i ] ) ) != NULL ) {
      register_any_filter( 0, service_name );
    }
    else {
      return false;
    }
  }

  return true;
}


static void
handle_add_filter_request( const messenger_context_handle *handle, add_packetin_filter_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );

  request->entry.service_name[ MESSENGER_SERVICE_NAME_LENGTH - 1 ] = '\0';
  if ( strlen( request->entry.service_name ) == 0 ) {
    error( "Service name must be specified." );
    return;
  }
  struct ofp_match match;
  ntoh_match( &match, &request->entry.match );
  bool ret = add_packetin_match_entry( match, ntohs( request->entry.priority ), request->entry.service_name );

  add_packetin_filter_reply reply;
  memset( &reply, 0, sizeof( add_packetin_filter_reply ) );
  reply.status = ( uint8_t ) ( ret ? PACKETIN_FILTER_OPERATION_SUCCEEDED : PACKETIN_FILTER_OPERATION_FAILED );
  ret = send_reply_message( handle, MESSENGER_ADD_PACKETIN_FILTER_REPLY,
                            &reply, sizeof( add_packetin_filter_reply ) );
  if ( ret == false ) {
    error( "Failed to send an add filter reply." );
  }
}


static void
delete_filter_walker( struct ofp_match match, uint16_t priority, void *data, void *user_data ) {
  UNUSED( data );
  buffer *reply_buffer = user_data;
  assert( reply_buffer != NULL );

  delete_packetin_filter_reply *reply = reply_buffer->data;
  list_element *head = delete_match_strict_entry( match, priority );
  for ( list_element *services = head; services != NULL; services = services->next ) {
    xfree( services->data );
    reply->n_deleted++;
  }
  if ( head != NULL ) {
    delete_list( head );
  }
}


static void
handle_delete_filter_request( const messenger_context_handle *handle, delete_packetin_filter_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );

  buffer *buf = alloc_buffer_with_length( sizeof( delete_packetin_filter_reply ) );
  delete_packetin_filter_reply *reply = append_back_buffer( buf, sizeof( delete_packetin_filter_reply ) );
  reply->status = PACKETIN_FILTER_OPERATION_SUCCEEDED;
  reply->n_deleted = 0;

  struct ofp_match match;
  ntoh_match( &match, &request->criteria.match );
  uint16_t priority = ntohs( request->criteria.priority );
  if ( request->flags & PACKETIN_FILTER_FLAG_MATCH_STRICT ) {
    int n_deleted = delete_packetin_match_entry( match, priority, request->criteria.service_name );
    reply->n_deleted += ( uint32_t ) n_deleted;
  }
  else {
    map_match_table( match, delete_filter_walker, buf );
  }
  reply->n_deleted = htonl( reply->n_deleted );

  bool ret = send_reply_message( handle, MESSENGER_DELETE_PACKETIN_FILTER_REPLY, buf->data, buf->length );
  free_buffer( buf );
  if ( ret == false ) {
    error( "Failed to send a dump filter reply." );
  }
}


static void
dump_filter_walker( struct ofp_match match, uint16_t priority, void *data, void *user_data ) {
  buffer *reply_buffer = user_data;
  assert( reply_buffer != NULL );

  dump_packetin_filter_reply *reply = reply_buffer->data;
  list_element *services = data;
  while ( services != NULL ) {
    reply->n_entries++;
    packetin_filter_entry *entry = append_back_buffer( reply_buffer, sizeof( packetin_filter_entry ) );
    hton_match( &entry->match, &match );
    entry->priority = htons( priority );
    strncpy( entry->service_name, services->data, sizeof( entry->service_name ) );
    entry->service_name[ sizeof( entry->service_name ) - 1 ] = '\0';
    services = services->next;
  }
}


static void
handle_dump_filter_request( const messenger_context_handle *handle, dump_packetin_filter_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );

  buffer *buf = alloc_buffer_with_length( 2048 );
  dump_packetin_filter_reply *reply = append_back_buffer( buf, offsetof( dump_packetin_filter_reply, entries ) );
  reply->status = PACKETIN_FILTER_OPERATION_SUCCEEDED;
  reply->n_entries = 0;

  struct ofp_match match;
  ntoh_match( &match, &request->criteria.match );
  uint16_t priority = ntohs( request->criteria.priority );
  if ( request->flags & PACKETIN_FILTER_FLAG_MATCH_STRICT ) {
    list_element *services = lookup_match_strict_entry( match, priority );
    while ( services != NULL ) {
      if ( strcmp( services->data, request->criteria.service_name ) == 0 ) {
        packetin_filter_entry *entry = append_back_buffer( buf, sizeof( packetin_filter_entry ) );
        reply->n_entries++;
        entry->match = request->criteria.match;
        entry->priority = request->criteria.priority;
        strncpy( entry->service_name, services->data, sizeof( entry->service_name ) );
        entry->service_name[ sizeof( entry->service_name ) - 1 ] = '\0';
      }
      services = services->next;
    }
  }
  else {
    map_match_table( match, dump_filter_walker, buf );
  }
  reply->n_entries = htonl( reply->n_entries );

  bool ret = send_reply_message( handle, MESSENGER_DUMP_PACKETIN_FILTER_REPLY, buf->data, buf->length );
  free_buffer( buf );
  if ( ret == false ) {
    error( "Failed to send a dump packetin filter reply." );
  }
}


static void
handle_request( const messenger_context_handle *handle, uint16_t tag, void *data, size_t length ) {
  assert( handle != NULL );

  debug( "Handling a request ( handle = %p, tag = %#x, data = %p, length = %zu ).",
         handle, tag, data, length );

  switch ( tag ) {
    case MESSENGER_ADD_PACKETIN_FILTER_REQUEST:
    {
      if ( length != sizeof( add_packetin_filter_request ) ) {
        error( "Invalid add packetin filter request ( length = %zu ).", length );
        return;
      }

      handle_add_filter_request( handle, data );
    }
    break;
    case MESSENGER_DELETE_PACKETIN_FILTER_REQUEST:
    {
      if ( length != sizeof( delete_packetin_filter_request ) ) {
        error( "Invalid delete packetin filter request ( length = %zu ).", length );
        return;
      }

      handle_delete_filter_request( handle, data );
    }
    break;
    case MESSENGER_DUMP_PACKETIN_FILTER_REQUEST:
    {
      if ( length != sizeof( dump_packetin_filter_request ) ) {
        error( "Invalid dump packetin filter request ( length = %zu ).", length );
        return;
      }

      handle_dump_filter_request( handle, data );
    }
    break;
    default:
    {
      warn( "Undefined request tag ( tag = %#x ).", tag );
    }
    break;
  }
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  init_packetin_match_table();

  // built-in packetin-filter-rule
  if ( !set_match_type( argc, argv ) ) {
    usage();
    finalize_packetin_match_table();
    exit( EXIT_FAILURE );
  }

  set_packet_in_handler( handle_packet_in, NULL );
  add_message_requested_callback( PACKETIN_FILTER_MANAGEMENT_SERVICE, handle_request );

  start_trema();

  finalize_packetin_match_table();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
