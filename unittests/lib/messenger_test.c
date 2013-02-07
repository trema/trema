/*
 * Unit tests for Trema messenger.
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


#include <arpa/inet.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "doubly_linked_list.h"
#include "hash_table.h"
#include "event_handler.h"
#include "messenger.h"
#include "timer.h"
#include "wrapper.h"


#define static extern


enum {
  MESSAGE_TYPE_NOTIFY,
  MESSAGE_TYPE_REQUEST,
  MESSAGE_TYPE_REPLY,
};

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

static messenger_context *insert_context( void *user_data );
static messenger_context *get_context( uint32_t transaction_id );
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


/********************************************************************************
 * Mocks.
 ********************************************************************************/

void
mock_execute_timer_events( int *next_timeout_usec ) {
  UNUSED( next_timeout_usec );
  // Do nothing.
}


bool
mock_add_periodic_event_callback( const time_t seconds, void ( *callback )( void *user_data ), void *user_data ) {
  UNUSED( seconds );
  UNUSED( callback );
  UNUSED( user_data );
  return true;
}


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
  UNUSED( s );
  UNUSED( level );
  UNUSED( optname );
  UNUSED( optval );
  UNUSED( optlen );
  return 0;
}


int
mock_clock_gettime( clockid_t clk_id, struct timespec *tp ) {
  UNUSED( clk_id );
  UNUSED( tp );

  return ( int ) mock();
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


static bool check_warn = false;
static void
mock_warn_check( char *format, va_list args ) {
  char message[ 1000 ];
  vsnprintf( message, 1000, format, args );

  check_expected( message );
}


void
mock_warn( char *format, ... ) {
  UNUSED( format );
  if ( check_warn ) {
    va_list arg;
    va_start( arg, format );
    mock_warn_check( format, arg );
    va_end( arg );
  }
}


/********************************************************************************
 * Callbacks.
 ********************************************************************************/

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

  stop_event_handler();
  assert_true( stop_messenger() );
}


/********************************************************************************
 * Helpers.
 ********************************************************************************/

static void
reset_messenger() {
  initialized = false;
  finalized = false;

  execute_timer_events = mock_execute_timer_events;

  check_warn = false;
}


/********************************************************************************
 * Init and finalize messenger tests.
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
  assert_false( finalize_messenger() );
}


static void
test_finalize_without_init() {
  assert_false( finalize_messenger() );
}


/********************************************************************************
 * Message callback tests.
 ********************************************************************************/

static void
callback_hello( uint16_t tag, void *data, size_t len ) {
  check_expected( tag );
  check_expected( data );
  check_expected( len );

  stop_event_handler();
  stop_messenger();
}


static void
test_send_then_message_received_callback_is_called() {
  init_messenger( "/tmp" );

  const char service_name[] = "Say HELLO";

  expect_value( callback_hello, tag, 43556 );
  expect_string( callback_hello, data, "HELLO" );
  expect_value( callback_hello, len, 6 );

  add_message_received_callback( service_name, callback_hello );
  send_message( service_name, 43556, "HELLO", strlen( "HELLO" ) + 1 );
  start_messenger();
  start_event_handler();

  delete_message_received_callback( service_name, callback_hello );
  delete_send_queue( lookup_hash_entry( send_queues, service_name ) );

  finalize_messenger();
}


static void callback_req_hello( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) {
  UNUSED( handle );
  check_expected( tag );
  check_expected( data );
  check_expected( len );

  send_reply_message( handle, 65534, "OLLEH", strlen("OLLEH")+1 );
}


static void callback_req_hello2( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) {
  UNUSED( handle );
  UNUSED( tag );
  UNUSED( data );
  UNUSED( len );
}


static void callback_rep_hello_end( uint16_t tag, void *data, size_t len, void *user_data ) {
  check_expected( tag );
  check_expected( data );
  check_expected( len );
  check_expected( user_data );

  stop_event_handler();
  stop_messenger();
}


static void callback_rep_hello2( uint16_t tag, void *data, size_t len, void *user_data ) {
  UNUSED( tag );
  UNUSED( data );
  UNUSED( len );
  UNUSED( user_data );
}


static void
test_send_then_message_requested_and_replied_callback_is_called() {
  init_messenger( "/tmp" );

  const char service_name[] = "Say HELLO";

  expect_value( callback_req_hello, tag, 43556 );
  expect_string( callback_req_hello, data, "HELLO" );
  expect_value( callback_req_hello, len, 6 );
  add_message_requested_callback( service_name, callback_req_hello );

  expect_value( callback_rep_hello_end, tag, 65534 );
  expect_string( callback_rep_hello_end, data, "OLLEH" );
  expect_value( callback_rep_hello_end, len, 6 );
  expect_value( callback_rep_hello_end, user_data, NULL );
  add_message_replied_callback( service_name, callback_rep_hello_end );

  send_request_message( service_name, service_name,  43556, "HELLO", strlen( "HELLO" ) + 1, NULL );

  start_messenger();
  start_event_handler();

  delete_message_replied_callback( service_name, callback_rep_hello_end );
  delete_message_requested_callback( service_name, callback_req_hello );
  delete_send_queue( lookup_hash_entry( send_queues, service_name ) );

  finalize_messenger();
}


static void
test_double_add_message_requested_callback_prints_error_message() {
  init_messenger( "/tmp" );


  const char service_name[] = "Some Service";
  char expected_mes[ 1024+1 ] = {};
  snprintf( expected_mes, 1024, "Multiple message_requested/replied handler is not supported. ( service_name = %s, message_type = %#x, callback = %p )",
           service_name, MESSAGE_TYPE_REQUEST, callback_req_hello2 );


  check_warn = true;

  assert_true( add_message_requested_callback( service_name, callback_req_hello ) );

  expect_string( mock_warn_check, message, expected_mes );
  assert_true( add_message_requested_callback( service_name, callback_req_hello2 ) );

  check_warn = false;

  finalize_messenger();
}

static void
test_double_add_message_replied_callback_prints_error_message() {
  init_messenger( "/tmp" );


  const char service_name[] = "Some Service";
  char expected_mes[ 1024+1 ] = {};
  snprintf( expected_mes, 1024, "Multiple message_requested/replied handler is not supported. ( service_name = %s, message_type = %#x, callback = %p )",
           service_name, MESSAGE_TYPE_REPLY, callback_rep_hello2 );


  check_warn = true;

  assert_true( add_message_replied_callback( service_name, callback_rep_hello_end ) );

  expect_string( mock_warn_check, message, expected_mes );
  assert_true( add_message_replied_callback( service_name, callback_rep_hello2 ) );

  check_warn = false;

  finalize_messenger();
}


static void
test_add_1_message_requested_and_replied_callback_each_prints_no_error_message() {
  init_messenger( "/tmp" );

  const char service_name[] = "Some Service";

  check_warn = true;

  assert_true( add_message_requested_callback( service_name, callback_req_hello ) );

  assert_true( add_message_replied_callback( service_name, callback_rep_hello_end ) );

  check_warn = false;

  finalize_messenger();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // init and finalize messenger tests.
    unit_test_setup_teardown( test_init_and_finalize,
                              reset_messenger,
                              reset_messenger ),
    unit_test_setup_teardown( test_init_twice,
                              reset_messenger,
                              reset_messenger ),
    unit_test_setup_teardown( test_finalize_twice,
                              reset_messenger,
                              reset_messenger ),
    unit_test_setup_teardown( test_finalize_without_init,
                              reset_messenger,
                              reset_messenger ),

    // Message callback tests.
    unit_test_setup_teardown( test_send_then_message_received_callback_is_called,
                              reset_messenger,
                              reset_messenger ),
    // Message request callback tests.
    unit_test_setup_teardown( test_send_then_message_requested_and_replied_callback_is_called,
                              reset_messenger,
                              reset_messenger ),
    // Message request duplicate registrationcallback tests.
    unit_test_setup_teardown( test_double_add_message_requested_callback_prints_error_message,
                              reset_messenger,
                              reset_messenger ),
    unit_test_setup_teardown( test_double_add_message_replied_callback_prints_error_message,
                              reset_messenger,
                              reset_messenger ),
    unit_test_setup_teardown( test_add_1_message_requested_and_replied_callback_each_prints_no_error_message,
                              reset_messenger,
                              reset_messenger ),

  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
