/*
 * Management command to change logging level in operation.
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
static char logging_level[ LOGGING_LEVEL_STR_LENGTH ];


void
usage( void ) {
  printf( "Usage: set_logging_level SERVICE_NAME LEVEL\n" );
}


static void
print_usage_and_exit( void ) {
  usage();
  exit( EXIT_FAILURE );
}


static void
parse_arguments( int argc, char **argv ) {
  assert( argv != NULL );

  if ( argc != 3 ) {
    print_usage_and_exit();
  }

  if ( argv[ 1 ] == NULL || ( argv[ 1 ] != NULL && strlen( argv[ 1 ] ) == 0 ) ) {
    print_usage_and_exit();
  }

  if ( argv[ 2 ] == NULL ||
       ( argv[ 2 ] != NULL && ( strlen( argv[ 2 ] ) == 0 || strlen( argv[ 2 ] ) >= LOGGING_LEVEL_STR_LENGTH ) ) ) {
    print_usage_and_exit();
  }

  memset( service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  snprintf( service_name, MESSENGER_SERVICE_NAME_LENGTH, "%s", get_management_service_name( argv[ 1 ] ) );

  memset( logging_level, '\0', LOGGING_LEVEL_STR_LENGTH );
  snprintf( logging_level, LOGGING_LEVEL_STR_LENGTH, "%s", argv[ 2 ] );
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
  assert( length == sizeof( management_set_logging_level_reply ) );

  management_set_logging_level_reply *reply = data;
  assert( ntohs( reply->header.type ) == MANAGEMENT_SET_LOGGING_LEVEL_REPLY );
  assert( ntohl( reply->header.length ) == sizeof( management_set_logging_level_reply ) );

  if ( reply->header.status == MANAGEMENT_REQUEST_SUCCEEDED ) {
    printf( "Logging level is set to %s.\n", logging_level );
  }
  else {
    printf( "Failed to set logging level.\n" );
  }

  stop_trema();
}


static void
send_set_logging_level_request() {
  management_set_logging_level_request request;
  memset( &request, 0, sizeof( management_set_logging_level_request ) );
  request.header.type = htons( MANAGEMENT_SET_LOGGING_LEVEL_REQUEST );
  request.header.length = htonl( sizeof( management_set_logging_level_request ) );
  strncpy( request.level, logging_level, sizeof( request.level ) );

  bool ret = send_request_message( service_name, get_trema_name(), MESSENGER_MANAGEMENT_REQUEST,
                                   &request, sizeof( management_set_logging_level_request ), NULL );
  if ( !ret ) {
    printf( "Failed to send a set logging level request to %s.\n", service_name );
    exit( EXIT_FAILURE );
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );

  // Parse arguments
  parse_arguments( argc, argv );

  // Set a handler to handle set logging level reply
  add_message_replied_callback( get_trema_name(), handle_reply );

  // Send a set logging level request
  send_set_logging_level_request();

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
