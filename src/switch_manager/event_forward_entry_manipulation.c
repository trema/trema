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
#include "trema.h"
#include "event_forward_entry_manipulation.h"


void
management_event_forward_entry_add( list_element **service_list,
                             const event_forward_operation_request *request, size_t request_len ) {
  if ( request->n_services == 0 ) return;
  if ( request->n_services > 1 ) {
    warn( "Only 1 service name expected for EVENT_FWD_ENTRY_ADD. Ignoring others." );
  }
  const size_t service_name_len = request_len - offsetof( event_forward_operation_request, service_list );
  if ( service_name_len == 0 ) return;

  char *service_name = xcalloc( service_name_len + 1, sizeof( char ) );
  strncpy( service_name, request->service_list, service_name_len );

  const char *match = find_list_custom( *service_list, string_equal, service_name );
  if ( match == NULL ) {
    info( "Adding '%s' to event filter.", service_name );
    append_to_tail( service_list, service_name );
  }
  else {
    // already there
    xfree( service_name );
  }
}


void
management_event_forward_entry_delete( list_element **service_list,
                                const event_forward_operation_request *request, size_t request_len ) {
  if ( request->n_services == 0 ) return;
  if ( request->n_services > 1 ) {
    warn( "Only 1 service name expected for EVENT_FWD_ENTRY_DELETE. Ignoring others." );
  }
  const size_t service_name_len = request_len - offsetof( event_forward_operation_request, service_list );
  if ( service_name_len == 0 ) return;

  char *service_name = xcalloc( service_name_len + 1, sizeof( char ) );
  strncpy( service_name, request->service_list, service_name_len );

  const char *match = find_list_custom( *service_list, string_equal, service_name );
  if ( match == NULL ) {
    // didn't exist
    xfree( service_name );
  }
  else {
    info( "Deleting '%s' from event filter.", service_name );
    bool success = delete_element( service_list, match );
    assert( success );
  }
}


void
management_event_forward_entries_set( list_element **service_list,
                                const event_forward_operation_request *request, size_t request_len ) {
  const size_t service_name_list_len = request_len - offsetof( event_forward_operation_request, service_list );

  const char **service_name_list = xcalloc( request->n_services, sizeof( char * ) );

  // split null terminated string list.
  unsigned int n_services = 0;
  const char *name_begin = request->service_list;
  for ( size_t i = 0 ; i < service_name_list_len ; ++i ) {
    if ( request->service_list[ i ] == '\0' ) {
      service_name_list[ n_services++ ] = name_begin;
      if ( n_services == request->n_services ) {
        if ( i + 1 != service_name_list_len ) {
          warn( "Expecting %d name(s) for EVENT_FWD_ENTRY_SET, but more exist. Ignoring.", request->n_services );
        }
        break;
      }
      name_begin = &request->service_list[ i + 1 ];
    }
  }
  if ( n_services != request->n_services ) {
    warn( "Expected %d name(s) for EVENT_FWD_ENTRY_SET, but found %d.", request->n_services, n_services );
  }

  info( "Resetting event filter(s)." );
  // clear current list
  iterate_list( *service_list, xfree_data, NULL );
  delete_list( *service_list );
  create_list( service_list );

  // set new list
  for ( unsigned int i = 0 ; i < n_services ; ++i ) {
    const size_t service_name_len = strlen( service_name_list[ i ] );
    if ( service_name_len == 0 ) {
      warn( "Ignoring 0 length service name in EVENT_FWD_ENTRY_SET" );
      continue;
    }
    info( " Adding '%s' to event filter.", service_name_list[ i ] );
    append_to_tail( service_list, xstrdup( service_name_list[ i ] ) );
  }

  xfree( service_name_list );
}


