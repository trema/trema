/*
 * tremashark: A bridge for printing Trema IPC messages on Wireshark
 * 
 * Author: Yasunobu Chiba, Yasunori Nakazawa
 *
 * Copyright (C) 2008-2011 NEC Corporation
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
#include <linux/limits.h>
#include <pcap.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"
#include "buffer.h"
#include "pcap_queue.h"


#define FIFO_NAME "tremashark"
#define WIRESHARK "wireshark"
#define TSHARK "tshark"
#define FLUSH_INTERVAL 100000  // nano seconds

#define WRITE_SUCCESS 0
#define WRITE_BUSY 1
#define WRITE_ERROR -1


static char fifo_pathname[ PATH_MAX ];
static char pcap_file_pathname[ PATH_MAX ];
static bool output_to_pcap_file = false;
static bool launch_wireshark = true;
static bool launch_tshark = false;
static int outfile_fd = -1;
static uint64_t total = 0;
static uint64_t lost = 0;


// Special purpose header for telling extra information to wireshark
typedef struct message_pcap_dump_header {
  uint16_t dump_type;
  struct {
    uint32_t sec;
    uint32_t nsec;
  } sent_time;
  uint16_t app_name_len;
  uint16_t service_name_len;
  uint32_t data_len;
} __attribute__( ( packed ) ) message_pcap_dump_header;


// Structure defined in messenger.c
typedef struct message_header {
  uint8_t version;         // version = 0 (unused)
  uint8_t message_type;    // MESSAGE_TYPE_
  uint16_t tag;            // user defined
  uint32_t message_length; // message length including header
  uint8_t value[ 0 ];
} message_header;


static void
init_fifo_pathname() {
  assert( strlen( get_trema_tmp() ) + strlen( FIFO_NAME ) + 1 <= PATH_MAX );

  snprintf( fifo_pathname, PATH_MAX, "%s/%s", get_trema_tmp(), FIFO_NAME );
}


/**
 *  Write message to file.
 *  If it failed in writing whole message, param "packet" might be updated for remained.
 */
static int
write_to_file( buffer *packet ) {
  assert( packet != NULL && packet->data != NULL && packet->length != 0 );
  assert( outfile_fd >= 0 );

  int ret = write( outfile_fd, packet->data, packet->length );
  if ( ret < 0 ) {
    int err = errno;
    if ( err == EAGAIN || err == EWOULDBLOCK ) {
      return WRITE_BUSY;
    }
    
    error( "write error. errno = %d\n", err );
    return WRITE_ERROR;
  }

  if ( ret != ( int ) packet->length ) {
    packet->data = ( char * ) packet->data + ret;
    packet->length = packet->length - ( unsigned int ) ret;
    return WRITE_BUSY;
  }
  return WRITE_SUCCESS;
}


/**
 *  callback function to receive message from messenger.
 */
static void
dump_message( uint16_t tag, void *data, size_t len ) {
  char *app_name, *service_name;
  const char *type_str[] = { "sent", "received", "recv-connected", "recv-overflow", "recv-closed",
                             "send-connected", "send-refused", "send-overflow", "send-closed" };
  size_t pcap_dump_header_length;
  struct pcap_pkthdr pcap_header;
  message_dump_header *dump_hdr;
  message_pcap_dump_header *pcap_dump_hdr;
  message_header *hdr;
  buffer *packet;
  queue_return ret;

  assert( outfile_fd >= 0 );

  dump_hdr = data;

  app_name = ( char * ) ( dump_hdr + 1 );
  service_name = app_name + ntohs( dump_hdr->app_name_length );
  hdr = ( message_header * ) ( service_name + ntohs( dump_hdr->service_name_length ) );

  debug( "app: %s type: %s service_name: %s",
         app_name, type_str[ tag ], service_name );

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
  char *pcap_dump_service_name = pcap_dump_app_name + ntohs( pcap_dump_hdr->app_name_len );
  memcpy( pcap_dump_app_name, app_name, ntohs( pcap_dump_hdr->app_name_len ) );
  memcpy( pcap_dump_service_name, service_name, ntohs( pcap_dump_hdr->service_name_len ) );

  memset( &pcap_header, 0, sizeof( struct pcap_pkthdr ) );
  gettimeofday( &pcap_header.ts, NULL );

  len -= dump_header_length;

  pcap_header.caplen = pcap_dump_header_length + ntohl( pcap_dump_hdr->data_len );
  pcap_header.len = pcap_dump_header_length + ntohl( pcap_dump_hdr->data_len );

  packet = create_pcap_packet( &pcap_header, sizeof( struct pcap_pkthdr ),
                               pcap_dump_hdr, pcap_dump_header_length,
                               hdr, ntohl( pcap_dump_hdr->data_len ) );
  ret = push_pcap_packet( packet );
  if ( ret == QUEUE_FULL ) {
    error( "tremashark queue is full. packet is discarded.\n" );
    delete_pcap_packet( packet );
    lost++;
  }
  total++;

  fsync( outfile_fd );

  xfree( pcap_dump_hdr );
}


/**
 *  callback function to write pcap packets.
 */
static void
write_pcap_packet( void *user_data ) {
  UNUSED( user_data );

  for ( ; ; ) {
    buffer *packet;
    queue_return q_ret = pop_pcap_packet( &packet );
    if ( q_ret == QUEUE_EMPTY ) {
      break;
    }

    int ret = write_to_file( packet );
    switch ( ret ) {
    case WRITE_SUCCESS:
      delete_pcap_packet( packet );
      break;

    case WRITE_BUSY:
      push_pcap_packet_in_front( packet );
      return;

    case WRITE_ERROR:
      push_pcap_packet_in_front( packet );
      return;

    default:
      assert( 0 );
    }
  }

  return;
}


static void
set_timer_event() {
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
  return;
}


static void
init_pcap() {
  int ret;
  mode_t mode;
  struct pcap_file_header header;

  memset( &header, 0, sizeof( struct pcap_file_header ) );

  header.magic = 0xa1b2c3d4;
  header.version_major = PCAP_VERSION_MAJOR;
  header.version_minor = PCAP_VERSION_MINOR;
  header.thiszone = 0;
  header.sigfigs = 0;
  header.snaplen = UINT16_MAX; // FIXME
  header.linktype = DLT_USER0; // FIXME

  mode = ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

  if ( output_to_pcap_file ) {
    outfile_fd = open( pcap_file_pathname, O_RDWR | O_CREAT, mode );
    if ( outfile_fd < 0 ) {
      critical( "Failed to open a file (%s).", pcap_file_pathname );
      assert( 0 );
    }
  }
  else {
    ret = mkfifo( fifo_pathname, mode );
    if ( ret < 0 ) {
      critical( "Failed to create a named pipe." );
      assert( 0 );
    }

    outfile_fd = open( fifo_pathname, O_RDWR | O_APPEND | O_NONBLOCK );
    if ( outfile_fd < 0 ) {
      critical( "Failed to open a named pipe." );
      assert( 0 );
    }
  }

  ret = write( outfile_fd, &header, sizeof( struct pcap_file_header ) );

  if ( ret != sizeof( struct pcap_file_header ) ) {
    critical( "Failed to write a pcap header." );
    assert( 0 );
  }

  fsync( outfile_fd );
}


static void
finalize_pcap() {
  if ( outfile_fd >= 0 ) {
    close( outfile_fd );
    outfile_fd = -1;
  }

  unlink( fifo_pathname );
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
    }
    else if ( launch_tshark ) {
      execlp( TSHARK, "tshark", "-V", "-i", fifo_pathname, NULL );
    }
  }
}


static void
print_usage_and_exit() {
  fprintf( stderr, "Usage: tremashark [-p] [-t] [-w filename] [-s SERVICE_NAME]\n" );
  fprintf( stderr, "  Options:\n" );
  fprintf( stderr, "    -p: do not launch wireshark or tshark\n" );
  fprintf( stderr, "    -t: launch tshark instead of wireshark\n" );
  fprintf( stderr, "    -w: save messages to a pcap file\n" );
  fprintf( stderr, "    -s: specify service name\n" );
  exit( -1 );
}


int
main( int argc, char *argv[] ) {
  char *service_name = NULL;
  int opt;

  init_fifo_pathname();

  // Initialize the Trema world
  init_trema( &argc, &argv );

  while( 1 ) {
    opt = getopt( argc, argv, "s:tw:p" );

    if( opt < 0 ){
      break;
    }

    switch ( opt ){
    case 'p':
      launch_wireshark = false;
      launch_tshark = false;
      break;

    case 't':
      launch_wireshark = false;
      launch_tshark = true;
      break;

    case 's':
      if( optarg && service_name == NULL ){
        service_name = xmalloc( strlen( optarg ) + 1 );
        strcpy( service_name, optarg );
      }
      else {
        print_usage_and_exit();
      }
      break;

    case 'w':
      if( optarg ){
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

    default:
      print_usage_and_exit();
    }
  }

  // Set an event handler
  if ( service_name == NULL ) {
    add_message_received_callback( DEFAULT_DUMP_SERVICE_NAME, dump_message );
  }
  else {
    add_message_received_callback( service_name, dump_message );
    xfree( service_name );
  }

  // create queue for storing pcap packet
  create_queue();

  // Initialize an interface to wireshark
  init_pcap();

  // Start wireshark/tshark if necessary
  start_wireshark();

  // start timer event to write packet
  set_timer_event();
  
  // Main loop
  start_trema();

  // Cleanup
  finalize_pcap();
  delete_queue();

  if ( lost > 0 ) {
    error( "%llu/%llu messages lost.", lost, total );
  }

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
