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


#include "trema.h"


static void
timeout( void *user_data ) {
  UNUSED( user_data );

  stop_trema();
}


static void
delete_filter_completed( int status, int n_deleted, void *user_data ) {
  UNUSED( user_data );

  if ( status != PACKETIN_FILTER_OPERATION_SUCCEEDED ) {
    error( "Failed to delete packetin filters." );
  }
  info( "%d packetin filters were deleted.", n_deleted );
}


static void
dump_filters( int status, int n_entries, packetin_filter_entry *entries, void *user_data ) {
  UNUSED( user_data );

  if ( status != PACKETIN_FILTER_OPERATION_SUCCEEDED ) {
    error( "Failed to dump packetin filters." );
  }
  info( "%d packetin filters found.", n_entries );
  for ( int i = 0; i < n_entries; i++ ) {
    char match_string[ 256 ];
    match_to_string( &entries[ i ].match, match_string, sizeof( match_string ) );
    info( "match = [%s], priority = %u, service_name = %s.",
          match_string, entries[ i ].priority, entries[ i ].service_name );
  }
}


static void
dump_and_delete_filters( int status, int n_entries, packetin_filter_entry *entries, void *user_data ) {
  dump_filters( status, n_entries, entries, user_data );

  struct ofp_match *match = user_data;
  char *service_name = xstrdup( get_trema_name() );
  bool ret = delete_packetin_filter( *match, UINT16_MAX, service_name, true, delete_filter_completed, user_data );
  xfree( service_name );
  if ( ret == false ) {
    error( "Failed to delete a packetin filter." );
  }
  ret = delete_packetin_filter( *match, UINT16_MAX, NULL, false, delete_filter_completed, user_data );
  if ( ret == false ) {
    error( "Failed to delete all packetin filters." );
  }
}


static void
add_filter_completed( int status, void *user_data ) {
  if ( status != PACKETIN_FILTER_OPERATION_SUCCEEDED ) {
    error( "Failed to add a packetin filter." );
  }
  info( "A packetin filter was added successfully." );

  struct ofp_match *match = user_data;
  char *service_name = xstrdup( get_trema_name() );
  bool ret = dump_packetin_filter( *match, UINT16_MAX, service_name, true, dump_filters, user_data );
  xfree( service_name );
  if ( ret == false ) {
    error( "Failed to dump a packetin filter." );
  }
  ret = dump_packetin_filter( *match, UINT16_MAX, NULL, false, dump_and_delete_filters, user_data );
  if ( ret == false ) {
    error( "Failed to dump all packetin filters." );
  }
}


static void
add_filter( void ) {
  static struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;
  char *service_name = xstrdup( get_trema_name() );

  bool ret = add_packetin_filter( match, UINT16_MAX, service_name, add_filter_completed, &match );
  xfree( service_name );
  if ( ret == false ) {
    error( "Failed to add a packetin filter." );
  }
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  init_packetin_filter_interface();

  add_periodic_event_callback( 5, timeout, NULL );

  add_filter();

  start_trema();

  finalize_packetin_filter_interface();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
