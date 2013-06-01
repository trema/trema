/*
 * packet_capture: An application that captures packets from a network interface
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
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <pcap.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "pcap_private.h"
#include "queue.h"
#include "trema.h"


static char *dump_service_name = NULL;
static char *interface_name = NULL;
static char *filter_expression = NULL;
static queue *packet_queue = NULL;
static pthread_t *capture_thread = NULL;


static void
handle_packet( u_char *args, const struct pcap_pkthdr *header, const u_char *packet ) {
  // allocate buffer
  char *app_name = interface_name;
  uint16_t app_name_length = ( uint16_t ) ( strlen( interface_name ) + 1 );
  char *service_name = xstrdup( get_trema_name() );
  uint16_t service_name_length = ( uint16_t ) ( strlen( service_name ) + 1 );
  size_t buffer_length = sizeof( message_dump_header ) + app_name_length + service_name_length + sizeof( pcap_dump_header ) + sizeof( struct pcap_pkthdr_private ) + header->caplen;
  buffer *buf = alloc_buffer_with_length( buffer_length );

  // message_dump_header + app_name + service_name
  message_dump_header *mdh = append_back_buffer( buf, sizeof( message_dump_header ) );
  mdh->sent_time.sec = htonl( ( uint32_t ) header->ts.tv_sec );
  mdh->sent_time.nsec = htonl( ( uint32_t ) ( header->ts.tv_usec * 1000 ) );
  mdh->app_name_length = htons( app_name_length );
  mdh->service_name_length = htons( service_name_length );
  mdh->data_length = htonl( ( uint32_t ) ( sizeof( pcap_dump_header ) + sizeof( struct pcap_pkthdr_private ) + header->caplen ) );
  void *apn = append_back_buffer( buf, app_name_length );
  memcpy( apn, app_name, app_name_length );
  void *svn = append_back_buffer( buf, service_name_length );
  memcpy( svn, service_name, service_name_length );
  xfree( service_name );

  // pcap_dump_header
  pcap_dump_header *pdh = append_back_buffer( buf, sizeof( pcap_dump_header ) );
  int *dlt = ( int * ) args;
  pdh->datalink = htonl( ( uint32_t ) *dlt );
  strncpy( ( char * ) pdh->interface, interface_name, sizeof( pdh->interface ) );
  pdh->interface[ sizeof( pdh->interface ) - 1 ] = '\0';

  // pcap_pkthdr_private + packet
  struct pcap_pkthdr_private *pph = append_back_buffer( buf, sizeof( struct pcap_pkthdr_private ) );
  pph->ts.tv_sec = ( bpf_int32 ) htonl( ( uint32_t ) header->ts.tv_sec );
  pph->ts.tv_usec = ( bpf_int32 ) htonl( ( uint32_t ) header->ts.tv_usec );
  pph->caplen = htonl( header->caplen );
  pph->len = htonl( header->len );
  void *pkt = append_back_buffer( buf, header->caplen );
  memcpy( pkt, packet, header->caplen );

  enqueue( packet_queue, buf );
}


static void *
capture_main( void *args ) {
  UNUSED( args );

  info( "Starting packet capture ( interface_name = %s ).", interface_name );

  packet_queue = create_queue();
  pcap_t *cd = NULL;
  if ( packet_queue == NULL ) {
    error( "Failed to create packet queue." );
    goto error;
  }

  char errbuf[ PCAP_ERRBUF_SIZE ];
  bpf_u_int32 mask = 0;
  bpf_u_int32 net = 0;
  int ret = pcap_lookupnet( interface_name, &net, &mask, errbuf );
  if ( ret < 0 ) {
    error( "Failed to get netmask for device %s ( error = %s ).", interface_name, errbuf );
    net = 0;
    mask = 0;
  }

  cd = pcap_open_live( interface_name, UINT16_MAX, 1, 100, errbuf );
  if ( cd == NULL ) {
    error( "Failed to open network interface ( interface_name = %s, error = %s ).",
           interface_name, errbuf );
    goto error;
  }

  if ( filter_expression != NULL ) {
    struct bpf_program fp;
    ret = pcap_compile( cd, &fp, filter_expression, 0, net );
    if ( ret < 0 ) {
      error( "Failed to parse filter `%s' ( error = %s ).", filter_expression, pcap_geterr( cd ) );
      goto error;
    }
    ret = pcap_setfilter( cd, &fp );
    if ( ret < 0 ) {
      error( "Failed to set filter `%s' ( error = %s ).", filter_expression, pcap_geterr( cd ) );
      goto error;
    }
  }

  int dlt = pcap_datalink( cd );

  pcap_loop( cd, -1, handle_packet, ( u_char * ) &dlt );

error:
  if ( cd != NULL ) {
    pcap_close( cd );
  }
  if ( packet_queue != NULL ) {
    delete_queue( packet_queue );
  }

  return NULL;
}


static void
start_capture( void ) {
  pthread_attr_t attr;
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  capture_thread = xmalloc( sizeof( pthread_t ) );
  pthread_create( capture_thread, &attr, capture_main, NULL );
}


static void
stop_capture( void ) {
  if ( capture_thread != NULL ) {
    pthread_cancel( *capture_thread );
    xfree( capture_thread );
  }
  capture_thread = NULL;
}


static void
flush_packet_buffer( void *user_data ) {
  UNUSED( user_data );

  if ( packet_queue == NULL ) {
    return;
  }

  if ( packet_queue->length == 0 ) {
    return;
  }

  debug( "Flushing packet queue ( length = %u ).", packet_queue->length );

  buffer *packet = peek( packet_queue );
  while ( packet != NULL ) {
    bool ret = send_message( dump_service_name, MESSENGER_DUMP_PCAP, packet->data, packet->length );
    if ( ret ) {
      packet = dequeue( packet_queue );
      free_buffer( packet );
    }
    else {
      break;
    }
    packet = peek( packet_queue );
  }
}


static void
set_timer_event( void ) {
  struct itimerspec ts;

  ts.it_value.tv_sec = 0;
  ts.it_value.tv_nsec = 0;
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 1000000;

  bool ret = add_timer_event_callback( &ts, flush_packet_buffer, NULL );
  if ( !ret ) {
    error( "Failed to set queue flush timer." );
  }
}


void
usage( void ) {
  printf(
    "Usage: packet_capture -i NETWORK_INTERFACE [OPTION]...\n"
    "\n"
    "  -i NETWORK_INTERFACE            network interface for packet capturing\n"
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
    opt = getopt( *argc, *argv, "i:s:" );

    if ( opt < 0 ) {
      break;
    }

    switch ( opt ) {
      case 'i':
        if ( optarg && interface_name == NULL ) {
          interface_name = xstrdup( optarg );
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

  if ( interface_name == NULL ) {
    fprintf( stderr, "-i option is mandatory.\n" );
    print_usage_and_exit();
  }

  if ( dump_service_name == NULL ) {
    dump_service_name = xstrdup( DEFAULT_DUMP_SERVICE_NAME );
  }

  char expression[ ARG_MAX ];
  memset( expression, '\0', sizeof( expression ) );
  while ( optind < *argc ) {
    const char *primitive = ( *argv )[ optind ];
    strncat( expression, primitive, strlen( primitive ) );
    optind++;
    if ( optind <= *argc ) {
      strncat( expression, " ", 1 );
    }
  }

  if ( strlen( expression ) > 0 ) {
    filter_expression = xstrdup( expression );
    debug( "filter expression: %s", filter_expression );
  }
}


static void
init_packet_capture( int *argc, char **argv[] ) {
  parse_options( argc, argv );
}


static void
start_packet_capture( void ) {
  set_timer_event();

  if ( set_external_callback != NULL ) {
    set_external_callback( start_capture );
  }
}


static void
stop_packet_capture( void ) {
  stop_capture();
}


static void
finalize_packet_capture( void ) {
  if ( dump_service_name != NULL ) {
    xfree( dump_service_name );
  }
  if ( interface_name != NULL ) {
    xfree( interface_name );
  }
  if ( filter_expression != NULL ) {
    xfree( filter_expression );
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );
  init_packet_capture( &argc, &argv );

  // Start packet capture
  start_packet_capture();

  // Main loop
  start_trema();

  // Cleanup
  stop_packet_capture();
  finalize_packet_capture();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
