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
#include <inttypes.h>
#include <linux/limits.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "doubly_linked_list.h"
#include "event_handler.h"
#include "hash_table.h"
#include "log.h"
#include "messenger.h"
#include "timer.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#define static

// Redirect socket functions to mock functions in the unit test.
#ifdef socket
#undef socket
#endif
#define socket mock_socket
extern int mock_socket( int domain, int type, int protocol );

#ifdef bind
#undef bind
#endif
#define bind mock_bind
extern int mock_bind( int sockfd, const struct sockaddr *addr, socklen_t addrlen );

#ifdef listen
#undef listen
#endif
#define listen mock_listen
extern int mock_listen( int sockfd, int backlog );

#ifdef close
#undef close
#endif
#define close mock_close
extern int mock_close( int fd );

#ifdef connect
#undef connect
#endif
#define connect mock_connect
extern int mock_connect( int sockfd, const struct sockaddr *addr, socklen_t addrlen );

#ifdef select
#undef select
#endif
#define select mock_select
extern int mock_select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout );

#ifdef accept
#undef accept
#endif
#define accept mock_accept
extern int mock_accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen );

#ifdef recv
#undef recv
#endif
#define recv mock_recv
extern ssize_t mock_recv( int sockfd, void *buf, size_t len, int flags );

#ifdef send
#undef send
#endif
#define send mock_send
extern ssize_t mock_send( int sockfd, const void *buf, size_t len, int flags );

#ifdef setsockopt
#undef setsockopt
#endif
#define setsockopt mock_setsockopt
extern int mock_setsockopt( int s, int level, int optname, const void *optval, socklen_t optlen );

#ifdef clock_gettime
#undef clock_gettime
#endif
#define clock_gettime mock_clock_gettime
extern int mock_clock_gettime( clockid_t clk_id, struct timespec *tp );

#ifdef error
#undef error
#endif
#define error mock_error
extern void mock_error( const char *format, ... );

#ifdef debug
#undef debug
#endif
#define debug mock_debug
extern void mock_debug( const char *format, ... );

#ifdef warn
#undef warn
#endif
#define warn mock_warn
extern void mock_warn( const char *format, ... );

#ifdef add_periodic_event_callback
#undef add_periodic_event_callback
#endif
#define add_periodic_event_callback mock_add_periodic_event_callback
extern bool mock_add_periodic_event_callback( const time_t seconds, void ( *callback )( void *user_data ), void *user_data );

#endif // UNIT_TESTING


enum {
  MESSAGE_TYPE_NOTIFY,
  MESSAGE_TYPE_REQUEST,
  MESSAGE_TYPE_REPLY,
};

typedef struct message_buffer {
  void *buffer;
  size_t data_length;
  size_t size;
  size_t head_offset;
} message_buffer;

typedef struct messenger_socket {
  int fd;
} messenger_socket;

typedef struct messenger_context {
  uint32_t transaction_id;
  int life_count;
  void *user_data;
} messenger_context;

typedef struct receive_queue_callback {
  void  *function;
  uint8_t message_type;
} receive_queue_callback;

typedef struct receive_queue {
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  dlist_element *message_callbacks;
  int listen_socket;
  struct sockaddr_un listen_addr;
  dlist_element *client_sockets;
  message_buffer *buffer;
} receive_queue;

typedef struct send_queue {
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  int server_socket;
  int refused_count;
  struct timespec reconnect_interval;
  struct sockaddr_un server_addr;
  message_buffer *buffer;
  bool running_timer;
  uint32_t overflow;
  uint64_t overflow_total_length;
  int socket_buffer_size;
} send_queue;


#define MESSENGER_RECV_BUFFER 100000
static const uint32_t messenger_send_queue_length = MESSENGER_RECV_BUFFER * 4;
static const uint32_t messenger_send_length_for_flush = MESSENGER_RECV_BUFFER;
static const uint32_t messenger_bucket_size = MESSENGER_RECV_BUFFER;
static const uint32_t messenger_recv_queue_length = MESSENGER_RECV_BUFFER * 2;
static const uint32_t messenger_recv_queue_reserved = MESSENGER_RECV_BUFFER;

char socket_directory[ PATH_MAX ];
static bool initialized = false;
static bool finalized = false;
static hash_table *receive_queues = NULL;
static hash_table *send_queues = NULL;
static hash_table *context_db = NULL;
static char *_dump_service_name = NULL;
static char *_dump_app_name = NULL;
static uint32_t last_transaction_id = 0;

static void on_accept( int fd, void *data );
static void on_recv( int fd, void *data );
static void on_send_write( int fd, void *data );
static void on_send_read( int fd, void *data );

static void
_delete_context( void *key, void *value, void *user_data ) {
  assert( value != NULL );
  UNUSED( key );
  UNUSED( user_data );
  messenger_context *context = value;

  debug( "Deleting a context ( transaction_id = %#x, life_count = %d, user_data = %p ).",
         context->transaction_id, context->life_count, context->user_data );

  delete_hash_entry( context_db, &context->transaction_id );
  xfree( context );
}


static void
delete_context( messenger_context *context ) {
  assert( context != NULL );

  _delete_context( &context->transaction_id, context, NULL );
}


static void
_age_context( void *key, void *value, void *user_data ) {
  assert( value != NULL );
  UNUSED( key );
  UNUSED( user_data );
  messenger_context *context = value;
  context->life_count--;
  if ( context->life_count <= 0 ) {
    delete_context( context );
  }
}


static void
age_context_db( void *user_data ) {
  UNUSED( user_data );

  debug( "Aging context database ( context_db = %p ).", context_db );

  foreach_hash( context_db, _age_context, NULL );
}


bool
init_messenger( const char *working_directory ) {
  assert( working_directory != NULL );

  init_event_handler();

  if ( initialized ) {
    warn( "Messenger is already initialized." );
    return true;
  }

  strcpy( socket_directory, working_directory );

  receive_queues = create_hash_with_size( compare_string, hash_string, 8 );
  send_queues = create_hash_with_size( compare_string, hash_string, 8 );
  context_db = create_hash_with_size( compare_uint32, hash_uint32, 128 );

  initialized = true;
  finalized = false;

  return initialized;
}


static void
delete_context_db( void ) {
  debug( "Deleting context database ( context_db = %p ).", context_db );

  if ( context_db != NULL ) {
    foreach_hash( context_db, _delete_context, NULL );
    delete_hash( context_db );
    context_db = NULL;
  }
}


static void
free_message_buffer( message_buffer *buf ) {
  assert( buf != NULL );

  xfree( buf->buffer );
  xfree( buf );
}


static void *
get_message_buffer_head( message_buffer *buf ) {
  return ( char * ) buf->buffer + buf->head_offset;
}


static void
delete_send_queue( send_queue *sq ) {
  assert( NULL != sq );

  debug( "Deleting a send queue ( service_name = %s, fd = %d ).", sq->service_name, sq->server_socket );

  free_message_buffer( sq->buffer );
  if ( sq->server_socket != -1 ) {
    set_readable( sq->server_socket, false );
    set_writable( sq->server_socket, false );
    delete_fd_handler( sq->server_socket );

    close( sq->server_socket );
  }
  if ( send_queues != NULL ) {
    delete_hash_entry( send_queues, sq->service_name );
  }
  else {
    error( "All send queues are already deleted or not created yet." );
  }
  xfree( sq );
}


static void
delete_all_send_queues() {
  hash_iterator iter;
  hash_entry *e;

  debug( "Deleting all send queues ( send_queues = %p ).", send_queues );

  if ( send_queues != NULL ) {
    init_hash_iterator( send_queues, &iter );
    while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
      delete_send_queue( e->value );
    }
    delete_hash( send_queues );
    send_queues = NULL;
  }
  else {
    error( "All send queues are already deleted or not created yet." );
  }
}


static void
send_dump_message( uint16_t dump_type, const char *service_name, const void *data, uint32_t data_len ) {
  assert( service_name != NULL );

  debug( "Sending a dump message ( dump_type = %#x, service_name = %s, data = %p, data_len = %u ).",
         dump_type, service_name, data, data_len );

  size_t service_name_len, app_name_len;
  char *dump_buf, *p;
  message_dump_header *dump_hdr;
  size_t dump_buf_len;

  if ( _dump_service_name == NULL ) {
    debug( "Dump service name is not set." );
    return;
  }
  if ( strcmp( service_name, _dump_service_name ) == 0 ) {
    debug( "Source service name and destination service name are the same ( service name = %s ).", service_name );
    return;
  }

  struct timespec now;
  if ( clock_gettime( CLOCK_REALTIME, &now ) == -1 ) {
    error( "Failed to retrieve system-wide real-time clock ( %s [%d] ).", strerror( errno ), errno );
    return;
  }

  service_name_len = strlen( service_name ) + 1;
  app_name_len = strlen( _dump_app_name ) + 1;
  dump_buf_len = sizeof( message_dump_header ) + app_name_len + service_name_len + data_len;
  dump_buf = xmalloc( dump_buf_len );
  dump_hdr = ( message_dump_header * ) dump_buf;

  // header
  dump_hdr->sent_time.sec = htonl( ( uint32_t ) now.tv_sec );
  dump_hdr->sent_time.nsec = htonl( ( uint32_t ) now.tv_nsec );
  dump_hdr->app_name_length = htons( ( uint16_t ) app_name_len );
  dump_hdr->service_name_length = htons( ( uint16_t ) service_name_len );
  dump_hdr->data_length = htonl( data_len );

  // app name
  p = dump_buf;
  p += sizeof( message_dump_header );
  memcpy( p, _dump_app_name, app_name_len );

  // service name
  p += app_name_len;
  memcpy( p, service_name, service_name_len );

  // data
  p += service_name_len;
  memcpy( p, data, data_len );

  // send
  send_message( _dump_service_name, dump_type, dump_buf, dump_buf_len );

  xfree( dump_buf );
}


/**
 * closes accepted sockets and listening socket, and releases memories.
 */
static void
delete_receive_queue( void *service_name, void *_rq, void *user_data ) {
  debug( "Deleting a receive queue ( service_name = %s, _rq = %p, user_data = %p ).", ( char * ) service_name, _rq, user_data );

  receive_queue *rq = _rq;
  messenger_socket *client_socket;
  dlist_element *element;
  receive_queue_callback *cb;

  assert( rq != NULL );
  for ( element = rq->message_callbacks->next; element; element = element->next ) {
    cb = element->data;
    debug( "Deleting a callback ( function = %p, message_type = %#x ).", cb->function, cb->message_type );
    xfree( cb );
  }
  delete_dlist( rq->message_callbacks );

  for ( element = rq->client_sockets->next; element; element = element->next ) {
    client_socket = element->data;

    debug( "Closing a client socket ( fd = %d ).", client_socket->fd );

    set_readable( client_socket->fd, false );
    delete_fd_handler( client_socket->fd );

    close( client_socket->fd );
    xfree( client_socket );
    send_dump_message( MESSENGER_DUMP_RECV_CLOSED, rq->service_name, NULL, 0 );
  }
  delete_dlist( rq->client_sockets );

  set_readable( rq->listen_socket, false );
  delete_fd_handler( rq->listen_socket );

  close( rq->listen_socket );
  free_message_buffer( rq->buffer );
  unlink( rq->listen_addr.sun_path );

  if ( receive_queues != NULL ) {
    delete_hash_entry( receive_queues, rq->service_name );
  }
  else {
    error( "All receive queues are already deleted or not created yet." );
  }
  xfree( rq );
}


static void
delete_all_receive_queues() {
  debug( "Deleting all receive queues ( receive_queues = %p ).", receive_queues );

  if ( receive_queues != NULL ) {
    foreach_hash( receive_queues, delete_receive_queue, NULL );
    delete_hash( receive_queues );
    receive_queues = NULL;
  }
  else {
    error( "All receive queues are already deleted or not created yet." );
  }
}


bool
finalize_messenger() {
  debug( "Finalizing messenger." );

  if ( !initialized ) {
    warn( "Messenger is not initialized yet." );
    return false;
  }
  if ( finalized ) {
    warn( "Messenger is already finalized." );
    return true;
  }

  if ( messenger_dump_enabled() ) {
    stop_messenger_dump();
  }
  if ( receive_queues != NULL ) {
    delete_all_receive_queues();
  }
  if ( send_queues != NULL ) {
    delete_all_send_queues();
  }
  if ( context_db != NULL ) {
    delete_context_db();
  }

  initialized = false;
  finalized = true;

  finalize_event_handler();

  return true;
}


static message_buffer *
create_message_buffer( size_t size ) {
  message_buffer *buf = xmalloc( sizeof( message_buffer ) );

  buf->buffer = xmalloc( size );
  buf->size = size;
  buf->data_length = 0;
  buf->head_offset = 0;

  return buf;
}


static receive_queue *
create_receive_queue( const char *service_name ) {
  assert( service_name != NULL );
  assert( strlen( service_name ) < MESSENGER_SERVICE_NAME_LENGTH );

  debug( "Creating a receive queue ( service_name = %s ).", service_name );

  assert( receive_queues != NULL );
  receive_queue *rq = lookup_hash_entry( receive_queues, service_name );
  if ( rq != NULL ) {
    warn( "Receive queue for %s is already created.", service_name );
    return rq;
  }

  rq = xmalloc( sizeof( receive_queue ) );
  memset( rq->service_name, 0, MESSENGER_SERVICE_NAME_LENGTH );
  strncpy( rq->service_name, service_name, MESSENGER_SERVICE_NAME_LENGTH );

  memset( &rq->listen_addr, 0, sizeof( struct sockaddr_un ) );
  rq->listen_addr.sun_family = AF_UNIX;
  sprintf( rq->listen_addr.sun_path, "%s/trema.%s.sock", socket_directory, service_name );
  debug( "Set sun_path to %s.", rq->listen_addr.sun_path );

  rq->listen_socket = socket( AF_UNIX, SOCK_SEQPACKET, 0 );
  if ( rq->listen_socket == -1 ) {
    error( "Failed to call socket ( errno = %s [%d] ).", strerror( errno ), errno );
    xfree( rq );
    return NULL;
  }

  unlink( rq->listen_addr.sun_path ); // FIXME: handle error correctly

  int ret;
  ret = bind( rq->listen_socket, ( struct sockaddr * ) &rq->listen_addr, sizeof( struct sockaddr_un ) );
  if ( ret == -1 ) {
    error( "Failed to bind ( fd = %d, sun_path = %s, errno = %s [%d] ).",
           rq->listen_socket, rq->listen_addr.sun_path, strerror( errno ), errno );
    close( rq->listen_socket );
    xfree( rq );
    return NULL;
  }

  ret = listen( rq->listen_socket, SOMAXCONN );
  if ( ret == -1 ) {
    error( "Failed to listen ( fd = %d, sun_path = %s, errno = %s [%d] ).",
           rq->listen_socket, rq->listen_addr.sun_path, strerror( errno ), errno );
    close( rq->listen_socket );
    xfree( rq );
    return NULL;
  }

  ret = fcntl( rq->listen_socket, F_SETFL, O_NONBLOCK );
  if ( ret < 0 ) {
    error( "Failed to set O_NONBLOCK ( %s [%d] ).", strerror( errno ), errno );
    close( rq->listen_socket );
    xfree( rq );
    return NULL;
  }

  set_fd_handler( rq->listen_socket, on_accept, rq, NULL, NULL );
  set_readable( rq->listen_socket, true );

  rq->message_callbacks = create_dlist();
  rq->client_sockets = create_dlist();
  rq->buffer = create_message_buffer( messenger_recv_queue_length );

  insert_hash_entry( receive_queues, rq->service_name, rq );

  return rq;
}


static bool
add_message_callback( const char *service_name, uint8_t message_type, void *callback ) {
  assert( receive_queues != NULL );
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Adding a message callback ( service_name = %s, message_type = %#x, callback = %p ).",
         service_name, message_type, callback );

  receive_queue *rq = lookup_hash_entry( receive_queues, service_name );
  if ( rq == NULL ) {
    debug( "No receive queue found. Creating." );
    rq = create_receive_queue( service_name );
    if ( rq == NULL ) {
      error( "Failed to create a receive queue." );
      return false;
    }
  }

  if ( message_type == MESSAGE_TYPE_REQUEST || message_type == MESSAGE_TYPE_REPLY ) {
    for ( dlist_element *e = rq->message_callbacks->next; e; e = e->next ) {
      receive_queue_callback *cb = e->data;
      if ( cb->message_type == message_type ) {
        warn( "Multiple message_requested/replied handler is not supported. ( service_name = %s, message_type = %#x, callback = %p )",
              service_name, message_type, callback );
        break;
      }
    }
  }

  receive_queue_callback *cb = xmalloc( sizeof( receive_queue_callback ) );
  cb->message_type = message_type;
  cb->function = callback;
  insert_after_dlist( rq->message_callbacks, cb );

  return true;
}


static bool
_add_message_received_callback( const char *service_name, const callback_message_received callback ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Adding a message received callback ( service_name = %s, callback = %p ).",
         service_name, callback );

  return add_message_callback( service_name, MESSAGE_TYPE_NOTIFY, callback );
}
bool ( *add_message_received_callback )( const char *service_name, const callback_message_received function ) = _add_message_received_callback;


static bool
_add_message_requested_callback( const char *service_name,
                                 void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Adding a message requested callback ( service_name = %s, callback = %p ).",
         service_name, callback );

  return add_message_callback( service_name, MESSAGE_TYPE_REQUEST, callback );
}
bool ( *add_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) = _add_message_requested_callback;


static bool
_add_message_replied_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Adding a message replied callback ( service_name = %s, callback = %p ).",
         service_name, callback );

  return add_message_callback( service_name, MESSAGE_TYPE_REPLY, callback );
}
bool ( *add_message_replied_callback )( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) = _add_message_replied_callback;


static bool
delete_message_callback( const char *service_name, uint8_t message_type, void ( *callback ) ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Deleting a message callback ( service_name = %s, message_type = %#x, callback = %p ).",
         service_name, message_type, callback );

  if ( receive_queues == NULL ) {
    debug( "All receive queues are already deleted or not created yet." );
    return false;
  }

  receive_queue *rq = lookup_hash_entry( receive_queues, service_name );
  receive_queue_callback *cb;

  if ( NULL != rq ) {
    dlist_element *e;
    for ( e = rq->message_callbacks->next; e; e = e->next ) {
      cb = e->data;
      if ( ( cb->function == callback ) && ( cb->message_type == message_type ) ) {
        debug( "Deleting a callback ( message_type = %#x, callback = %p ).", message_type, callback );
        xfree( cb );
        delete_dlist_element( e );
        if ( rq->message_callbacks->next == NULL ) {
          debug( "No more callback for message_type = %#x.", message_type );
          delete_receive_queue( rq->service_name, rq, NULL );
        }
        return true;
      }
    }
  }

  error( "No registered message callback found." );

  return false;
}


static bool
_delete_message_received_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len ) ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Deleting a message received callback ( service_name = %s, callback = %p ).",
         service_name, callback );

  return delete_message_callback( service_name, MESSAGE_TYPE_NOTIFY, callback );
}
bool ( *delete_message_received_callback )( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len ) ) = _delete_message_received_callback;


static bool
_delete_message_requested_callback( const char *service_name,
  void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Deleting a message requested callback ( service_name = %s, callback = %p ).",
         service_name, callback );

  return delete_message_callback( service_name, MESSAGE_TYPE_REQUEST, callback );
}
bool ( *delete_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) = _delete_message_requested_callback;


static bool
_delete_message_replied_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  assert( service_name != NULL );
  assert( callback != NULL );

  debug( "Deleting a message replied callback ( service_name = %s, callback = %p ).",
         service_name, callback );

  return delete_message_callback( service_name, MESSAGE_TYPE_REPLY, callback );
}
bool ( *delete_message_replied_callback )( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) = _delete_message_replied_callback;


static bool
rename_message_callback( const char *old_service_name, const char *new_service_name ) {
  assert( old_service_name != NULL );
  assert( new_service_name != NULL );
  assert( receive_queues != NULL );

  receive_queue *old_rq = lookup_hash_entry( receive_queues, old_service_name );
  receive_queue *new_rq = lookup_hash_entry( receive_queues, new_service_name );
  dlist_element *element;
  receive_queue_callback *cb;

  if ( old_rq == NULL ) {
    error( "No receive queue for old service name ( %s ) found.", old_service_name );
    return false;
  }
  else if ( new_rq != NULL ) {
    error( "Receive queue for new service name ( %s ) is already created.", new_service_name );
    return false;
  }

  for ( element = old_rq->message_callbacks->next; element; element = element->next ) {
    cb = element->data;
    add_message_callback( new_service_name, cb->message_type, cb->function );
  }

  delete_receive_queue( old_rq->service_name, old_rq, NULL );

  return true;
}

static bool
_rename_message_received_callback( const char *old_service_name, const char *new_service_name ) {
  assert( old_service_name != NULL );
  assert( new_service_name != NULL );
  assert( receive_queues != NULL );

  debug( "Renaming a message received callback ( old_service_name = %s, new_service_name = %s ).",
         old_service_name, new_service_name );

  return rename_message_callback( old_service_name, new_service_name );
}
bool ( *rename_message_received_callback )( const char *old_service_name, const char *new_service_name ) = _rename_message_received_callback;


static bool
_rename_message_requested_callback( const char *old_service_name, const char *new_service_name ) {
  assert( old_service_name != NULL );
  assert( new_service_name != NULL );
  assert( receive_queues != NULL );

  debug( "Renaming a message requested callback ( old_service_name = %s, new_service_name = %s ).",
         old_service_name, new_service_name );

  return rename_message_callback( old_service_name, new_service_name );
}
bool ( *rename_message_requested_callback )( const char *old_service_name, const char *new_service_name ) = _rename_message_requested_callback;


static size_t
message_buffer_remain_bytes( message_buffer *buf ) {
  assert( buf != NULL );

  return buf->size - buf->data_length;
}


/**
 * connects send_queue to the service
 * return value: -1:error, 0:refused (retry), 1:connected
 */
static int
send_queue_connect( send_queue *sq ) {
  assert( sq != NULL );

  sq->running_timer = false;
  if ( ( sq->server_socket = socket( AF_UNIX, SOCK_SEQPACKET, 0 ) ) == -1 ) {
    error( "Failed to call socket ( errno = %s [%d] ).", strerror( errno ), errno );
    return -1;
  }

  if ( geteuid() == 0 ) {
    int wmem_size = 1048576;
    int ret = setsockopt( sq->server_socket, SOL_SOCKET, SO_SNDBUFFORCE, ( const void * ) &wmem_size, ( socklen_t ) sizeof( wmem_size ) );
    if ( ret < 0 ) {
      error( "Failed to set SO_SNDBUFFORCE to %d ( %s [%d] ).", wmem_size, strerror( errno ), errno );
      close( sq->server_socket );
      sq->server_socket = -1;
      return -1;
    }
  }
  int ret = fcntl( sq->server_socket, F_SETFL, O_NONBLOCK );
  if ( ret < 0 ) {
    error( "Failed to set O_NONBLOCK ( %s [%d] ).", strerror( errno ), errno );
    close( sq->server_socket );
    sq->server_socket = -1;
    return -1;
  }

  if ( connect( sq->server_socket, ( struct sockaddr * ) &sq->server_addr, sizeof( struct sockaddr_un ) ) == -1 ) {
    debug( "Connection refused ( service_name = %s, sun_path = %s, fd = %d, errno = %s [%d] ).",
           sq->service_name, sq->server_addr.sun_path, sq->server_socket, strerror( errno ), errno );

    send_dump_message( MESSENGER_DUMP_SEND_REFUSED, sq->service_name, NULL, 0 );
    close( sq->server_socket );
    sq->server_socket = -1;

    return 0;
  }

  set_fd_handler( sq->server_socket, on_send_read, sq, &on_send_write, sq );
  set_readable( sq->server_socket, true );

  if ( sq->buffer != NULL && sq->buffer->data_length >= sizeof( message_header ) ) {
    set_writable( sq->server_socket, true );
  }

  debug( "Connection established ( service_name = %s, sun_path = %s, fd = %d ).",
         sq->service_name, sq->server_addr.sun_path, sq->server_socket );

  socklen_t optlen = sizeof( sq->socket_buffer_size );
  if ( getsockopt( sq->server_socket, SOL_SOCKET, SO_SNDBUF, &sq->socket_buffer_size, &optlen ) == -1 ) {
    sq->socket_buffer_size = 0;
  }

  send_dump_message( MESSENGER_DUMP_SEND_CONNECTED, sq->service_name, NULL, 0 );

  return 1;
}


static int send_queue_connect_timer( send_queue *sq );

static int
send_queue_connect_timeout( send_queue *sq ) {
  sq->running_timer = false;
  return send_queue_connect_timer( sq );
}

// Remember to clean up timer if we delete the send_queue.

static int
send_queue_connect_timer( send_queue *sq ) {
  struct itimerspec interval;
  if ( sq->server_socket != -1 ) {
    return 1;
  }
  if ( sq->running_timer ) {
    sq->running_timer = false;
    delete_timer_event( ( timer_callback ) send_queue_connect_timeout, sq );
  }

  int ret = send_queue_connect( sq );

  switch ( ret ) {
  case -1:
    // Print an error, and find a better way of indicating the send
    // queue has an error.
    sq->reconnect_interval.tv_sec = -1;
    sq->reconnect_interval.tv_nsec = 0;
    return -1;

  case 0:
    // Try again later.
    sq->refused_count++;
    sq->reconnect_interval.tv_sec = ( 1 << ( sq->refused_count > 4 ? 4 : sq->refused_count - 1 ) );

    interval.it_interval.tv_sec = 0;
    interval.it_interval.tv_nsec = 0;
    interval.it_value = sq->reconnect_interval;
    add_timer_event_callback( &interval, ( void ( * )( void * ) ) send_queue_connect_timeout, ( void * ) sq );
    sq->running_timer = true;

    debug( "refused_count = %d, reconnect_interval = %" PRIu64 ".", sq->refused_count, ( int64_t ) sq->reconnect_interval.tv_sec );
    return 0;

  case 1:
    // Success.
    sq->refused_count = 0;
    sq->reconnect_interval.tv_sec = 0;
    sq->reconnect_interval.tv_nsec = 0;
    return 1;

  default:
    die( "Got invalid value from send_queue_connect_timer( send_queue* )." );
  }

  return -1;
}


static int
send_queue_try_connect( send_queue *sq ) {
  // TODO: Add a proper check for this.
  if ( sq->reconnect_interval.tv_sec != 0 ) {
    return 0;
  }

  return send_queue_connect_timer( sq );
}


/**
 * creates send_queue and connects to specified service name.
 */
static send_queue *
create_send_queue( const char *service_name ) {
  assert( service_name != NULL );

  debug( "Creating a send queue ( service_name = %s ).", service_name );

  send_queue *sq;

  assert( send_queues != NULL );

  sq = lookup_hash_entry( send_queues, service_name );
  if ( NULL != sq ) {
    warn( "Send queue for %s is already created.", service_name );
    return sq;
  }

  sq = xmalloc( sizeof( send_queue ) );
  memset( sq->service_name, 0, MESSENGER_SERVICE_NAME_LENGTH );
  strncpy( sq->service_name, service_name, MESSENGER_SERVICE_NAME_LENGTH );

  memset( &sq->server_addr, 0, sizeof( struct sockaddr_un ) );
  sq->server_addr.sun_family = AF_UNIX;
  sprintf( sq->server_addr.sun_path, "%s/trema.%s.sock", socket_directory, service_name );
  debug( "Set sun_path to %s.", sq->server_addr.sun_path );

  sq->server_socket = -1;
  sq->buffer = NULL;
  sq->refused_count = 0;
  sq->reconnect_interval.tv_sec = 0;
  sq->reconnect_interval.tv_nsec = 0;
  sq->running_timer = false;
  sq->overflow = 0;
  sq->overflow_total_length = 0;
  sq->socket_buffer_size = 0;

  if ( send_queue_try_connect( sq ) == -1 ) {
    xfree( sq );
    error( "Failed to create a send queue for %s.", service_name );
    return NULL;
  }

  sq->buffer = create_message_buffer( messenger_send_queue_length );

  insert_hash_entry( send_queues, sq->service_name, sq );

  return sq;
}


static bool
write_message_buffer( message_buffer *buf, const void *data, size_t len ) {
  assert( buf != NULL );

  if ( message_buffer_remain_bytes( buf ) < len ) {
    return false;
  }

  if ( ( buf->head_offset + buf->data_length + len ) <= buf->size ) {
    memcpy( ( char * ) get_message_buffer_head( buf ) + buf->data_length, data, len );
  }
  else {
    memmove( buf->buffer, ( char * ) get_message_buffer_head( buf ), buf->data_length );
    buf->head_offset = 0;
    memcpy( ( char * ) buf->buffer + buf->data_length, data, len );
  }
  buf->data_length += len;

  return true;
}


static bool
push_message_to_send_queue( const char *service_name, const uint8_t message_type, const uint16_t tag, const void *data, size_t len ) {
  assert( service_name != NULL );

  debug( "Pushing a message to send queue ( service_name = %s, message_type = %#x, tag = %#x, data = %p, len = %zu ).",
         service_name, message_type, tag, data, len );

  message_header header;

  if ( send_queues == NULL ) {
    error( "All send queues are already deleted or not created yet." );
    return false;
  }

  send_queue *sq = lookup_hash_entry( send_queues, service_name );

  if ( NULL == sq ) {
    sq = create_send_queue( service_name );
    assert( sq != NULL );
  }

  header.version = 0;
  header.message_type = message_type;
  header.tag = htons( tag );
  uint32_t length = ( uint32_t ) ( sizeof( message_header ) + len );
  header.message_length = htonl( length );

  if ( message_buffer_remain_bytes( sq->buffer ) < length ) {
    if ( sq->overflow == 0 ) {
      warn( "Could not write a message to send queue due to overflow ( service_name = %s, fd = %u, length = %u ).", sq->service_name, sq->server_socket, length );
    }
    ++sq->overflow;
    sq->overflow_total_length += length;
    send_dump_message( MESSENGER_DUMP_SEND_OVERFLOW, sq->service_name, NULL, 0 );
    return false;
  }
  if ( sq->overflow > 1 ) {
    warn( "Could not write a message to send queue due to overflow ( service_name = %s, fd = %u, count = %u, total length = %" PRIu64 " ).", sq->service_name, sq->server_socket, sq->overflow, sq->overflow_total_length );
  }
  sq->overflow = 0;
  sq->overflow_total_length = 0;

  write_message_buffer( sq->buffer, &header, sizeof( message_header ) );
  write_message_buffer( sq->buffer, data, len );

  if ( sq->server_socket == -1 ) {
    debug( "Tried to send message on closed send queue, connecting..." );

    send_queue_try_connect( sq );
    return true;
  }

  set_writable( sq->server_socket, true );
  if ( sq->buffer->data_length > messenger_send_length_for_flush ) {
    on_send_write( sq->server_socket, sq );
  }
  return true;
}


static bool
_send_message( const char *service_name, const uint16_t tag, const void *data, size_t len ) {
  assert( service_name != NULL );

  debug( "Sending a message ( service_name = %s, tag = %#x, data = %p, len = %zu ).",
         service_name, tag, data, len );

  return push_message_to_send_queue( service_name, MESSAGE_TYPE_NOTIFY, tag, data, len );
}
bool ( *send_message )( const char *service_name, const uint16_t tag, const void *data, size_t len ) = _send_message;


static messenger_context *
insert_context( void *user_data ) {
  messenger_context *context = xmalloc( sizeof( messenger_context ) );

  context->transaction_id = ++last_transaction_id;
  context->life_count = 10;
  context->user_data = user_data;

  debug( "Inserting a new context ( transaction_id = %#x, life_count = %d, user_data = %p ).",
         context->transaction_id, context->life_count, context->user_data );

  messenger_context *old = insert_hash_entry( context_db, &context->transaction_id, context );
  if ( old != NULL ) {
    delete_context( old );
  }

  return context;
}


static bool
_send_request_message( const char *to_service_name, const char *from_service_name, const uint16_t tag, const void *data, size_t len, void *user_data ) {
  assert( to_service_name != NULL );
  assert( from_service_name != NULL );

  debug( "Sending a request message ( to_service_name = %s, from_service_name = %s, tag = %#x, data = %p, len = %zu, user_data = %p ).",
         to_service_name, from_service_name, tag, data, len, user_data );

  char *request_data, *p;
  size_t from_service_name_len = strlen( from_service_name ) + 1;
  size_t handle_len = sizeof( messenger_context_handle ) + from_service_name_len;
  messenger_context *context;
  messenger_context_handle *handle;
  bool return_value;

  context = insert_context( user_data );

  request_data = xmalloc( handle_len + len );
  handle = ( messenger_context_handle * ) request_data;
  handle->transaction_id = htonl( context->transaction_id );
  handle->service_name_len = htons( ( uint16_t ) from_service_name_len );
  strcpy( handle->service_name, from_service_name );
  p = request_data + handle_len;
  memcpy( p, data, len );

  return_value = push_message_to_send_queue( to_service_name, MESSAGE_TYPE_REQUEST, tag, request_data, handle_len + len );

  xfree( request_data );

  return return_value;
}
bool ( *send_request_message )( const char *to_service_name, const char *from_service_name, const uint16_t tag, const void *data, size_t len, void *user_data ) = _send_request_message;


static bool
_send_reply_message( const messenger_context_handle *handle, const uint16_t tag, const void *data, size_t len ) {
  assert( handle != NULL );

  debug( "Sending a reply message ( handle = [ transaction_id = %#x, service_name_len = %u, service_name = %s ], "
         "tag = %#x, data = %p, len = %zu ).",
         handle->transaction_id, handle->service_name_len, handle->service_name, tag, data, len );

  char *reply_data;
  messenger_context_handle *reply_handle;
  bool return_value;

  reply_data = xmalloc( sizeof( messenger_context_handle ) + len );
  reply_handle = ( messenger_context_handle * ) reply_data;
  reply_handle->transaction_id = htonl( handle->transaction_id );
  reply_handle->service_name_len = htons( 0 );
  memcpy( reply_handle->service_name, data, len );

  return_value = push_message_to_send_queue( handle->service_name, MESSAGE_TYPE_REPLY, tag, reply_data, sizeof( messenger_context_handle ) + len );

  xfree( reply_data );

  return return_value;
}
bool ( *send_reply_message )( const messenger_context_handle *handle, const uint16_t tag, const void *data, size_t len ) = _send_reply_message;


static bool
_clear_send_queue( const char *service_name ) {
  assert( service_name != NULL );

  debug( "Deleting all messages from send queue ( service_name = %s ).", service_name );

  if ( send_queues == NULL ) {
    error( "All send queues are already deleted or not created yet." );
    return false;
  }

  send_queue *sq = lookup_hash_entry( send_queues, service_name );

  if ( NULL == sq ) {
    error( "Send queue is already deleted or not created yet ( service_name = %s ).", service_name );
    return false;
  }
  if ( NULL == sq->buffer ) {
    error( "Message buffer is already deleted or not created yet ( send_queue = %p, service_name = %s ).",
           sq, service_name );
    return false;
  }

  if ( sq->buffer->data_length > 0 ) {
    set_writable( sq->server_socket, false );
  }

  sq->buffer->head_offset = 0;
  sq->buffer->data_length = 0;

  return true;
}
bool ( *clear_send_queue )( const char *service_name ) = _clear_send_queue;


static void
number_of_send_queue( int *connected_count, int *sending_count, int *reconnecting_count, int *closed_count ) {
  assert( connected_count != NULL );
  assert( sending_count != NULL );
  assert( reconnecting_count != NULL );
  assert( closed_count != NULL );

  debug( "Checking queue statuses." );

  hash_iterator iter;
  hash_entry *e;

  *connected_count = 0;
  *sending_count = 0;
  *reconnecting_count = 0;
  *closed_count = 0;

  if ( send_queues == NULL ) {
    error( "All send queues are already deleted or not created yet." );
    return;
  }

  init_hash_iterator( send_queues, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    send_queue *sq = e->value;
    if ( sq->server_socket != -1 ) {
      if ( sq->buffer->data_length == 0 ) {
          ( *connected_count )++;
      }
      else {
        if ( sq->refused_count > 0 ) {
            ( *reconnecting_count )++;
        }
        else {
            ( *sending_count )++;
        }
      }
    }
    else {
        ( *closed_count )++;
    }
  }

  debug( "connected_count = %d, reconnecting_count = %d, sending_count = %d, closed_count = %d.",
         *connected_count, *reconnecting_count, *sending_count, *closed_count );
}


static void
add_recv_queue_client_fd( receive_queue *rq, int fd ) {
  assert( rq != NULL );
  assert( fd >= 0 );

  debug( "Adding a client fd to receive queue ( fd = %d, service_name = %s ).", fd, rq->service_name );

  messenger_socket *socket;

  socket = xmalloc( sizeof( messenger_socket ) );
  socket->fd = fd;
  insert_after_dlist( rq->client_sockets, socket );

  set_fd_handler( fd, on_recv, rq, NULL, NULL );
  set_readable( fd, true );
}


static void
on_accept( int fd, void *data ) {
  receive_queue *rq = ( receive_queue * ) data;

  assert( rq != NULL );

  int client_fd;
  struct sockaddr_un addr;

  socklen_t addr_len = sizeof( struct sockaddr_un );

  if ( ( client_fd = accept( fd, ( struct sockaddr * ) &addr, &addr_len ) ) == -1 ) {
    error( "Failed to accept ( fd = %d, errno = %s [%d] ).", fd, strerror( errno ), errno );
    return;
  }

  if ( geteuid() == 0 ) {
    int rmem_size = 1048576;
    int ret = setsockopt( client_fd, SOL_SOCKET, SO_RCVBUFFORCE, ( const void * ) &rmem_size, ( socklen_t ) sizeof( rmem_size ) );
    if ( ret < 0 ) {
      error( "Failed to set SO_RCVBUFFORCE to %d ( %s [%d] ).", rmem_size, strerror( errno ), errno );
      close( client_fd );
      return;
    }
  }
  int ret = fcntl( client_fd, F_SETFL, O_NONBLOCK );
  if ( ret < 0 ) {
    error( "Failed to set O_NONBLOCK ( %s [%d] ).", strerror( errno ), errno );
    close( client_fd );
    return;
  }

  add_recv_queue_client_fd( rq, client_fd );
  send_dump_message( MESSENGER_DUMP_RECV_CONNECTED, rq->service_name, NULL, 0 );
}


static int
del_recv_queue_client_fd( receive_queue *rq, int fd ) {
  assert( rq != NULL );
  assert( fd >= 0 );

  messenger_socket *socket;
  dlist_element *element;

  debug( "Deleting a client fd from receive queue ( fd = %d, service_name = %s ).", fd, rq->service_name );

  for ( element = rq->client_sockets->next; element; element = element->next ) {
    socket = element->data;
    if ( socket->fd == fd ) {
      set_readable( fd, false );
      delete_fd_handler( fd );

      debug( "Deleting fd ( %d ).", fd );
      delete_dlist_element( element );
      xfree( socket );
      return 1;
    }
  }

  return 0;
}


static void
truncate_message_buffer( message_buffer *buf, size_t len ) {
  assert( buf != NULL );

  if ( len == 0 || buf->data_length == 0 ) {
    return;
  }

  if ( len > buf->data_length ) {
    len = buf->data_length;
  }

  if ( ( buf->head_offset + len ) <= buf->size ) {
    buf->head_offset += len;
  }
  else {
    memmove( buf->buffer, ( char * ) buf->buffer + buf->head_offset + len, buf->data_length - len );
    buf->head_offset = 0;
  }
  buf->data_length -= len;
}


/**
 * pulls message data from recv_queue.
 * returns 1 if succeeded, otherwise 0.
 */
static int
pull_from_recv_queue( receive_queue *rq, uint8_t *message_type, uint16_t *tag, void *data, size_t *len, size_t maxlen ) {
  assert( rq != NULL );
  assert( message_type != NULL );
  assert( tag != NULL );
  assert( data != NULL );
  assert( len != NULL );

  debug( "Pulling a message from receive queue ( service_name = %s ).", rq->service_name );

  message_header *header;

  if ( rq->buffer->data_length < sizeof( message_header ) ) {
    debug( "Queue length is smaller than a message header ( queue length = %zu ).", rq->buffer->data_length );
    return 0;
  }

  header = ( message_header * ) get_message_buffer_head( rq->buffer );

  uint32_t length = ntohl( header->message_length );
  assert( length != 0 );
  assert( length < messenger_recv_queue_length );
  if ( rq->buffer->data_length < length ) {
    debug( "Queue length is smaller than message length ( queue length = %zu, message length = %u ).",
           rq->buffer->data_length, length );
    return 0;
  }

  *message_type = header->message_type;
  *tag = ntohs( header->tag );
  *len = length - sizeof( message_header );
  memcpy( data, header->value, *len > maxlen ? maxlen : *len );
  truncate_message_buffer( rq->buffer, length );

  debug( "A message is retrieved from receive queue ( message_type = %#x, tag = %#x, len = %zu, data = %p ).",
         *message_type, *tag, *len, data );

  return 1;
}


static messenger_context *
get_context( uint32_t transaction_id ) {
  debug( "Looking up a context ( transaction_id = %#x ).", transaction_id );

  return lookup_hash_entry( context_db, &transaction_id );
}


static void
call_message_callbacks( receive_queue *rq, const uint8_t message_type, const uint16_t tag, void *data, size_t len ) {
  assert( rq != NULL );

  dlist_element *element;
  receive_queue_callback *cb;

  debug( "Calling message callbacks ( service_name = %s, message_type = %#x, tag = %#x, data = %p, len = %zu ).",
         rq->service_name, message_type, tag, data, len );

  for ( element = rq->message_callbacks->next; element; element = element->next ) {
    cb = element->data;
    if ( cb->message_type != message_type ) {
      continue;
    }
    switch ( message_type ) {
    case MESSAGE_TYPE_NOTIFY:
      {
        void ( *received_callback )( uint16_t tag, void *data, size_t len );
        received_callback = cb->function;

        debug( "Calling a callback ( %p ) for MESSAGE_TYPE_NOTIFY (%#x) ( tag = %#x, data = %p, len = %zu ).",
               cb->function, message_type, tag, data, len );

        received_callback( tag, data, len );
      }
      break;
    case MESSAGE_TYPE_REQUEST:
      {
        void ( *requested_callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len );
        messenger_context_handle *handle;
        char *requested_data;
        size_t header_len;

        requested_callback = cb->function;
        handle = ( messenger_context_handle * ) data;
        handle->transaction_id = ntohl( handle->transaction_id );
        handle->service_name_len = ntohs( handle->service_name_len );
        header_len = sizeof( messenger_context_handle ) + handle->service_name_len;
        requested_data = ( ( char * ) data ) + header_len;

        debug( "Calling a callback ( %p ) for MESSAGE_TYPE_REQUEST (%#x) ( handle = %p, tag = %#x, requested_data = %p, len = %zu ).",
               cb->function, message_type, handle, tag, requested_data, len - header_len );

        requested_callback( handle, tag, ( void * ) requested_data, len - header_len );
      }
      break;
    case MESSAGE_TYPE_REPLY:
      {
        debug( "Calling a callback ( %p ) for MESSAGE_TYPE_REPLY (%#x).", cb->function, message_type );

        void ( *replied_callback )( uint16_t tag, void *data, size_t len, void *user_data );
        messenger_context_handle *reply_handle;
        messenger_context *context;

        replied_callback = cb->function;
        reply_handle = data;
        reply_handle->transaction_id = ntohl( reply_handle->transaction_id );
        reply_handle->service_name_len = ntohs( reply_handle->service_name_len );

        context = get_context( reply_handle->transaction_id );

        if ( NULL != context ) {
          debug( "tag = %#x, data = %p, len = %zu, user_data = %p.",
                 tag, reply_handle->service_name, len - sizeof( messenger_context_handle ), context->user_data );
          replied_callback( tag, reply_handle->service_name, len - sizeof( messenger_context_handle ), context->user_data );
          delete_context( context );
        }
        else {
          warn( "No context found." );
        }
      }
      break;
    default:
      error( "Unknown message type ( %#x ).", message_type );
      assert( 0 );
    }
  }
}


static void
on_recv( int fd, void *data ) {
  receive_queue *rq = ( receive_queue * ) data;

  assert( rq != NULL );
  assert( fd >= 0 );

  debug( "Receiving data from remote ( fd = %d, service_name = %s ).", fd, rq->service_name );

  uint8_t buf[ MESSENGER_RECV_BUFFER ];
  ssize_t recv_len;
  size_t buf_len;
  uint8_t message_type;
  uint16_t tag;

  while ( ( buf_len = message_buffer_remain_bytes( rq->buffer ) ) > messenger_recv_queue_reserved ) {
    if ( buf_len > sizeof( buf ) ) {
      buf_len = sizeof( buf );
    }
    recv_len = recv( fd, buf, buf_len, 0 );
    if ( recv_len == -1 ) {
      if ( errno != EAGAIN && errno != EWOULDBLOCK ) {
        error( "Failed to recv ( fd = %d, errno = %s [%d] ).", fd, strerror( errno ), errno );
        send_dump_message( MESSENGER_DUMP_RECV_CLOSED, rq->service_name, NULL, 0 );
        del_recv_queue_client_fd( rq, fd );
        close( fd );
      }
      else {
        debug( "Failed to recv ( fd = %d, errno = %s [%d] ).", fd, strerror( errno ), errno );
      }
      break;
    }
    else if ( recv_len == 0 ) {
      debug( "Connection closed ( fd = %d, service_name = %s ).", fd, rq->service_name );
      send_dump_message( MESSENGER_DUMP_RECV_CLOSED, rq->service_name, NULL, 0 );
      del_recv_queue_client_fd( rq, fd );
      close( fd );
      break;
    }

    if ( !write_message_buffer( rq->buffer, buf, ( size_t ) recv_len ) ) {
      warn( "Could not write a message to receive queue due to overflow ( service_name = %s, len = %zd ).", rq->service_name, recv_len );
      send_dump_message( MESSENGER_DUMP_RECV_OVERFLOW, rq->service_name, buf, ( uint32_t ) recv_len );
    }
    else {
      debug( "Pushing a message to receive queue ( service_name = %s, len = %zd ).", rq->service_name, recv_len );
      send_dump_message( MESSENGER_DUMP_RECEIVED, rq->service_name, buf, ( uint32_t ) recv_len );
    }
  }

  while ( pull_from_recv_queue( rq, &message_type, &tag, buf, &buf_len, sizeof( buf ) ) == 1 ) {
    call_message_callbacks( rq, message_type, tag, buf, buf_len );
  }
}


static uint32_t
get_send_data( send_queue *sq, size_t offset ) {
  assert( sq != NULL );

  uint32_t bucket_size = messenger_bucket_size;
  if ( sq->socket_buffer_size != 0 ) {
    int used;
    if ( ioctl( sq->server_socket, SIOCOUTQ, &used ) == 0 ) {
      if ( used < sq->socket_buffer_size ) {
        bucket_size = ( uint32_t ) ( sq->socket_buffer_size - used ) << 1;
        if ( bucket_size > messenger_bucket_size ) {
          bucket_size = messenger_bucket_size;
        }
      }
      else {
        bucket_size = 1;
      }
    }
  }

  uint32_t length = 0;
  message_header *header;
  while ( ( sq->buffer->data_length - offset ) >= sizeof( message_header ) ) {
    header = ( message_header * ) ( ( char * ) get_message_buffer_head( sq->buffer ) + offset );
    uint32_t message_length = ntohl( header->message_length );
    assert( message_length != 0 );
    assert( message_length < messenger_recv_queue_length );
    if ( length + message_length > bucket_size ) {
      if ( length == 0 ) {
        length = message_length;
      }
      break;
    }
    length += message_length;
    offset += message_length;
  }
  return length;
}


static void
on_send_read( int fd, void *data ) {
  UNUSED( fd );

  char buf[ 256 ];
  send_queue *sq = ( send_queue * ) data;

  if ( recv( sq->server_socket, buf, sizeof( buf ), 0 ) <= 0 ) {
    send_dump_message( MESSENGER_DUMP_SEND_CLOSED, sq->service_name, NULL, 0 );

    set_readable( sq->server_socket, false );
    set_writable( sq->server_socket, false );
    delete_fd_handler( sq->server_socket );

    close( sq->server_socket );
    sq->server_socket = -1;

    // Tries to reconnecting immediately, else adds a reconnect timer.
    if ( sq->buffer->data_length > 0 ) {
      send_queue_try_connect( sq );
    }
    else {
      delete_send_queue( sq );
    }
  }
}


static void
on_send_write( int fd, void *data ) {
  send_queue *sq = ( send_queue * ) data;

  assert( sq != NULL );
  assert( fd >= 0 );

  debug( "Sending data to remote ( fd = %d, service_name = %s, buffer = %p, data_length = %zu ).",
         fd, sq->service_name, get_message_buffer_head( sq->buffer ), sq->buffer->data_length );

  if ( sq->buffer->data_length < sizeof( message_header ) ) {
    set_writable( sq->server_socket, false );
    return;
  }

  void *send_data;
  size_t send_len;
  ssize_t sent_len;
  size_t sent_total = 0;

  while ( ( send_len = get_send_data( sq, sent_total ) ) > 0 ) {
    send_data = ( ( char * ) get_message_buffer_head( sq->buffer ) + sent_total );
    sent_len = send( fd, send_data, send_len, MSG_DONTWAIT );
    if ( sent_len == -1 ) {
      int err = errno;
      if ( err != EAGAIN && err != EWOULDBLOCK ) {
        error( "Failed to send ( service_name = %s, fd = %d, errno = %s [%d] ).",
               sq->service_name, fd, strerror( err ), err );
        send_dump_message( MESSENGER_DUMP_SEND_CLOSED, sq->service_name, NULL, 0 );

        set_readable( sq->server_socket, false );
        set_writable( sq->server_socket, false );
        delete_fd_handler( sq->server_socket );

        close( sq->server_socket );
        sq->server_socket = -1;
        sq->refused_count = 0;

        // Tries to reconnecting immediately, else adds a reconnect timer.
        send_queue_try_connect( sq );
      }
      truncate_message_buffer( sq->buffer, sent_total );
      if ( err == EMSGSIZE || err == ENOBUFS || err == ENOMEM ) {
        warn( "Dropping %zu bytes data in send queue ( service_name = %s ).", sq->buffer->data_length, sq->service_name );
        truncate_message_buffer( sq->buffer, sq->buffer->data_length );
      }
      return;
    }
    assert( sent_len != 0 );
    assert( send_len == ( size_t ) sent_len );
    send_dump_message( MESSENGER_DUMP_SENT, sq->service_name, send_data, ( uint32_t ) sent_len );
    sent_total += ( size_t ) sent_len;
  }

  truncate_message_buffer( sq->buffer, sent_total );
  if ( sq->buffer->data_length == 0 ) {
    set_writable( sq->server_socket, false );
  }
}


int
flush_messenger() {
  int connected_count, sending_count, reconnecting_count, closed_count;

  debug( "Flushing send queues." );

  while ( true ) {
    number_of_send_queue( &connected_count, &sending_count, &reconnecting_count, &closed_count );
    if ( sending_count == 0 ) {
      return reconnecting_count;
    }
    run_event_handler_once( 100000 );
  }
}


bool
start_messenger() {
  debug( "Starting messenger." );

  add_periodic_event_callback( 10, age_context_db, NULL );

  return true;
}


bool
stop_messenger() {
  debug( "Terminating messenger." );

  return true;
}


void
start_messenger_dump( const char *dump_app_name, const char *dump_service_name ) {
  assert( dump_app_name != NULL );
  assert( dump_service_name != NULL );

  debug( "Starting a message dumper ( dump_app_name = %s, dump_service_name = %s ).",
         dump_app_name, dump_service_name );

  if ( messenger_dump_enabled() ) {
    stop_messenger_dump();
  }
  _dump_service_name = xstrdup( dump_service_name );
  _dump_app_name = xstrdup( dump_app_name );
}


void
stop_messenger_dump( void ) {
  assert( _dump_service_name != NULL );
  assert( _dump_app_name != NULL );

  debug( "Terminating a message dumper ( dump_app_name = %s, dump_service_name = %s ).",
         _dump_app_name, _dump_service_name );

  assert( send_queues != NULL );
  send_queue *sq = lookup_hash_entry( send_queues, _dump_service_name );
  if ( sq != NULL ) {
    delete_send_queue( sq );
  }

  xfree( _dump_service_name );
  _dump_service_name = NULL;

  xfree( _dump_app_name );
  _dump_app_name = NULL;
}


bool
messenger_dump_enabled( void ) {
  if ( _dump_service_name != NULL && _dump_app_name != NULL ) {
    return true;
  }

  return false;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
