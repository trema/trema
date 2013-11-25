/*
 * Unit tests for external_callback.
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


#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "external_callback.h"
#include "utility.h"

typedef struct {
  external_callback_t *buffer;
  unsigned int size;
  unsigned int write_position;
  unsigned int read_position;
} ring_buffer;

#define MAX_EXTERNAL_CALLBACK 16
extern ring_buffer *external_callbacks;


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static ring_buffer ring_buffer0;
static external_callback_t buffer0[ MAX_EXTERNAL_CALLBACK ];

void *
mock_xcalloc( size_t nmemb, size_t size ) {
  check_expected( nmemb );
  check_expected( size );
  return ( void * ) ( intptr_t ) mock();
}


void
mock_xfree( void *ptr ) {
  check_expected( ptr );
}


void
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


static void
reset() {
  external_callbacks = NULL;
}


static void
setup() {
  memset( &ring_buffer0, 0, sizeof( ring_buffer0 ) );
  memset( buffer0, 0, sizeof( buffer0 ) );
  external_callbacks = &ring_buffer0;
  external_callbacks->buffer = buffer0;
  external_callbacks->size = MAX_EXTERNAL_CALLBACK;
  external_callbacks->write_position = 0;
  external_callbacks->read_position = 0;
}


static void
teardown() {
  external_callbacks = NULL;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_init_external_callback() {
  expect_value( mock_xcalloc, nmemb, 1 );
  expect_value( mock_xcalloc, size, sizeof( ring_buffer ) );
  memset( &ring_buffer0, 0, sizeof( ring_buffer0 ) );
  will_return( mock_xcalloc, &ring_buffer0 );
  expect_value( mock_xcalloc, nmemb, MAX_EXTERNAL_CALLBACK );
  expect_value( mock_xcalloc, size, sizeof( external_callback_t ) );
  memset( buffer0, 0, sizeof( buffer0 ) );
  will_return( mock_xcalloc, buffer0 );

  init_external_callback();

  assert_true( external_callbacks == &ring_buffer0 );
  assert_true( external_callbacks->buffer == buffer0 );
  assert_true( external_callbacks->size == MAX_EXTERNAL_CALLBACK );
  assert_true( external_callbacks->write_position == 0 );
  assert_true( external_callbacks->read_position == 0 );
}


static void
test_init_external_callback_fails_if_already_initialized() {
  expect_assert_failure( init_external_callback() );
}


static void
test_finalize_external_callback() {
  expect_value( mock_xfree, ptr, buffer0 );
  expect_value( mock_xfree, ptr, &ring_buffer0 );

  finalize_external_callback();

  assert_true( external_callbacks == NULL );
}


static void
test_finalize_external_callback_fails_if_not_initialized() {
  expect_assert_failure( finalize_external_callback() );
}


static void
test_finalize_external_callback_fails_if_already_finalized() {
  expect_value( mock_xfree, ptr, buffer0 );
  expect_value( mock_xfree, ptr, &ring_buffer0 );
  finalize_external_callback();

  expect_assert_failure( finalize_external_callback() );
}


static void
external_callback_helper() {
  mock();
}


static void
test_push_external_callback() {
  push_external_callback( external_callback_helper );

  assert_true( external_callbacks->write_position == 1 );
  assert_true( external_callbacks->read_position == 0 );
  assert_true( external_callbacks->buffer[ 0 ] == external_callback_helper );
  for ( int i = 1; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    assert_true( external_callbacks->buffer[ i ] == NULL );
  }
}


static void
test_push_external_callback_fails_if_callback_is_NULL() {
  assert_true( push_external_callback( NULL ) == false );
}


static void
test_push_external_callback_fails_if_not_initialized() {
  assert_true( push_external_callback( external_callback_helper ) == false );
}


static void
test_push_external_callback_succeeds_with_multiple_callbacks() {
  external_callbacks->write_position = 5;
  external_callbacks->read_position = 5;

  for ( int i = 0; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    push_external_callback( external_callback_helper );
  }

  assert_true( external_callbacks->write_position == 5 + MAX_EXTERNAL_CALLBACK );
  assert_true( external_callbacks->read_position == 5 );
  for ( int i = 0; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    assert_true( external_callbacks->buffer[ i ] == external_callback_helper );
  }
}


static void
test_push_external_callback_fails_if_buffer_overflows() {
  external_callbacks->write_position = 3;
  external_callbacks->read_position = 3;

  for ( int i = 0; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    push_external_callback( external_callback_helper );
  }
  expect_assert_failure( push_external_callback( external_callback_helper ) );
}


static void
test_run_external_callback() {
  external_callbacks->write_position = UINT_MAX;
  external_callbacks->read_position = UINT_MAX;
  push_external_callback( external_callback_helper );
  assert_true( external_callbacks->write_position == UINT_MAX + 1 );
  assert_true( external_callbacks->read_position == UINT_MAX );
  will_return( external_callback_helper, 0 );

  run_external_callback();

  assert_true( external_callbacks->write_position == UINT_MAX + 1 );
  assert_true( external_callbacks->read_position == UINT_MAX + 1 );
  for ( int i = 0; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    assert_true( external_callbacks->buffer[ i ] == NULL );
  }
}


static void
test_run_external_callback_succeeds_with_multiple_callbacks() {
  external_callbacks->write_position = UINT_MAX;
  external_callbacks->read_position = UINT_MAX;
  for ( int i = 0; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    push_external_callback( external_callback_helper );
  }
  assert_true( external_callbacks->write_position == UINT_MAX + MAX_EXTERNAL_CALLBACK );
  assert_true( external_callbacks->read_position == UINT_MAX );
  will_return_count( external_callback_helper, 0, MAX_EXTERNAL_CALLBACK );

  run_external_callback();

  assert_true( external_callbacks->write_position == UINT_MAX + MAX_EXTERNAL_CALLBACK );
  assert_true( external_callbacks->read_position == UINT_MAX + MAX_EXTERNAL_CALLBACK );
  for ( int i = 0; i < MAX_EXTERNAL_CALLBACK; i++ ) {
    assert_true( external_callbacks->buffer[ i ] == NULL );
  }
}

/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_init_external_callback, reset, teardown ),
    unit_test_setup_teardown( test_init_external_callback_fails_if_already_initialized, setup, teardown ),

    unit_test_setup_teardown( test_finalize_external_callback, setup, teardown ),
    unit_test_setup_teardown( test_finalize_external_callback_fails_if_not_initialized, reset, teardown ),
    unit_test_setup_teardown( test_finalize_external_callback_fails_if_already_finalized, setup, teardown ),

    unit_test_setup_teardown( test_push_external_callback, setup, teardown ),
    unit_test_setup_teardown( test_push_external_callback_fails_if_callback_is_NULL, setup, teardown ),
    unit_test_setup_teardown( test_push_external_callback_fails_if_not_initialized, reset, teardown ),

    unit_test_setup_teardown( test_push_external_callback_succeeds_with_multiple_callbacks, setup, teardown ),
    unit_test_setup_teardown( test_push_external_callback_fails_if_buffer_overflows, setup, teardown ),

    unit_test_setup_teardown( test_run_external_callback, setup, teardown ),
    unit_test_setup_teardown( test_run_external_callback_succeeds_with_multiple_callbacks, setup, teardown ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
