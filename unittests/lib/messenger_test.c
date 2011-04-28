/*
 * Unit tests for Trema messenger.
 *
 * Author: Toshio Koide
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


#include <arpa/inet.h>
#include <linux/limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "checks.h"
#include "cmockery.h"
#include "doubly_linked_list.h"
#include "hash_table.h"
#include "messenger.h"
#include "unittest.h"
#include "wrapper.h"


#define static extern


enum {
  MESSAGE_TYPE_NOTIFY,
  MESSAGE_TYPE_REQUEST,
  MESSAGE_TYPE_REPLY,
};

typedef struct message_header {
  uint8_t version;         // version = 0 (unused) 
  uint8_t message_type;    // MESSAGE_TYPE_
  uint16_t tag;            // user defined 
  uint32_t message_length; // message length including header 
  uint8_t  value[0];
} message_header;

typedef struct message_buffer {
  void *buffer;
  size_t data_length;
  size_t size;
} message_buffer;

typedef struct messenger_socket {
  int fd;
} messenger_socket;

typedef struct messenger_context {
  uint32_t transaction_id;
  int life_count;
  void *user_data;
} messenger_context;

typedef struct timer_callback {
  void ( *function )( void *user_data );
  struct timespec expires_at;
  struct timespec interval;
  void *user_data;
} timer_callback;

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
  struct timespec reconnect_at;
  struct sockaddr_un server_addr;
  message_buffer *buffer;
} send_queue;


static void send_dump_message( uint16_t dump_type, const char *service_name, const void *data, uint32_t data_len );

static bool run_once( void );

static void on_accept( int fd, receive_queue *queue );
static void on_recv( int fd, receive_queue *queue );
static void on_send( int fd, send_queue *queue );
static void on_timer( timer_callback *callback );

static receive_queue *create_receive_queue( const char *service_name );
static void delete_all_receive_queues( void );
static void delete_receive_queue( void *service_name, void *queue, void *user_data );
static int pull_from_recv_queue( receive_queue *queue, uint8_t *message_type, uint16_t *tag, void *data, size_t *len, size_t maxlen );
static void set_recv_queue_fd_set( fd_set *read_set );
static void check_recv_queue_fd_isset( fd_set *read_set );
static void add_recv_queue_client_fd( receive_queue *queue, int fd );
static int del_recv_queue_client_fd( receive_queue *queue, int fd );
static void call_message_callbacks( receive_queue *rq, const uint8_t message_type, const uint16_t tag, void *data, size_t len );

static send_queue *create_send_queue( const char *service_name );
static int send_queue_connect( send_queue *queue );
static void delete_all_send_queues( void );
static void delete_send_queue( send_queue *sq );
static void number_of_send_queue( int *connected_count, int *sending_count, int *reconnecting_count, int *closed_count );
static bool push_message_to_send_queue( const char *service_name, const uint8_t message_type, const uint16_t tag, const void *data, size_t len );
static void set_send_queue_fd_set( fd_set *read_set, fd_set *write_set );
static void check_send_queue_fd_isset( fd_set *read_set, fd_set *write_set );

static message_buffer *create_message_buffer( size_t size );
static bool write_message_buffer( message_buffer *buf, const void *data, size_t len );
static void truncate_message_buffer( message_buffer *buf, size_t len );
static void free_message_buffer( message_buffer *buf );
static size_t message_buffer_remain_bytes( message_buffer *buf );

static void delete_timer_callbacks( void );
static void execute_timer_events( void );

static messenger_context* insert_context( void *user_data );
static messenger_context* get_context( uint32_t transaction_id );
static void delete_context( messenger_context *context );
static void delete_context_db( void );
static void age_context_db( void * );

static const uint32_t messenger_buffer_length;

static char socket_directory[ PATH_MAX ];
static bool running;
static bool initialized;
static bool finalized;
static hash_table *receive_queues;
static hash_table *send_queues;
static hash_table *context_db;
static dlist_element *timer_callbacks;
static char *_dump_service_name;
static char *_dump_app_name;
static void ( *external_fd_set )( fd_set *read_set, fd_set *write_set );
static void ( *external_check_fd_isset )( fd_set *read_set, fd_set *write_set );
static uint32_t last_transaction_id;



#undef static


#define SERVICE_NAME1 "test1"
#define SERVICE_NAME2 "test2"
#define MESSAGE1 "message1"
#define MESSAGE2 "message2"
#define MESSAGE_TYPE1 1234
#define DUMP_SERVICE_NAME "dump"
#define DUMP_APP_NAME "appname"
#define TAG1 1
#define TAG2 2
#define CONTEXT_DATA "context data"


static int pipefd[ 2 ];
static bool external_received, external_sent;
static int dump_scenario_count = 0;


/********************************************************************************
 * Mocks.
 ********************************************************************************/


static bool fail_mock_socket = false;
int
mock_socket( int domain, int type, int protocol ) {
  return fail_mock_socket ? -1  : socket( domain, type, protocol );
}


static bool fail_mock_bind = false;
int
mock_bind( int sockfd, const struct sockaddr *addr, socklen_t addrlen ) {
  return fail_mock_bind ? -1 : bind( sockfd, addr, addrlen );
}


static bool fail_mock_listen = false;
int
mock_listen( int sockfd, int backlog ) {
  return fail_mock_listen ? -1 :listen( sockfd, backlog );
}


static bool fail_mock_close = false;
int
mock_close( int fd ) {
  return fail_mock_close ? -1 : close( fd );
}


static bool fail_mock_connect = false;
int
mock_connect( int sockfd, const struct sockaddr *addr, socklen_t addrlen ) {
  return fail_mock_connect ? -1 : connect( sockfd, addr, addrlen );
}


static bool fail_mock_select = false;
int
mock_select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout ) {
  return fail_mock_select ? -1 : select( nfds, readfds, writefds, exceptfds, timeout );
}


static bool fail_mock_accept = false;
int
mock_accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen ) {
  return fail_mock_accept ? -1 : accept( sockfd, addr, addrlen );
}


static bool fail_mock_recv = false;
ssize_t
mock_recv( int sockfd, void *buf, size_t len, int flags ) {
  return fail_mock_recv ? -1 : recv( sockfd, buf, len, flags );
}


static bool fail_mock_send = false;
ssize_t
mock_send( int sockfd, const void *buf, size_t len, int flags ) {
  return fail_mock_send ? -1 : send( sockfd, buf, len, flags );
}


int
mock_setsockopt( int s, int level, int optname, const void *optval, socklen_t optlen ) {
  return 0;
}


static bool fail_mock_clock_gettime = false;
int
mock_clock_gettime( clockid_t clk_id, struct timespec *tp ) {
  return fail_mock_clock_gettime ? -1 : clock_gettime( clk_id, tp );
}


void
mock_die( char *format, ... ) {
  UNUSED( format );
}


void
mock_error( char *format, ... ) {
  UNUSED( format );
}


void
mock_debug( char *format, ... ) {
  UNUSED( format );
}


void
mock_warn( char *format, ... ) {
  UNUSED( format );
}


/********************************************************************************
 * Callbacks.
 ********************************************************************************/

static void
message_received_callback1( uint16_t tag, void *data, size_t len ) {
  assert_true( tag == MESSAGE_TYPE1 );
  assert_string_equal( ( char * ) data, MESSAGE1 );
  assert_true( len == strlen( MESSAGE1 ) + 1 );
}


static void
stop_messenger_callback( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  UNUSED( data );
  UNUSED( len );

  assert_true( running );
  stop_messenger();
  assert_false( running );
}


static void
timer_event_callback2( void *user_data ) {
  int *count = ( int * ) user_data;

  ( *count )++;

  if ( *count == 5 ) {
    assert_true( delete_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  }
  if ( *count == 15 ) {
    assert_true( stop_messenger() );
  }
  assert_true( send_message( SERVICE_NAME1, MESSAGE_TYPE1, MESSAGE1, strlen( MESSAGE1 ) + 1 ) );
}


static void
external_fd_set_callback( fd_set *read_set, fd_set *write_set ) {
  FD_SET( pipefd[ 0 ], read_set );
  FD_SET( pipefd[ 1 ], write_set );
}


static void
external_check_fd_isset_callback( fd_set *read_set, fd_set *write_set ) {
  char buf[ 100 ];

  if ( FD_ISSET( pipefd[ 0 ], read_set ) ) {
    assert_int_equal( read( pipefd[ 0 ], buf, sizeof( buf ) ), strlen( MESSAGE1 ) + 1 );
    assert_string_equal( buf, MESSAGE1 );
    external_received = true;
    stop_messenger();
  }
  if ( FD_ISSET( pipefd[ 1 ], write_set ) ) {
    write( pipefd[ 1 ], MESSAGE1, strlen( MESSAGE1 ) + 1 );
    external_sent = true;
  }
}


static void
dump_received_callback( uint16_t tag, void *data, size_t len ) {
  message_dump_header *hdr;
  char *app_name, *service_name, *dump_data;
  const int dump_type_scenario[] = {
    MESSENGER_DUMP_SEND_CONNECTED,
    MESSENGER_DUMP_RECV_CONNECTED,
    MESSENGER_DUMP_SENT,
    MESSENGER_DUMP_RECEIVED,
    MESSENGER_DUMP_SENT,
    MESSENGER_DUMP_RECEIVED,
    MESSENGER_DUMP_SENT,
    MESSENGER_DUMP_RECEIVED,
    MESSENGER_DUMP_SENT,
    MESSENGER_DUMP_RECEIVED,
    MESSENGER_DUMP_RECV_CLOSED,
    MESSENGER_DUMP_SEND_CLOSED,
    MESSENGER_DUMP_SEND_REFUSED,
    MESSENGER_DUMP_SEND_REFUSED,
    MESSENGER_DUMP_SEND_REFUSED,
    MESSENGER_DUMP_SEND_REFUSED,
    MESSENGER_DUMP_SEND_REFUSED,
    MESSENGER_DUMP_SEND_REFUSED,
    MESSENGER_DUMP_SEND_REFUSED,
  };

  hdr = data;
  app_name = ( char * ) ( hdr + 1 );
  service_name = app_name + ntohs( hdr->app_name_length );
  dump_data = service_name + ntohs( hdr->service_name_length );

  UNUSED( len );

  assert_string_equal( app_name, DUMP_APP_NAME );
  assert_string_equal( service_name, SERVICE_NAME1 );
  assert_int_equal( dump_type_scenario[ dump_scenario_count ], tag );
  dump_scenario_count++;
}


void
message_requested_callback( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) {
  assert_true( handle != NULL );
  assert_int_equal( tag, TAG1 );
  assert_string_equal( data, MESSAGE1 );
  assert_int_equal( ( int ) len, strlen( MESSAGE1 ) + 1 );
  assert_true( send_reply_message( handle, TAG2, MESSAGE2, strlen( MESSAGE2 ) + 1 ) );
}


void
message_replied_callback( uint16_t tag, void *data, size_t len, void *user_data ) {
  assert_int_equal( tag, TAG2 );
  assert_string_equal( data, MESSAGE2 );
  assert_int_equal( ( int ) len, strlen( MESSAGE2 ) + 1 );
  assert_string_equal( user_data, CONTEXT_DATA );
  xfree( user_data );

  assert_true( stop_messenger() );
}


/********************************************************************************
 * Helpers.
 ********************************************************************************/

static timer_callback*
find_timer_callback( void ( *callback )( void *user_data ) ) {
  dlist_element *e;
  timer_callback *cb;

  cb = NULL;
  for ( e = timer_callbacks->next; e; e = e->next ) {
    cb = e->data;
    if ( cb->function == callback ) {
      return cb;
    }
  }
  return NULL;
}


static receive_queue_callback*
find_message_callback( const char *service_name, uint8_t message_type, void *callback ) {
  dlist_element *e;
  receive_queue *rq;
  receive_queue_callback *cb, *ret;

  rq = lookup_hash_entry( receive_queues, service_name );
  if ( rq == NULL ) {
    return NULL;
  }

  ret = NULL;
  for ( e = rq->message_callbacks->next; e; e = e->next ) {
    cb = e->data;
    if ( ( cb->function == callback ) && ( cb->message_type == message_type ) ) {
      ret = cb;
      break;
    }
  }
  return ret;
}


static void
reset_messenger() {
  initialized = false;
  finalized = false;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_init_and_finalize() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( finalize_messenger() );
}


static void
test_init_twice() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( init_messenger( "/tmp" ) );

  finalize_messenger();
}


static void
test_finalize_twice() {
  init_messenger( "/tmp" );

  assert_true( finalize_messenger() );
  assert_true( finalize_messenger() );
}


static void
test_finalize_without_init() {
  assert_false( finalize_messenger() );
}


static void
test_add_and_delete_message_received_callback() {
  receive_queue *rq;
  receive_queue_callback *cb;

  assert_true( init_messenger( "/tmp" ) );

  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq != NULL );
  assert_string_equal( rq->service_name, SERVICE_NAME1 );
  cb = find_message_callback( SERVICE_NAME1, MESSAGE_TYPE_NOTIFY, message_received_callback1 );
  assert_true( cb != NULL );
  assert_true( cb->function == message_received_callback1 );
  assert_int_equal( cb->message_type, MESSAGE_TYPE_NOTIFY );

  assert_true( delete_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq == NULL );

  assert_true( finalize_messenger() );
}


/********************************************************************************
 * Timer callback tests.
 ********************************************************************************/

static void
mock_timer_event_callback( void *user_data ) {
  stop_messenger();
}


static void
test_add_and_delete_timer_event_callback() {
  struct itimerspec interval;
  interval.it_value.tv_sec = 1;
  interval.it_value.tv_nsec = 1000;
  interval.it_interval.tv_sec = 2;
  interval.it_interval.tv_nsec = 2000;
  assert_true( add_timer_event_callback( &interval, mock_timer_event_callback, "It's time!!!" ) );

  timer_callback *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->function == mock_timer_event_callback );
  assert_string_equal( callback->user_data, "It's time!!!" );
  assert_int_equal( callback->interval.tv_sec, 2 );
  assert_int_equal( callback->interval.tv_nsec, 2000 );

  start_messenger();

  delete_timer_event_callback( mock_timer_event_callback );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );
}


static void
test_add_and_delete_periodic_event_callback() {
  assert_true( add_periodic_event_callback( 1, mock_timer_event_callback, "It's time!!!" ) );

  timer_callback *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->function == mock_timer_event_callback );
  assert_true( callback->user_data == "It's time!!!" );
  assert_int_equal( callback->interval.tv_sec, 1 );
  assert_int_equal( callback->interval.tv_nsec, 0 );

  assert_true( start_messenger() );

  delete_periodic_event_callback( mock_timer_event_callback );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );
}


static void
test_add_and_delete_message_replied_callback() {
  receive_queue *rq;
  receive_queue_callback *cb;

  assert_true( init_messenger( "/tmp" ) );

  assert_true( add_message_replied_callback( SERVICE_NAME1, message_replied_callback ) );
  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq != NULL );
  assert_string_equal( rq->service_name, SERVICE_NAME1 );
  cb = find_message_callback( SERVICE_NAME1, MESSAGE_TYPE_REPLY, message_replied_callback );
  assert_true( cb != NULL );
  assert_true( cb->function == message_replied_callback );
  assert_int_equal( cb->message_type, MESSAGE_TYPE_REPLY );

  assert_true( delete_message_replied_callback( SERVICE_NAME1, message_replied_callback ) );
  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq == NULL );

  assert_true( finalize_messenger() );
}


static void
test_add_and_delete_message_requested_callback() {
  receive_queue *rq;
  receive_queue_callback *cb;

  assert_true( init_messenger( "/tmp" ) );

  assert_true( add_message_requested_callback( SERVICE_NAME1, message_requested_callback ) );
  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq != NULL );
  assert_string_equal( rq->service_name, SERVICE_NAME1 );
  cb = find_message_callback( SERVICE_NAME1, MESSAGE_TYPE_REQUEST, message_requested_callback );
  assert_true( cb != NULL );
  assert_true( cb->function == message_requested_callback );
  assert_int_equal( cb->message_type, MESSAGE_TYPE_REQUEST );

  assert_true( delete_message_requested_callback( SERVICE_NAME1, message_requested_callback ) );
  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq == NULL );

  assert_true( finalize_messenger() );
}


static void
test_rename_message_received_callback() {

  assert_true( init_messenger( "/tmp" ) );

  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  assert_true( NULL != find_message_callback( SERVICE_NAME1, MESSAGE_TYPE_NOTIFY, message_received_callback1 ) );
  assert_true( NULL == find_message_callback( SERVICE_NAME2, MESSAGE_TYPE_NOTIFY, message_received_callback1 ) );

  assert_true( rename_message_received_callback( SERVICE_NAME1, SERVICE_NAME2 ) );
  assert_true( NULL == find_message_callback( SERVICE_NAME1, MESSAGE_TYPE_NOTIFY, message_received_callback1 ) );
  assert_true( NULL != find_message_callback( SERVICE_NAME2, MESSAGE_TYPE_NOTIFY, message_received_callback1 ) );

  assert_true( finalize_messenger() );
}


static void
init_messenger_for_unit_test() {
  reset_messenger();
  init_messenger( "/tmp" );
}


static void
finalize_messenger_for_unit_test() {
  finalize_messenger();
  reset_messenger();
}


static void
callback_hello( uint16_t tag, void *data, size_t len ) {
  check_expected( tag );
  check_expected( data );
  check_expected( len );

  stop_messenger();
}


static void
test_send_then_message_received_callback_is_called() {
  const char service_name[] = "Say HELLO";

  expect_value( callback_hello, tag, 43556 );
  expect_string( callback_hello, data, "HELLO" );
  expect_value( callback_hello, len, 6 );

  add_message_received_callback( service_name, callback_hello );
  send_message( service_name, 43556, "HELLO", strlen( "HELLO" ) + 1 );
  start_messenger();

  delete_message_received_callback( service_name, callback_hello );
  delete_send_queue( lookup_hash_entry( send_queues, service_name ) );
}


static void
test_start_and_stop_messenger_via_messaging() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( add_message_received_callback( SERVICE_NAME1, stop_messenger_callback ) );
  assert_true( send_message( SERVICE_NAME1, MESSAGE_TYPE1, MESSAGE1, strlen( MESSAGE1 ) + 1 ) );
  assert_true( start_messenger() );
  assert_false( running );
  assert_true( finalize_messenger() );
}


static void
test_start_and_stop_dump() {
  struct itimerspec interval;
  int count = 0;

  assert_true( init_messenger( "/tmp" ) );

  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 100 * 1000 * 1000;
  assert_true( add_timer_event_callback( &interval, timer_event_callback2, &count ) );

  assert_true( add_message_received_callback( DUMP_SERVICE_NAME, dump_received_callback ) );
  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );

  start_messenger_dump( DUMP_APP_NAME, DUMP_SERVICE_NAME );
  assert_string_equal( _dump_app_name, DUMP_APP_NAME );
  assert_string_equal( _dump_service_name, DUMP_SERVICE_NAME );

  //to change app_name and/or service_name, call start_messenger_dump() twice like this:
  start_messenger_dump( DUMP_APP_NAME, DUMP_SERVICE_NAME );

  assert_true( start_messenger() );

  stop_messenger_dump();
  assert_true( _dump_app_name == NULL );
  assert_true( _dump_service_name == NULL );

  assert_true( finalize_messenger() );
}


static void
test_external_fd_send_and_receive() {
  external_received = external_sent = false;
  assert_true( pipe( pipefd ) == 0 );
  assert_true( init_messenger( "/tmp" ) );

  set_fd_set_callback( external_fd_set_callback );
  set_check_fd_isset_callback( external_check_fd_isset_callback );
  assert_true( external_fd_set == external_fd_set_callback );
  assert_true( external_check_fd_isset == external_check_fd_isset_callback );

  assert_true( start_messenger() );
  assert_true( external_sent );
  assert_true( external_received );

  assert_true( finalize_messenger() );
  assert_true( external_fd_set == NULL );
  assert_true( external_check_fd_isset == NULL );
}


/*
 * test_request_reply():
 *
 * SERVICE_NAME1               SERVICE_NAME2
 *       |                           |
 * add_message_replied_callback()    |
 *       |                           |
 *       |           add_message_requested_callback()
 *       |                           |
 * send_request_message()----------->|
 *       | with user_data    requested_callback()
 *       |                           | obtain handle
 *       | <---------------- send_reply_message()
 * replied_callback()                | with handle
 *       | obtain user_data          |
 */

static void
test_request_reply() {
  char *user_data = xstrdup( CONTEXT_DATA );

  assert_true( init_messenger( "/tmp" ) );

  assert_true( add_message_replied_callback( SERVICE_NAME1, message_replied_callback ) );
  assert_true( add_message_requested_callback( SERVICE_NAME2, message_requested_callback ) );

  assert_true( send_request_message( SERVICE_NAME2, SERVICE_NAME1, TAG1, MESSAGE1, strlen( MESSAGE1 ) + 1, user_data ) );
  assert_int_equal( ( int ) context_db->length, 1 );

  assert_true( start_messenger() );
  assert_int_equal( ( int ) context_db->length, 0 );

  assert_true( finalize_messenger() );
}


static void
test_socket_fail_eacces() {
  assert_true( init_messenger( "/tmp" ) );

  fail_mock_socket = true;
  errno = EACCES;
  assert_false( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  fail_mock_socket = false;

  assert_true( finalize_messenger() );
}


static void
test_bind_fail_eacces() {
  assert_true( init_messenger( "/tmp" ) );

  fail_mock_bind = true;
  errno = EACCES;
  assert_false( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  fail_mock_bind = false;

  assert_true( finalize_messenger() );
}


static void
test_listen_fail_eaddrinuse() {
  assert_true( init_messenger( "/tmp" ) );

  fail_mock_listen = true;
  errno = EADDRINUSE;
  assert_false( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  fail_mock_listen = false;

  assert_true( finalize_messenger() );
}


static void
test_clock_gettime_fail_einval() {
  assert_true( init_messenger( "/tmp" ) );

  fail_mock_clock_gettime = true;
  errno = EINVAL;
  assert_false( add_periodic_event_callback( 1, mock_timer_event_callback, mock_timer_event_callback ) );
  fail_mock_clock_gettime = false;

  assert_true( finalize_messenger() );
}


static void
test_add_timer_event_callback_fail_invailid_timespec() {
  struct itimerspec interval;

  assert_true( init_messenger( "/tmp" ) );

  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 0;
  assert_false( add_timer_event_callback( &interval, mock_timer_event_callback, mock_timer_event_callback ) );

  assert_true( finalize_messenger() );
}


static void
test_delete_message_received_callback_for_nonexistent_service_name() {
  assert_true( init_messenger( "/tmp" ) );
  assert_false( delete_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  assert_true( finalize_messenger() );
}


static void
test_delete_timer_event_callback_for_nonexistent_service_name() {
  assert_true( init_messenger( "/tmp" ) );
  assert_false( delete_timer_event_callback( mock_timer_event_callback ) );
  assert_true( finalize_messenger() );
}


static void
test_rename_message_received_callback_from_nonexistent_service_name() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  assert_false( rename_message_received_callback( SERVICE_NAME2, SERVICE_NAME1 ) );
  assert_true( finalize_messenger() );
}


static void
test_rename_message_received_callback_to_existing_service_name() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  assert_true( add_message_received_callback( SERVICE_NAME2, message_received_callback1 ) );
  assert_false( rename_message_received_callback( SERVICE_NAME1, SERVICE_NAME2 ) );
  assert_true( finalize_messenger() );
}


static void
test_select_fail_eintr() {
  struct itimerspec interval;

  assert_true( init_messenger( "/tmp" ) );

  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 1000 * 1000;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 0;
  assert_true( add_timer_event_callback( &interval, mock_timer_event_callback, mock_timer_event_callback ) );

  errno = EINTR;
  fail_mock_select = true;
  assert_true( start_messenger() );
  fail_mock_select = false;

  assert_true( finalize_messenger() );
}


static void
test_select_fail_no_eintr() {
  assert_true( init_messenger( "/tmp" ) );

  errno = EBADF;
  fail_mock_select = true;
  assert_false( start_messenger() );
  fail_mock_select = false;

  assert_true( finalize_messenger() );
}


static void
test_accept_fail_enomem() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  assert_true( send_message( SERVICE_NAME1, MESSAGE_TYPE1, MESSAGE1, strlen( MESSAGE1 ) + 1 ) );

  fail_mock_accept = true;
  errno = ENOMEM;
  assert_int_equal( flush_messenger(), 0 );
  fail_mock_accept = false;

  assert_true( finalize_messenger() );
}


static void
test_create_send_queue_for_existing_service_name() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( create_send_queue( SERVICE_NAME1 ) );
  assert_true( create_send_queue( SERVICE_NAME1 ) );
  assert_true( finalize_messenger() );
}


static void
test_create_receive_queue_for_existing_service_name() {
  assert_true( init_messenger( "/tmp" ) );
  assert_true( create_receive_queue( SERVICE_NAME1 ) ); 
  assert_true( create_receive_queue( SERVICE_NAME1 ) ); 
  assert_true( finalize_messenger() );
}


static void
test_send_queue_connect_fail_by_clock_gettime() {
  assert_true( init_messenger( "/tmp" ) );

  fail_mock_clock_gettime = true;
  errno = EINVAL;
  create_send_queue( SERVICE_NAME1 );
  fail_mock_clock_gettime = false;

  assert_true( finalize_messenger() );
}


static void
test_send_queue_connect_fail_by_socket_eacces() {
  send_queue *sq;
  assert_true( init_messenger( "/tmp" ) );

  sq = create_send_queue( SERVICE_NAME1 );
  assert_true( sq != NULL );

  fail_mock_socket = true;
  errno = EACCES;
  assert_int_equal( -1, send_queue_connect( sq ) );
  fail_mock_socket = false;

  assert_true( finalize_messenger() );
}


static void
test_close_client_socket() {
  receive_queue *rq;
  send_queue *sq;

  assert_true( init_messenger( "/tmp" ) );
  assert_true( add_message_received_callback( SERVICE_NAME1, message_received_callback1 ) );
  assert_true( send_message( SERVICE_NAME1, MESSAGE_TYPE1, MESSAGE1, strlen( MESSAGE1 ) + 1 ) );
  assert_int_equal( flush_messenger(), 0 );

  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq->client_sockets->next != NULL );

  sq = lookup_hash_entry( send_queues, SERVICE_NAME1 );
  delete_send_queue( sq );
  run_once();
  run_once();
  run_once();

  rq = lookup_hash_entry( receive_queues, SERVICE_NAME1 );
  assert_true( rq->client_sockets->next == NULL );

  assert_true( finalize_messenger() );
}


static void
test_aging() {
  assert_true( init_messenger( "/tmp" ) );

  assert_true( send_request_message( SERVICE_NAME2, SERVICE_NAME1, TAG1, MESSAGE1, strlen( MESSAGE1 ) + 1, NULL ) );
  int i;
  for ( i = 0; i < 10; i++ ) {
    assert_int_equal( ( int ) context_db->length, 1 );
    age_context_db( NULL );
  }
  assert_int_equal( ( int ) context_db->length, 0 );

  assert_true( finalize_messenger() );
}


static void
test_on_recv_fail_with_invalid_fd() {
  receive_queue *rq;
  assert_true( init_messenger( "/tmp" ) );
  rq = create_receive_queue( SERVICE_NAME2 );
  assert_true( rq != NULL );

  expect_assert_failure( on_recv( -1, rq ) );

  assert_true( finalize_messenger() );
}


static void
test_on_send_fail_with_invalid_fd() {
  send_queue *sq;
  assert_true( init_messenger( "/tmp" ) );
  sq = create_send_queue( SERVICE_NAME2 );
  assert_true( sq != NULL );

  expect_assert_failure( on_send( -1, sq ) );

  assert_true( finalize_messenger() );
}


static void
test_del_recv_queue_client_fd_fail_with_invalid_fd() {
  receive_queue *rq;
  assert_true( init_messenger( "/tmp" ) );
  rq = create_receive_queue( SERVICE_NAME2 );
  assert_true( rq != NULL );

  expect_assert_failure( del_recv_queue_client_fd( rq, -1 ) );

  assert_true( finalize_messenger() );
}

/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_init_and_finalize, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_init_twice, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_finalize_twice, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_finalize_without_init, reset_messenger, reset_messenger ),

    unit_test_setup_teardown( test_add_and_delete_message_received_callback, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_rename_message_received_callback, reset_messenger, reset_messenger ),

    unit_test_setup_teardown( test_send_then_message_received_callback_is_called,
                              init_messenger_for_unit_test,
                              finalize_messenger_for_unit_test ),

    // Timer callback tests.
    unit_test_setup_teardown( test_add_and_delete_timer_event_callback,
                              init_messenger_for_unit_test,
                              finalize_messenger_for_unit_test ),
    unit_test_setup_teardown( test_add_and_delete_periodic_event_callback,
                              init_messenger_for_unit_test,
                              finalize_messenger_for_unit_test ),

    unit_test_setup_teardown( test_start_and_stop_messenger_via_messaging, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_external_fd_send_and_receive, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_start_and_stop_dump, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_add_and_delete_message_replied_callback, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_add_and_delete_message_requested_callback, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_request_reply, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_socket_fail_eacces, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_bind_fail_eacces, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_listen_fail_eaddrinuse, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_clock_gettime_fail_einval, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_add_timer_event_callback_fail_invailid_timespec, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_delete_message_received_callback_for_nonexistent_service_name, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_delete_timer_event_callback_for_nonexistent_service_name, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_rename_message_received_callback_from_nonexistent_service_name, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_rename_message_received_callback_to_existing_service_name, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_select_fail_eintr, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_select_fail_no_eintr, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_accept_fail_enomem, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_create_send_queue_for_existing_service_name, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_create_receive_queue_for_existing_service_name, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_send_queue_connect_fail_by_clock_gettime, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_send_queue_connect_fail_by_socket_eacces, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_close_client_socket, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_aging, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_on_recv_fail_with_invalid_fd, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_on_send_fail_with_invalid_fd, reset_messenger, reset_messenger ),
    unit_test_setup_teardown( test_del_recv_queue_client_fd_fail_with_invalid_fd, reset_messenger, reset_messenger ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
