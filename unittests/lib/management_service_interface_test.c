/*
 * Unit tests for management_service_interface.[ch]
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


#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"
#include "trema_wrapper.h"
#include "cmockery_trema.h"


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static void ( *original_die )( const char *format, ... );
static void ( *original_error )( const char *format, ... );
static void ( *original_set_management_application_request_handler )( management_application_request_handler callback, void *user_data );
static bool ( *original_send_reply_message )( const messenger_context_handle *handle, const uint16_t tag, const void *data, size_t length );

static void
mock_die( const char *format, ... ) {
  UNUSED( format );
  mock_assert( false, "mock_die", __FILE__, __LINE__ ); } // Hoaxes gcov.


static void
mock_error( const char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
}


static void
mock_set_management_application_request_handler( management_application_request_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );
}


static bool
mock_send_reply_message( const messenger_context_handle *handle, const uint16_t tag, const void *data, size_t length ) {
  check_expected( handle );
  check_expected( tag );
  check_expected( data );
  check_expected( length );

  return ( bool ) mock();
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

static void
setup() {
  original_die = die;
  die = mock_die;
  original_error = error;
  error = mock_error;
  original_set_management_application_request_handler = set_management_application_request_handler;
  set_management_application_request_handler = mock_set_management_application_request_handler;
  original_send_reply_message = send_reply_message;
  send_reply_message = mock_send_reply_message;
}


static void
teardown() {
  die = original_die;
  error = original_error;
  set_management_application_request_handler = original_set_management_application_request_handler;
  send_reply_message = original_send_reply_message;
}


/********************************************************************************
 * get_management_service_name() tests.
 ********************************************************************************/

static void
test_get_management_service_name_succeeds() {
  assert_string_equal( get_management_service_name( "tetris" ), "tetris.m" );
}


static void
test_get_management_service_name_fails_if_service_name_is_NULL() {
  expect_assert_failure( get_management_service_name( NULL ) );
}


static void
test_get_management_service_name_fails_if_service_name_length_is_zero() {
  expect_assert_failure( get_management_service_name( "" ) );
}


static void
test_get_management_service_name_fails_with_too_long_service_name() {
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  memset( service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  memset( service_name, 'a', MESSENGER_SERVICE_NAME_LENGTH - 1 );

  expect_assert_failure( get_management_service_name( service_name ) );
}


/********************************************************************************
 * set_management_application_request_handler() test.
 ********************************************************************************/

static void
test_set_management_application_request_handler_succeeds() {
  management_application_request_handler callback = ( void * ) ( intptr_t ) 0x1;
  void *user_data = ( void * ) ( intptr_t ) 0x2;

  expect_value( mock_set_management_application_request_handler, callback, callback );
  expect_value( mock_set_management_application_request_handler, user_data, user_data );
  set_management_application_request_handler( callback, user_data );
}


/********************************************************************************
 * create_management_application_reply() tests.
 ********************************************************************************/

static void
test_create_management_application_reply_succeeds_without_data() {
  uint8_t status = MANAGEMENT_REQUEST_SUCCEEDED;
  uint32_t id = 1;
  void *data = NULL;
  size_t data_length = 0;

  management_application_reply *reply = create_management_application_reply( status, id, data, data_length );
  assert_int_equal( ntohs( reply->header.type ), MANAGEMENT_APPLICATION_REPLY );
  assert_int_equal( reply->header.status, status );
  assert_int_equal( ntohl( reply->header.length ), offsetof( management_application_reply, data ) );
  assert_int_equal( ntohl( reply->application_id ), id );

  xfree( reply );
}


static void
test_create_management_application_reply_succeeds_with_data() {
  uint8_t status = MANAGEMENT_REQUEST_SUCCEEDED;
  uint32_t id = 1;
  size_t data_length = 128;
  uint8_t data[ data_length ];

  for ( unsigned int i = 0; i < data_length; i++ ) {
    data[ i ] = ( uint8_t ) i;
  }

  management_application_reply *reply = create_management_application_reply( status, id, ( void * ) data, data_length );
  assert_int_equal( ntohs( reply->header.type ), MANAGEMENT_APPLICATION_REPLY );
  assert_int_equal( reply->header.status, status );
  assert_int_equal( ntohl( reply->header.length ), offsetof( management_application_reply, data ) + data_length );
  assert_int_equal( ntohl( reply->application_id ), id );
  assert_memory_equal( reply->data, data, data_length );

  xfree( reply );
}


/********************************************************************************
 * send_management_application_reply() tests.
 ********************************************************************************/

static void
test_send_management_application_reply_succeeds() {
  uint8_t status = MANAGEMENT_REQUEST_SUCCEEDED;
  uint32_t id = 1;
  size_t data_length = 128;
  uint8_t data[ data_length ];

  for ( unsigned int i = 0; i < data_length; i++ ) {
    data[ i ] = ( uint8_t ) i;
  }

  management_application_reply *reply = create_management_application_reply( status, id, ( void * ) data, data_length );
  messenger_context_handle *handle = ( void * ) ( intptr_t ) 0x1;

  expect_value( mock_send_reply_message, handle, handle );
  expect_value( mock_send_reply_message, tag, MESSENGER_MANAGEMENT_REPLY );
  expect_memory( mock_send_reply_message, data, reply, ( size_t ) ntohl( reply->header.length ) );
  expect_value( mock_send_reply_message, length, ( size_t ) ntohl( reply->header.length ) );
  will_return( mock_send_reply_message, true );

  assert_true( send_management_application_reply( handle, reply ) );

  xfree( reply );
}


static void
test_send_management_application_reply_fails_without_reply_message() {
  messenger_context_handle *handle = ( void * ) ( intptr_t ) 0x1;

  expect_string( mock_error, message, "Both context handle and reply message must not be NULL ( handle = 0x1, reply = (nil) )." );
  assert_false( send_management_application_reply( handle, NULL ) );
}


static void
test_send_management_application_reply_fails_without_context_handle() {
  messenger_context_handle *handle = NULL;
  management_application_reply *reply = ( void * ) ( intptr_t ) 0x1;

  expect_string( mock_error, message, "Both context handle and reply message must not be NULL ( handle = (nil), reply = 0x1 )." );
  assert_false( send_management_application_reply( handle, reply ) );
}


static void
test_send_management_application_reply_fails_if_send_reply_message_returns_false() {
  uint8_t status = MANAGEMENT_REQUEST_SUCCEEDED;
  uint32_t id = 1;
  size_t data_length = 128;
  uint8_t data[ data_length ];

  for ( unsigned int i = 0; i < data_length; i++ ) {
    data[ i ] = ( uint8_t ) i;
  }

  management_application_reply *reply = create_management_application_reply( status, id, ( void * ) data, data_length );
  messenger_context_handle *handle = ( void * ) ( intptr_t ) 0x1;

  expect_value( mock_send_reply_message, handle, handle );
  expect_value( mock_send_reply_message, tag, MESSENGER_MANAGEMENT_REPLY );
  expect_memory( mock_send_reply_message, data, reply, ( size_t ) ntohl( reply->header.length ) );
  expect_value( mock_send_reply_message, length, ( size_t ) ntohl( reply->header.length ) );
  will_return( mock_send_reply_message, false );
  expect_string( mock_error, message, "Failed to send an application specific management reply." );

  assert_false( send_management_application_reply( handle, reply ) );

  xfree( reply );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // get_management_service_name() tests.
    unit_test_setup_teardown( test_get_management_service_name_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_get_management_service_name_fails_if_service_name_is_NULL,
                              setup, teardown ),
    unit_test_setup_teardown( test_get_management_service_name_fails_if_service_name_length_is_zero,
                              setup, teardown ),
    unit_test_setup_teardown( test_get_management_service_name_fails_with_too_long_service_name,
                              setup, teardown ),

    // set_management_application_request_handler() test.
    unit_test_setup_teardown( test_set_management_application_request_handler_succeeds,
                              setup, teardown ),

    // create_management_application_reply() tests.
    unit_test_setup_teardown( test_create_management_application_reply_succeeds_without_data,
                              setup, teardown ),
    unit_test_setup_teardown( test_create_management_application_reply_succeeds_with_data,
                              setup, teardown ),

    // send_management_application_reply() tests.
    unit_test_setup_teardown( test_send_management_application_reply_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_send_management_application_reply_fails_without_reply_message,
                              setup, teardown ),
    unit_test_setup_teardown( test_send_management_application_reply_fails_without_context_handle,
                              setup, teardown ),
    unit_test_setup_teardown( test_send_management_application_reply_fails_if_send_reply_message_returns_false,
                              setup, teardown ),
  };
  setup_leak_detector();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
