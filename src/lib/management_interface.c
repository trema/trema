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
static management_application_request_handler application_callback = NULL;
static void *application_user_data = NULL;


static void
handle_echo_request( const messenger_context_handle *handle, management_echo_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );
  assert( ntohs( request->header.type ) == MANAGEMENT_ECHO_REQUEST );
  assert( ntohl( request->header.length ) == sizeof( management_echo_request ) );

  debug( "Handling an echo request from %s ( sent_at = %u.%09u ).",
         handle->service_name, ntohl( request->sent_at.tv_sec ), ntohl( request->sent_at.tv_nsec ) );

  bool ret = true;
  management_echo_reply reply;
  memset( &reply, 0, sizeof( management_echo_reply ) );
  reply.header.type = htons( MANAGEMENT_ECHO_REPLY );
  reply.header.length = htonl( sizeof( management_echo_reply ) );
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
  assert( ntohl( request->header.length ) == sizeof( management_set_logging_level_request ) );

  request->level[ LOGGING_LEVEL_STR_LENGTH - 1 ] = '\0';

  debug( "Handling a set logging level request from %s ( level = %s ).", handle->service_name, request->level );

  management_set_logging_level_reply reply;
  memset( &reply, 0, sizeof( management_set_logging_level_reply ) );
  reply.header.type = htons( MANAGEMENT_SET_LOGGING_LEVEL_REPLY );
  reply.header.length = htonl( sizeof( management_set_logging_level_reply ) );

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
append_stat( const char *key, const uint64_t value, void *user_data ) {
  assert( key != NULL );
  assert( user_data != NULL );

  list_element **stats = user_data;

  stat_entry *entry = xmalloc( sizeof( stat_entry ) );
  memset( entry, 0, sizeof( stat_entry ) );
  memcpy( entry->key, key, sizeof( entry->key ) );
  entry->value = htonll( value );

  append_to_tail( stats, entry );
}


static void
handle_show_stats_request( const messenger_context_handle *handle, management_show_stats_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );
  assert( ntohs( request->header.type ) == MANAGEMENT_SHOW_STATS_REQUEST );
  assert( ntohl( request->header.length ) == sizeof( management_show_stats_request ) );

  debug( "Handling a show stats request from %s.", handle->service_name );

  list_element *stats = NULL;
  create_list( &stats );
  foreach_stat( append_stat, &stats );
  unsigned int n_stats = list_length_of( stats );

  const size_t send_queue_length = 400000; // MESSENGER_RECV_BUFFER * 4
  const size_t send_queue_margin = 256; // headroom for various headers
  unsigned int n_max_entries = ( unsigned int ) ( send_queue_length - send_queue_margin ) / sizeof( stat_entry );

  management_show_stats_reply *reply = NULL;
  size_t length = 0;

  if ( n_stats <= n_max_entries ) {
    length = offsetof( management_show_stats_reply, entries ) + sizeof( stat_entry ) * n_stats;
    reply = xmalloc( length );
    memset( reply, 0, length );
    reply->header.type = htons( MANAGEMENT_SHOW_STATS_REPLY );
    reply->header.length = htonl( ( uint32_t ) length );
    reply->header.status = MANAGEMENT_REQUEST_SUCCEEDED;

    stat_entry *p = reply->entries;
    for ( list_element *e = stats; e != NULL; e = e->next ) {
      assert( e->data != NULL );
      memcpy( p, e->data, sizeof( stat_entry ) );
      p++;
    }
  }
  else {
    // TODO: Send statistics via out-of-band channel or by multiple replies.

    error( "Too many statistic entries ( %u > %u ).", n_stats, n_max_entries );

    length = offsetof( management_show_stats_reply, entries );
    reply = xmalloc( length );
    memset( reply, 0, length );
    reply->header.type = htons( MANAGEMENT_SHOW_STATS_REPLY );
    reply->header.length = htonl( ( uint32_t ) length );
    reply->header.status = MANAGEMENT_REQUEST_FAILED;
  }

  if ( stats != NULL ) {
    for ( list_element *e = stats; e != NULL; e = e->next ) {
      assert( e->data != NULL );
      xfree( e->data );
    }
    delete_list( stats );
  }

  bool ret = send_reply_message( handle, MESSENGER_MANAGEMENT_REPLY, reply, length );
  if ( ret == false ) {
    error( "Failed to send a show stats reply." );
  }
  xfree( reply );
}


void
_set_management_application_request_handler( management_application_request_handler callback, void *user_data ) {
  application_callback = callback;
  application_user_data = user_data;
}


static void
handle_application_request( const messenger_context_handle *handle, management_application_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );
  assert( ntohs( request->header.type ) == MANAGEMENT_APPLICATION_REQUEST );
  assert( ntohl( request->header.length ) >= offsetof( management_application_request, data ) );

  debug( "Handling an application specific management request from %s ( application_id = %#x ).",
         handle->service_name, request->application_id );

  if ( application_callback != NULL ) {
    size_t length = ( size_t ) ntohl( request->header.length ) - offsetof( management_application_request, data );
    void *data = NULL;
    if ( length > 0 ) {
      data = request->data;
    }
    application_callback( handle, ntohl( request->application_id ), data, length, application_user_data );
  }
  else {
    management_application_reply *reply = create_management_application_reply( MANAGEMENT_REQUEST_FAILED,
                                                                               ntohl( request->application_id ), NULL, 0 );
    send_management_application_reply( handle, reply );
    xfree( reply );
  }
}


static void
handle_management_request( const messenger_context_handle *handle, void *data, size_t length ) {
  assert( handle != NULL );
  assert( data != NULL );
  assert( length >= sizeof( management_request_header ) );

  management_request_header *header = data;

  assert( length == ( size_t ) ntohl( header->length ) );

  switch ( ntohs( header->type ) ) {
    case MANAGEMENT_ECHO_REQUEST:
    {
      if ( length != sizeof( management_echo_request ) ) {
        error( "Invalid echo request ( length = %zu ).", length );
        return;
      }

      handle_echo_request( handle, data );
    }
    break;

    case MANAGEMENT_SET_LOGGING_LEVEL_REQUEST:
    {
      if ( length != sizeof( management_set_logging_level_request ) ) {
        error( "Invalid set logging level request ( length = %zu ).", length );
        return;
      }

      handle_set_logging_level_request( handle, data );
    }
    break;

    case MANAGEMENT_SHOW_STATS_REQUEST:
    {
      if ( length != sizeof( management_show_stats_request ) ) {
        error( "Invalid show stats request ( length = %zu ).", length );
        return;
      }

      handle_show_stats_request( handle, data );
    }
    break;

    case MANAGEMENT_APPLICATION_REQUEST:
    {
      if ( length < offsetof( management_application_request, data ) ) {
        error( "Invalid application specific management request ( length = %zu ).", length );
        return;
      }

      handle_application_request( handle, data );
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

  debug( "Handling a request ( handle = %p, tag = %#x, data = %p, length = %zu ).",
         handle, tag, data, length );

  switch ( tag ) {
    case MESSENGER_MANAGEMENT_REQUEST:
    {
      if ( length < sizeof( management_request_header ) ) {
        error( "Invalid management request. Too short message ( length = %zu ).", length );
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
