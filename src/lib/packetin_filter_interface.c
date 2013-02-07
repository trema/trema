/*
 * Copyright (C) 2011 NEC Corporation
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "buffer.h"
#include "byteorder.h"
#include "log.h"
#include "packetin_filter_interface.h"
#include "trema_wrapper.h"
#include "wrapper.h"


typedef struct {
  void *callback;
  void *user_data;
} handler_data;


static bool initialized = false;
static char client_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


static void
set_client_service_name( void ) {
  snprintf( client_service_name, sizeof( client_service_name ), "packetin_filter.%u", trema_getpid() );
  client_service_name[ sizeof( client_service_name ) - 1 ] = '\0';
}


static const char *
get_client_service_name( void ) {
  return client_service_name;
}


static void
maybe_init_packetin_filter_interface( void ) {
  if ( initialized ) {
    return;
  }

  init_packetin_filter_interface();
}


static void
addition_completed( int status, handler_data *data ) {
  if ( data->callback != NULL ) {
    add_packetin_filter_handler callback = data->callback;
    callback( status, data->user_data );
  }
  xfree( data );
}


static void
deletion_completed( int status, int n_deleted, handler_data *data ) {
  if ( data->callback != NULL ) {
    delete_packetin_filter_handler callback = data->callback;
    callback( status, n_deleted, data->user_data );
  }
  xfree( data );
}


static void
ntoh_packetin_filter_entry( packetin_filter_entry *dst, packetin_filter_entry *src ) {
  ntoh_match( &dst->match, &src->match );
  dst->priority = ntohs( src->priority );
  memcpy( dst->service_name, src->service_name, sizeof( dst->service_name ) );
}


static void
dump_completed( int status, int n_entries, packetin_filter_entry *entries, handler_data *data ) {
  if ( data->callback != NULL ) {
    dump_packetin_filter_handler callback = data->callback;
    callback( status, n_entries, entries, data->user_data );
  }
  xfree( data );
}


bool
add_packetin_filter( struct ofp_match match, uint16_t priority, char *service_name,
                     add_packetin_filter_handler callback, void *user_data ) {
  if ( service_name == NULL || ( service_name != NULL && strlen( service_name ) == 0 ) ) {
    error( "Service name must be specified." );
    return false;
  }

  maybe_init_packetin_filter_interface();

  handler_data *data = xmalloc( sizeof( handler_data ) );
  data->callback = callback;
  data->user_data = user_data;

  add_packetin_filter_request request;
  memset( &request, 0, sizeof( add_packetin_filter_request ) );
  hton_match( &request.entry.match, &match );
  request.entry.priority = htons( priority );
  memcpy( request.entry.service_name, service_name, sizeof( request.entry.service_name ) );

  bool ret = send_request_message( PACKETIN_FILTER_MANAGEMENT_SERVICE,
                                   get_client_service_name(),
                                   MESSENGER_ADD_PACKETIN_FILTER_REQUEST,
                                   &request, sizeof( add_packetin_filter_request ), data );

  return ret;
}


bool
delete_packetin_filter( struct ofp_match match, uint16_t priority, char *service_name, bool strict,
                        delete_packetin_filter_handler callback, void *user_data ) {
  maybe_init_packetin_filter_interface();

  handler_data *data = xmalloc( sizeof( handler_data ) );
  data->callback = callback;
  data->user_data = user_data;

  delete_packetin_filter_request request;
  memset( &request, 0, sizeof( delete_packetin_filter_request ) );
  hton_match( &request.criteria.match, &match );
  request.criteria.priority = htons( priority );
  if ( service_name != NULL ) {
    strncpy( request.criteria.service_name, service_name, sizeof( request.criteria.service_name ) );
    request.criteria.service_name[ sizeof( request.criteria.service_name ) - 1 ] = '\0';
  }
  if ( strict ) {
    request.flags = PACKETIN_FILTER_FLAG_MATCH_STRICT;
  }
  else {
    request.flags = PACKETIN_FILTER_FLAG_MATCH_LOOSE;
  }

  bool ret = send_request_message( PACKETIN_FILTER_MANAGEMENT_SERVICE,
                                   get_client_service_name(),
                                   MESSENGER_DELETE_PACKETIN_FILTER_REQUEST,
                                   &request, sizeof( delete_packetin_filter_request ), data );

  return ret;
}


bool
dump_packetin_filter( struct ofp_match match, uint16_t priority, char *service_name, bool strict,
                      dump_packetin_filter_handler callback, void *user_data ) {
  maybe_init_packetin_filter_interface();

  handler_data *data = xmalloc( sizeof( handler_data ) );
  data->callback = callback;
  data->user_data = user_data;

  dump_packetin_filter_request request;
  memset( &request, 0, sizeof( dump_packetin_filter_request ) );
  hton_match( &request.criteria.match, &match );
  request.criteria.priority = htons( priority );
  if ( service_name != NULL ) {
    strncpy( request.criteria.service_name, service_name, sizeof( request.criteria.service_name ) );
    request.criteria.service_name[ sizeof( request.criteria.service_name ) - 1 ] = '\0';
  }
  if ( strict ) {
    request.flags = PACKETIN_FILTER_FLAG_MATCH_STRICT;
  }
  else {
    request.flags = PACKETIN_FILTER_FLAG_MATCH_LOOSE;
  }

  bool ret = send_request_message( PACKETIN_FILTER_MANAGEMENT_SERVICE,
                                   get_client_service_name(),
                                   MESSENGER_DUMP_PACKETIN_FILTER_REQUEST,
                                   &request, sizeof( dump_packetin_filter_request ), data );

  return ret;
}


static void
handle_reply( uint16_t tag, void *data, size_t length, void *user_data ) {
  switch ( tag ) {
    case MESSENGER_ADD_PACKETIN_FILTER_REPLY:
    {
      if ( length != sizeof( add_packetin_filter_reply ) ) {
        error( "Invalid add packetin filter reply ( length = %zu ).", length );
        return;
      }
      add_packetin_filter_reply *reply = data;
      addition_completed( reply->status, user_data );
    }
    break;
    case MESSENGER_DELETE_PACKETIN_FILTER_REPLY:
    {
      if ( length != sizeof( delete_packetin_filter_reply ) ) {
        error( "Invalid delete packetin filter reply ( length = %zu ).", length );
        return;
      }
      delete_packetin_filter_reply *reply = data;
      deletion_completed( reply->status, ( int ) ntohl( reply->n_deleted ), user_data );
    }
    break;
    case MESSENGER_DUMP_PACKETIN_FILTER_REPLY:
    {
      if ( length < offsetof( dump_packetin_filter_reply, entries ) ) {
        error( "Invalid dump packetin filter reply ( length = %zu ).", length );
        return;
      }
      dump_packetin_filter_reply *reply = data;
      size_t expected_length = offsetof( dump_packetin_filter_reply, entries ) + sizeof( packetin_filter_entry ) * ntohl( reply->n_entries );
      if ( length != expected_length ) {
        error( "Invalid dump packetin filter reply ( length = %zu ).", length );
        return;
      }
      packetin_filter_entry *entries = NULL;
      int n_entries = ( int ) ntohl( reply->n_entries );
      if ( n_entries > 0 && reply->entries != NULL ) {
        entries = xmalloc( sizeof( packetin_filter_entry ) * ( size_t ) n_entries );
        for ( int i = 0; i < n_entries; i++ ) {
          ntoh_packetin_filter_entry( &entries[ i ], &reply->entries[ i ] );
        }
      }
      dump_completed( reply->status, n_entries, entries, user_data );
      if ( entries != NULL ) {
        xfree( entries );
      }
    }
    break;
    default:
    {
      warn( "Undefined reply tag ( tag = %#x, length = %zu ).", tag, length );
    }
    return;
  }
}


bool
init_packetin_filter_interface( void ) {
  if ( initialized ) {
    return false;
  }

  set_client_service_name();
  add_message_replied_callback( get_client_service_name(), handle_reply );

  initialized = true;

  return true;
}


bool
finalize_packetin_filter_interface( void ) {
  if ( !initialized ) {
    return false;
  }

  delete_message_replied_callback( get_client_service_name(), handle_reply );

  initialized = false;

  return true;
}



/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
