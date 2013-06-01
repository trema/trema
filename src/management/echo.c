/*
 * Management command to check if a specified application/service is alive or not.
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
#include <time.h>
#include "trema.h"


#define SUB_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec - ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec - ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec < 0 ) {                         \
      ( _return )->tv_sec--;                                  \
      ( _return )->tv_nsec += 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )


static char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


void
usage( void ) {
  printf( "Usage: echo SERVICE_NAME\n" );
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
  assert( length == sizeof( management_echo_reply ) );

  management_echo_reply *reply = data;
  assert( ntohs( reply->header.type ) == MANAGEMENT_ECHO_REPLY );
  assert( ntohl( reply->header.length ) == sizeof( management_echo_reply ) );

  if ( reply->header.status != MANAGEMENT_REQUEST_SUCCEEDED ) {
    printf( "Failed to execute echo request/reply transaction.\n" );
    stop_trema();
    return;
  }

  struct timespec sent_at = { 0, 0 };
  sent_at.tv_sec = ( time_t ) ntohl( reply->sent_at.tv_sec );
  sent_at.tv_nsec = ( long ) ntohl( reply->sent_at.tv_nsec );

  struct timespec received_at = { 0, 0 };
  received_at.tv_sec = ( time_t ) ntohl( reply->received_at.tv_sec );
  received_at.tv_nsec = ( long ) ntohl( reply->received_at.tv_nsec );

  struct timespec now = { 0, 0 };
  clock_gettime( CLOCK_REALTIME, &now );

  struct timespec roundtrip = { 0, 0 };
  SUB_TIMESPEC( &now, &sent_at, &roundtrip );

  long double roundtrip_msec = roundtrip.tv_sec * 1000 + ( long double ) roundtrip.tv_nsec / 1000000;

  printf( "Reqeust sent at: %d.%09d\n", ( int ) sent_at.tv_sec, ( int ) sent_at.tv_nsec );
  printf( "Reqeust received at: %d.%09d\n", ( int ) received_at.tv_sec, ( int ) received_at.tv_nsec );
  printf( "Reply received at: %d.%09d\n", ( int ) now.tv_sec, ( int ) now.tv_nsec );
  printf( "Roundtrip time: %Lf [msec]\n", roundtrip_msec );

  stop_trema();
}


static void
send_echo_request() {
  management_echo_request request;
  memset( &request, 0, sizeof( management_echo_request ) );
  request.header.type = MANAGEMENT_ECHO_REQUEST;
  request.header.length = htonl( sizeof( management_echo_request ) );

  struct timespec now = { 0, 0 };
  int retval = clock_gettime( CLOCK_REALTIME, &now );
  if ( retval < 0 ) {
    printf( "Failed to retrieve clock ( errno = %s [%d] ).\n", strerror( errno ), errno );
    exit( EXIT_FAILURE );
  }
  request.sent_at.tv_sec = htonl( ( uint32_t ) now.tv_sec );
  request.sent_at.tv_nsec = htonl( ( uint32_t ) now.tv_nsec );

  bool ret = send_request_message( service_name, get_trema_name(), MESSENGER_MANAGEMENT_REQUEST,
                                   &request, sizeof( management_echo_request ), NULL );
  if ( !ret ) {
    printf( "Failed to send an echo request to %s.\n", service_name );
    exit( EXIT_FAILURE );
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );

  // Parse arguments
  parse_arguments( argc, argv );

  // Set a handler to handle echo reply
  add_message_replied_callback( get_trema_name(), handle_reply );

  // Send an echo request
  send_echo_request();

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
