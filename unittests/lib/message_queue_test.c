/*
 * Unit tests for message queue.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "log.h"
#include "message_queue.h"
#include "wrapper.h"


/*************************************************************************
 * Helper.
 *************************************************************************/

// Setup and teardown function.

static void ( *original_die )( const char *format, ... );

static void
mock_die( const char *format, ... ) {
  UNUSED( format );
  mock_assert( false, "mock_die", __FILE__, __LINE__ ); } // Hoaxes gcov.


static void
setup() {
  original_die = die;
  die = mock_die;
}


static void
teardown() {
  die = original_die;
}


/*************************************************************************
 * create and delete tests.
 *************************************************************************/

static void
test_create_and_delete_message_queue() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );
  assert_int_equal( queue->length, 0 );

  assert_true( delete_message_queue( queue ) );
}


static void
test_delete_message_queue_if_queue_is_not_created() {
  message_queue *queue = NULL;
  expect_assert_failure( delete_message_queue( queue ) );
}


static void
test_enqueue_and_delete_message_queue() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );
  assert_int_equal( queue->length, 0 );

  buffer *buf = alloc_buffer();
  assert_true( enqueue_message( queue, buf ) );
  assert_int_equal( queue->length, 1 );

  assert_true( delete_message_queue( queue ) );
}


/*************************************************************************
 * enqeue and dequeue tests.
 *************************************************************************/

static void
test_enqueue_and_dequeue_message_queue() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );

  buffer *enq_buf = alloc_buffer();
  assert_true( enqueue_message( queue, enq_buf ) );
  assert_int_equal( queue->length, 1 );

  buffer *deq_buf = dequeue_message( queue );
  assert_true( deq_buf == enq_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 0 );

  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == NULL );
  assert_int_equal( queue->length, 0 );

  assert_true( delete_message_queue( queue ) );
}


static void
test_multiple_queueing() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );

  buffer *first_buf = alloc_buffer();
  assert_true( enqueue_message( queue, first_buf ) );
  assert_int_equal( queue->length, 1 );

  buffer *deq_buf = dequeue_message( queue );
  assert_true( deq_buf == first_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 0 );


  first_buf = alloc_buffer();
  assert_true( enqueue_message( queue, first_buf ) );
  assert_int_equal( queue->length, 1 );
  buffer *second_buf = alloc_buffer();
  assert_true( enqueue_message( queue, second_buf ) );
  assert_int_equal( queue->length, 2 );

  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == first_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 1 );

  buffer *third_buf = alloc_buffer();
  assert_true( enqueue_message( queue, third_buf ) );
  assert_int_equal( queue->length, 2 );
  buffer *fourth_buf = alloc_buffer();
  assert_true( enqueue_message( queue, fourth_buf ) );
  assert_int_equal( queue->length, 3 );

  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == second_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 2 );
  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == third_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 1 );
  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == fourth_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 0 );


  assert_true( delete_message_queue( queue ) );
}


static void
test_enqueue_message_if_queue_is_not_created() {
  message_queue *queue = NULL;
  buffer *enq_buf = alloc_buffer();
  expect_assert_failure( enqueue_message( queue, enq_buf ) );
  free_buffer( enq_buf );
}


static void
test_enqueue_message_if_message_is_NULL() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );
  assert_int_equal( queue->length, 0 );

  expect_assert_failure( enqueue_message( queue, NULL ) );

  assert_true( delete_message_queue( queue ) );
}


static void
test_dequeue_message_if_queue_is_not_created() {
  message_queue *queue = NULL;
  expect_assert_failure( dequeue_message( queue ) );
}


/*************************************************************************
 * peek tests.
 *************************************************************************/

static void
test_peek_message() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );

  buffer *peek_buf = peek_message( queue );
  assert_true( peek_buf == NULL );

  buffer *first_buf = alloc_buffer();
  assert_true( enqueue_message( queue, first_buf ) );
  assert_int_equal( queue->length, 1 );
  buffer *second_buf = alloc_buffer();
  assert_true( enqueue_message( queue, second_buf ) );
  assert_int_equal( queue->length, 2 );
  buffer *third_buf = alloc_buffer();
  assert_true( enqueue_message( queue, third_buf ) );
  assert_int_equal( queue->length, 3 );

  peek_buf = peek_message( queue );
  assert_true( peek_buf == first_buf );
  buffer *deq_buf = dequeue_message( queue );
  assert_true( deq_buf == first_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 2 );

  peek_buf = peek_message( queue );
  assert_true( peek_buf == second_buf );
  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == second_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 1 );

  peek_buf = peek_message( queue );
  assert_true( peek_buf == third_buf );
  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == third_buf );
  free_buffer( deq_buf );
  assert_int_equal( queue->length, 0 );

  peek_buf = peek_message( queue );
  assert_true( peek_buf == NULL );
  deq_buf = dequeue_message( queue );
  assert_true( deq_buf == NULL );
  assert_int_equal( queue->length, 0 );

  assert_true( delete_message_queue( queue ) );
}


static void
test_peek_message_if_queue_is_not_created() {
  message_queue *queue = NULL;
  expect_assert_failure( peek_message( queue ) );
}


/*************************************************************************
 * foreach_message_queue tests.
 *************************************************************************/


static int count = 0;

static bool
test_foreach_message_queue_helper( buffer *message, void *user_data ) {
  const char *str = message->data;
  assert_string_equal( ( char * ) user_data, "user_data" );
  switch ( ++count ) {
    case 1:
      assert_string_equal( str, "second_buffer" );
      break;
    case 2:
      assert_string_equal( str, "third_buffer" );
      break;
    default:
      assert_true( false );
      break;
  }

  return true;
}


static void
append_string( buffer *buf, const char *str ) {
  char *text = append_back_buffer( buf, strlen( str ) + 1 );
  strcpy( text, str );
}


static void
test_foreach_message_queue() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );

  buffer *first_buf = alloc_buffer();
  append_string( first_buf, "first_buffer" );
  assert_true( enqueue_message( queue, first_buf ) );
  assert_int_equal( queue->length, 1 );
  buffer *second_buf = alloc_buffer();
  append_string( second_buf, "second_buffer" );
  assert_true( enqueue_message( queue, second_buf ) );
  assert_int_equal( queue->length, 2 );
  buffer *third_buf = alloc_buffer();
  append_string( third_buf, "third_buffer" );
  assert_true( enqueue_message( queue, third_buf ) );
  assert_int_equal( queue->length, 3 );

  buffer *deq_buf = dequeue_message( queue );
  assert_true( deq_buf == first_buf );

  char *user_data = xstrdup( "user_data" );
  count = 0;

  foreach_message_queue( queue, test_foreach_message_queue_helper, user_data );

  assert_int_equal( count, 2 );

  xfree( user_data );
  assert_true( delete_message_queue( queue ) );
}


static bool
test_foreach_message_queue_if_queue_is_empty_helper( buffer *message, void *user_data ) {
  UNUSED( message );
  UNUSED( user_data );
  assert_true( false );

  return true;
}


static void
test_foreach_message_queue_if_queue_is_empty() {
  message_queue *queue = create_message_queue();
  assert_true( queue != NULL );

  char *user_data = xstrdup( "user_data" );
  foreach_message_queue( queue, test_foreach_message_queue_if_queue_is_empty_helper, user_data );

  assert_int_equal( count, 2 );

  xfree( user_data );
  assert_true( delete_message_queue( queue ) );
}


/*************************************************************************
 * Run tests.
 *************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_create_and_delete_message_queue, setup, teardown ),
    unit_test_setup_teardown( test_delete_message_queue_if_queue_is_not_created, setup, teardown ),
    unit_test_setup_teardown( test_enqueue_and_delete_message_queue, setup, teardown ),
    unit_test_setup_teardown( test_enqueue_and_dequeue_message_queue, setup, teardown ),
    unit_test_setup_teardown( test_multiple_queueing, setup, teardown ),
    unit_test_setup_teardown( test_enqueue_message_if_queue_is_not_created, setup, teardown ),
    unit_test_setup_teardown( test_enqueue_message_if_message_is_NULL, setup, teardown ),
    unit_test_setup_teardown( test_dequeue_message_if_queue_is_not_created, setup, teardown ),
    unit_test_setup_teardown( test_peek_message, setup, teardown ),
    unit_test_setup_teardown( test_peek_message_if_queue_is_not_created, setup, teardown ),
    unit_test_setup_teardown( test_foreach_message_queue, setup, teardown ),
    unit_test_setup_teardown( test_foreach_message_queue_if_queue_is_empty, setup, teardown ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
