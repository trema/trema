/*
 * syslog_relay: An application that relays syslog messages to tremashark
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


#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"


static char *dump_service_name = NULL;
static uint16_t listen_port = 514;
static int syslog_fd = -1;


static void
relay_syslog_message( buffer *message ) {
  // retrieve current time
  struct timespec now;
  if ( clock_gettime( CLOCK_REALTIME, &now ) == -1 ) {
    error( "Failed to retrieve system-wide real-time clock ( %s [%d] ).", strerror( errno ), errno );
    return;
  }

  // allocate buffer
  char *service_name = xstrdup( get_trema_name() );
  uint16_t service_name_length = ( uint16_t ) ( strlen( service_name ) + 1 );
  size_t buffer_length = sizeof( message_dump_header ) + service_name_length + sizeof( syslog_dump_header ) + message->length;
  buffer *buf = alloc_buffer_with_length( buffer_length );

  // syslog_dump_header + service_name
  message_dump_header *mdh = append_back_buffer( buf, sizeof( message_dump_header ) );
  mdh->sent_time.sec = htonl( ( uint32_t ) now.tv_sec );
  mdh->sent_time.nsec = htonl( ( uint32_t ) now.tv_nsec );
  mdh->app_name_length = htons( 0 );
  mdh->service_name_length = htons( service_name_length );
  mdh->data_length = htonl( ( uint32_t ) ( sizeof( syslog_dump_header ) + message->length ) );
  void *svn = append_back_buffer( buf, service_name_length );
  memcpy( svn, service_name, service_name_length );
  xfree( service_name );

  // syslog_dump_header
  syslog_dump_header *sdh = append_back_buffer( buf, sizeof( syslog_dump_header ) );
  sdh->sent_time.sec = htonl( ( uint32_t ) now.tv_sec );
  sdh->sent_time.nsec = htonl( ( uint32_t ) now.tv_nsec );

  // message
  void *p = append_back_buffer( buf, message->length );
  memcpy( p, message->data, message->length );

  bool ret = send_message( dump_service_name, MESSENGER_DUMP_SYSLOG, buf->data, buf->length );
  if ( !ret ) {
    error( "Failed to relay syslog message." );
  }
  free_buffer( buf );
}


static void
recv_syslog_message( int fd, void *data ) {
  UNUSED( data );

  char buf[ 1024 ];
  ssize_t ret = read( fd, buf, sizeof( buf ) );

  if ( ret < 0 ) {
    if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
      return;
    }

    error( "Receive error ( errno = %s [%d] ).", strerror( errno ), errno );

    set_readable( fd, false );
    delete_fd_handler( fd );
    return;
  }

  buffer *message = alloc_buffer_with_length( ( size_t ) ret );
  void *p = append_back_buffer( message, ( size_t ) ret );
  memcpy( p, buf, ( size_t ) ret );

  relay_syslog_message( message );

  free_buffer( message );
}


void
usage( void ) {
  printf(
    "Usage: syslog_relay [OPTION]...\n"
    "\n"
    "  -p LISTEN_PORT                  listen port for receiving syslog messages\n"
    "  -s DUMP_SERVICE_NAME            dump service name\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
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
    opt = getopt( *argc, *argv, "p:s:" );

    if ( opt < 0 ) {
      break;
    }

    switch ( opt ) {
      case 'p':
        if ( ( optarg != NULL ) && ( atoi( optarg ) <= UINT16_MAX ) && ( atoi ( optarg ) >= 0 ) ) {
          listen_port = ( uint16_t ) atoi( optarg );
        }
        else {
          print_usage_and_exit();
        }
        break;

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
init_syslog_relay( int *argc, char **argv[] ) {
  parse_options( argc, argv );

  if ( syslog_fd >= 0 ) {
    close( syslog_fd );
    syslog_fd = -1;
  }

  syslog_fd = socket( PF_INET, SOCK_DGRAM, 0 );
  if ( syslog_fd < 0 ) {
    error( "Failed to create socket ( errno = %s [%d] ).", strerror( errno ), errno );
    return false;
  }

  struct sockaddr_in addr;
  memset( &addr, 0, sizeof( struct sockaddr_in ) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( listen_port );
  addr.sin_addr.s_addr = htonl( INADDR_ANY );

  int ret = bind( syslog_fd, ( struct sockaddr * ) &addr, sizeof( struct sockaddr_in ) );
  if ( ret < 0 ) {
    error( "Failed to bind socket ( errno = %s [%d] ).", strerror( errno ), errno );
    close( syslog_fd );
    syslog_fd = -1;
    return false;
  }

  set_fd_handler( syslog_fd, recv_syslog_message, NULL, NULL, NULL );
  set_readable( syslog_fd, true );

  return true;
}


static bool
finalize_syslog_relay( void ) {
  if ( syslog_fd >= 0 ) {
    set_readable( syslog_fd, false );
    delete_fd_handler( syslog_fd );

    close( syslog_fd );
    syslog_fd = -1;
  }

  xfree( dump_service_name );
  dump_service_name = NULL;

  return true;
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );
  init_syslog_relay( &argc, &argv );

  // Main loop
  start_trema();

  // Cleanup
  finalize_syslog_relay();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
