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


#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <syslog.h>
#include "chibach_private.h"
#include "hash_table.h"
#include "log.h"
#include "messenger.h"
#include "openflow_message.h"
#include "openflow_service_interface.h"
#include "openflow_switch_interface.h"
#include "secure_channel.h"
#include "timer.h"
#include "wrapper.h"


typedef struct {
  uint64_t datapath_id;
  struct {
    uint32_t ip;
    uint16_t port;
  } controller;
} openflow_switch_config;

typedef bool ( *message_send_handler )( buffer *message, void *user_data );

typedef struct {
  uint32_t transaction_id;
  buffer *message;
  message_send_handler send_callback;
  void *user_data;
  time_t created_at;
} openflow_context;


static bool openflow_switch_interface_initialized = false;
static openflow_event_handlers event_handlers;
static openflow_switch_config config;
static const int CONTEXT_LIFETIME = 5;
static hash_table *contexts = NULL;


static bool
compare_context( const void *x, const void *y ) {
  const openflow_context *cx = x;
  const openflow_context *cy = y;

  return ( cx->transaction_id == cy->transaction_id ) ? true : false;
}


static unsigned int
hash_context( const void *key ) {
  return ( unsigned int ) *( ( const uint32_t * ) key );
}


static openflow_context *
lookup_context( uint32_t transaction_id ) {
  assert( contexts != NULL );

  return lookup_hash_entry( contexts, &transaction_id );
}


static bool
save_context( uint32_t transaction_id, buffer *message, message_send_handler callback, void *user_data ) {
  assert( contexts != NULL );

  openflow_context *context = lookup_context( transaction_id );
  if ( context != NULL ) {
    return false;
  }

  context = xmalloc( sizeof( openflow_context ) );
  memset( context, 0, sizeof( openflow_context ) );
  context->transaction_id = transaction_id;
  context->message = duplicate_buffer( message );
  context->send_callback = callback;
  context->user_data = user_data;
  context->created_at = time( NULL );

  insert_hash_entry( contexts, &context->transaction_id, context );

  return true;
}


static void
delete_context( uint32_t transaction_id ) {
  assert( contexts != NULL );

  openflow_context *context = delete_hash_entry( contexts, &transaction_id );
  if ( context == NULL ) {
    return;
  }

  if ( context->message != NULL ) {
    free_buffer( context->message );
  }
  if ( context->user_data != NULL ) {
    xfree( context->user_data );
  }

  xfree( context );
}


static void
age_contexts( void *user_data ) {
  UNUSED( user_data );

  time_t now = time( NULL );

  hash_iterator iter;
  init_hash_iterator( contexts, &iter );
  hash_entry *e = NULL;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    openflow_context *context = e->value;
    if ( ( context != NULL ) && ( ( context->created_at + CONTEXT_LIFETIME ) <= now ) ) {
      delete_context( context->transaction_id );
    }
  }
}


static void
init_context() {
  assert( contexts == NULL );

  contexts = create_hash_with_size( compare_context, hash_context, 128 );
}


static void
finalize_context() {
  assert( contexts != NULL );

  hash_iterator iter;
  init_hash_iterator( contexts, &iter );
  hash_entry *e = NULL;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    openflow_context *context = e->value;
    if ( context != NULL ) {
      delete_context( context->transaction_id );
    }
  }
  delete_hash( contexts );
  contexts = NULL;
}


bool
openflow_switch_interface_is_initialized() {
  return openflow_switch_interface_initialized;
}


bool
switch_set_openflow_event_handlers( const openflow_event_handlers handlers ) {
  assert( openflow_switch_interface_initialized );

  memcpy( &event_handlers, &handlers, sizeof( event_handlers ) );

  return true;
}


bool
set_controller_connected_handler( controller_connected_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a controller connected handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.controller_connected_callback = callback;
  event_handlers.controller_connected_user_data = user_data;

  return true;
}


bool
set_controller_disconnected_handler( controller_disconnected_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a controller disconnected handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.controller_disconnected_callback = callback;
  event_handlers.controller_disconnected_user_data = user_data;

  return true;
}


bool
set_hello_handler( hello_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a hello handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.hello_callback = callback;
  event_handlers.hello_user_data = user_data;

  return true;
}


bool
switch_set_error_handler( error_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting an error handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.error_callback = callback;
  event_handlers.error_user_data = user_data;

  return true;
}


bool
set_echo_request_handler( echo_request_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting an echo request handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.echo_request_callback = callback;
  event_handlers.echo_request_user_data = user_data;

  return true;
}


bool
switch_set_echo_reply_handler( echo_reply_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting an echo reply handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.echo_reply_callback = callback;
  event_handlers.echo_reply_user_data = user_data;

  return true;
}


bool
switch_set_vendor_handler( vendor_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a vendor handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.vendor_callback = callback;
  event_handlers.vendor_user_data = user_data;

  return true;
}


bool
set_features_request_handler( features_request_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a features request handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.features_request_callback = callback;
  event_handlers.features_request_user_data = user_data;

  return true;
}


bool
set_get_config_request_handler( get_config_request_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a get config request handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.get_config_request_callback = callback;
  event_handlers.get_config_request_user_data = user_data;

  return true;
}


bool
set_set_config_handler( set_config_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a set config handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.set_config_callback = callback;
  event_handlers.set_config_user_data = user_data;

  return true;
}


bool
set_packet_out_handler( packet_out_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a packet out handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.packet_out_callback = callback;
  event_handlers.packet_out_user_data = user_data;

  return true;
}


bool
set_flow_mod_handler( flow_mod_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a flow mod handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.flow_mod_callback = callback;
  event_handlers.flow_mod_user_data = user_data;

  return true;
}


bool
set_port_mod_handler( port_mod_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a port mod handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.port_mod_callback = callback;
  event_handlers.port_mod_user_data = user_data;

  return true;
}


bool
set_stats_request_handler( stats_request_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a stats request handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.stats_request_callback = callback;
  event_handlers.stats_request_user_data = user_data;

  return true;
}


bool
set_barrier_request_handler( barrier_request_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a barrier request handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.barrier_request_callback = callback;
  event_handlers.barrier_request_user_data = user_data;

  return true;
}


bool
set_queue_get_config_request_handler( queue_get_config_request_handler callback, void *user_data ) {
  assert( callback != NULL );
  assert( openflow_switch_interface_initialized );

  debug( "Setting a queue get config request handler ( callback = %p, user_data = %p ).", callback, user_data );

  event_handlers.queue_get_config_request_callback = callback;
  event_handlers.queue_get_config_request_user_data = user_data;

  return true;
}


static bool
empty( const buffer *data ) {
  if ( ( data == NULL ) || ( ( data != NULL ) && ( data->length == 0 ) ) ) {
    return true;
  }

  return false;
}


static void
handle_hello( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_header *hello = data->data;

  uint32_t transaction_id = ntohl( hello->xid );
  uint8_t version = hello->version;

  debug( "A hello message is received ( transaction_id = %#x, version = %#x ).", transaction_id, version );

  if ( event_handlers.hello_callback == NULL ) {
    debug( "Callback function for hello events is not set." );
    return;
  }

  debug( "Calling hello handler ( callback = %p, user_data = %p ).",
         event_handlers.hello_callback, event_handlers.hello_user_data );

  event_handlers.hello_callback( transaction_id,
                                 version,
                                 event_handlers.hello_user_data );
}


static void
handle_error( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_error_msg *error_msg = ( struct ofp_error_msg * ) data->data;

  uint32_t transaction_id = ntohl( error_msg->header.xid );
  uint16_t type = ntohs( error_msg->type );
  uint16_t code = ntohs( error_msg->code );

  buffer *body = duplicate_buffer( data );
  remove_front_buffer( body, offsetof( struct ofp_error_msg, data ) );

  debug( "An error message is received ( transaction_id = %#x, type = %#x, code = %#x, data length = %zu ).",
         transaction_id, type, code, body->length );

  if ( event_handlers.error_callback == NULL ) {
    debug( "Callback function for error events is not set." );
    free_buffer( body );
    return;
  }

  debug( "Calling error handler ( callback = %p, user_data = %p ).",
         event_handlers.error_callback, event_handlers.error_user_data );

  event_handlers.error_callback( transaction_id,
                                 type,
                                 code,
                                 body,
                                 event_handlers.error_user_data );

  free_buffer( body );
}


static void
handle_echo_request( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_header *header = data->data;
  uint32_t transaction_id = htonl( header->xid );
  uint16_t length = htons( header->length );

  debug( "An echo request is received ( transaction_id = %#x, len = %u ).", transaction_id, length );

  if ( event_handlers.echo_request_callback == NULL ) {
    debug( "Callback function for echo request events is not set." );
    return;
  }

  buffer *body = NULL;
  if ( ( length - sizeof( struct ofp_header ) ) > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, sizeof( struct ofp_header ) );
  }

  debug( "Calling echo request handler ( callback = %p, body = %p, user_data = %p ).",
         event_handlers.echo_request_callback,
         body,
         event_handlers.echo_request_user_data );

  event_handlers.echo_request_callback( transaction_id, body, event_handlers.echo_request_user_data );

  if ( body != NULL ) {
    free_buffer( body );
  }
}


static void
handle_echo_reply( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_header *header = data->data;
  uint32_t transaction_id = htonl( header->xid );
  uint16_t length = htons( header->length );

  debug( "An echo reply is received ( transaction_id = %#x, len = %u ).", transaction_id, length );

  if ( event_handlers.echo_reply_callback == NULL ) {
    debug( "Callback function for echo reply events is not set." );
    return;
  }

  buffer *body = NULL;
  if ( ( length - sizeof( struct ofp_header ) ) > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, sizeof( struct ofp_header ) );
  }

  debug( "Calling echo reply handler ( callback = %p, body = %p, user_data = %p ).",
         event_handlers.echo_reply_callback,
         body,
         event_handlers.echo_reply_user_data );

  event_handlers.echo_reply_callback( transaction_id, body, event_handlers.echo_reply_user_data );

  if ( body != NULL ) {
    free_buffer( body );
  }
}


static void
handle_vendor( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_vendor_header *vendor_header = ( struct ofp_vendor_header * ) data->data;

  uint32_t transaction_id = ntohl( vendor_header->header.xid );
  uint32_t vendor = ntohl( vendor_header->vendor );

  uint16_t body_length = ( uint16_t ) ( ntohs( vendor_header->header.length )
                                        - sizeof( struct ofp_vendor_header ) );

  debug( "A vendor message is received ( transaction_id = %#x, vendor = %#x, body length = %u ).",
         transaction_id, vendor, body_length );

  if ( event_handlers.vendor_callback == NULL ) {
    debug( "Callback function for vendor events is not set." );
    return;
  }

  buffer *body = NULL;
  if ( body_length > 0 ) {
    body = duplicate_buffer( data );
    remove_front_buffer( body, sizeof( struct ofp_vendor_header ) );
  }

  debug( "Calling vendor handler ( callback = %p, user_data = %p ).",
         event_handlers.vendor_callback, event_handlers.vendor_user_data );

  event_handlers.vendor_callback( transaction_id,
                                  vendor,
                                  body,
                                  event_handlers.vendor_user_data );

  if ( body != NULL ) {
    free_buffer( body );
  }
}


static void
handle_features_request( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_header *header = data->data;

  uint32_t transaction_id = ntohl( header->xid );

  debug( "A features request is received ( transaction_id = %#x ).", transaction_id );

  if ( event_handlers.features_request_callback == NULL ) {
    debug( "Callback function for features request events is not set." );
    return;
  }

  debug( "Calling features request handler ( callback = %p, user_data = %p ).",
         event_handlers.features_request_callback,
         event_handlers.features_request_user_data );

  event_handlers.features_request_callback( transaction_id,
                                            event_handlers.features_request_user_data );
}


static void
handle_get_config_request( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_header *header = data->data;

  uint32_t transaction_id = ntohl( header->xid );

  debug( "A get config request is received ( transaction_id = %#x ).", transaction_id );

  if ( event_handlers.get_config_request_callback == NULL ) {
    debug( "Callback function for get config request events is not set." );
    return;
  }

  debug( "Calling get config request handler ( callback = %p, user_data = %p ).",
         event_handlers.get_config_request_callback,
         event_handlers.get_config_request_user_data );

  event_handlers.get_config_request_callback( transaction_id,
                                              event_handlers.get_config_request_user_data );
}


static void
handle_set_config( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_switch_config *config = data->data;

  uint32_t transaction_id = ntohl( config->header.xid );
  uint16_t flags = ntohs( config->flags );
  uint16_t miss_send_len = ntohs( config->miss_send_len );

  debug( "A set config is received ( transaction_id = %#x, flags = %#x, miss_send_len = %u ).",
         transaction_id, flags, miss_send_len );

  if ( event_handlers.set_config_callback == NULL ) {
    debug( "Callback function for set config events is not set." );
    return;
  }

  debug( "Calling set config handler ( callback = %p, user_data = %p ).",
         event_handlers.set_config_callback,
         event_handlers.set_config_user_data );

  event_handlers.set_config_callback( transaction_id, flags, miss_send_len,
                                      event_handlers.set_config_user_data );
}


static void
handle_packet_out( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_packet_out *packet_out = data->data;

  uint32_t transaction_id = ntohl( packet_out->header.xid );
  uint32_t buffer_id = ntohl( packet_out->buffer_id );
  uint16_t in_port = ntohs( packet_out->in_port );
  size_t actions_len = ntohs( packet_out->actions_len );
  openflow_actions *actions = NULL;
  if ( actions_len > 0 ) {
    actions = create_actions();
    void *actions_p = packet_out->actions;
    while ( actions_len > 0 ) {
      struct ofp_action_header *ah = actions_p;
      ntoh_action( ah, ah );
      actions_len -= ah->len;
      actions_p = ( char * ) actions_p + ah->len;

      void *action = xmalloc( ah->len );
      memcpy( action, ah, ah->len );
      append_to_tail( &actions->list, ( void * ) action );
      actions->n_actions++;
    }
  }

  buffer *frame = NULL;
  actions_len = ntohs( packet_out->actions_len );
  size_t frame_length = ntohs( packet_out->header.length ) - offsetof( struct ofp_packet_out, actions ) - actions_len;
  if ( frame_length > 0 ) {
    frame = alloc_buffer_with_length( frame_length );
    void *p = append_back_buffer( frame, frame_length );
    size_t offset = offsetof( struct ofp_packet_out, actions ) + actions_len;
    memcpy( p, ( char * ) packet_out + offset, frame_length );
  }

  debug( "A packet-out is received ( transaction_id = %#x, buffer_id = %#x, in_port = %u, "
         "actions_len = %zu, frame_length = %zu ).",
         transaction_id, buffer_id, in_port, actions_len, frame_length );

  if ( event_handlers.packet_out_callback == NULL ) {
    debug( "Callback function for packet-out events is not set." );
    return;
  }

  debug( "Calling packet-outhandler ( callback = %p, user_data = %p ).",
         event_handlers.packet_out_callback,
         event_handlers.packet_out_user_data );

  event_handlers.packet_out_callback( transaction_id, buffer_id, in_port, actions, frame,
                                      event_handlers.packet_out_user_data );

  if ( actions != NULL ) {
    delete_actions( actions );
  }
  if ( frame != NULL ) {
    free_buffer( frame );
  }
}


static void
handle_flow_mod( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_flow_mod *flow_mod = data->data;

  uint32_t transaction_id = ntohl( flow_mod->header.xid );
  struct ofp_match match;
  ntoh_match( &match, &flow_mod->match );
  uint64_t cookie = ntohll( flow_mod->cookie );
  uint16_t command = ntohs( flow_mod->command );
  uint16_t idle_timeout = ntohs( flow_mod->idle_timeout );
  uint16_t hard_timeout = ntohs( flow_mod->hard_timeout );
  uint16_t priority = ntohs( flow_mod->priority );
  uint32_t buffer_id = ntohl( flow_mod->buffer_id );
  uint16_t out_port = ntohs( flow_mod->out_port );
  uint16_t flags = ntohs( flow_mod->flags );

  size_t actions_length = ntohs( flow_mod->header.length ) - offsetof( struct ofp_flow_mod, actions );
  openflow_actions *actions = NULL;
  if ( actions_length > 0 ) {
    actions = create_actions();
    void *actions_p = flow_mod->actions;
    while ( actions_length > 0 ) {
      struct ofp_action_header *ah = actions_p;
      ntoh_action( ah, ah );
      actions_length -= ah->len;
      actions_p = ( char * ) actions_p + ah->len;

      void *action = xmalloc( ah->len );
      memcpy( action, ah, ah->len );
      append_to_tail( &actions->list, ( void * ) action );
      actions->n_actions++;
    }
  }

  if ( get_logging_level() >= LOG_DEBUG ) {
    char match_str[ 256 ];
    match_to_string( &match, match_str, sizeof( match_str ) );
    debug( "A flow modification is received ( transaction_id = %#x, match = [%s], cookie = %#" PRIx64 ", "
           "command = %#x, idle_timeout = %u, hard_timeout = %u, priority = %u, buffer_id = %#x, "
           "out_port = %u, flags = %#x ).",
           transaction_id, match_str, cookie, command, idle_timeout, hard_timeout, priority, buffer_id,
           out_port, flags );
  }

  if ( event_handlers.flow_mod_callback == NULL ) {
    debug( "Callback function for flow modification events is not set." );
    return;
  }

  debug( "Calling flow modification handler ( callback = %p, user_data = %p ).",
         event_handlers.flow_mod_callback,
         event_handlers.flow_mod_user_data );

  event_handlers.flow_mod_callback( transaction_id, match, cookie, command, idle_timeout, hard_timeout,
                                    priority, buffer_id, out_port, flags, actions,
                                    event_handlers.flow_mod_user_data );

  if ( actions != NULL ) {
    delete_actions( actions );
  }
}


static void
handle_port_mod( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_port_mod *port_mod = data->data;

  uint32_t transaction_id = ntohl( port_mod->header.xid );
  uint16_t port_no = ntohs( port_mod->port_no );
  uint8_t hw_addr[ OFP_ETH_ALEN ];
  memcpy( hw_addr, port_mod->hw_addr, OFP_ETH_ALEN );
  uint32_t config = ntohl( port_mod->config );
  uint32_t mask = ntohl( port_mod->mask );
  uint32_t advertise = ntohl( port_mod->advertise );

  debug( "A port modification is received ( transaction_id = %#x, port_no = %u, "
         "hw_addr = %02x:%02x:%02x:%02x:%02x:%02x, config = %#x, mask = %#x, advertise = %#x ).",
         transaction_id, port_no,
         hw_addr[ 0 ], hw_addr[ 1 ], hw_addr[ 2 ], hw_addr[ 3 ], hw_addr[ 4 ], hw_addr[ 5 ],
         config, mask, advertise );

  if ( event_handlers.port_mod_callback == NULL ) {
    debug( "Callback function for port modification events is not set." );
    return;
  }

  debug( "Calling port modification handler ( callback = %p, user_data = %p ).",
         event_handlers.port_mod_callback,
         event_handlers.port_mod_user_data );

  event_handlers.port_mod_callback( transaction_id, port_no, hw_addr, config, mask, advertise,
                                    event_handlers.port_mod_user_data );

}


static void
handle_stats_request( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_stats_request *stats_request = data->data;

  uint32_t transaction_id = ntohl( stats_request->header.xid );
  uint16_t type = ntohs( stats_request->type );
  uint16_t flags = ntohs( stats_request->flags );

  size_t body_length = ntohs( stats_request->header.length ) - offsetof( struct ofp_stats_request, body );
  buffer *body = NULL;
  if ( body_length > 0 ) {
    body = alloc_buffer_with_length( body_length );
    void *p = append_back_buffer( body, body_length );
    memcpy( p, stats_request->body, body_length );

    switch ( type ) {
    case OFPST_FLOW:
    {
      struct ofp_flow_stats_request *flow = p;
      ntoh_match( &flow->match, &flow->match );
      flow->out_port = ntohs( flow->out_port );
    }
    break;

    case OFPST_AGGREGATE:
    {
      struct ofp_aggregate_stats_request *aggregate = p;
      ntoh_match( &aggregate->match, &aggregate->match );
      aggregate->out_port = ntohs( aggregate->out_port );
    }
    break;

    case OFPST_PORT:
    {
      struct ofp_port_stats_request *port = p;
      port->port_no = ntohs( port->port_no );
    }
    break;

    case OFPST_VENDOR:
    {
      uint32_t *vendor_id = p;
      *vendor_id = ntohl( *vendor_id );
    }
    break;

    default:
    break;
    }
  }

  debug( "A stats request is received ( transaction_id = %#x, type = %#x, flags = %#x ).",
         transaction_id, type, flags );

  if ( event_handlers.stats_request_callback == NULL ) {
    debug( "Callback function for stats request events is not set." );
    return;
  }

  debug( "Calling stats request handler ( callback = %p, user_data = %p ).",
         event_handlers.stats_request_callback,
         event_handlers.stats_request_user_data );

  event_handlers.stats_request_callback( transaction_id, type, flags, body,
                                         event_handlers.stats_request_user_data );

  if ( body_length > 0 && body != NULL ) {
    free_buffer( body );
  }
}


static void
handle_barrier_request( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_header *header = data->data;

  uint32_t transaction_id = ntohl( header->xid );

  debug( "A barrier request is received ( transaction_id = %#x ).", transaction_id );

  if ( event_handlers.barrier_request_callback == NULL ) {
    debug( "Callback function for barrier request events is not set." );
    return;
  }

  debug( "Calling barrier request handler ( callback = %p, user_data = %p ).",
         event_handlers.barrier_request_callback,
         event_handlers.barrier_request_user_data );

  event_handlers.barrier_request_callback( transaction_id,
                                           event_handlers.barrier_request_user_data );
}


static void
handle_queue_get_config_request( buffer *data ) {
  assert( empty( data ) == false );

  struct ofp_queue_get_config_request *queue_get_config_request = data->data;

  uint32_t transaction_id = ntohl( queue_get_config_request->header.xid );
  uint16_t port = ntohs( queue_get_config_request->port );

  debug( "A queue get config request is received ( transaction_id = %#x, port = %u ).",
         transaction_id, port );

  if ( event_handlers.queue_get_config_request_callback == NULL ) {
    debug( "Callback function for queue get config request events is not set." );
    return;
  }

  debug( "Calling queue get config request handler ( callback = %p, user_data = %p ).",
         event_handlers.queue_get_config_request_callback,
         event_handlers.queue_get_config_request_user_data );

  event_handlers.queue_get_config_request_callback( transaction_id, port,
                                                    event_handlers.queue_get_config_request_user_data );
}


static void
handle_controller_connected() {
  if ( event_handlers.controller_connected_callback == NULL ) {
    debug( "Callback function for controller connected events is not set." );
    return;
  }

  event_handlers.controller_connected_callback( event_handlers.controller_connected_user_data );
}


static void
handle_controller_disconnected() {
  if ( event_handlers.controller_disconnected_callback == NULL ) {
    debug( "Callback function for controller disconnected events is not set." );
    return;
  }

  event_handlers.controller_disconnected_callback( event_handlers.controller_disconnected_user_data );
}


static bool
handle_openflow_message( buffer *message ) {
  debug( "An OpenFlow message is received from remote." );

  assert( message != NULL );
  assert( message->length >= sizeof( struct ofp_header ) );

  int ret = validate_openflow_message( message );
  if ( ret < 0 ) {
    error( "Failed to validate an OpenFlow message ( code = %d, length = %zu ).", ret, message->length );
    return false;
  }

  ret = true;
  struct ofp_header *header = ( struct ofp_header * ) message->data;

  switch ( header->type ) {
  case OFPT_HELLO:
    handle_hello( message );
    break;
  case OFPT_ERROR:
    handle_error( message );
    break;
  case OFPT_ECHO_REQUEST:
    handle_echo_request( message );
    break;
  case OFPT_ECHO_REPLY:
    handle_echo_reply( message );
    break;
  case OFPT_VENDOR:
    handle_vendor( message);
    break;
  case OFPT_FEATURES_REQUEST:
    handle_features_request( message );
    break;
  case OFPT_GET_CONFIG_REQUEST:
    handle_get_config_request( message );
    break;
  case OFPT_SET_CONFIG:
    handle_set_config( message );
    break;
  case OFPT_PACKET_OUT:
    handle_packet_out( message );
    break;
  case OFPT_FLOW_MOD:
    handle_flow_mod( message );
    break;
  case OFPT_PORT_MOD:
    handle_port_mod( message );
    break;
  case OFPT_STATS_REQUEST:
    handle_stats_request( message );
    break;
  case OFPT_BARRIER_REQUEST:
    handle_barrier_request( message );
    break;
  case OFPT_QUEUE_GET_CONFIG_REQUEST:
    handle_queue_get_config_request( message );
    break;
  default:
    error( "Unhandled OpenFlow message ( type = %#x ).", header->type );
    ret = false;
    break;
  }

  return ret;
}


static bool
send_openflow_message_to_secure_channel( buffer *message, void *user_data ) {
  assert( user_data == NULL );

  return send_message_to_secure_channel( message );
}


static bool
send_openflow_message_to_local( buffer *message, void *user_data ) {
  char *service_name = user_data;
  size_t service_name_length = strlen( service_name ) + 1;
  size_t service_header_length = sizeof( openflow_service_header_t ) + service_name_length;
  openflow_service_header_t *service_header = append_front_buffer( message, service_header_length );
  service_header->service_name_length = htons( ( uint16_t ) service_name_length );
  memcpy( ( char * ) service_header + sizeof( openflow_service_header_t ), service_name, service_name_length );

  return send_message( service_name, MESSENGER_OPENFLOW_MESSAGE, message->data, message->length );
}


bool
switch_send_openflow_message( buffer *message ) {
  assert( message != NULL );
  assert( message->length >= sizeof( struct ofp_header ) );

  struct ofp_header *header = message->data;
  uint32_t transaction_id = ntohl( header->xid );
  openflow_context *context = lookup_context( transaction_id );
  if ( context != NULL ) {
    assert( context->send_callback != NULL );
    return context->send_callback( message, context->user_data );
  }

  return send_openflow_message_to_secure_channel( message, NULL );
}


bool
handle_secure_channel_message( buffer *message ) {
  debug( "An OpenFlow message is received from remote." );

  assert( message != NULL );
  assert( message->length >= sizeof( struct ofp_header ) );

  struct ofp_header *header = message->data;

  save_context( ntohl( header->xid ), message, send_openflow_message_to_secure_channel, NULL );

  return handle_openflow_message( message );
}


static void
handle_local_message( uint16_t tag, void *data, size_t length ) {
  assert( data != NULL );
  assert( length >= sizeof( openflow_service_header_t ) );

  debug( "A message is received from remote ( tag = %#x, data = %p, length = %zu ).", tag, data, length );

  switch ( tag ) {
  case MESSENGER_OPENFLOW_MESSAGE:
  {
    openflow_service_header_t *header = data;
    uint16_t service_name_length = ntohs( header->service_name_length );
    size_t ofp_offset = sizeof( openflow_service_header_t ) + service_name_length;
    size_t ofp_length = length - ofp_offset;
    buffer *message = alloc_buffer_with_length( ofp_length );
    char *p = append_back_buffer( message, ofp_length );
    memcpy( p, ( char * ) data + ofp_offset, ofp_length );
    char *service_name = strndup( ( char * ) data + sizeof( openflow_service_header_t ), service_name_length );

    save_context( ntohl( ( ( struct ofp_header * ) p )->xid ), message, send_openflow_message_to_local,
                  service_name );

    handle_openflow_message( message );
  }
  break;
  default:
    break;
  }
}


bool
init_openflow_switch_interface( const uint64_t datapath_id, uint32_t controller_ip, uint16_t controller_port ) {
  debug( "Initializing OpenFlow Switch Interface ( datapath_id = %#" PRIx64 ", controller_ip = %#x, controller_port = %u ).",
         datapath_id, controller_ip, controller_port );

  if ( openflow_switch_interface_is_initialized() ) {
    error( "OpenFlow Switch Interface is already initialized." );
    return false;
  }

  bool ret = init_secure_channel( controller_ip, controller_port,
                                  handle_controller_connected, handle_controller_disconnected );
  if ( ret == false ) {
    error( "Failed to initialize a secure chanel." );
    return false;
  }


  memset( &event_handlers, 0, sizeof( openflow_event_handlers ) );
  memset( &config, 0, sizeof( openflow_switch_config ) );

  config.datapath_id = datapath_id;
  config.controller.ip = controller_ip;
  config.controller.port = controller_port;

  init_context();

  add_periodic_event_callback( 5, age_contexts, NULL );
  add_message_received_callback( get_chibach_name(), handle_local_message );

  openflow_switch_interface_initialized = true;

  return true;
}


bool
finalize_openflow_switch_interface() {
  if ( !openflow_switch_interface_is_initialized() ) {
    error( "OpenFlow Switch Interface is not initialized." );
    return false;
  }

  finalize_secure_channel();

  delete_timer_event( age_contexts, NULL );
  finalize_context();

  openflow_switch_interface_initialized = false;

  return true;
}


static const buffer *
get_openflow_message( uint32_t transaction_id ) {
  openflow_context *context = lookup_context( transaction_id );
  if ( context == NULL ) {
    return NULL;
  }

  return context->message;
}


bool
send_error_message( uint32_t transaction_id, uint16_t type, uint16_t code ) {
  buffer *data = NULL;
  switch ( type ) {
  case OFPET_HELLO_FAILED:
  {
    switch ( code ) {
    case OFPHFC_INCOMPATIBLE:
    {
      const char *description = "Incompatible OpenFlow version.";
      size_t length = strlen( description ) + 1;
      data = alloc_buffer_with_length ( length );
      void *p = append_back_buffer( data, length );
      strncpy( p, description, length );
    }
    break;
    case OFPHFC_EPERM:
    {
      const char *description = "Permissions error.";
      size_t length = strlen( description ) + 1;
      data = alloc_buffer_with_length ( length );
      void *p = append_back_buffer( data, length );
      strncpy( p, description, length );
    }
    break;
    default:
      error( "Undefined error code ( type = %#x, code = %#x ).", type, code );
      return false;
    }
  }
  break;

  case OFPET_BAD_REQUEST:
  {
    switch ( code ) {
    case OFPBRC_BAD_VERSION:
    case OFPBRC_BAD_TYPE:
    case OFPBRC_BAD_STAT:
    case OFPBRC_BAD_VENDOR:
    case OFPBRC_BAD_SUBTYPE:
    case OFPBRC_EPERM:
    case OFPBRC_BAD_LEN:
    case OFPBRC_BUFFER_EMPTY:
    case OFPBRC_BUFFER_UNKNOWN:
    {
      const buffer *original_message = get_openflow_message( transaction_id );
      if ( original_message != NULL ) {
        data = duplicate_buffer( original_message );
        if ( data->length > 64 ) {
          data->length = 64;
        }
      }
    }
    break;
    default:
      error( "Undefined error code ( type = %#x, code = %#x ).", type, code );
      return false;
    }
  }
  break;

  case OFPET_BAD_ACTION:
  {
    switch ( code ) {
    case OFPBAC_BAD_TYPE:
    case OFPBAC_BAD_LEN:
    case OFPBAC_BAD_VENDOR:
    case OFPBAC_BAD_VENDOR_TYPE:
    case OFPBAC_BAD_OUT_PORT:
    case OFPBAC_BAD_ARGUMENT:
    case OFPBAC_EPERM:
    case OFPBAC_TOO_MANY:
    case OFPBAC_BAD_QUEUE:
    {
      const buffer *original_message = get_openflow_message( transaction_id );
      if ( original_message != NULL ) {
        data = duplicate_buffer( original_message );
        if ( data->length > 64 ) {
          data->length = 64;
        }
      }
    }
    break;
    default:
      error( "Undefined error code ( type = %#x, code = %#x ).", type, code );
      return false;
    }

  }
  break;

  case OFPET_FLOW_MOD_FAILED:
  {
    switch ( code ) {
    case OFPFMFC_ALL_TABLES_FULL:
    case OFPFMFC_OVERLAP:
    case OFPFMFC_EPERM:
    case OFPFMFC_BAD_EMERG_TIMEOUT:
    case OFPFMFC_BAD_COMMAND:
    case OFPFMFC_UNSUPPORTED:
    {
      const buffer *original_message = get_openflow_message( transaction_id );
      if ( original_message != NULL ) {
        data = duplicate_buffer( original_message );
        if ( data->length > 64 ) {
          data->length = 64;
        }
      }
    }
    break;
    default:
      error( "Undefined error code ( type = %#x, code = %#x ).", type, code );
      return false;
    }
  }
  break;

  case OFPET_PORT_MOD_FAILED:
  {
    switch ( code ) {
    case OFPPMFC_BAD_PORT:
    case OFPPMFC_BAD_HW_ADDR:
    {
      const buffer *original_message = get_openflow_message( transaction_id );
      if ( original_message != NULL ) {
        data = duplicate_buffer( original_message );
        if ( data->length > 64 ) {
          data->length = 64;
        }
      }
    }
    break;
    default:
      error( "Undefined error code ( type = %#x, code = %#x ).", type, code );
      return false;
    }
  }
  break;

  case OFPET_QUEUE_OP_FAILED:
  {
    switch ( code ) {
    case OFPQOFC_BAD_PORT:
    case OFPQOFC_BAD_QUEUE:
    case OFPQOFC_EPERM:
    {
      const buffer *original_message = get_openflow_message( transaction_id );
      if ( original_message != NULL ) {
        data = duplicate_buffer( original_message );
        if ( data->length > 64 ) {
          data->length = 64;
        }
      }
    }
    break;
    default:
      error( "Undefined error code ( type = %#x, code = %#x ).", type, code );
      return false;
    }
  }
  break;

  default:
    error( "Undefined error type ( type = %#x, code = %#x ).", type, code );
    return false;
  }

  buffer *err = create_error( transaction_id, type, code, data );
  bool ret = switch_send_openflow_message( err );
  if ( !ret ) {
    error( "Failed to send an error message ( transaction_id = %#x, type = %#x, code = %#x ).",
           transaction_id, type, code );
  }

  free_buffer( err );
  if ( data != NULL ) {
    free_buffer( data );
  }

  return ret;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
