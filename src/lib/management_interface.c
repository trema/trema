/*
 * Author: Yasunobu Chiba
 *
 * Copyright (C) 2012 NEC Corporation
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
#include <string.h>
#include "bool.h"
#include "checks.h"
#include "log.h"
#include "management_interface.h"
#include "management_service_interface.h"
#include "trema.h" // FIXME: don't include trema.h here (move get_trema_name() to trema_private.c)
#include "trema_private.h"
#include "trema_wrapper.h"
#include "wrapper.h"


static bool initialized = false;


static void
handle_echo_request( const messenger_context_handle *handle, management_echo_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );
  assert( ntohs( request->header.type ) == MANAGEMENT_ECHO_REQUEST );

  debug( "Handling an echo request from %s ( sent_at = %u.%09u ).",
         handle->service_name, ntohl( request->sent_at.tv_sec ), ntohl( request->sent_at.tv_nsec ) );

  bool ret = true;
  management_echo_reply reply;
  memset( &reply, 0, sizeof( management_echo_reply ) );
  reply.header.type = htons( MANAGEMENT_ECHO_REPLY );
  reply.sent_at = request->sent_at;

  struct timespec now = { 0, 0 };
  int retval = clock_gettime( CLOCK_REALTIME, &now );
  if ( retval < 0 ) {
    error( "Failed to retrieve clock." );
    ret = false;
  }
  reply.received_at.tv_sec = htonl( ( uint32_t ) now.tv_sec );
  reply.received_at.tv_nsec = htonl( ( uint32_t ) now.tv_nsec );
  reply.header.status = ( uint8_t ) ( ret == true ? MANAGEMENT_REQUEST_SUCCEEDED : MANAGEMENT_REQUEST_FAILED );

  ret = send_reply_message( handle, MESSENGER_MANAGEMENT_REPLY, &reply, sizeof( management_echo_reply ) );
  if ( ret == false ) {
    error( "Failed to send an echo reply." );
  }
}


static void
handle_set_logging_level_request( const messenger_context_handle *handle, management_set_logging_level_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );
  assert( ntohs( request->header.type ) == MANAGEMENT_SET_LOGGING_LEVEL_REQUEST );

  request->level[ LOGGING_LEVEL_STR_LENGTH - 1 ] = '\0';

  debug( "Handling a set logging level request from %s ( level = %s ).", handle->service_name, request->level );

  management_set_logging_level_reply reply;
  memset( &reply, 0, sizeof( management_set_logging_level_reply ) );
  reply.header.type = htons( MANAGEMENT_SET_LOGGING_LEVEL_REPLY );

  bool ret = valid_logging_level( request->level );
  if ( ret ) {
    ret = set_logging_level( request->level );
  }

  if ( ret ) {
    reply.header.status = MANAGEMENT_REQUEST_SUCCEEDED;
  }
  else {
    reply.header.status = MANAGEMENT_REQUEST_FAILED;
  }

  ret = send_reply_message( handle, MESSENGER_MANAGEMENT_REPLY, &reply, sizeof( management_set_logging_level_reply ) );
  if ( ret == false ) {
    error( "Failed to send a set logging level reply." );
  }
}


static void
handle_management_request( const messenger_context_handle *handle, void *data, size_t length ) {
  assert( handle != NULL );
  assert( data != NULL );
  assert( length >= sizeof( management_request_header ) );

  management_request_header *header = data;

  switch ( ntohs( header->type ) ) {
    case MANAGEMENT_ECHO_REQUEST:
    {
      if ( length != sizeof( management_echo_request ) ) {
        error( "Invalid echo request ( length = %u ).", length );
        return;
      }

      handle_echo_request( handle, data );
    }
    break;

    case MANAGEMENT_SET_LOGGING_LEVEL_REQUEST:
    {
      if ( length != sizeof( management_set_logging_level_request ) ) {
        error( "Invalid set logging level request ( length = %u ).", length );
        return;
      }

      handle_set_logging_level_request( handle, data );
    }
    break;

    case MANAGEMENT_APPLICATION_REQUEST:
    {
      warn( "Application specific management request is not implemented yet." );
    }
    break;

    default:
    {
      error( "Undefined management request type ( type = %#x ).", header->type );
    }
    break;
  }
}


static void
handle_request( const messenger_context_handle *handle, uint16_t tag, void *data, size_t length ) {
  assert( handle != NULL );

  debug( "Handling a request ( handle = %p, tag = %#x, data = %p, length = %u ).",
         handle, tag, data, length );

  switch ( tag ) {
    case MESSENGER_MANAGEMENT_REQUEST:
    {
      if ( length < sizeof( management_request_header ) ) {
        error( "Invalid management request. Too short message ( length = %u ).", length );
        return;
      }

      handle_management_request( handle, data, length );
    }
    break;

    default:
    {
      warn( "Undefined request tag ( tag = %#x ).", tag );
    }
    break;
  }
}


bool
init_management_interface() {
  if ( initialized ) {
    error( "Management interface is already initialized." );
    return false;
  }

  add_message_requested_callback( get_management_service_name( get_trema_name() ), handle_request );

  initialized = true;

  debug( "Management interface is initialized ( trema_name = %s, service_name = %s ).",
         get_trema_name(), get_management_service_name( get_trema_name() ) );

  return true;
}


bool
finalize_management_interface() {
  if ( !initialized ) {
    error( "Management interface is not initialized yet or already finalized." );
    return false;
  }

  delete_message_requested_callback( get_management_service_name( get_trema_name() ), handle_request );

  initialized = false;

  debug( "Management interface is finalized ( trema_name = %s ).", get_trema_name() );

  return true;
}


bool *
_get_management_interface_initialized() {
  return &initialized;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
