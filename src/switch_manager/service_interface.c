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


#include <inttypes.h>
#include <string.h>
#include "ofpmsg_send.h"
#include "openflow_service_interface.h"
#include "service_interface.h"
#include "trema.h"


static buffer *
create_openflow_application_message( uint64_t *datapath_id, buffer *data ) {
  openflow_service_header_t *message;
  buffer *buf;
  void *append;
  size_t append_len = 0;

  if ( data != NULL ) {
     append_len = data->length;
  }
  buf = alloc_buffer_with_length( sizeof( openflow_service_header_t ) + append_len );
  message = append_back_buffer( buf, sizeof( openflow_service_header_t ) );
  if ( datapath_id == NULL ) {
    message->datapath_id = ~0U; // FIXME: defined invalid datapath_id
  }
  else {
    message->datapath_id = htonll( *datapath_id );
  }
  message->service_name_length = htons( 0 );
  // TODO: append ipaddress and port
  if ( append_len > 0 ) {
    append = append_back_buffer( buf, append_len );
    memcpy( append, data->data, append_len );
  }

  return buf;
}


void
service_send_to_reply( char *service_name, uint16_t message_type, uint64_t *datapath_id, buffer *data ) {
  buffer *buf;

  if ( service_name == NULL ) {
    return;
  }

  buf = create_openflow_application_message( datapath_id, data );
  if ( !send_message( service_name, message_type, buf->data, buf->length ) ) {
    error( "Failed to send to reply ( service_name = %s ).", service_name );
  }
  free_buffer( buf );
}


void
service_send_to_application( list_element *service_name_list, uint16_t message_type, uint64_t *datapath_id, buffer *data ) {
  buffer *buf;
  list_element *list;
  char *service_name;

  if ( service_name_list == NULL ) {
    return;
  }

  buf = create_openflow_application_message( datapath_id, data );

  static const char *error_service_name = NULL;
  for ( list = service_name_list; list != NULL; list = list->next ) {
    service_name = list->data;
    if ( !send_message( service_name, message_type,
                        buf->data, buf->length ) ) {
      if ( error_service_name != service_name ) {
        warn( "Failed to send message ( service_name = %s ).", service_name );
      }
      error_service_name = service_name;
    }
    else {
      error_service_name = NULL;
    }
  }
  free_buffer( buf );
}


static void
handle_openflow_message( uint64_t *datapath_id, char *service_name, buffer *buf ) {
  struct ofp_header *header;
  int ret;

  ret = validate_openflow_message( buf );
  if ( ret != 0 ) {
    header = buf->data;
    notice( "Validation error. dpid = %#" PRIx64 ", type %u, errno %d, service_name = %s",
            *datapath_id, header->type, ret, service_name );
    free_buffer( buf );

    return;
  }

  switch_event_recv_from_application( datapath_id, service_name, buf );
}


static void
handle_openflow_disconnect_request( uint64_t *datapath_id ) {
  switch_event_disconnect_request( datapath_id );
}


void
service_recv_from_application( uint16_t message_type, buffer *buf ) {
   openflow_service_header_t *message;
   uint64_t datapath_id;
   uint16_t service_name_length;
   char *service_name;

  if ( buf->length < sizeof( openflow_service_header_t ) ) {
    error( "Too short openflow application message(%zu).", buf->length );
    free_buffer( buf );

    return;
  }

  message = buf->data;
  datapath_id = ntohll( message->datapath_id );
  service_name_length = ntohs( message->service_name_length );
  service_name = remove_front_buffer( buf, sizeof( openflow_service_header_t ) );
  if ( service_name_length < 1 ) {
    error( "Invalid service name length %u.", service_name_length );
    free_buffer( buf );

    return;
  }
  if ( service_name[ service_name_length - 1 ] != '\0' ) {
    error( "Service name is not null terminated." );
    free_buffer( buf );

    return;
  }
  remove_front_buffer( buf, service_name_length );

  switch ( message_type ) {
  case MESSENGER_OPENFLOW_MESSAGE:
    handle_openflow_message( &datapath_id, service_name, buf );
    break;
  case MESSENGER_OPENFLOW_DISCONNECT_REQUEST:
    free_buffer( buf );
    handle_openflow_disconnect_request( &datapath_id );
    break;
  default:
    error( "Unknown message type %d.", message_type );
    free_buffer( buf );
    break;
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
