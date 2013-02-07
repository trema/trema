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


#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "buffer.h"
#include "checks.h"
#include "event_handler.h"
#include "log.h"
#include "message_queue.h"
#include "openflow.h"
#include "openflow_switch_interface.h"
#include "secure_channel.h"
#include "timer.h"
#include "wrapper.h"


enum connection_state {
  INIT,
  CONNECTING,
  CONNECTED,
  DISCONNECTED,
};

static char connection_state_string[][ 13 ] = { "INIT", "CONNECTING", "CONNECTED", "DISCONNECTED" };


typedef struct {
  uint32_t ip;
  uint16_t port;
  int fd;
  int state;
  connected_handler connected_callback;
  disconnected_handler disconnected_callback;
} secure_channel_connection;


static secure_channel_connection connection = { 0, 0, -1, INIT, NULL, NULL };
static bool secure_channel_initialized = false;

static message_queue *send_queue = NULL;
static message_queue *recv_queue = NULL;

static const size_t RECEIVE_BUFFFER_SIZE = UINT16_MAX + sizeof( struct ofp_packet_in ) - 2;
static buffer *fragment_buf = NULL;


static void
transit_state( int state ) {
  switch ( connection.state ) {
  case INIT:
  {
    if ( state != CONNECTING ) {
      goto invalid_transition;
    }
    connection.state = state;
  }
  break;

  case CONNECTING:
  {
    if ( state != CONNECTED && state != CONNECTING ) {
      goto invalid_transition;
    }
  }
  break;

  case CONNECTED:
  {
    if ( state != DISCONNECTED && state != INIT ) {
      goto invalid_transition;
    }
  }
  break;

  case DISCONNECTED:
  {
    if ( state != CONNECTED ) {
      goto invalid_transition;
    }
  }
  break;

  default:
  {
    error( "Invalid state ( %d ).", connection.state );
    return;
  }
  break;
  }

  debug( "State transition: %s -> %s.",
         connection_state_string[ connection.state ],
         connection_state_string[ state ] );

  connection.state = state;

  return;

invalid_transition:
  error( "Invalid state transition ( %d -> %d ).", connection.state, state );
}


static void
clear_connection() {
  if ( connection.fd >= 0 ) {
    close( connection.fd );
    set_readable( connection.fd, false );
    set_writable( connection.fd, false );
    delete_fd_handler( connection.fd );
  }

  connection.fd = -1;
  connection.state = INIT;
}


static void reconnect( void *user_data );


static void
disconnected() {
  transit_state( DISCONNECTED );

  if ( connection.disconnected_callback != NULL ) {
    connection.disconnected_callback();
  }

  clear_connection();
}


static bool
recv_message_from_secure_channel() {
  assert( recv_queue != NULL );

  if ( recv_queue->length == 0 ) {
    return false;
  }

  buffer *message = dequeue_message( recv_queue );
  handle_secure_channel_message( message ); // FIXME: handle error properly
  free_buffer( message );

  return true;
}


static void
recv_from_secure_channel( int fd, void *user_data ) {
  UNUSED( fd );
  UNUSED( user_data );

  // all queued messages should be processed before receiving new messages from remote
  if ( recv_queue->length > 0 ) {
    return;
  }

  if ( fragment_buf == NULL ) {
    fragment_buf = alloc_buffer_with_length( RECEIVE_BUFFFER_SIZE );
  }

  size_t remaining_length = RECEIVE_BUFFFER_SIZE - fragment_buf->length;
  char *recv_buf = ( char * ) fragment_buf->data + fragment_buf->length;
  ssize_t recv_length = read( connection.fd, recv_buf, remaining_length );
  if ( recv_length < 0 ) {
    if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
      return;
    }
    error( "Receive error ( errno = %s [%d] ).", strerror( errno ), errno );
    return;
  }
  if ( recv_length == 0 ) {
    debug( "Connection closed by peer." );
    disconnected();
    reconnect( NULL );
    return;
  }
  fragment_buf->length += ( size_t ) recv_length;

  size_t read_total = 0;
  while ( fragment_buf->length >= sizeof( struct ofp_header ) ) {
    struct ofp_header *header = fragment_buf->data;
    uint16_t message_length = ntohs( header->length );
    if ( message_length > fragment_buf->length ) {
      break;
    }
    buffer *message = alloc_buffer_with_length( message_length );
    char *p = append_back_buffer( message, message_length );
    memcpy( p, fragment_buf->data, message_length );
    remove_front_buffer( fragment_buf, message_length );
    enqueue_message( recv_queue, message );
    read_total += message_length;
  }

  // remove headroom manually for next call
  if ( read_total > 0 ) {
    memmove( ( char * ) fragment_buf->data - read_total, fragment_buf->data, fragment_buf->length );
    fragment_buf->data = ( char * ) fragment_buf->data - read_total;
  }

  while ( recv_message_from_secure_channel() == true );
}


static void
flush_send_queue( int fd, void *user_data ) {
  UNUSED( fd );
  UNUSED( user_data );

  assert( send_queue != NULL );
  assert( connection.fd >= 0 );

  debug( "Flushing send queue ( length = %u ).", send_queue->length );

  set_writable( connection.fd, false );

  buffer *buf = NULL;
  while ( ( buf = peek_message( send_queue ) ) != NULL ) {
    ssize_t write_length = write( connection.fd, buf->data, buf->length );
    if ( write_length < 0 ) {
      if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
        set_writable( connection.fd, true );
        return;
      }
      error( "Failed to send a message to secure channel ( errno = %s [%d] ).",
             strerror( errno ), errno );
      return;
    }
    if ( ( size_t ) write_length < buf->length ) {
      remove_front_buffer( buf, ( size_t ) write_length );
      set_writable( connection.fd, true );
      return;
    }

    buf = dequeue_message( send_queue );
    free_buffer( buf );
  }
}


static void
connected() {
  transit_state( CONNECTED );

  set_fd_handler( connection.fd, recv_from_secure_channel, NULL, flush_send_queue, NULL );
  set_readable( connection.fd, true );
  set_writable( connection.fd, false );

  if ( connection.connected_callback != NULL ) {
    connection.connected_callback();
  }
}


static bool try_connect( void );

static void
reconnect( void *user_data ) {
  UNUSED( user_data );
  bool ret = try_connect();
  if ( ret == false ) {
    error( "Failed to reconnect." );
    clear_connection();
  }
}


static void
backoff() {
  if ( connection.fd >= 0 ) {
    close( connection.fd );
  }

  struct itimerspec spec = { { 0, 0 }, { 5, 0 } };
  add_timer_event_callback( &spec, reconnect, NULL );
}


static void
check_connected( void *user_data ) {
  UNUSED( user_data );

  debug( "Checking a connection ( fd = %d, ip = %#x, port = %u ).", connection.fd, connection.ip, connection.port );

  assert( secure_channel_initialized );
  assert( connection.fd >= 0 );

  set_writable( connection.fd, false );
  delete_fd_handler( connection.fd );

  int err = 0;
  socklen_t length = sizeof( error );
  int ret = getsockopt( connection.fd, SOL_SOCKET, SO_ERROR, &err, &length );
  if ( ret < 0 ) {
    error( "Failed to retrieve error code ( fd = %d, ret = %d, errno = %s [%d] ).",
           connection.fd, ret, strerror( errno ), errno );
    return;
  }

  switch ( err ) {
    case 0:
      connected();
      break;

    case EINTR:
    case EAGAIN:
    case ECONNREFUSED:
    case ENETUNREACH:
    case ETIMEDOUT:
      warn( "Failed to connect ( fd = %d, errno = %s [%d] ).", connection.fd, strerror( err ), err );
      backoff();
      return;

    case EINPROGRESS:
      set_fd_handler( connection.fd, NULL, NULL, ( event_fd_callback ) check_connected, NULL );
      set_writable( connection.fd, true );
      break;

    default:
      error( "Failed to connect ( fd = %d, errno = %s [%d] ).", connection.fd, strerror( err ), err );
      clear_connection();
      return;
  }
}


static bool
try_connect() {
  assert( connection.state != CONNECTED );

  int fd = socket( PF_INET, SOCK_STREAM, 0 );
  if ( fd < 0 ) {
    error( "Failed to create a socket ( ret = %d, errno = %s [%d] ).",
           fd, strerror( errno ), errno );
    return false;
  }

  int flag = 1;
  int ret = setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) );
  if ( ret < 0 ) {
    error( "Failed to set socket options ( fd = %d, ret = %d, errno = %s [%d] ).",
           fd, ret, strerror( errno ), errno );
    return false;
  }

  ret = fcntl( fd, F_SETFL, O_NONBLOCK );
  if ( ret < 0 ) {
    error( "Failed to enable non-blocking mode ( fd = %d, ret = %d, errno = %s [%d] ).",
           fd, ret, strerror( errno ), errno );

    close( fd );
    return false;
  }

  connection.fd = fd;

  struct sockaddr_in addr;
  memset( &addr, 0, sizeof( struct sockaddr_in ) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( connection.port );
  addr.sin_addr.s_addr = htonl( connection.ip );

  transit_state( CONNECTING );

  ret = connect( connection.fd, ( struct sockaddr * ) &addr, sizeof( struct sockaddr_in ) );
  if ( ret < 0 ) {
    switch ( errno ) {
      case EINTR:
      case EAGAIN:
      case ECONNREFUSED:
      case ENETUNREACH:
      case ETIMEDOUT:
        warn( "Failed to connect ( fd = %d, ret = %d, errno = %s [%d] ).",
              connection.fd, ret, strerror( errno ), errno );
        backoff();
        return true;

      case EINPROGRESS:
        break;

      default:
        error( "Failed to connect ( fd = %d, ret = %d, errno = %s [%d] ).",
               connection.fd, ret, strerror( errno ), errno );
        clear_connection();
        return false;
    }
  }

  set_fd_handler( connection.fd, NULL, NULL, ( event_fd_callback ) check_connected, NULL );
  set_writable( connection.fd, true );

  return true;
}


bool
init_secure_channel( uint32_t ip, uint16_t port, connected_handler connected_callback, disconnected_handler disconnected_callback ) {
  assert( !secure_channel_initialized );

  connection.ip = ip;
  connection.port = port;
  connection.fd = -1;
  connection.connected_callback = connected_callback;
  connection.disconnected_callback = disconnected_callback;

  bool ret = try_connect();
  if ( ret == false ) {
    clear_connection();
    return false;
  }

  send_queue = create_message_queue();
  recv_queue = create_message_queue();

  secure_channel_initialized = true;

  return true;
}


bool
finalize_secure_channel() {
  assert( secure_channel_initialized );

  clear_connection();

  if ( send_queue != NULL ) {
    delete_message_queue( send_queue );
    send_queue = NULL;
  }
  if ( recv_queue != NULL ) {
    delete_message_queue( recv_queue );
    recv_queue = NULL;
  }

  secure_channel_initialized = false;

  return true;
}


bool
send_message_to_secure_channel( buffer *message ) {
  assert( send_queue != NULL );
  assert( message != NULL );
  assert( message->length > 0 );
  assert( connection.state == CONNECTED );
  assert( connection.fd >= 0 );

  debug( "Enqueuing a message to send queue ( queue length = %u, message length = %zu ).",
        send_queue->length, message->length );

  if ( send_queue->length == 0 ) {
    set_writable( connection.fd, true );
  }

  buffer *duplicated = duplicate_buffer( message );
  enqueue_message( send_queue, duplicated );

  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
