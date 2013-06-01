/*
 * Management command for any application specific managements.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trema.h"


static char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
static uint32_t application_id = 0;
static uint8_t *data = NULL;
static size_t data_length = 0;


void
usage( void ) {
  printf( "Usage: application SERVICE_NAME APPLICATION_ID [DATA_IN_HEX]\n" );
}


static void
print_usage_and_exit( void ) {
  usage();
  exit( EXIT_FAILURE );
}


static bool
hex_string_to_data( char *string ) {
  assert( string != NULL );

  size_t string_length = strlen( string );
  if ( string_length % 2 ) {
    return false;
  }

  data_length = string_length / 2;
  data = xmalloc( data_length );
  memset( data, 0, data_length );

  int byte = 0;
  for ( size_t i = 0; i < string_length; i++ ) {
    char c = string[ i ];
    byte <<= 4;

    if ( ( c >= '0' ) && ( c <= '9' ) ) {
      byte += c - '0';
    }
    else if ( ( c >= 'a' ) && ( c <= 'f' ) ) {
      byte += c - 'a' + 10;
    }
    else if ( ( c >= 'A' ) && ( c <= 'F' ) ) {
      byte += c - 'A' + 10;
    }
    else {
      xfree( data );
      data = NULL;
      data_length = 0;
      return false;
    }

    if ( i % 2 ) {
      data[ i / 2 ] = ( uint8_t ) byte;
      byte = 0;
    }
  }

  return true;
}


static void
parse_arguments( int argc, char **argv ) {
  assert( argv != NULL );

  if ( argc < 3 || argc > 4 ) {
    print_usage_and_exit();
  }

  if ( argv[ 1 ] == NULL || ( argv[ 1 ] != NULL && strlen( argv[ 1 ] ) == 0 ) ) {
    print_usage_and_exit();
  }

  if ( argv[ 2 ] == NULL ) {
    print_usage_and_exit();
  }

  memset( service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  snprintf( service_name, MESSENGER_SERVICE_NAME_LENGTH, "%s", get_management_service_name( argv[ 1 ] ) );

  char *endp = NULL;
  application_id = ( uint32_t ) strtoul( argv[ 2 ], &endp, 0 );
  if ( *endp != '\0' ) {
    print_usage_and_exit();
  }

  if ( argc >= 4 && argv[ 3 ] != NULL ) {
    hex_string_to_data( argv[ 3 ] );
    if ( data == NULL ) {
      print_usage_and_exit();
    }
  }
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
  assert( length >= offsetof( management_application_reply, data ) );

  management_application_reply *reply = data;
  assert( ntohs( reply->header.type ) == MANAGEMENT_APPLICATION_REPLY );
  assert( ntohl( reply->header.length ) == length );
  uint32_t id = ntohl( reply->application_id );

  if ( reply->header.status == MANAGEMENT_REQUEST_SUCCEEDED ) {
    printf( "An application specific management command is executed successfully ( application_id = %#x ).\n", id );
  }
  else {
    printf( "Failed to execute an application specific management command ( application_id = %#x ).\n", id );
  }

  size_t data_length = length - offsetof( management_application_reply, data );
  if ( data_length > 0 ) {
    printf( "Data: " );
    for ( size_t i = 0; i < data_length; i++ ) {
      printf( "%02x", reply->data[ i ] );
    }
    printf( "\n" );
  }

  stop_trema();
}


static void
send_application_request() {
  size_t length = offsetof( management_application_request, data ) + data_length;
  management_application_request *request = xmalloc( length );
  memset( request, 0, sizeof( management_application_request ) );
  request->header.type = htons( MANAGEMENT_APPLICATION_REQUEST );
  request->header.length = htonl( ( uint32_t ) length );
  request->application_id = htonl( application_id );
  if ( data_length > 0 && data != NULL ) {
    memcpy( request->data, data, data_length );
    xfree( data );
  }

  bool ret = send_request_message( service_name, get_trema_name(), MESSENGER_MANAGEMENT_REQUEST,
                                   request, length, NULL );
  xfree( request );
  if ( !ret ) {
    printf( "Failed to send an application specific management request to %s.\n", service_name );
    exit( EXIT_FAILURE );
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );

  // Parse arguments
  parse_arguments( argc, argv );

  // Set a handler to handle application specific management reply
  add_message_replied_callback( get_trema_name(), handle_reply );

  // Send an application specific management request
  send_application_request();

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
