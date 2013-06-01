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


#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"
#include "log.h"
#include "messenger.h"
#include "openflow_application_interface.h"
#include "openflow_message.h"
#include "packet_info.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#define static

#ifdef get_trema_name
#undef get_trema_name
#endif
#define get_trema_name mock_get_trema_name
const char *mock_get_trema_name( void );

#ifdef send_message
#undef send_message
#endif
#define send_message mock_send_message
bool mock_send_message( char *service_name, uint16_t tag, void *data, size_t len );

#ifdef send_request_message
#undef send_request_message
#endif
#define send_request_message mock_send_request_message
bool mock_send_request_message( const char *to_service_name, char *from_service_name, uint16_t tag,
                                void *data, size_t len, void *user_data );

#ifdef init_openflow_message
#undef init_openflow_message
#endif
#define init_openflow_message mock_init_openflow_message
bool mock_init_openflow_message( void );

#ifdef add_message_received_callback
#undef add_message_received_callback
#endif
#define add_message_received_callback mock_add_message_received_callback
bool mock_add_message_received_callback( char *service_name,
                                         void ( *callback )( uint16_t tag, void *data, size_t len ) );

#ifdef add_message_replied_callback
#undef add_message_replied_callback
#endif
#define add_message_replied_callback mock_add_message_replied_callback
bool mock_add_message_replied_callback( char *service_name,
                                        void ( *callback )( uint16_t tag, void *data, size_t len, void *yser_data ) );

#ifdef delete_message_received_callback
#undef delete_message_received_callback
#endif
#define delete_message_received_callback mock_delete_message_received_callback
bool mock_delete_message_received_callback( char *service_name,
                                            void ( *callback )( uint16_t tag, void *data, size_t len ) );

#ifdef delete_message_replied_callback
#undef delete_message_replied_callback
#endif
#define delete_message_replied_callback mock_delete_message_replied_callback
bool mock_delete_message_replied_callback( char *service_name,
                                           void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) );

#ifdef clear_send_queue
#undef clear_send_queue
#endif
#define clear_send_queue mock_clear_send_queue
bool mock_clear_send_queue( const char *service_name );

#ifdef getpid
#undef getpid
#endif
#define getpid mock_getpid
pid_t mock_getpid( void );

#ifdef parse_packet
#undef parse_packet
#endif
#define parse_packet mock_parse_packet
bool mock_parse_packet( buffer *buf );

#ifdef die
#undef die
#endif
#define die mock_die
void mock_die( const char *format, ... );

#ifdef debug
#undef debug
#endif
#define debug mock_debug
extern void mock_debug( const char *format, ... );

#ifdef info
#undef info
#endif
#define info mock_info
extern void mock_info( const char *format, ... );

#ifdef warn
#undef warn
#endif
#define warn mock_warn
extern void mock_warn( const char *format, ... );

#ifdef error
#undef error
#endif
#define error mock_error
extern void mock_error( const char *format, ... );

#ifdef critical
#undef critical
#endif
#define critical mock_critical
extern void mock_critical( const char *format, ... );

#endif // UNIT_TESTING

static bool openflow_application_interface_initialized = false;
static openflow_event_handlers_t event_handlers;
static char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


static void handle_message( uint16_t message_type, void *data, size_t length );
static void handle_list_switches_reply( uint16_t message_type, void *dpid, size_t length, void *user_data );


enum {
  OPENFLOW_MESSAGE_SEND = 0,
  OPENFLOW_MESSAGE_RECEIVE,
};


bool
openflow_application_interface_is_initialized() {
  return openflow_application_interface_initialized;
}


static bool
maybe_init_openflow_application_interface() {
  if ( !openflow_application_interface_is_initialized() ) {
    debug( "OpenFlow Application Interface is not initialized yet. Initializing..." );
    return init_openflow_application_interface( get_trema_name() );
  }
  return true;
}


bool
init_openflow_application_interface( const char *custom_service_name ) {
  assert( custom_service_name != NULL );

  debug( "Initializing OpenFlow Application Interface." );

  if ( openflow_application_interface_is_initialized() ) {
    error( "OpenFlow Application Interface is already initialized." );
    return false;
  }

  memset( &event_handlers, 0, sizeof( openflow_event_handlers_t ) );
  memset( service_name, '\0', sizeof( service_name ) );

  size_t length = strlen( custom_service_name ) + 1;
  if ( length > MESSENGER_SERVICE_NAME_LENGTH ) {
    error( "Too long custom service name ( %s ).", custom_service_name );
    return false;
  }
  assert( length <= sizeof( service_name ) );
  memcpy( service_name, custom_service_name, length );

  init_openflow_message();

  add_message_received_callback( service_name, handle_message );
  add_message_replied_callback( service_name, handle_list_switches_reply );

  openflow_application_interface_initialized = true;

  return true;
}


bool
finalize_openflow_application_interface() {
  debug( "Finalizing OpenFlow Application Interface." );

  assert( openflow_application_interface_initialized );

  delete_message_received_callback( service_name, handle_message );
  delete_message_replied_callback( service_name, handle_list_switches_reply );

  memset( &event_handlers, 0, sizeof( openflow_event_handlers_t ) );
  memset( service_name, '\0', sizeof( service_name ) );

  openflow_application_interface_initialized = false;

  return true;
}


bool
set_openflow_event_handlers( const openflow_event_handlers_t handlers ) {
  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  memcpy( &event_handlers, &handlers, sizeof( event_handlers ) );

  return true;
}


bool
_set_switch_ready_handler( bool simple_callback, void *callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Invalid callback function for switch_ready event." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a switch ready handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.simple_switch_ready_callback = simple_callback;
  event_handlers.switch_ready_callback = callback;
  event_handlers.switch_ready_user_data = user_data;

  return true;
}


bool
set_switch_disconnected_handler( switch_disconnected_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( switch_disconnected_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a switch disconnected handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.switch_disconnected_callback = callback;
  event_handlers.switch_disconnected_user_data = user_data;

  return true;
}


bool
set_error_handler( error_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( error_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting an error handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.error_callback = callback;
  event_handlers.error_user_data = user_data;

  return true;
}


bool
set_echo_reply_handler( echo_reply_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( echo_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a echo reply handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.echo_reply_callback = callback;
  event_handlers.echo_reply_user_data = user_data;

  return true;
}


bool
set_vendor_handler( vendor_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( vendor_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a vendor handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.vendor_callback = callback;
  event_handlers.vendor_user_data = user_data;

  return true;
}


bool
set_features_reply_handler( features_reply_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( features_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a features reply handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.features_reply_callback = callback;
  event_handlers.features_reply_user_data = user_data;

  return true;
}


bool
set_get_config_reply_handler( get_config_reply_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( get_config_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a get config reply handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.get_config_reply_callback = callback;
  event_handlers.get_config_reply_user_data = user_data;

  return true;
}


bool
_set_packet_in_handler( bool simple_callback, void *callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function (packet_in_handler) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a packet-in handler (callback = %p, user_data = %p).", callback, user_data );

  event_handlers.simple_packet_in_callback = simple_callback;
  event_handlers.packet_in_callback = callback;
  event_handlers.packet_in_user_data = user_data;

  return true;
}


bool
_set_flow_removed_handler( bool simple_callback, void *callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function (flow_removed_handler) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a flow removed handler (callback = %p, user_data = %p).", callback, user_data );

  event_handlers.simple_flow_removed_callback = simple_callback;
  event_handlers.flow_removed_callback = callback;
  event_handlers.flow_removed_user_data = user_data;

  return true;
}


bool
set_port_status_handler( port_status_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( port_status_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a port status handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.port_status_callback = callback;
  event_handlers.port_status_user_data = user_data;

  return true;
}


bool
set_stats_reply_handler( stats_reply_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( stats_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a stats reply handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.stats_reply_callback = callback;
  event_handlers.stats_reply_user_data = user_data;

  return true;
}


bool
set_barrier_reply_handler( barrier_reply_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( barrier_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a barrier reply handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.barrier_reply_callback = callback;
  event_handlers.barrier_reply_user_data = user_data;

  return true;
}


bool
set_queue_get_config_reply_handler( queue_get_config_reply_handler callback, void *user_data ) {
  if ( callback == NULL ) {
    die( "Callback function ( queue_get_config_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a queue get config reply handler ( callback = %p, user_data = %p ).",
         callback, user_data );

  event_handlers.queue_get_config_reply_callback = callback;
  event_handlers.queue_get_config_reply_user_data = user_data;

  return true;
}


bool
set_list_switches_reply_handler( list_switches_reply_handler callback ) {
  if ( callback == NULL ) {
    die( "Callback function ( list_switches_reply_handler ) must not be NULL." );
  }
  assert( callback != NULL );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Setting a list switches reply handler ( callback = %p ).", callback );

  event_handlers.list_switches_reply_callback = callback;

  return true;
}


static void
handle_error( const uint64_t datapath_id, buffer *data ) {
  uint16_t type, code;
  uint32_t transaction_id;
  buffer *body;
  struct ofp_error_msg *error_msg;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_error()." );
    assert( 0 );
  }

  error_msg = ( struct ofp_error_msg * ) data->data;

  transaction_id = ntohl( error_msg->header.xid );
  type = ntohs( error_msg->type );
  code = ntohs( error_msg->code );

  body = duplicate_buffer( data );
  remove_front_buffer( body, offsetof( struct ofp_error_msg, data ) );

  debug( "An error message is received from %#" PRIx64 " "
         "( transaction_id = %#x, type = %#x, code = %#x, data length = %zu ).",
         datapath_id, transaction_id, type, code, body->length );

  if ( event_handlers.error_callback == NULL ) {
    debug( "Callback function for error events is not set." );
    free_buffer( body );
    return;
  }

  debug( "Calling error handler ( callback = %p, user_data = %p ).",
         event_handlers.error_callback, event_handlers.error_user_data );

  event_handlers.error_callback( datapath_id,
                                 transaction_id,
                                 type,
                                 code,
                                 body,
                                 event_handlers.error_user_data );

  free_buffer( body );
}


static void
handle_echo_reply( const uint64_t datapath_id, buffer *data ) {
  uint16_t body_length;
  uint32_t transaction_id;
  buffer *body;
  struct ofp_header *header;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_echo_reply()." );
    assert( 0 );
  }

  header = ( struct ofp_header * ) data->data;

  transaction_id = ntohl( header->xid );

  body_length = ( uint16_t ) ( ntohs( header->length )
                               - sizeof( struct ofp_header ) );

  debug( "A echo reply message is received from %#" PRIx64
         " ( transaction_id = %#x, body length = %u ).",
         datapath_id, transaction_id, body_length );

  if ( event_handlers.echo_reply_callback == NULL ) {
    debug( "Callback function for echo reply events is not set." );
    return;
  }

  if ( body_length > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, sizeof( struct ofp_header ) );
  }
  else {
    body = NULL;
  }

  debug( "Calling echo reply handler ( callback = %p, user_data = %p ).",
         event_handlers.echo_reply_callback, event_handlers.echo_reply_user_data );

  event_handlers.echo_reply_callback( datapath_id,
                                      transaction_id,
                                      body,
                                      event_handlers.echo_reply_user_data );

  if ( body != NULL ) {
    free_buffer( body );
  }
}



static void
handle_vendor( const uint64_t datapath_id, buffer *data ) {
  uint16_t body_length;
  uint32_t vendor, transaction_id;
  buffer *body;
  struct ofp_vendor_header *vendor_header;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_vendor()." );
    assert( 0 );
  }

  vendor_header = ( struct ofp_vendor_header * ) data->data;

  transaction_id = ntohl( vendor_header->header.xid );
  vendor = ntohl( vendor_header->vendor );

  body_length = ( uint16_t ) ( ntohs( vendor_header->header.length )
                               - sizeof( struct ofp_vendor_header ) );

  debug( "A vendor message is received from %#" PRIx64
         " ( transaction_id = %#x, vendor = %#x, body length = %u ).",
         datapath_id, transaction_id, vendor, body_length );

  if ( event_handlers.vendor_callback == NULL ) {
    debug( "Callback function for vendor events is not set." );
    return;
  }

  if ( body_length > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, sizeof( struct ofp_vendor_header ) );
  }
  else {
    body = NULL;
  }

  debug( "Calling vendor handler ( callback = %p, user_data = %p ).",
         event_handlers.vendor_callback, event_handlers.vendor_user_data );

  event_handlers.vendor_callback( datapath_id,
                                  transaction_id,
                                  vendor,
                                  body,
                                  event_handlers.vendor_user_data );

  if ( body != NULL ) {
    free_buffer( body );
  }
}


static void
handle_features_reply( const uint64_t datapath_id, buffer *data ) {
  char description[ 1024 ];
  int i, n_phy_ports;
  uint8_t n_tables;
  uint16_t phy_ports_length;
  uint32_t transaction_id, n_buffers, capabilities, actions;
  list_element *element, *phy_ports_head;
  struct ofp_phy_port *p, *phy_port;
  struct ofp_switch_features *switch_features;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_features_reply()." );
    assert( 0 );
  }

  switch_features = ( struct ofp_switch_features * ) data->data;

  transaction_id = ntohl( switch_features->header.xid );
  n_buffers = ntohl( switch_features->n_buffers );
  n_tables = switch_features->n_tables;
  capabilities = ntohl( switch_features->capabilities );
  actions = ntohl( switch_features->actions );

  phy_ports_length = ( uint16_t ) ( ntohs( switch_features->header.length )
                                    - offsetof( struct ofp_switch_features, ports ) );

  n_phy_ports = phy_ports_length / sizeof( struct ofp_phy_port );

  debug( "A features reply message is received from %#" PRIx64
         " ( transaction_id = %#x, n_buffers = %u, n_tables = %u, "
         "capabilities = %#x, actions = %#x, # of phy ports = %u ).",
         datapath_id, transaction_id, n_buffers, n_tables,
         capabilities, actions, n_phy_ports );

  if ( n_phy_ports > 0 ) {
    create_list( &phy_ports_head );
    phy_port = ( struct ofp_phy_port * ) switch_features->ports;
    for ( i = 0; i < n_phy_ports; i++ ) {
      p = ( struct ofp_phy_port * ) xcalloc( 1, sizeof( struct ofp_phy_port ) );
      ntoh_phy_port( p, phy_port );
      append_to_tail( &phy_ports_head, ( void * ) p );
      phy_port_to_string( p, description, sizeof( description ) );
      debug( "[%p] %s", phy_port, description );
      phy_port++;
    }
  }
  else {
    phy_ports_head = NULL;
  }

  if ( event_handlers.features_reply_callback == NULL ) {
    debug( "Callback function for features reply events is not set." );
    if ( phy_ports_head != NULL ) {
      element = phy_ports_head;
      while ( element != NULL ) {
        xfree( element->data );
        element = element->next;
      }
      delete_list( phy_ports_head );
    }
    return;
  }

  debug( "Calling features reply handler ( callback = %p, user_data = %p ).",
         event_handlers.features_reply_callback, event_handlers.features_reply_user_data );

  event_handlers.features_reply_callback( datapath_id,
                                          transaction_id,
                                          n_buffers,
                                          n_tables,
                                          capabilities,
                                          actions,
                                          phy_ports_head,
                                          event_handlers.features_reply_user_data );

  if ( phy_ports_head != NULL ) {
    element = phy_ports_head;
    while ( element != NULL ) {
      xfree( element->data );
      element = element->next;
    }
    delete_list( phy_ports_head );
  }
}


static void
handle_get_config_reply( const uint64_t datapath_id, buffer *data ) {
  uint16_t flags, miss_send_len;
  uint32_t transaction_id;
  struct ofp_switch_config *switch_config;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_get_config_reply()." );
    assert( 0 );
  }

  switch_config = ( struct ofp_switch_config * ) data->data;

  transaction_id = ntohl( switch_config->header.xid );
  flags = ntohs( switch_config->flags );
  miss_send_len = ntohs( switch_config->miss_send_len );

  debug( "A get config reply message is received from %#" PRIx64
         " ( transaction_id = %#x, flags = %#x, miss_send_len = %u ).",
         datapath_id, transaction_id, flags, miss_send_len );

  if ( event_handlers.get_config_reply_callback == NULL ) {
    debug( "Callback function for get config reply events is not set." );
    return;
  }

  debug( "Calling get config reply handler ( callback = %p, user_data = %p ).",
         event_handlers.get_config_reply_callback, event_handlers.get_config_reply_user_data );

  event_handlers.get_config_reply_callback( datapath_id,
                                            transaction_id,
                                            flags,
                                            miss_send_len,
                                            event_handlers.get_config_reply_user_data );
}


static bool
empty( const buffer *data ) {
  return ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) );
}


static void
handle_packet_in( const uint64_t datapath_id, buffer *data ) {
  if ( empty( data ) ) {
    die( "handle_packet_in(): packet_in message should not be empty." );
  }

  struct ofp_packet_in *_packet_in = ( struct ofp_packet_in * ) data->data;
  uint32_t transaction_id = ntohl( _packet_in->header.xid );
  uint32_t buffer_id = ntohl( _packet_in->buffer_id );
  uint16_t total_len = ntohs( _packet_in->total_len );
  uint16_t in_port = ntohs( _packet_in->in_port );
  uint8_t reason = _packet_in->reason;
  uint16_t body_length = ( uint16_t ) ( ntohs( _packet_in->header.length ) - offsetof( struct ofp_packet_in, data ) );

  debug(
    "A packet_in message is received from %#" PRIx64
    " ( transaction_id = %#x, buffer_id = %#x, total_len = %u, in_port = %u, reason = %#x, body length = %u ).",
    datapath_id,
    transaction_id,
    buffer_id,
    total_len,
    in_port,
    reason,
    body_length
  );

  if ( event_handlers.packet_in_callback == NULL ) {
    debug( "Callback function for packet_in events is not set." );
    return;
  }

  buffer *body = NULL;
  if ( body_length > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, offsetof( struct ofp_packet_in, data ) );
    bool parse_ok = parse_packet( body );
    if ( !parse_ok ) {
      error( "Failed to parse a packet." );
      // ???: Is it OK to drop malformed packets?
      free_buffer( body );
      return;
    }
  }
  else {
    body = NULL;
  }

  assert( event_handlers.packet_in_callback != NULL );
  debug( "Calling packet_in handler ( callback = %p, user_data = %p ).",
         event_handlers.packet_in_callback,
         event_handlers.packet_in_user_data
  );
  if ( event_handlers.simple_packet_in_callback ) {
    packet_in message = {
      datapath_id,
      transaction_id,
      buffer_id,
      total_len,
      in_port,
      reason,
      body,
      event_handlers.packet_in_user_data
    };
    ( ( simple_packet_in_handler * ) event_handlers.packet_in_callback )( datapath_id, message );
  }
  else {
    ( ( packet_in_handler * ) event_handlers.packet_in_callback )(
      datapath_id,
      transaction_id,
      buffer_id,
      total_len,
      in_port,
      reason,
      body,
      event_handlers.packet_in_user_data
    );
  }

  if ( body != NULL ) {
    free_buffer( body );
  }
}


static void
handle_flow_removed( const uint64_t datapath_id, buffer *data ) {
  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_flow_removed()." );
    assert( 0 );
  }

  struct ofp_flow_removed *_flow_removed = ( struct ofp_flow_removed * ) data->data;
  uint32_t transaction_id = ntohl( _flow_removed->header.xid );
  struct ofp_match match;
  ntoh_match( &match, &_flow_removed->match );
  uint64_t cookie = ntohll( _flow_removed->cookie );
  uint16_t priority = ntohs( _flow_removed->priority );
  uint8_t reason = _flow_removed->reason;
  uint32_t duration_sec = ntohl( _flow_removed->duration_sec );
  uint32_t duration_nsec = ntohl( _flow_removed->duration_nsec );
  uint16_t idle_timeout = ntohs( _flow_removed->idle_timeout );
  uint64_t packet_count = ntohll( _flow_removed->packet_count );
  uint64_t byte_count = ntohll( _flow_removed->byte_count );

  char match_string[ 1024 ];
  match_to_string( &match, match_string, sizeof( match_string ) );

  debug(
    "A flow removed message is received from %#" PRIx64
    " ( transaction_id = %#x, match = [%s], cookie = %#" PRIx64 ", "
    "priority = %u, reason = %#x, duration_sec = %u, duration_nsec = %u, "
    "idle_timeout = %u, packet_count = %" PRIu64 ", byte_count = %" PRIu64 " ).",
    datapath_id,
    transaction_id,
    match_string,
    cookie,
    priority,
    reason,
    duration_sec,
    duration_nsec,
    idle_timeout,
    packet_count,
    byte_count
  );

  if ( event_handlers.flow_removed_callback == NULL ) {
    debug( "Callback function for flow removed events is not set." );
    return;
  }

  debug(
    "Calling flow removed handler ( callback = %p, user_data = %p ).",
    event_handlers.flow_removed_callback,
    event_handlers.flow_removed_user_data
  );
  if ( event_handlers.simple_flow_removed_callback ) {
    flow_removed message = {
      datapath_id,
      transaction_id,
      match,
      cookie,
      priority,
      reason,
      duration_sec,
      duration_nsec,
      idle_timeout,
      packet_count,
      byte_count,
      event_handlers.flow_removed_user_data
    };
    ( ( simple_flow_removed_handler * ) event_handlers.flow_removed_callback )( datapath_id, message );
  }
  else {
    ( ( flow_removed_handler * ) event_handlers.flow_removed_callback )(
      datapath_id,
      transaction_id,
      match,
      cookie,
      priority,
      reason,
      duration_sec,
      duration_nsec,
      idle_timeout,
      packet_count,
      byte_count,
      event_handlers.flow_removed_user_data
    );
  }
}


static void
handle_port_status( const uint64_t datapath_id, buffer *data ) {
  char description[ 1024 ];
  uint8_t reason;
  uint32_t transaction_id;
  struct ofp_phy_port phy_port;
  struct ofp_port_status *port_status;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_port_status()." );
    assert( 0 );
  }

  port_status = ( struct ofp_port_status * ) data->data;

  transaction_id = ntohl( port_status->header.xid );
  reason = port_status->reason;
  ntoh_phy_port( &phy_port, &port_status->desc );

  phy_port_to_string( &phy_port, description, sizeof( description ) );

  debug( "A port status message is received from %#" PRIx64
         " ( transaction_id = %#x, reason = %#x, desc = [%s] ).",
         datapath_id, transaction_id, reason, description );

  if ( event_handlers.port_status_callback == NULL ) {
    debug( "Callback function for port status events is not set." );
    return;
  }

  debug( "Calling port status handler ( callback = %p, user_data = %p ).",
         event_handlers.port_status_callback, event_handlers.port_status_user_data );

  event_handlers.port_status_callback( datapath_id,
                                       transaction_id,
                                       reason,
                                       phy_port,
                                       event_handlers.port_status_user_data );
}


static void
handle_stats_reply( const uint64_t datapath_id, buffer *data ) {
  uint16_t type, flags, body_length;
  uint32_t transaction_id;
  buffer *body = NULL;
  buffer *body_h = NULL;
  struct ofp_stats_reply *stats_reply;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_stats_reply()." );
    assert( 0 );
  }

  stats_reply = ( struct ofp_stats_reply * ) data->data;

  transaction_id = ntohl( stats_reply->header.xid );
  type = ntohs( stats_reply->type );
  flags = ntohs( stats_reply->flags );

  body_length = ( uint16_t ) ( ntohs( stats_reply->header.length )
                               - offsetof( struct ofp_stats_reply, body ) );

  debug( "A stats reply message is received from %#" PRIx64
         " ( transaction_id = %#x, type = %#x, flags = %#x, body length = %u ).",
         datapath_id, transaction_id, type, flags, body_length );

  if ( event_handlers.stats_reply_callback == NULL ) {
    debug( "Callback function for stats reply events is not set." );
    return;
  }

  if ( body_length > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, offsetof( struct ofp_stats_reply, body ) );
  }

  if ( body != NULL ) {
    switch ( type ) {
    case OFPST_DESC:
      {
        body_h = body;
        body = NULL;
      }
      break;
    case OFPST_FLOW:
      {
        struct ofp_flow_stats *src, *dst;

        body_h = alloc_buffer_with_length( body_length );
        append_back_buffer( body_h, body_length );

        src = ( struct ofp_flow_stats * ) body->data;
        dst = ( struct ofp_flow_stats * ) body_h->data;

        while ( body_length > 0 ) {
          ntoh_flow_stats( dst, src );

          body_length = ( uint16_t ) ( body_length - dst->length );

          src = ( struct ofp_flow_stats * ) ( ( char * ) src + dst->length );
          dst = ( struct ofp_flow_stats * ) ( ( char * ) dst + dst->length );
        }
      }
      break;
    case OFPST_AGGREGATE:
      {
        struct ofp_aggregate_stats_reply *src, *dst;

        body_h = alloc_buffer_with_length( body_length );
        append_back_buffer( body_h, body_length );

        src = ( struct ofp_aggregate_stats_reply * ) body->data;
        dst = ( struct ofp_aggregate_stats_reply * ) body_h->data;

        ntoh_aggregate_stats( dst, src );
      }
      break;
    case OFPST_TABLE:
      {
        struct ofp_table_stats *src, *dst;

        body_h = alloc_buffer_with_length( body_length );
        append_back_buffer( body_h, body_length );

        src = ( struct ofp_table_stats * ) body->data;
        dst = ( struct ofp_table_stats * ) body_h->data;

        while ( body_length > 0 ) {
          ntoh_table_stats( dst, src );

          body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_table_stats ) );

          src++;
          dst++;
        }
      }
      break;
    case OFPST_PORT:
      {
        struct ofp_port_stats *src, *dst;

        body_h = alloc_buffer_with_length( body_length );
        append_back_buffer( body_h, body_length );

        src = ( struct ofp_port_stats * ) body->data;
        dst = ( struct ofp_port_stats * ) body_h->data;

        while ( body_length > 0 ) {
          ntoh_port_stats( dst, src );

          body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_port_stats ) );

          src++;
          dst++;
        }
      }
      break;
    case OFPST_QUEUE:
      {
        struct ofp_queue_stats *src, *dst;

        body_h = alloc_buffer_with_length( body_length );
        append_back_buffer( body_h, body_length );

        src = ( struct ofp_queue_stats * ) body->data;
        dst = ( struct ofp_queue_stats * ) body_h->data;

        while ( body_length > 0 ) {
          ntoh_queue_stats( dst, src );

          body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_queue_stats ) );

          src++;
          dst++;
        }
      }
      break;
    case OFPST_VENDOR:
      {
        uint32_t *src, *dst;
        body_h = alloc_buffer_with_length( body_length );
        append_back_buffer( body_h, body_length );

        memcpy( body_h->data, body->data, body_length );

        src = ( uint32_t * ) body->data;
        dst = ( uint32_t * ) body_h->data;

        *dst = ntohl( *src ); // vendor id
      }
      break;
    default:
      if ( body != NULL ) {
        free_buffer( body );
      }
      critical( "Unhandled stats type ( type = %#x ).", type );
      assert( 0 );
      break;
    }
  }

  debug( "Calling stats reply handler ( callback = %p, user_data = %p ).",
         event_handlers.stats_reply_callback, event_handlers.stats_reply_user_data );

  event_handlers.stats_reply_callback( datapath_id,
                                       transaction_id,
                                       type,
                                       flags,
                                       body_h,
                                       event_handlers.stats_reply_user_data );

  if ( body != NULL ) {
    free_buffer( body );
  }
  if ( body_h != NULL ) {
    free_buffer( body_h );
  }
}


static void
handle_barrier_reply( const uint64_t datapath_id, buffer *data ) {
  uint32_t transaction_id;
  struct ofp_header *header;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_barrier_reply()." );
    assert( 0 );
  }

  header = ( struct ofp_header * ) data->data;

  transaction_id = ntohl( header->xid );

  debug( "A barrier reply message is received from %#" PRIx64 " ( transaction_id = %#x ).",
         datapath_id, transaction_id );

  if ( event_handlers.barrier_reply_callback == NULL ) {
    debug( "Callback function for barrier reply events is not set." );
    return;
  }

  debug( "Calling barrier reply handler ( callback = %p, user_data = %p ).",
         event_handlers.barrier_reply_callback, event_handlers.barrier_reply_user_data );

  event_handlers.barrier_reply_callback( datapath_id,
                                         transaction_id,
                                         event_handlers.barrier_reply_user_data );
}


static void
handle_queue_get_config_reply( const uint64_t datapath_id, buffer *data ) {
  uint16_t port, queues_length;
  uint32_t transaction_id;
  list_element *queues_head = NULL;
  list_element *element = NULL;
  struct ofp_packet_queue *pq, *packet_queue;
  struct ofp_queue_get_config_reply *queue_get_config_reply;

  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    critical( "An OpenFlow message must be filled before calling handle_queue_get_config_reply()." );
    assert( 0 );
  }

  queue_get_config_reply = ( struct ofp_queue_get_config_reply * ) data->data;

  transaction_id = ntohl( queue_get_config_reply->header.xid );
  port = ntohs( queue_get_config_reply->port );

  queues_length = ( uint16_t ) ( ntohs( queue_get_config_reply->header.length )
                  - offsetof( struct ofp_queue_get_config_reply, queues ) );

  debug( "A queue get config reply message is received from %#" PRIx64
         " ( transaction_id = %#x, port = %u, queues length = %u ).",
         datapath_id, transaction_id, port, queues_length );

  if ( event_handlers.queue_get_config_reply_callback == NULL ) {
    debug( "Callback function for queue get config reply events is not set." );
    return;
  }

  if ( queues_length > 0 ) {
    create_list( &queues_head );
  }
  else {
    critical( "No queues found." );
    assert( 0 );
  }

  packet_queue = ( struct ofp_packet_queue * ) queue_get_config_reply->queues;

  while ( queues_length > 0 ) {
    pq = ( struct ofp_packet_queue * ) xcalloc( 1, ntohs( packet_queue->len ) );

    ntoh_packet_queue( pq, packet_queue );
    append_to_tail( &queues_head, pq );

    packet_queue = ( struct ofp_packet_queue * ) ( ( char * ) packet_queue + pq->len );
    queues_length = ( uint16_t ) ( queues_length - pq->len );
  }

  debug( "Calling queue get config reply handler ( callback = %p, user_data = %p ).",
         event_handlers.queue_get_config_reply_callback,
         event_handlers.queue_get_config_reply_user_data );

  event_handlers.queue_get_config_reply_callback( datapath_id,
                                                  transaction_id,
                                                  port,
                                                  queues_head,
                                                  event_handlers.queue_get_config_reply_user_data );

  if ( queues_head != NULL ) {
    element = queues_head;
    while ( element != NULL ) {
      xfree( element->data );
      element = element->next;
    }
    delete_list( queues_head );
  }
}


static void
update_switch_event_stats( uint16_t type, int send_receive, bool result ) {
  char key[ STAT_KEY_LENGTH ];
  char suffix[ 16 ];
  char direction[ 16 ];
  const char *prefix = "openflow_application_interface.";

  memset( suffix, '\0', sizeof( suffix ) );

  if ( result ) {
    sprintf( suffix, "_succeeded" );
  }
  else {
    sprintf( suffix, "_failed" );
  }

  memset( direction, '\0', sizeof( direction ) );

  switch ( send_receive ) {
  case OPENFLOW_MESSAGE_SEND:
    sprintf( direction, "_send" );
    break;
  case OPENFLOW_MESSAGE_RECEIVE:
    sprintf( direction, "_receive" );
    break;
  default:
    return;
  }

  switch ( type ) {
  case MESSENGER_OPENFLOW_CONNECTED:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "switch_connected", direction, suffix );
    break;
  case MESSENGER_OPENFLOW_READY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "switch_ready", direction, suffix );
    break;
  case MESSENGER_OPENFLOW_DISCONNECTED:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "switch_disconnected", direction, suffix );
    break;
  case MESSENGER_OPENFLOW_FAILD_TO_CONNECT:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "switch_failed_to_connect", direction, suffix );
    break;
  default:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "undefined_switch_event", direction, suffix );
    break;
  }

  increment_stat( key );
}


static void
handle_switch_ready( uint64_t datapath_id ) {
  if ( event_handlers.switch_ready_callback == NULL ) {
    debug( "Callback function for switch_ready events is not set." );
    return;
  }

  assert( event_handlers.switch_ready_callback != NULL );
  if ( event_handlers.simple_switch_ready_callback ) {
    switch_ready event = {
      datapath_id,
      event_handlers.switch_ready_user_data
    };
    ( ( simple_switch_ready_handler * ) event_handlers.switch_ready_callback )( event );
  }
  else {
    ( ( switch_ready_handler * ) event_handlers.switch_ready_callback )(
      datapath_id,
      event_handlers.switch_ready_user_data
    );
  }
}


static void
handle_messenger_openflow_disconnected( uint64_t datapath_id ) {
  if ( event_handlers.switch_disconnected_callback != NULL ) {
    debug( "Calling switch disconnected handler ( callback = %p, user_data = %p ).",
           event_handlers.switch_disconnected_callback, event_handlers.switch_disconnected_user_data );
    event_handlers.switch_disconnected_callback( datapath_id, event_handlers.switch_disconnected_user_data );
  }
  else {
    debug( "Callback function for switch disconnected events is not set." );
  }
  delete_openflow_messages( datapath_id );
}


static void
handle_switch_events( uint16_t type, void *data, size_t length ) {
  assert( data != NULL );
  assert( length == sizeof( openflow_service_header_t ) );

  debug( "Received a switch event ( type = %#x ) from remote.", type );

  openflow_service_header_t *message = data;
  uint64_t datapath_id = ntohll( message->datapath_id );

  switch ( type ) {
    case MESSENGER_OPENFLOW_CONNECTED:
    case MESSENGER_OPENFLOW_FAILD_TO_CONNECT:
      // Do nothing.
      break;
    case MESSENGER_OPENFLOW_READY:
      handle_switch_ready( datapath_id );
      break;
    case MESSENGER_OPENFLOW_DISCONNECTED:
      handle_messenger_openflow_disconnected( datapath_id );
      break;
    default:
      error( "Unhandled switch event ( type = %#x ).", type );
      break;
  }

  update_switch_event_stats( type, OPENFLOW_MESSAGE_RECEIVE, true );
}


static void
update_openflow_stats( uint8_t type, int send_receive, bool result ) {
  char key[ STAT_KEY_LENGTH ];
  char suffix[ 16 ];
  char direction[ 16 ];
  const char *prefix = "openflow_application_interface.";

  memset( suffix, '\0', sizeof( suffix ) );

  if ( result ) {
    sprintf( suffix, "_succeeded" );
  }
  else {
    sprintf( suffix, "_failed" );
  }

  memset( direction, '\0', sizeof( direction ) );

  switch ( send_receive ) {
  case OPENFLOW_MESSAGE_SEND:
    sprintf( direction, "_send" );
    break;
  case OPENFLOW_MESSAGE_RECEIVE:
    sprintf( direction, "_receive" );
    break;
  default:
    return;
  }

  switch ( type ) {
  case OFPT_HELLO:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "hello", direction, suffix );
    break;
  case OFPT_ERROR:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "error", direction, suffix );
    break;
  case OFPT_ECHO_REQUEST:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "echo_request", direction, suffix );
    break;
  case OFPT_ECHO_REPLY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "echo_reply", direction, suffix );
    break;
  case OFPT_VENDOR:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "vendor", direction, suffix );
    break;
  case OFPT_FEATURES_REQUEST:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "features_request", direction, suffix );
    break;
  case OFPT_FEATURES_REPLY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "features_reply", direction, suffix );
    break;
  case OFPT_GET_CONFIG_REQUEST:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "get_config_request", direction, suffix );
    break;
  case OFPT_GET_CONFIG_REPLY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "get_config_reply", direction, suffix );
    break;
  case OFPT_SET_CONFIG:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "set_config", direction, suffix );
    break;
  case OFPT_PACKET_IN:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "packet_in", direction, suffix );
    break;
  case OFPT_FLOW_REMOVED:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "flow_removed", direction, suffix );
    break;
  case OFPT_PORT_STATUS:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "port_status", direction, suffix );
    break;
  case OFPT_PACKET_OUT:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "packet_out", direction, suffix );
    break;
  case OFPT_FLOW_MOD:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "flow_mod", direction, suffix );
    break;
  case OFPT_PORT_MOD:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "port_mod", direction, suffix );
    break;
  case OFPT_STATS_REQUEST:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "stats_request", direction, suffix );
    break;
  case OFPT_STATS_REPLY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "stats_reply", direction, suffix );
    break;
  case OFPT_BARRIER_REQUEST:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "barrier_request", direction, suffix );
    break;
  case OFPT_BARRIER_REPLY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "barrier_reply", direction, suffix );
    break;
  case OFPT_QUEUE_GET_CONFIG_REQUEST:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "queue_get_config_request", direction, suffix );
    break;
  case OFPT_QUEUE_GET_CONFIG_REPLY:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "queue_get_config_reply", direction, suffix );
    break;
  default:
    snprintf( key, STAT_KEY_LENGTH, "%s%s%s%s", prefix, "undefined_message_type", direction, suffix );
    break;
  }

  increment_stat( key );
}


static void
handle_openflow_message( void *data, size_t length ) {
  void *p;
  int ret;
  uint64_t datapath_id;
  buffer *buffer;
  struct ofp_header *header;
  openflow_service_header_t *message;

  assert( data != NULL );
  assert( length >= ( sizeof( openflow_service_header_t ) + sizeof( struct ofp_header ) ) );

  debug( "An OpenFlow message is received from remote." );

  message = ( openflow_service_header_t * ) data;

  datapath_id = ntohll( message->datapath_id );

  buffer = alloc_buffer_with_length( length );

  assert( buffer != NULL );

  p = append_back_buffer( buffer, length );
  memcpy( p, data, length );
  remove_front_buffer( buffer, sizeof( openflow_service_header_t ) );

  ret = validate_openflow_message( buffer );

  if ( ret < 0 ) {
    error( "Failed to validate an OpenFlow message ( code = %d, length = %zu ).", ret, length );
    free_buffer( buffer );

    return;
  }

  header = ( struct ofp_header * ) buffer->data;

  switch ( header->type ) {
  case OFPT_ERROR:
    handle_error( datapath_id, buffer );
    break;
  case OFPT_ECHO_REPLY:
    handle_echo_reply( datapath_id, buffer );
    break;
  case OFPT_VENDOR:
    handle_vendor( datapath_id, buffer );
    break;
  case OFPT_FEATURES_REPLY:
    handle_features_reply( datapath_id, buffer );
    break;
  case OFPT_GET_CONFIG_REPLY:
    handle_get_config_reply( datapath_id, buffer );
    break;
  case OFPT_PACKET_IN:
    handle_packet_in( datapath_id, buffer );
    break;
  case OFPT_FLOW_REMOVED:
    handle_flow_removed( datapath_id, buffer );
    break;
  case OFPT_PORT_STATUS:
    handle_port_status( datapath_id, buffer );
    break;
  case OFPT_STATS_REPLY:
    handle_stats_reply( datapath_id, buffer );
    break;
  case OFPT_BARRIER_REPLY:
    handle_barrier_reply( datapath_id, buffer );
    break;
  case OFPT_QUEUE_GET_CONFIG_REPLY:
    handle_queue_get_config_reply( datapath_id, buffer );
    break;
  default:
    error( "Unhandled OpenFlow message ( type = %#x ).", header->type );
    break;
  }

  update_openflow_stats( header->type, OPENFLOW_MESSAGE_RECEIVE, true );

  free_buffer( buffer );
}


static void
handle_message( uint16_t type, void *data, size_t length ) {
  assert( data != NULL );
  assert( length >= sizeof( openflow_service_header_t ) );

  debug( "A message is received from remote ( type = %#x ).", type );

  switch ( type ) {
  case MESSENGER_OPENFLOW_MESSAGE:
    return handle_openflow_message( data, length );
  case MESSENGER_OPENFLOW_CONNECTED:
  case MESSENGER_OPENFLOW_FAILD_TO_CONNECT:
  case MESSENGER_OPENFLOW_READY:
  case MESSENGER_OPENFLOW_DISCONNECTED:
    return handle_switch_events( type, data, length );
  default:
    error( "Unhandled message ( type = %#x ).", type );
    update_switch_event_stats( type, OPENFLOW_MESSAGE_RECEIVE, true );
    break;
  }
}


static void
insert_dpid( list_element **head, uint64_t *dpid ) {
  assert( head != NULL );
  assert( dpid != NULL );

  list_element *element;
  for ( element = *head; element != NULL; element = element->next ) {
    if ( *dpid <= *( uint64_t * ) element->data ) {
      break;
    }
  }
  if ( element == NULL ) {
    append_to_tail( head, dpid );
  }
  else if ( element == *head ) {
    insert_in_front( head, dpid );
  }
  else {
    insert_before( head, element->data, dpid );
  }
}


static void
handle_list_switches_reply( uint16_t message_type, void *data, size_t length, void *user_data ) {
  UNUSED( message_type );
  assert( data != NULL );

  uint64_t *dpid = ( uint64_t * ) data;
  size_t num_switch = length / sizeof( uint64_t );

  debug( "A list switches reply message is received ( number of switches = %zu ).",
         num_switch );

  if ( event_handlers.list_switches_reply_callback == NULL ) {
    debug( "Callback function for list switches reply events is not set." );
    return;
  }

  list_element *switches;
  create_list( &switches );

  unsigned int i;
  for ( i = 0; i < num_switch; ++i ) {
    uint64_t *datapath_id = ( uint64_t * ) xmalloc( sizeof( uint64_t ) );
    *datapath_id = ntohll( dpid[ i ] );
    insert_dpid( &switches, datapath_id );
  }

  debug( "Calling list switches reply handler ( callback = %p ).",
         event_handlers.list_switches_reply_callback );

  event_handlers.list_switches_reply_callback( switches, user_data );

  list_element *element;
  for ( element = switches; element != NULL; element = element->next ) {
    xfree( element->data );
  }
  delete_list( switches );
}


bool
send_openflow_message( const uint64_t datapath_id, buffer *message ) {
  bool ret;
  void *data;
  char remote_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  uint16_t header_length;
  buffer *buffer;
  struct ofp_header *ofp;
  openflow_service_header_t header;

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  if ( ( message == NULL ) || ( ( message != NULL ) && ( message->length == 0 ) ) ) {
    critical( "An OpenFlow message must be passed to send_openflow_message()." );
    assert( 0 );
  }

  ofp = ( struct ofp_header * ) message->data;
  buffer = duplicate_buffer( message );

  assert( buffer != NULL );

  header_length = ( uint16_t ) ( sizeof( openflow_service_header_t )
                  + strlen( service_name ) + 1 );

  header.datapath_id = htonll( datapath_id );
  header.service_name_length = htons( ( uint16_t ) ( strlen( service_name ) + 1 ) );

  data = append_front_buffer( buffer, header_length );
  memset( data, '\0', header_length );
  memcpy( data, &header, sizeof( openflow_service_header_t ) );
  memcpy( ( char * ) data + sizeof( openflow_service_header_t ),
          service_name, strlen( service_name ) );

  memset( remote_service_name, '\0', sizeof( remote_service_name ) );
  snprintf( remote_service_name, sizeof( remote_service_name ),
            "switch.%#" PRIx64, datapath_id );

  debug( "Sending an OpenFlow message to %#" PRIx64
         " ( service_name = %s, remote_service_name = %s, "
         "ofp_header = [version = %#x, type = %#x, length = %u, transaction_id = %#x] ).",
         datapath_id, service_name, remote_service_name,
         ofp->version, ofp->type, ntohs( ofp->length ), ntohl( ofp->xid ) );

  ret =  send_message( remote_service_name, MESSENGER_OPENFLOW_MESSAGE,
                       buffer->data, buffer->length );

  free_buffer( buffer );

  update_openflow_stats( ofp->type, OPENFLOW_MESSAGE_SEND, ret );

  return ret;
}


bool
send_list_switches_request( void *user_data ) {
  uint16_t message_type = 0;
  void *data = NULL;
  size_t data_length = 0;

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  debug( "Sending a list switches request ( service_name = %s ).", service_name );

  return send_request_message( "switch_manager", service_name, message_type,
                               data, data_length, user_data );
}


bool
delete_openflow_messages( uint64_t datapath_id ) {
  debug( "Deleting OpenFlow messages in a send queue ( datapath_id = %#" PRIx64 " ).", datapath_id );

  char remote_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  memset( remote_service_name, '\0', sizeof( remote_service_name ) );
  snprintf( remote_service_name, sizeof( remote_service_name ),
            "switch.%#" PRIx64, datapath_id );
  return clear_send_queue( remote_service_name );
}


bool
disconnect_switch( uint64_t datapath_id ) {
  debug( "Disconnecting a switch ( datapath_id = %#" PRIx64 " ).", datapath_id );

  maybe_init_openflow_application_interface();
  assert( openflow_application_interface_initialized );

  size_t service_name_length = strlen( service_name ) + 1;
  size_t length = sizeof( openflow_service_header_t ) + service_name_length;
  buffer *buf = alloc_buffer_with_length( length );
  openflow_service_header_t *header = append_back_buffer( buf, sizeof( openflow_service_header_t ) );
  header->datapath_id = htonll( datapath_id );
  header->service_name_length = htons( ( uint16_t ) service_name_length );
  char *name = append_back_buffer( buf, service_name_length );
  memcpy( name, service_name, service_name_length );

  char remote_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  snprintf( remote_service_name, sizeof( remote_service_name ),
            "switch.%#" PRIx64, datapath_id );

  bool ret =  send_message( remote_service_name, MESSENGER_OPENFLOW_DISCONNECT_REQUEST,
                            buf->data, buf->length );

  free_buffer( buf );

  return ret;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
