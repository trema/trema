/*
 * Unit tests for timer.[ch]
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


#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "doubly_linked_list.h"
#include "timer.h"


/********************************************************************************
 * Static data and types
 ********************************************************************************/

// FIXME
typedef struct timer_callback_info {
  void ( *function )( void *user_data );
  struct timespec expires_at;
  struct timespec interval;
  void *user_data;
} timer_callback_info;


extern dlist_element *timer_callbacks;


/********************************************************************************
 * Mocks.
 ********************************************************************************/

int
mock_clock_gettime( clockid_t clk_id, struct timespec *tp ) {
  UNUSED( clk_id );
  UNUSED( tp );

  return ( int ) mock();
}


void
mock_error( const char *format, ... ) {
  // Do nothing.
  UNUSED( format );
}


void
mock_debug( const char *format, ... ) {
  // Do nothing.
  UNUSED( format );
}


/********************************************************************************
 * Helper functions.
 ********************************************************************************/

static timer_callback_info *
find_timer_callback( void ( *callback )( void *user_data ) ) {
  dlist_element *e;
  timer_callback_info *cb;

  cb = NULL;
  for ( e = timer_callbacks->next; e; e = e->next ) {
    cb = e->data;
    if ( cb->function == callback ) {
      return cb;
    }
  }
  return NULL;
}


/********************************************************************************
 * Tests
 ********************************************************************************/

static void
mock_timer_event_callback( void *user_data ) {
  UNUSED( user_data );
}


static void
test_timer_event_callback() {
  init_timer();

  will_return_count( mock_clock_gettime, 0, -1 );

  char user_data[] = "It's time!!!";
  struct itimerspec interval;
  interval.it_value.tv_sec = 1;
  interval.it_value.tv_nsec = 1000;
  interval.it_interval.tv_sec = 2;
  interval.it_interval.tv_nsec = 2000;
  assert_true( add_timer_event_callback( &interval, mock_timer_event_callback, user_data ) );

  timer_callback_info *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->function == mock_timer_event_callback );
  assert_string_equal( callback->user_data, "It's time!!!" );
  assert_int_equal( callback->interval.tv_sec, 2 );
  assert_int_equal( callback->interval.tv_nsec, 2000 );

  delete_timer_event( mock_timer_event_callback, user_data );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );

  finalize_timer();
}


static void
test_periodic_event_callback() {
  init_timer();

  char user_data[] = "It's time!!!";
  will_return_count( mock_clock_gettime, 0, -1 );
  assert_true( add_periodic_event_callback( 1, mock_timer_event_callback, user_data ) );

  timer_callback_info *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->function == mock_timer_event_callback );
  assert_string_equal( callback->user_data, "It's time!!!" );
  assert_int_equal( callback->interval.tv_sec, 1 );
  assert_int_equal( callback->interval.tv_nsec, 0 );

  delete_timer_event( mock_timer_event_callback, user_data );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );

  finalize_timer();
}


static void
test_add_timer_event_callback_fail_with_invalid_timespec() {
  init_timer();

  will_return_count( mock_clock_gettime, 0, -1 );

  char user_data[] = "It's time!!!";
  struct itimerspec interval;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 0;
  assert_false( add_timer_event_callback( &interval, mock_timer_event_callback, user_data ) );

  finalize_timer();
}


static void
test_delete_timer_event() {
  init_timer();

  will_return_count( mock_clock_gettime, 0, -1 );

  char user_data_1[] = "1";
  char user_data_2[] = "2";

  struct itimerspec interval;
  interval.it_value.tv_sec = 1;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 1;
  interval.it_interval.tv_nsec = 0;
  assert_true( add_timer_event_callback( &interval, mock_timer_event_callback, user_data_1 ) );
  interval.it_value.tv_sec = 2;
  interval.it_interval.tv_sec = 2;
  assert_true( add_timer_event_callback( &interval, mock_timer_event_callback, user_data_2 ) );

  delete_timer_event( mock_timer_event_callback, user_data_1 );

  timer_callback_info *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->user_data == user_data_2 );

  delete_timer_event( mock_timer_event_callback, user_data_2 );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );

  finalize_timer();
}


static void
test_nonexistent_timer_event_callback() {
  assert_false( delete_timer_event( mock_timer_event_callback, NULL ) );
}


static void
test_clock_gettime_fail_einval() {
  init_timer();

  char user_data[] = "It's time!!!";
  will_return_count( mock_clock_gettime, -1, -1 );
  assert_false( add_periodic_event_callback( 1, mock_timer_event_callback, user_data ) );

  finalize_timer();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_timer_event_callback ),
    unit_test( test_periodic_event_callback ),
    unit_test( test_add_timer_event_callback_fail_with_invalid_timespec ),
    unit_test( test_delete_timer_event ),
    unit_test( test_nonexistent_timer_event_callback ),
    unit_test( test_clock_gettime_fail_einval ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
