/*
 * stdin_relay: An application that relays text string from stdin to tremashark
 *
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


#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"


static char *dump_service_name = NULL;
static buffer *stdin_read_buffer = NULL;


static void
relay_string( buffer *string ) {
  // retrieve current time
  struct timespec now;
  if ( clock_gettime( CLOCK_REALTIME, &now ) == -1 ) {
    error( "Failed to retrieve system-wide real-time clock ( %s [%d] ).", strerror( errno ), errno );
    return;
  }

  // allocate buffer
  char *service_name = xstrdup( get_trema_name() );
  uint16_t service_name_length = ( uint16_t ) ( strlen( service_name ) + 1 );
  size_t buffer_length = sizeof( message_dump_header ) + service_name_length + sizeof( syslog_dump_header ) + string->length + 1;
  buffer *buf = alloc_buffer_with_length( buffer_length );

  // syslog_dump_header + service_name
  message_dump_header *mdh = append_back_buffer( buf, sizeof( message_dump_header ) );
  mdh->sent_time.sec = htonl( ( uint32_t ) now.tv_sec );
  mdh->sent_time.nsec = htonl( ( uint32_t ) now.tv_nsec );
  mdh->app_name_length = htons( 0 );
  mdh->service_name_length = htons( service_name_length );
  mdh->data_length = htonl( ( uint32_t ) ( sizeof( text_dump_header ) + string->length + 1 ) );
  void *svn = append_back_buffer( buf, service_name_length );
  memcpy( svn, service_name, service_name_length );
  xfree( service_name );

  // text_dump_header
  text_dump_header *tdh = append_back_buffer( buf, sizeof( text_dump_header ) );
  tdh->sent_time.sec = htonl( ( uint32_t ) now.tv_sec );
  tdh->sent_time.nsec = htonl( ( uint32_t ) now.tv_nsec );

  // message
  void *p = append_back_buffer( buf, string->length + 1 );
  memset( p, '\0', string->length + 1 );
  memcpy( p, string->data, string->length );

  bool ret = send_message( dump_service_name, MESSENGER_DUMP_TEXT, buf->data, buf->length );
  if ( !ret ) {
    error( "Failed to relay syslog message." );
  }
  free_buffer( buf );
}


static char *
find_character( char *string, size_t length, char character ) {
  assert( string != NULL );

  for ( size_t i = 0; i < length; i++ ) {
    if ( string[ i ] == character ) {
      return &( string[ i ] );
    }
  }

  return NULL;
}


static void
read_stdin( int fd, void *data ) {
  UNUSED( data );

  char buf[ 1024 ];
  memset( buf, '\0', sizeof( buf ) );
  ssize_t ret = read( fd, buf, sizeof( buf ) );
  if ( ret < 0 ) {
    if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
      return;
    }

    set_readable( fd, false );

    error( "Read error ( errno = %s [%d] ).", strerror( errno ), errno );
    return;
  }
  else if ( ret == 0 ) {
    return;
  }

  if ( stdin_read_buffer == NULL ) {
    set_readable( fd, false );

    error( "Read buffer is not allocated yet" );
    return;
  }

  char *p = append_back_buffer( stdin_read_buffer, ( size_t ) ret );
  memcpy( p, buf, ( size_t ) ret );
  char *lf = find_character( stdin_read_buffer->data, stdin_read_buffer->length, '\n' );
  while ( lf != NULL ) {
    size_t length = ( size_t ) ( lf - ( char * ) stdin_read_buffer->data );
    if ( length > 0 ) {
      buffer *string = alloc_buffer_with_length( length );
      p = append_back_buffer( string, length );
      memcpy( p, stdin_read_buffer->data, length );
      relay_string( string );
      free_buffer( string );
      remove_front_buffer( stdin_read_buffer, length + 1 );
    }
    else {
      if ( stdin_read_buffer->length > 0 ) {
        remove_front_buffer( stdin_read_buffer, 1 );
      }
    }
    if ( stdin_read_buffer->length == 0 ) {
      break;
    }
    lf = find_character( stdin_read_buffer->data, stdin_read_buffer->length, '\n' );
  }

  // truncate head room manually
  buffer *truncated = duplicate_buffer( stdin_read_buffer );
  free_buffer( stdin_read_buffer );
  stdin_read_buffer = truncated;
}


void
usage( void ) {
  printf(
    "Usage: stdin_relay [OPTION]...\n"
    "\n"
    "  -s DUMP_SERVICE_NAME            dump service name\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
  );
}


static void
print_usage_and_exit( void ) {
  usage();
  exit( EXIT_FAILURE );
}


static void
parse_options( int *argc, char **argv[] ) {
  int opt;

  while ( 1 ) {
    opt = getopt( *argc, *argv, "s:" );

    if ( opt < 0 ) {
      break;
    }

    switch ( opt ) {
      case 's':
        if ( optarg && dump_service_name == NULL ) {
          dump_service_name = xstrdup( optarg );
        }
        else {
          print_usage_and_exit();
        }
        break;

      default:
        print_usage_and_exit();
    }
  }

  if ( dump_service_name == NULL ) {
    dump_service_name = xstrdup( DEFAULT_DUMP_SERVICE_NAME );
  }
}


static bool
init_stdin_relay( int *argc, char **argv[] ) {
  parse_options( argc, argv );

  stdin_read_buffer = alloc_buffer_with_length( 1024 );

  set_fd_handler( STDIN_FILENO, read_stdin, NULL, NULL, NULL );
  set_readable( STDIN_FILENO, true );

  return true;
}


static bool
finalize_stdin_relay( void ) {
  set_readable( STDIN_FILENO, false );
  delete_fd_handler( STDIN_FILENO );

  if ( dump_service_name != NULL ) {
    xfree( dump_service_name );
    dump_service_name = NULL;
  }
  if ( stdin_read_buffer != NULL ) {
    free_buffer( stdin_read_buffer );
    stdin_read_buffer = NULL;
  }

  return true;
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );
  init_stdin_relay( &argc, &argv );

  // Main loop
  start_trema();

  // Cleanup
  finalize_stdin_relay();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
