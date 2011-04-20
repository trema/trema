/*
 * Unit tests for buf functions and macros.
 *
 * Author: Shin-ya Zenke
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


#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "checks.h"
#include "cmockery.h"
#include "openflow.h"
#include "packet_info.h"
#include "unittest.h"


typedef struct tea {
  const char *name;
  const char *origin;
} tea;

static tea CEYLON = { "Ceylon", "Sri Lanka" };
static tea DARJEELING = { "Darjeeling", "India" };


typedef struct private_buffer {
  buffer public;
  size_t real_length;
  void *top;
  pthread_mutex_t *mutex;
} private_buffer;


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static bool mutex_initialized = false;

int
mock_pthread_mutex_init( pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr ) {
  UNUSED( mutex );
  UNUSED( mutexattr );

  mutex_initialized = true;

  return 0;
}


int
mock_pthread_mutexattr_init( pthread_mutexattr_t *attr ) {
  UNUSED( attr );

  return 0;
}


int
mock_pthread_mutexattr_settype( pthread_mutexattr_t *attr, int kind ) {
  UNUSED( attr );
  UNUSED( kind );

  return 0;
}


int
mock_pthread_mutex_lock( pthread_mutex_t *mutex ) {
  check_expected( mutex );

  return 0;
}


int
mock_pthread_mutex_unlock( pthread_mutex_t *mutex ) {
  check_expected( mutex );

  return 0;
}


int
mock_pthread_mutex_destroy( pthread_mutex_t *mutex ) {
  UNUSED( mutex );

  mutex_initialized = false;

  return 0;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_alloc_buffer_succeeds() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );
  assert_true( mutex_initialized );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_alloc_buffer_with_length_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );
  assert_true( mutex_initialized );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_alloc_buffer_with_length_fails_if_length_is_0() {
  expect_assert_failure( alloc_buffer_with_length( 0 ) );
  assert_false( mutex_initialized );
}


static void
test_free_buffer_succeeds() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_free_buffer_fails_if_buffer_pointer_is_NULL() {
  expect_assert_failure( free_buffer( NULL ) );
}


static void
test_append_front_buffer_new_alloc_succeeds() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_front_twice_suceeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  memcpy( data_pointer, &CEYLON, sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  tea *tea_data = ( tea * ) ( ( char * ) data_pointer + sizeof( tea ) );
  assert_true( 0 == strcmp( tea_data->name, CEYLON.name ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_front_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_front_buffer_resize_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_front_buffer_fails_if_buffer_is_NULL() {
  expect_assert_failure( append_front_buffer( NULL, sizeof( tea ) ) );
}


static void
test_append_front_buffer_fails_if_length_is_0() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  expect_assert_failure( append_front_buffer( buf, 0 ) );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}

static void
test_remove_front_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  data_pointer = remove_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_remove_front_buffer_text_insert_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  memcpy( ( char * ) data_pointer + sizeof( tea ), &CEYLON, sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  data_pointer = remove_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );
  tea *tea_data = ( tea * ) ( ( char * ) buf->data );
  assert_true( 0 == strcmp( tea_data->name, CEYLON.name ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_remove_front_buffer_all_removed() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( data_pointer == buf->data );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  data_pointer = remove_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == 0 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_remove_front_buffer_fails_if_large_length_remove() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( data_pointer == buf->data );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_assert_failure( remove_front_buffer( buf, sizeof( tea ) * 2 ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_remove_front_buffer_fails_if_buffer_is_NULL() {
  expect_assert_failure( remove_front_buffer( NULL, sizeof( tea ) ) );
}


static void
test_remove_front_buffer_fails_if_remove_size_is_0() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  expect_assert_failure( remove_front_buffer( buf, 0 ) );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_back_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_back_twice_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 3 );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  memcpy( data_pointer, &CEYLON, sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  memcpy( data_pointer, &DARJEELING, sizeof( tea ) );

  tea *tea_data = ( tea * ) ( ( char * ) buf->data );
  assert_true( 0 == strcmp( tea_data->name, CEYLON.name ) );
  tea_data = ( tea * ) ( ( char * ) buf->data + sizeof( tea ) );
  assert_true( 0 == strcmp( tea_data->name, DARJEELING.name ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_back_buffer_resize_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_back_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_back_buffer_succeeds_if_initialize_length_is_0() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_append_back_buffer_fails_if_buffer_is_NULL() {
  expect_assert_failure( append_back_buffer( NULL, sizeof( tea ) ) );
}


static void
test_append_back_buffer_fails_if_length_is_0() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  expect_assert_failure( append_back_buffer( buf, 0 ) );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
test_duplicate_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  buffer *duplicate = duplicate_buffer( buf );
  assert_true( duplicate != NULL );
  assert_true( duplicate->user_data == buf->user_data );
  assert_true( duplicate->length == buf->length );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
  pthread_mutex_t *expected_duplicate_mutex = ( ( private_buffer * ) duplicate )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_duplicate_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_duplicate_mutex );
  free_buffer( duplicate );
}


static void
test_duplicate_buffer_succeeds_if_initialize_length_is_0() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );
  assert_true( buf->data == NULL );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  buffer *duplicate = duplicate_buffer( buf );
  assert_true( duplicate != NULL );
  assert_true( duplicate->data == NULL );
  assert_true( duplicate->user_data == NULL );
  assert_true( duplicate->length == 0 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
  pthread_mutex_t *expected_duplicate_mutex = ( ( private_buffer * ) duplicate )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_duplicate_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_duplicate_mutex );
  free_buffer( duplicate );
}


static void
test_duplicate_buffer_fails_if_buffer_is_NULL() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  expect_assert_failure( duplicate_buffer( NULL ) );

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


static void
dump_function( const char *format, ... ) {
  char hex[ 1000 ];

  va_list args;
  va_start( args, format );
  vsnprintf( hex, sizeof( hex ), format, args );
  va_end( args );

  check_expected( hex );
}


static void
test_dump_buffer() {
  buffer *buf = alloc_buffer();

  pthread_mutex_t *expected_mutex = ( ( private_buffer * ) buf )->mutex;
  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  void *datap = append_back_buffer( buf, ( size_t ) 1 );
  int data255 = 255;
  memcpy( datap, &data255, ( size_t ) 1 );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_string( dump_function, hex, "ff" );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  dump_buffer( buf, dump_function );

  expect_value( mock_pthread_mutex_lock, mutex, expected_mutex );
  expect_value( mock_pthread_mutex_unlock, mutex, expected_mutex );
  free_buffer( buf );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_alloc_buffer_succeeds ),
    unit_test( test_alloc_buffer_with_length_succeeds ),
    unit_test( test_alloc_buffer_with_length_fails_if_length_is_0 ),

    unit_test( test_free_buffer_succeeds ),
    unit_test( test_free_buffer_fails_if_buffer_pointer_is_NULL ),

    unit_test( test_append_front_twice_suceeds ),
    unit_test( test_append_front_buffer_succeeds ),
    unit_test( test_append_front_buffer_resize_succeeds ),
    unit_test( test_append_front_buffer_new_alloc_succeeds ),
    unit_test( test_append_front_buffer_fails_if_buffer_is_NULL ),
    unit_test( test_append_front_buffer_fails_if_length_is_0 ),

    unit_test( test_remove_front_buffer_succeeds ),
    unit_test( test_remove_front_buffer_text_insert_succeeds ),
    unit_test( test_remove_front_buffer_all_removed ),
    unit_test( test_remove_front_buffer_fails_if_large_length_remove ),
    unit_test( test_remove_front_buffer_fails_if_buffer_is_NULL ),
    unit_test( test_remove_front_buffer_fails_if_remove_size_is_0 ),

    unit_test( test_append_back_buffer_succeeds ),
    unit_test( test_append_back_buffer_resize_succeeds ),
    unit_test( test_append_back_buffer_succeeds_if_initialize_length_is_0 ),
    unit_test( test_append_back_twice_succeeds ),
    unit_test( test_append_back_buffer_fails_if_buffer_is_NULL ),
    unit_test( test_append_back_buffer_fails_if_length_is_0 ),

    unit_test( test_duplicate_buffer_succeeds ),
    unit_test( test_duplicate_buffer_succeeds_if_initialize_length_is_0 ),
    unit_test( test_duplicate_buffer_fails_if_buffer_is_NULL ),

    unit_test( test_dump_buffer ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
