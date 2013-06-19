/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include <inttypes.h>
#include <netinet/in.h>
#include <assert.h>
#include "trema.h"
#include "service_management.h"
#include "subscriber_table.h"
#include "discovery_management.h"

static service_management_options options;

static link_status_updated_hook g_link_status_updated_hook = NULL;
static void *g_link_status_updated_hook_param = NULL;

static port_status_updated_hook g_port_status_updated_hook = NULL;
static void *g_port_status_updated_hook_param = NULL;

static switch_status_updated_hook g_switch_status_updated_hook = NULL;
static void *g_switch_status_updated_hook_param = NULL;


static char topology_messenger_name[ MESSENGER_SERVICE_NAME_LENGTH ] = {};


bool
_set_switch_status_updated_hook( switch_status_updated_hook callback, void *param ) {
  g_switch_status_updated_hook = callback;
  g_switch_status_updated_hook_param = param;
  return true;
}
bool ( *set_switch_status_updated_hook )( switch_status_updated_hook, void *user_data ) = _set_switch_status_updated_hook;


bool
_set_port_status_updated_hook( port_status_updated_hook callback, void *param ) {
  g_port_status_updated_hook = callback;
  g_port_status_updated_hook_param = param;

  return true;
}
bool ( *set_port_status_updated_hook )( port_status_updated_hook, void *user_data ) = _set_port_status_updated_hook;


bool
_set_link_status_updated_hook( link_status_updated_hook callback, void *param ) {
  g_link_status_updated_hook = callback;
  g_link_status_updated_hook_param = param;
  return true;
}
bool ( *set_link_status_updated_hook )( link_status_updated_hook, void *user_data ) = _set_link_status_updated_hook;


static buffer *
create_topology_response_message( uint8_t status ) {
  buffer *buf = alloc_buffer_with_length( sizeof( topology_response ) );
  topology_response *response = append_back_buffer( buf, sizeof( topology_response ) );
  response->status = status;

  return buf;
}


#define LINK_STATUS_BUFFER_SIZE 2048

static buffer *
create_link_status_message( void ) {
  return alloc_buffer_with_length( LINK_STATUS_BUFFER_SIZE );
}


static buffer *
add_link_status_message( buffer *buf, port_entry *port ) {
  topology_link_status *status = NULL;

  status = append_back_buffer( buf, sizeof( topology_link_status ) );

  status->from_dpid = htonll( port->sw->datapath_id );
  status->from_portno = htons( port->port_no );
  if ( port->link_to == NULL ) {
    status->to_dpid = 0;
    status->to_portno = 0;
    status->status = TD_LINK_DOWN;
  } else {
    status->to_dpid = htonll( port->link_to->datapath_id );
    status->to_portno = htons( port->link_to->port_no );
    if ( port->link_to->up ) {
      status->status = TD_LINK_UP;
    } else {
      status->status = TD_LINK_DOWN;
    }
  }

  return buf;
}


#define PORT_STATUS_BUFFER_SIZE 2048

static buffer *
create_port_status_message( void ) {
  return alloc_buffer_with_length( PORT_STATUS_BUFFER_SIZE );
}


static buffer *
add_port_status_message( buffer *buf, port_entry *port ) {
  topology_port_status *status = NULL;

  status = append_back_buffer( buf, sizeof( topology_port_status ) );

  status->dpid = htonll( port->sw->datapath_id );
  status->port_no = htons( port->port_no );
  memcpy( status->name, port->name, OFP_MAX_PORT_NAME_LEN );
  memcpy( status->mac, port->mac, ETH_ADDRLEN );
  if ( port->up ) {
    status->status = TD_PORT_UP;
  } else {
    status->status = TD_PORT_DOWN;
  }
  if ( port->external ) {
    status->external = TD_PORT_EXTERNAL;
  } else {
    status->external = TD_PORT_INACTIVE;
  }

  return buf;
}


#define SWITCH_STATUS_BUFFER_SIZE 2048

static buffer *
create_switch_status_message( void ) {
  return alloc_buffer_with_length( SWITCH_STATUS_BUFFER_SIZE );
}


static buffer *
add_switch_status_message( buffer *buf, sw_entry *sw ) {
  topology_switch_status *status = NULL;

  status = append_back_buffer( buf, sizeof( topology_switch_status ) );

  status->dpid = htonll( sw->datapath_id );
  status->status = (sw->up? TD_SWITCH_UP : TD_SWITCH_DOWN );

  return buf;
}


static void
subscribe( const messenger_context_handle *handle, void *data, size_t len ) {
  uint8_t status = TD_RESPONSE_OK;
  topology_request *req = data;

  if ( len == 0 ) {
    error( "Invalid topology subscribe request length(%zu)", len );
    return;
  }

  debug( "Received subscribe request from '%s'", req->name );

  subscriber_entry *entry = lookup_subscriber_entry( req->name );
  if ( entry != NULL ) {
    debug( "subscriber '%s' already subscribed", req->name );
    status = TD_RESPONSE_ALREADY_SUBSCRIBED;
  } else {
    insert_subscriber_entry( req->name );
    status = TD_RESPONSE_OK;
  }

  buffer *response = create_topology_response_message( status );
  send_reply_message( handle, TD_MSGTYPE_SUBSCRIBE_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
unsubscribe( const messenger_context_handle *handle, void *data, size_t len ) {
  uint8_t status = TD_RESPONSE_OK;
  topology_request *req = data;

  if ( len == 0 ) {
    error( "Invalid topology unsubscribe request length(%zu)", len );
    return;
  }

  debug( "Received unsubscribe request from '%s'", req->name );

  subscriber_entry *entry = lookup_subscriber_entry( req->name );
  if ( entry == NULL ) {
    status = TD_RESPONSE_NO_SUCH_SUBSCRIBER;
  } else {
    delete_subscriber_entry( entry );
    status = TD_RESPONSE_OK;
  }

  buffer *response = create_topology_response_message( status );
  send_reply_message( handle, TD_MSGTYPE_UNSUBSCRIBE_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
subscriber_has_discovery_enabled( subscriber_entry *entry, void *user_data ) {
  bool *isDiscoveryEnabled = user_data;
  if ( entry->use_discovery ) {
    *isDiscoveryEnabled = true;
  }
}


static void
enable_discovery_request( const messenger_context_handle *handle, void *data, size_t len) {
  if ( len == 0 ) {
    error( "Invalid topology discovery enable request length(%zu)", len );
    return;
  }

  topology_request *req = data;
  uint8_t status = TD_RESPONSE_OK;
  debug( "Received enable discovery request from '%s'", req->name );

  subscriber_entry *entry = lookup_subscriber_entry( req->name );
  if ( entry == NULL ) {
    notice( "'%s' was not subscribed. Subscribing", req->name );
    insert_subscriber_entry( req->name );
    entry = lookup_subscriber_entry( req->name );
  }
  assert( entry != NULL );

  // enable discovery if this is the first request.
  bool isDiscoveryEnabled = false;
  foreach_subscriber( subscriber_has_discovery_enabled, &isDiscoveryEnabled );

  entry->use_discovery = true;
  if ( !isDiscoveryEnabled ) {
    enable_discovery();
  }

  buffer *response = create_topology_response_message( status );
  send_reply_message( handle, TD_MSGTYPE_ENABLE_DISCOVERY_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
disable_discovery_request( const messenger_context_handle *handle, void *data, size_t len) {
  if ( len == 0 ) {
    error( "Invalid topology discovery disable request length(%zu)", len );
    return;
  }

  topology_request *req = data;
  uint8_t status = TD_RESPONSE_OK;
  debug( "Received disable discovery request from '%s'", req->name );

  subscriber_entry *entry = lookup_subscriber_entry( req->name );
  if ( entry == NULL ) {
    notice( "'%s' was not subscribed. Ignoring request", req->name );
    status = TD_RESPONSE_NO_SUCH_SUBSCRIBER;
  } else {
    entry->use_discovery = false;
    status = TD_RESPONSE_OK;
  }

  // disable discovery if this is the last request.
  bool isDiscoveryEnabled = false;
  foreach_subscriber( subscriber_has_discovery_enabled, &isDiscoveryEnabled );
  if ( !isDiscoveryEnabled ) {
    disable_discovery();
  }

  buffer *response = create_topology_response_message( status );
  send_reply_message( handle, TD_MSGTYPE_DISABLE_DISCOVERY_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
link_query_walker( port_entry *entry, void *user_data ) {
  buffer *reply = user_data;

  add_link_status_message( reply, entry );
}


static void
link_query( const messenger_context_handle *handle, void *data, size_t len ) {
  UNUSED( len );
  topology_request *req = data;
  debug( "Received link query request from '%s'", req->name );

  buffer *reply = create_link_status_message();
  foreach_port_entry( link_query_walker, reply );

  send_reply_message( handle, TD_MSGTYPE_QUERY_LINK_STATUS_RESPONSE,
                      reply->data, reply->length );
  free_buffer( reply );
}


static void
port_query_walker( port_entry *entry, void *user_data ) {
  buffer *reply = user_data;

  add_port_status_message( reply, entry );
}


static void
port_query( const messenger_context_handle *handle, void *data, size_t len) {
  UNUSED( len );
  topology_request *req = data;
  debug( "Received port query request from '%s'", req->name );

  buffer *reply = create_port_status_message();
  foreach_port_entry( port_query_walker, reply );

  send_reply_message( handle, TD_MSGTYPE_QUERY_PORT_STATUS_RESPONSE,
                      reply->data, reply->length );
  free_buffer( reply );
}


static void
switch_query_walker( sw_entry *entry, void *user_data ) {
  buffer *reply = user_data;

  add_switch_status_message( reply, entry );
}


static void
switch_query( const messenger_context_handle *handle, void *data, size_t len) {
  UNUSED( len );
  topology_request *req = data;
  debug( "Received switch query request from '%s'", req->name );

  buffer *reply = create_switch_status_message();
  foreach_sw_entry( switch_query_walker, reply );

  send_reply_message( handle, TD_MSGTYPE_QUERY_SWITCH_STATUS_RESPONSE,
                      reply->data, reply->length );
  free_buffer( reply );
}


static uint8_t
_set_discovered_link_status( topology_update_link_status *link_status ) {
  assert( link_status != NULL );

  sw_entry *sw = lookup_sw_entry( &link_status->from_dpid );
  if ( sw == NULL ) {
    info( "Not found datapath_id %#" PRIx64, link_status->from_dpid );
    return TD_RESPONSE_INVALID;
  }
  port_entry *port = lookup_port_entry_by_port( sw, link_status->from_portno );
  if ( port == NULL ) {
    info( "Not found port no %u. datapath_id %#" PRIx64, link_status->from_portno,
          link_status->from_dpid );
    return TD_RESPONSE_INVALID;
  }
  if ( !port->up ) {
    info( "Rejecting request to set link on port %u, which is down. datapath_id %#" PRIx64, link_status->from_portno,
          link_status->from_dpid );
    // TODO reconsider behavior for setting link on port which is down.
    return TD_RESPONSE_INVALID;
  }
  const bool link_up = ( link_status->status == TD_LINK_UP );
  bool notification_required = false;
  if ( port->link_to == NULL ) {
    notification_required = true;
  } else if ( port->link_to->up != link_up
              || port->link_to->datapath_id != link_status->to_dpid
              || port->link_to->port_no != link_status->to_portno ) {
    notification_required = true;
  }
  update_link_to( port, &( link_status->to_dpid ), link_status->to_portno,
                 link_up );
  if ( notification_required ) {
    if ( link_up ) {
      info( "Link up (%#" PRIx64 ":%u)->(%#" PRIx64 ":%u)", link_status->from_dpid, link_status->from_portno, link_status->to_dpid, link_status->to_portno );
    } else {
      info( "Link down (%#" PRIx64 ":%u)->(%#" PRIx64 ":%u)", link_status->from_dpid, link_status->from_portno, link_status->to_dpid, link_status->to_portno );
    }
    notify_link_status_for_all_user( port );
  }

  // port is up but link is down (=no LLDP received) then assume port as external.
  const bool external = ( link_status->status == TD_LINK_DOWN );
  if ( port->external != external ) {
    port->external = external;
    // Port status notification
    notify_port_status_for_all_user( port );
  }

  return TD_RESPONSE_OK;
}
uint8_t ( *set_discovered_link_status )( topology_update_link_status* link_status ) = _set_discovered_link_status;


static void
update_link_status( const messenger_context_handle *handle, void *data, size_t len) {
  topology_update_link_status *req = data;
  topology_update_link_status link_status;
  buffer *response = NULL;
  uint8_t status = TD_RESPONSE_OK;

  if ( len != sizeof( topology_update_link_status ) ) {
    error( "Invalid update link status request length(%zu)", len );
    status = TD_RESPONSE_INVALID;
    goto send_response;
  }

  link_status.from_dpid = ntohll( req->from_dpid );
  link_status.from_portno = ntohs( req->from_portno );
  link_status.to_dpid = ntohll( req->to_dpid );
  link_status.to_portno = ntohs( req->to_portno );
  link_status.status = req->status;

  info( "Link change req to status '%d' (%#" PRIx64 ":%u)->(%#" PRIx64 ":%u)", link_status.status, link_status.from_dpid, link_status.from_portno, link_status.to_dpid, link_status.to_portno );
  status = set_discovered_link_status( &link_status );

send_response:
  response = create_topology_response_message( status );
  send_reply_message( handle, TD_MSGTYPE_UPDATE_LINK_STATUS_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
recv_request( const messenger_context_handle *handle,
              uint16_t tag, void *data, size_t len ) {
  switch ( tag ) {
    case TD_MSGTYPE_SUBSCRIBE_REQUEST:
      subscribe( handle, data, len );
      break;

    case TD_MSGTYPE_UNSUBSCRIBE_REQUEST:
      unsubscribe( handle, data, len );
      break;

    case TD_MSGTYPE_QUERY_LINK_STATUS_REQUEST:
      link_query( handle, data, len );
      break;

    case TD_MSGTYPE_QUERY_PORT_STATUS_REQUEST:
      port_query( handle, data, len );
      break;

    case TD_MSGTYPE_QUERY_SWITCH_STATUS_REQUEST:
      switch_query( handle, data, len );
      break;

    case TD_MSGTYPE_UPDATE_LINK_STATUS_REQUEST:
      update_link_status( handle, data, len );
      break;

    case TD_MSGTYPE_PING_REQUEST:
      // TODO Respond to client -> service heart beat request message.
      break;

    case TD_MSGTYPE_ENABLE_DISCOVERY_REQUEST:
      enable_discovery_request( handle, data, len );
      break;

    case TD_MSGTYPE_DISABLE_DISCOVERY_REQUEST:
      disable_discovery_request( handle, data, len );
      break;

    default:
      notice( "%s: Invalid message type: %#x", __func__, (unsigned int)tag );
  }
}


static void
recv_ping_reply( uint16_t tag, void *data, size_t len, void *user_data ) {
  UNUSED( tag );
  UNUSED( user_data );

  if ( len == 0 ) {
    error( "%s: Ping reply length was too short. (len=%zu)", __func__, len );
    return;
  }

  topology_ping_response* res = data;

  debug( "Received ping reply from '%s'", res->name );

  subscriber_entry* entry = lookup_subscriber_entry( res->name );
  if ( entry == NULL ) {
    warn( "%s: Received ping reply from unsubscribed client '%s'", __func__, res->name );
    return;
  }

  entry->last_seen = time( NULL );
}


static void
recv_reply( uint16_t tag, void *data, size_t len, void *user_data ) {
  switch ( tag ) {
  case TD_MSGTYPE_PING_RESPONSE:
    recv_ping_reply( tag, data, len, user_data );
    break;

  default:
    notice( "%s: Invalid message type: %#x", __func__, (unsigned int)tag );
  }
}


static void
notify_link_status( subscriber_entry *entry, void *user_data ) {
  buffer *notify = user_data;

  send_message( entry->name, TD_MSGTYPE_LINK_STATUS_NOTIFICATION,
                notify->data, notify->length );

  debug( "notify link status to %s", entry->name );
}


static void
_notify_link_status_for_all_user( port_entry *port ) {
  if ( g_link_status_updated_hook != NULL ) {
      g_link_status_updated_hook( g_link_status_updated_hook_param, port );
  }

  buffer *notify = create_link_status_message();
  add_link_status_message( notify, port );

  foreach_subscriber( notify_link_status, notify );
  free_buffer( notify );
}
void ( *notify_link_status_for_all_user )( port_entry *port ) = _notify_link_status_for_all_user;


static void
notify_port_status( subscriber_entry *entry, void *user_data ) {
  buffer *notify = user_data;

  send_message( entry->name, TD_MSGTYPE_PORT_STATUS_NOTIFICATION,
                notify->data, notify->length );

  debug( "notify port status to %s", entry->name );
}


static void
_notify_port_status_for_all_user( port_entry *port ) {
  debug( "notify port status" );

  if ( g_port_status_updated_hook != NULL ) {
      g_port_status_updated_hook( g_port_status_updated_hook_param, port );
  }

  buffer *notify = create_port_status_message();
  add_port_status_message( notify, port );

  foreach_subscriber( notify_port_status, notify );
  free_buffer( notify );
}
void ( *notify_port_status_for_all_user )( port_entry *port ) = _notify_port_status_for_all_user;


static void
notify_switch_status( subscriber_entry *entry, void *user_data ) {
  buffer *notify = user_data;

  send_message( entry->name, TD_MSGTYPE_SWITCH_STATUS_NOTIFICATION,
                notify->data, notify->length );

  debug( "notify switch status to %s", entry->name );
}


static void
_notify_switch_status_for_all_user( sw_entry *sw ) {
  debug( "notify switch status" );

  if ( g_switch_status_updated_hook != NULL ) {
      g_switch_status_updated_hook( g_switch_status_updated_hook_param, sw );
  }

  buffer *notify = create_switch_status_message();
  add_switch_status_message( notify, sw );

  foreach_subscriber( notify_switch_status, notify );
  free_buffer( notify );
}
void ( *notify_switch_status_for_all_user )( sw_entry *sw ) = _notify_switch_status_for_all_user;


static buffer*
create_topology_request_message( const char *name ) {
  size_t req_len = strlen( name ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, name );
  return buf;
}


static void
ping_subscriber( subscriber_entry *entry, void *user_data ) {
  UNUSED( user_data );

  // ping age out
  const time_t current = time( NULL );
  if ( (current - entry->last_seen) > (options.ping_interval_sec * options.ping_ageout_cycles) ) {
    // remove aged out entry
    notice( "Aged out subscriber '%s'", entry->name );
    delete_subscriber_entry( entry );
  } else {
    // create ping message
    buffer *ping = create_topology_request_message( entry->name );

    debug( "Sending ping to '%s'", entry->name );
    bool success = send_request_message( entry->name, get_topology_messenger_name(),
                  TD_MSGTYPE_PING_REQUEST,
                  ping->data, ping->length, NULL );
    if ( !success ) {
      warn( "Failed to send ping to '%s'", entry->name );
    }
    free_buffer( ping );
  }
}


void
ping_all_subscriber( void* user_data ) {
  debug( "Sending ping to each subscribers" );
  foreach_subscriber( ping_subscriber, user_data );
}


bool
start_service_management( void ) {
  bool init_success = false;
  init_success = add_message_requested_callback( get_topology_messenger_name(), recv_request );
  if ( !init_success ) {
      error( "Failed to register message request call back." );
      return false;
  }

  init_success = add_message_replied_callback( get_topology_messenger_name(), recv_reply );
  if ( !init_success ) {
      error( "Failed to register message reply call back." );
      return false;
  }


  init_success = add_periodic_event_callback( options.ping_interval_sec, ping_all_subscriber, NULL );
  if ( !init_success ) {
      error( "Failed to register ping check timer event." );
      return false;
  }
  return true;
}


bool
init_service_management( service_management_options new_options ) {
  options = new_options;
  init_subscriber_table();
  return true;
}


void
finalize_service_management( void ) {
  finalize_subscriber_table();
}


const char*
get_topology_messenger_name( void ) {
  if ( strlen( topology_messenger_name ) > 0 ) {
    return topology_messenger_name;
  }
  const size_t server_name_len = strlen( get_trema_name() ) + strlen( ".t" ) + 1;
  if ( server_name_len > MESSENGER_SERVICE_NAME_LENGTH ) {
    die( "trema name too long to create topology service name ( %s ).", get_trema_name() );
  }
  snprintf( topology_messenger_name, server_name_len, "%s.t", get_trema_name() );

  return topology_messenger_name;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
