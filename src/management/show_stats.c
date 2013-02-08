/*
 * Management command to show statistics.
 *
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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "trema.h"


static char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


void
usage( void ) {
  printf( "Usage: show_stats SERVICE_NAME\n" );
}


static void
print_usage_and_exit( void ) {
  usage();
  exit( EXIT_FAILURE );
}


static void
parse_arguments( int argc, char **argv ) {
  assert( argv != NULL );

  if ( argc != 2 ) {
    print_usage_and_exit();
  }

  if ( argv[ 1 ] == NULL || ( argv[ 1 ] != NULL && strlen( argv[ 1 ] ) == 0 ) ) {
    print_usage_and_exit();
  }

  memset( service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  snprintf( service_name, MESSENGER_SERVICE_NAME_LENGTH, "%s", get_management_service_name( argv[ 1 ] ) );
}


static void
timeout( void *user_data ) {
  UNUSED( user_data );

  printf( "Timeout.\n" );

  delete_timer_event( timeout, NULL );

  stop_trema();
}


static void
handle_reply( uint16_t tag, void *data, size_t length, void *user_data ) {
  UNUSED( user_data );

  assert( tag == MESSENGER_MANAGEMENT_REPLY );
  assert( data != NULL );
  assert( length >= offsetof( management_show_stats_reply, entries ) );

  management_show_stats_reply *reply = data;
  assert( ntohs( reply->header.type ) == MANAGEMENT_SHOW_STATS_REPLY );
  assert( ntohl( reply->header.length ) == length );

  if ( reply->header.status == MANAGEMENT_REQUEST_FAILED ) {
    printf( "Failed to show statistics.\n" );
    stop_trema();
    return;
  }

  size_t entries_length = length - offsetof( management_show_stats_reply, entries );
  unsigned int n_entries = ( unsigned int ) entries_length / sizeof( stat_entry );
  stat_entry *entry = reply->entries;
  for ( unsigned int i = 0; i < n_entries; i++ ) {
    printf( "%s: %" PRIu64 "\n", entry->key, ntohll( entry->value ) );
    entry++;
  }

  stop_trema();
}


static void
send_show_stats_request() {
  management_show_stats_request request;
  memset( &request, 0, sizeof( management_show_stats_request ) );
  request.header.type = htons( MANAGEMENT_SHOW_STATS_REQUEST );
  request.header.length = htonl( sizeof( management_show_stats_request ) );

  bool ret = send_request_message( service_name, get_trema_name(), MESSENGER_MANAGEMENT_REQUEST,
                                   &request, sizeof( management_show_stats_request ), NULL );
  if ( !ret ) {
    printf( "Failed to send a show stats request to %s.\n", service_name );
    exit( EXIT_FAILURE );
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );

  // Parse arguments
  parse_arguments( argc, argv );

  // Set a handler to handle show stats reply
  add_message_replied_callback( get_trema_name(), handle_reply );

  // Send a show stats request
  send_show_stats_request();

  // Set timeout
  add_periodic_event_callback( 5, timeout, NULL );

  // Main loop
  start_trema();

  return EXIT_SUCCESS;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
