/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include <inttypes.h>
#include "trema.h"
#include "topology_service_interface.h"
#include "topology_table.h"
#include "subscriber_table.h"
#include "service_management.h"


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

  return buf;
}


static void
subscribe( const messenger_context_handle *handle, void *data, size_t len ) {
  uint8_t status = TD_RESPONSE_OK;
  topology_request *req = data;

  if ( len == 0 ) {
    error( "Invalid topology subscribe request length(%u)", len );
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
  send_reply_message( handle, TD_MSGTYPE_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
unsubscribe( const messenger_context_handle *handle, void *data, size_t len ) {
  uint8_t status = TD_RESPONSE_OK;
  topology_request *req = data;

  if ( len == 0 ) {
    error( "Invalid topology unsubscribe request length(%u)", len );
    return;
  }

  debug( "Received unsubscribe request from '%s'", req->name );

  subscriber_entry *entry = lookup_subscriber_entry( req->name );
  if ( entry != NULL ) {
    status = TD_RESPONSE_NO_SUCH_SUBSCRIBER;
  } else {
    delete_subscriber_entry( entry );
    status = TD_RESPONSE_OK;
  }

  buffer *response = create_topology_response_message( status );
  send_reply_message( handle, TD_MSGTYPE_RESPONSE,
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
  UNUSED( data );
  UNUSED( len );

  buffer *reply = create_link_status_message();
  foreach_port_entry( link_query_walker, reply );

  send_reply_message( handle, TD_MSGTYPE_LINK_STATUS,
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
  UNUSED( data );
  UNUSED( len );

  buffer *reply = create_port_status_message();
  foreach_port_entry( port_query_walker, reply );

  send_reply_message( handle, TD_MSGTYPE_PORT_STATUS,
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
  UNUSED( data );
  UNUSED( len );

  buffer *reply = create_switch_status_message();
  foreach_sw_entry( switch_query_walker, reply );

  send_reply_message( handle, TD_MSGTYPE_SWITCH_STATUS,
                      reply->data, reply->length );
  free_buffer( reply );
}


static void
update_link_status( const messenger_context_handle *handle, void *data, size_t len) {
  topology_update_link_status *req = data;
  topology_update_link_status link_status;
  buffer *response = NULL;

  if ( len != sizeof( topology_update_link_status ) ) {
    error( "Invalid update link status request length(%u)", len );
    goto send_response;
  }

  link_status.from_dpid = ntohll( req->from_dpid );
  link_status.from_portno = ntohs( req->from_portno );
  link_status.to_dpid = ntohll( req->to_dpid );
  link_status.to_portno = ntohs( req->to_portno );
  link_status.status = req->status;

  sw_entry *sw = lookup_sw_entry( &link_status.from_dpid );
  if ( sw == NULL ) {
    info( "Not found datapath_id %" PRIx64, link_status.from_dpid );
    goto send_response;
  }
  port_entry *port = lookup_port_entry( sw, link_status.from_portno, NULL );
  if ( port == NULL ) {
    info( "Not found port no %u. datapath_id %" PRIx64, link_status.from_portno,
          link_status.from_dpid );
    goto send_response;
  }
  if ( !port->up ) {
    info( "port %u is down. datapath_id %" PRIx64, link_status.from_portno,
          link_status.from_dpid );
    goto send_response;
  }
  bool link_up = ( link_status.status == TD_LINK_UP );
  bool notification_required = false;
  if ( port->link_to == NULL ) {
    notification_required = true;
  } else if ( port->link_to->up != link_up
              || port->link_to->datapath_id != link_status.to_dpid
              || port->link_to->port_no != link_status.to_portno ) {
    notification_required = true;
  }
  update_link_to( port, &( link_status.to_dpid ), link_status.to_portno,
                 link_up );
  if ( notification_required ) {
    notify_link_status_for_all_user( port );
  }

  bool external = ( link_status.status == TD_LINK_DOWN );
  if ( port->external != external ) {
    port->external = external;
    // Port status notification
    notify_port_status_for_all_user( port );
  }

send_response:
  response = create_topology_response_message( TD_RESPONSE_OK );
  send_reply_message( handle, TD_MSGTYPE_RESPONSE,
                      response->data, response->length );
  free_buffer( response );
}


static void
recv_request( const messenger_context_handle *handle,
              uint16_t tag, void *data, size_t len ) {
  switch ( tag ) {
    case TD_MSGTYPE_SUBSCRIBE:
      subscribe( handle, data, len );
      break;

    case TD_MSGTYPE_UNSUBSCRIBE:
      unsubscribe( handle, data, len );
      break;

    case TD_MSGTYPE_QUERY_LINK_STATUS:
      link_query( handle, data, len );
      break;

    case TD_MSGTYPE_QUERY_PORT_STATUS:
      port_query( handle, data, len );
      break;

    case TD_MSGTYPE_QUERY_SWITCH_STATUS:
      switch_query( handle, data, len );
      break;

    case TD_MSGTYPE_UPDATE_LINK_STATUS:
      update_link_status( handle, data, len );
      break;

    default:
      notice( "recv_request: Invalid message type: %d", tag );
      break;
  }
}


static void
notify_link_status( subscriber_entry *entry, void *user_data ) {
  buffer *notify = user_data;

  send_message( entry->name, TD_MSGTYPE_LINK_STATUS,
                notify->data, notify->length );

  debug( "notify link status to %s", entry->name );
}


void
notify_link_status_for_all_user( port_entry *port ) {
  buffer *notify = create_link_status_message();
  add_link_status_message( notify, port );

  foreach_subscriber( notify_link_status, notify );
  free_buffer( notify );
}


static void
notify_port_status( subscriber_entry *entry, void *user_data ) {
  buffer *notify = user_data;

  send_message( entry->name, TD_MSGTYPE_PORT_STATUS,
                notify->data, notify->length );

  debug( "notify port status to %s", entry->name );
}


void
notify_port_status_for_all_user( port_entry *port ) {
  debug( "notify port status" );

  buffer *notify = create_port_status_message();
  add_port_status_message( notify, port );

  foreach_subscriber( notify_port_status, notify );
  free_buffer( notify );
}


bool
start_service_management( void ) {
  init_subscriber_table();
  return add_message_requested_callback( get_trema_name(), recv_request );
}


void stop_service_management( void ) {
  finalize_subscriber_table();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
