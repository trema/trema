/*
 * tremashark: A bridge for printing various events on Wireshark
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
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <pcap.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"
#include "pcap_private.h"
#include "pcap_queue.h"


#define FIFO_NAME "tremashark"
#define WIRESHARK "wireshark"
#define TSHARK "tshark"
#define FLUSH_INTERVAL 250000000 // nanoseconds
#define MESSAGE_BUFFER_LENGTH 100000 // microseconds

#define WRITE_SUCCESS 0
#define WRITE_BUSY 1
#define WRITE_ERROR -1


static char fifo_pathname[ PATH_MAX ];
static char pcap_file_pathname[ PATH_MAX ];
static bool output_to_pcap_file = false;
static bool launch_wireshark = true;
static bool launch_tshark = false;
static bool trust_remote_clocks = true;
static bool use_circular_buffer = false;
static int circular_buffer_length = 1024;
static int outfile_fd = -1;
static uint64_t total = 0;
static uint64_t lost = 0;


// Special purpose header for telling extra information to wireshark
typedef struct {
  uint16_t dump_type;
  struct {
    uint32_t sec;
    uint32_t nsec;
  } sent_time;
  uint16_t app_name_len;
  uint16_t service_name_len;
  uint32_t data_len;
} __attribute__( ( packed ) ) message_pcap_dump_header;


static void
init_fifo_pathname() {
  assert( strlen( get_trema_tmp() ) + strlen( FIFO_NAME ) + 1 <= PATH_MAX );

  snprintf( fifo_pathname, PATH_MAX, "%s/%s", get_trema_tmp(), FIFO_NAME );
}


static int
write_to_file( buffer *packet ) {
  assert( packet != NULL && packet->data != NULL && packet->length > 0 );
  assert( outfile_fd >= 0 );

  ssize_t ret = write( outfile_fd, packet->data, packet->length );
  if ( ret < 0 ) {
    int err = errno;
    if ( err == EAGAIN || err == EWOULDBLOCK ) {
      return WRITE_BUSY;
    }
    error( "write error ( errno = %s [%d] ).", strerror( err ), err );
    return WRITE_ERROR;
  }

  if ( ret != ( ssize_t ) packet->length ) {
    packet->data = ( char * ) packet->data + ret;
    packet->length = packet->length - ( unsigned int ) ret;
    return WRITE_BUSY;
  }

  return WRITE_SUCCESS;
}


static void
dump_message( uint16_t tag, void *data, size_t len ) {
  char *app_name, *service_name;
  const char *type_str[] = { "sent", "received", "recv-connected", "recv-overflow", "recv-closed",
                             "send-connected", "send-refused", "send-overflow", "send-closed",
                             "logger", "pcap", "syslog", "text" };
  size_t pcap_dump_header_length;
  struct pcap_pkthdr_private pcap_header;
  message_dump_header *dump_hdr;
  message_pcap_dump_header *pcap_dump_hdr;
  message_header *hdr;
  buffer *packet;
  queue_status status;

  dump_hdr = data;

  app_name = ( char * ) ( dump_hdr + 1 );
  service_name = app_name + ntohs( dump_hdr->app_name_length );
  hdr = ( message_header * ) ( service_name + ntohs( dump_hdr->service_name_length ) );

  debug( "app: %s type: %s service_name: %s", app_name, type_str[ tag ], service_name );

  if ( ntohl( dump_hdr->data_length ) > 0 ) {
    debug( "message type: %d, length: %u", tag, ntohl( dump_hdr->data_length ) );
  }

  size_t dump_header_length = ( sizeof( message_dump_header ) +
                                ntohs( dump_hdr->app_name_length ) +
                                ntohs( dump_hdr->service_name_length ) );

  pcap_dump_header_length = ( sizeof( message_pcap_dump_header ) +
                              ntohs( dump_hdr->app_name_length ) +
                              ntohs( dump_hdr->service_name_length ) );

  pcap_dump_hdr = xmalloc( pcap_dump_header_length );

  pcap_dump_hdr->dump_type = htons( tag );
  pcap_dump_hdr->sent_time.sec = dump_hdr->sent_time.sec;
  pcap_dump_hdr->sent_time.nsec = dump_hdr->sent_time.nsec;
  pcap_dump_hdr->app_name_len = dump_hdr->app_name_length;
  pcap_dump_hdr->service_name_len = dump_hdr->service_name_length;
  pcap_dump_hdr->data_len = dump_hdr->data_length;

  char *pcap_dump_app_name = ( char * ) ( pcap_dump_hdr + 1 );
  memcpy( pcap_dump_app_name, app_name, ntohs( pcap_dump_hdr->app_name_len ) );
  if ( ntohs( pcap_dump_hdr->service_name_len ) > 0 ) {
    char *pcap_dump_service_name = pcap_dump_app_name + ntohs( pcap_dump_hdr->app_name_len );
    memcpy( pcap_dump_service_name, service_name, ntohs( pcap_dump_hdr->service_name_len ) );
  }

  memset( &pcap_header, 0, sizeof( struct pcap_pkthdr_private ) );
  if ( trust_remote_clocks ) {
    pcap_header.ts.tv_sec = ( bpf_int32 ) ntohl( dump_hdr->sent_time.sec );
    pcap_header.ts.tv_usec = ( bpf_int32 ) ( ntohl( dump_hdr->sent_time.nsec ) / 1000 );
  }
  else {
    struct timeval ts;
    gettimeofday( &ts, NULL );
    pcap_header.ts.tv_sec = ( bpf_int32 ) ts.tv_sec;
    pcap_header.ts.tv_usec = ( bpf_int32 ) ts.tv_usec;
  }

  len -= dump_header_length;

  pcap_header.caplen = ( bpf_u_int32 ) ( pcap_dump_header_length + ntohl( pcap_dump_hdr->data_len ) );
  pcap_header.len = ( bpf_u_int32 ) ( pcap_dump_header_length + ntohl( pcap_dump_hdr->data_len ) );

  packet = create_pcap_packet( &pcap_header, sizeof( struct pcap_pkthdr_private ),
                               pcap_dump_hdr, pcap_dump_header_length,
                               hdr, ntohl( pcap_dump_hdr->data_len ) );
  xfree( pcap_dump_hdr );

  if ( use_circular_buffer ) {
    buffer *dummy;
    while ( get_pcap_queue_length() >= circular_buffer_length ) {
      dummy = NULL;
      dequeue_pcap_packet( &dummy );
      if ( dummy != NULL ) {
        delete_pcap_packet( dummy );
      }
    }
  }

  status = enqueue_pcap_packet( packet );
  if ( status == QUEUE_FULL ) {
    warn( "tremashark queue is full. packet is discarded." );
    delete_pcap_packet( packet );
    lost++;
  }
  total++;
}


static void
write_pcap_packet( void *user_data ) {
  UNUSED( user_data );

  sort_pcap_queue();

  struct timeval now;
  gettimeofday( &now, NULL );
  struct timeval buffer_length;
  buffer_length.tv_sec = 0;
  buffer_length.tv_usec = MESSAGE_BUFFER_LENGTH;
  struct timeval threshold;

  timersub( &now, &buffer_length, &threshold );

  for ( ;; ) {
    buffer *packet;
    queue_status status = peek_pcap_packet( &packet );
    if ( status == QUEUE_EMPTY ) {
      break;
    }

    struct pcap_pkthdr_private *p = packet->data;
    if ( ( p->ts.tv_sec > threshold.tv_sec ) ||
         ( p->ts.tv_sec == threshold.tv_sec && p->ts.tv_usec > threshold.tv_usec ) ) {
      break;
    }

    int ret = write_to_file( packet );
    switch ( ret ) {
    case WRITE_SUCCESS:
      status = dequeue_pcap_packet( &packet );
      if ( status == QUEUE_SUCCESS && packet != NULL ) {
        delete_pcap_packet( packet );
      }
      break;

    case WRITE_BUSY:
    case WRITE_ERROR:
      return;

    default:
      assert( 0 );
    }
  }

  fsync( outfile_fd );
}


static void
set_timer_event() {
  if ( use_circular_buffer ) {
    return;
  }

  struct itimerspec interval;

  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = FLUSH_INTERVAL;

  bool ret = add_timer_event_callback( &interval, write_pcap_packet, NULL );
  if ( !ret ) {
    critical( "failed in set timer event" );
    abort();
  }
}


static void
init_pcap() {
  struct pcap_file_header header;
  memset( &header, 0, sizeof( struct pcap_file_header ) );
  header.magic = 0xa1b2c3d4;
  header.version_major = PCAP_VERSION_MAJOR;
  header.version_minor = PCAP_VERSION_MINOR;
  header.thiszone = 0;
  header.sigfigs = 0;
  header.snaplen = UINT16_MAX; // FIXME
  header.linktype = DLT_USER0; // FIXME

  mode_t mode = ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
  if ( output_to_pcap_file ) {
    outfile_fd = open( pcap_file_pathname, O_RDWR | O_CREAT | O_TRUNC, mode );
    if ( outfile_fd < 0 ) {
      critical( "Failed to open a file ( pcap file = %s ).", pcap_file_pathname );
      assert( 0 );
    }
  }
  else {
    int ret = mkfifo( fifo_pathname, mode );
    if ( ret < 0 ) {
      critical( "Failed to create a named pipe ( named pipe = %s ).", fifo_pathname );
      assert( 0 );
    }

    outfile_fd = open( fifo_pathname, O_RDWR | O_APPEND | O_NONBLOCK );
    if ( outfile_fd < 0 ) {
      critical( "Failed to open a named pipe ( named pipe = %s ).", fifo_pathname );
      assert( 0 );
    }
  }

  ssize_t ret = write( outfile_fd, &header, sizeof( struct pcap_file_header ) );

  if ( ret != sizeof( struct pcap_file_header ) ) {
    critical( "Failed to write a pcap header ( pcap file = %s ).", pcap_file_pathname );
    assert( 0 );
  }

  fsync( outfile_fd );
}


static void
finalize_pcap() {
  if ( outfile_fd >= 0 ) {
    fsync( outfile_fd );
    close( outfile_fd );
    outfile_fd = -1;
  }

  unlink( fifo_pathname );
}


static void
write_circular_buffer( void ) {
  if ( !output_to_pcap_file ) {
    return;
  }

  if ( outfile_fd < 0 ) {
    init_pcap();
  }

  sort_pcap_queue();

  foreach_pcap_queue( ( void * ) write_to_file );

  finalize_pcap();
}


static void
set_write_circular_buffer( void ) {
  if ( set_external_callback != NULL ) {
    set_external_callback( write_circular_buffer );
  }
}


static void
set_signal_handler( void ) {
  // Note that this overrides a default signal handler set by Trema.
  struct sigaction signal_usr2;
  memset( &signal_usr2, 0, sizeof( struct sigaction ) );
  signal_usr2.sa_handler = ( void * ) set_write_circular_buffer;
  sigaction( SIGUSR2, &signal_usr2, NULL );
}


static void
start_wireshark() {
  pid_t pid;

  if ( !launch_wireshark && !launch_tshark ) {
    return;
  }

  pid = fork();

  if ( pid < 0 ) {
    assert( 0 );
  }

  if ( pid == 0 ) {
    if ( launch_wireshark ) {
      execlp( WIRESHARK, "wireshark", "-k", "-i", fifo_pathname, NULL );
      error( "can't execute wireshark ( errno = %s [%d] ).", strerror( errno ), errno );
    }
    else if ( launch_tshark ) {
      execlp( TSHARK, "tshark", "-V", "-i", fifo_pathname, NULL );
      error( "can't execute tshark ( errno = %s [%d] ).", strerror( errno ), errno );
    }
    exit( EXIT_FAILURE );
  }
}


void
usage( void ) {
  printf(
    "Usage: tremashark [OPTION]...\n"
    "\n"
    "  -t                              launch tshark instead of wireshark\n"
    "  -w FILE_TO_SAVE                 save messages to pcap file\n"
    "  -p                              do not launch wireshark nor tshark\n"
    "  -r                              do not trust remote clock\n"
    "  -c NUMBER_OF_MESSAGES           save messages to circular buffer\n"
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
  char *service_name = NULL;
  int opt;

  while ( 1 ) {
    opt = getopt( *argc, *argv, "c:s:tw:pr" );

    if ( opt < 0 ) {
      break;
    }

    switch ( opt ) {
    case 'p':
      launch_wireshark = false;
      launch_tshark = false;
      break;

    case 't':
      launch_wireshark = false;
      launch_tshark = true;
      break;

    case 'r':
      trust_remote_clocks = false;
      break;

    case 's':
      if ( optarg && service_name == NULL ) {
        service_name = xmalloc( strlen( optarg ) + 1 );
        strcpy( service_name, optarg );
      }
      else {
        print_usage_and_exit();
      }
      break;

    case 'w':
      if ( optarg ) {
        // Save packets to a pcap file
        strncpy( pcap_file_pathname, optarg, PATH_MAX );
        output_to_pcap_file = true;
        launch_wireshark = false;
        launch_tshark = false;
      }
      else {
        print_usage_and_exit();
      }
      break;

    case 'c':
      use_circular_buffer = true;
      launch_wireshark = false;
      launch_tshark = false;
      if ( optarg ) {
        int n = atoi( optarg );
        if ( n > 0 ) {
          circular_buffer_length = n;
          set_max_pcap_queue_length( circular_buffer_length );
        }
        else {
          print_usage_and_exit();
        }
      }
      break;

    default:
      print_usage_and_exit();
    }
  }

  if ( use_circular_buffer && !output_to_pcap_file ) {
    printf( "-w FILE_NAME option must be specified in conjunction with -c.\n" );
    print_usage_and_exit();
  }

  // Set an event handler
  if ( service_name == NULL ) {
    add_message_received_callback( DEFAULT_DUMP_SERVICE_NAME, dump_message );
  }
  else {
    add_message_received_callback( service_name, dump_message );
    xfree( service_name );
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );
  parse_options( &argc, &argv );
  init_fifo_pathname();

  // create queue for storing pcap packets
  create_pcap_queue();

  // Initialize an interface to wireshark
  init_pcap();

  // Start wireshark/tshark if necessary
  start_wireshark();

  // Set timer event to write packet
  set_timer_event();

  // Set signal handler to dump circular buffer
  set_signal_handler();

  // Main loop
  start_trema();

  // Cleanup
  finalize_pcap();
  delete_pcap_queue();

  if ( lost > 0 ) {
    warn( "%" PRIu64 "/%" PRIu64 " messages lost.", lost, total );
  }

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
