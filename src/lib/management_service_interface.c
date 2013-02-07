/*
 * Copyright (C) 2013 NEC Corporation
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
#include <string.h>
#include "bool.h"
#include "checks.h"
#include "log.h"
#include "management_service_interface.h"
#include "messenger.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "utility.h"
#include "wrapper.h"


static char management_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


static const char *
_get_management_service_name( const char *service_name ) {
  if ( service_name == NULL ) {
    die( "Service name must not be NULL." );
  }
  if ( strlen( service_name ) == 0 ) {
    die( "Service name length must not be zero." );
  }
  if ( strlen( service_name ) >= ( MESSENGER_SERVICE_NAME_LENGTH - 2 ) ) {
    die( "Too long service name ( %s ).", service_name );
  }

  memset( management_service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  snprintf( management_service_name, MESSENGER_SERVICE_NAME_LENGTH, "%s.m", service_name );

  return management_service_name;
}
const char *( *get_management_service_name )( const char *service_name ) = _get_management_service_name;


management_application_reply *
_create_management_application_reply( uint8_t status, uint32_t application_id, void *data, size_t data_length ) {
  size_t length = offsetof( management_application_reply, data );
  if ( data != NULL ) {
    length += data_length;
  }
  management_application_reply *reply = xmalloc( length );
  memset( reply, 0, length );
  reply->header.type = htons( MANAGEMENT_APPLICATION_REPLY );
  reply->header.status = status;
  reply->header.length = htonl( ( uint32_t ) length );
  reply->application_id = htonl( application_id );
  if ( data != NULL ) {
    memcpy( reply->data, data, data_length );
  }

  return reply;
}
management_application_reply *( *create_management_application_reply )( uint8_t status, uint32_t application_id, void *data, size_t data_length ) = _create_management_application_reply;


bool
_send_management_application_reply( const messenger_context_handle *handle, const management_application_reply *reply ) {
  if ( handle == NULL || reply == NULL ) {
    error( "Both context handle and reply message must not be NULL ( handle = %p, reply = %p ).", handle, reply );
    return false;
  }
  bool ret = send_reply_message( handle, MESSENGER_MANAGEMENT_REPLY, reply, ( size_t ) ntohl( reply->header.length ) );
  if ( !ret ) {
    error( "Failed to send an application specific management reply." );
    return false;
  }

  return true;
}
bool ( *send_management_application_reply )( const messenger_context_handle *handle, const management_application_reply *reply ) = _send_management_application_reply;


void ( *set_management_application_request_handler )( management_application_request_handler callback, void *user_data ) = _set_management_application_request_handler;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
