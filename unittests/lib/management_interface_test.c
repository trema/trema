/*
 * Unit tests for management_interface.[ch]
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
#include "management_interface.h"
#include "trema.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "cmockery_trema.h"


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static void ( *original_error )( const char *format, ... );
static void ( *original_debug )( const char *format, ... );
static bool ( *original_add_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
static bool ( *original_delete_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
static const char *( *original_get_management_service_name )( const char *service_name );

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
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


static bool
mock_add_message_requested_callback( const char *service_name,
                                     void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  UNUSED( callback );

  check_expected( service_name );

  return ( bool ) mock();
}


static bool
mock_delete_message_requested_callback( const char *service_name,
                                        void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  UNUSED( callback );

  check_expected( service_name );

  return ( bool ) mock();
}


static const char *
mock_get_management_service_name( const char *service_name ) {
  check_expected( service_name );

  return ( const char * ) ( intptr_t ) mock();
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

static void
setup() {
  original_error = error;
  error = mock_error;
  original_debug = debug;
  debug = mock_debug;
  original_add_message_requested_callback= add_message_requested_callback;
  add_message_requested_callback = mock_add_message_requested_callback;
  original_delete_message_requested_callback= delete_message_requested_callback;
  delete_message_requested_callback = mock_delete_message_requested_callback;
  original_get_management_service_name = get_management_service_name;
  get_management_service_name = mock_get_management_service_name;
}


static void
teardown() {
  bool *initialized = _get_management_interface_initialized();
  *initialized = false;

  error = original_error;
  debug = original_debug;
  add_message_requested_callback= original_add_message_requested_callback;
  delete_message_requested_callback= original_delete_message_requested_callback;
  get_management_service_name = original_get_management_service_name;
}


/********************************************************************************
 * init_management_interface() tests.
 ********************************************************************************/

static void
test_init_management_interface_succeeds() {
  const char service_name[] = "tetris";
  const char management_service_name[] = "tetris.m";

  set_trema_name( service_name );

  expect_string( mock_get_management_service_name, service_name, service_name );
  will_return( mock_get_management_service_name, management_service_name );
  expect_string( mock_add_message_requested_callback, service_name, management_service_name );
  will_return( mock_add_message_requested_callback, true );
  expect_string( mock_get_management_service_name, service_name, service_name );
  will_return( mock_get_management_service_name, management_service_name );

  assert_true( init_management_interface() );
  bool *initialized = _get_management_interface_initialized();
  assert_true( *initialized );

  _free_trema_name();
}


static void
test_init_management_interface_fails_if_already_initialized() {
  const char service_name[] = "tetris";
  const char management_service_name[] = "tetris.m";

  set_trema_name( service_name );
  expect_string( mock_get_management_service_name, service_name, service_name );
  will_return( mock_get_management_service_name, management_service_name );
  expect_string( mock_add_message_requested_callback, service_name, management_service_name );
  will_return( mock_add_message_requested_callback, true );
  expect_string( mock_get_management_service_name, service_name, service_name );
  will_return( mock_get_management_service_name, management_service_name );
  init_management_interface();

  expect_string( mock_error, message, "Management interface is already initialized." );

  assert_false( init_management_interface() );
  bool *initialized = _get_management_interface_initialized();
  assert_true( *initialized );

  _free_trema_name();
}


/********************************************************************************
 * finalize_management_interface() tests.
 ********************************************************************************/

static void
test_finalize_management_interface_succeeds() {
  const char service_name[] = "tetris";
  const char management_service_name[] = "tetris.m";

  set_trema_name( service_name );
  bool *initialized = _get_management_interface_initialized();
  *initialized = true;

  expect_string( mock_get_management_service_name, service_name, service_name );
  will_return( mock_get_management_service_name, management_service_name );
  expect_string( mock_delete_message_requested_callback, service_name, management_service_name );
  will_return( mock_delete_message_requested_callback, true );

  assert_true( finalize_management_interface() );
  initialized = _get_management_interface_initialized();
  assert_false( *initialized );

  _free_trema_name();
}


static void
test_finalize_management_interface_fails_if_not_initialized() {
  expect_string( mock_error, message, "Management interface is not initialized yet or already finalized." );

  assert_false( finalize_management_interface() );
  bool *initialized = _get_management_interface_initialized();
  assert_false( *initialized );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // init_management_interface() tests.
    unit_test_setup_teardown( test_init_management_interface_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_init_management_interface_fails_if_already_initialized,
                              setup, teardown ),
    unit_test_setup_teardown( test_finalize_management_interface_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_finalize_management_interface_fails_if_not_initialized,
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
