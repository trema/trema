/*
 * Unit tests for linked list library.
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


#include <stdio.h>
#include <string.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "linked_list.h"
#include "utility.h"


/********************************************************************************
 * Setup and Teardown
 ********************************************************************************/

static void ( *original_die )( const char *format, ... );

static void
mock_die( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );

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


/********************************************************************************
 * List and elements.
 ********************************************************************************/

static list_element *new_list;
static char alpha[] = "alpha";
static char bravo[] = "bravo";
static char charlie[] = "charlie";
static char delete_me[] = "delete me!";


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_create_list() {
  assert_true( create_list( &new_list ) );
  assert_true( new_list == NULL );
}


static void
test_create_and_delete_empty_list() {
  create_list( &new_list );
  delete_list( new_list );
}


static void
test_create_list_aborts_with_NULL() {
  expect_string( mock_die, output, "list must not be NULL" );
  expect_assert_failure( create_list( NULL ) );
}


static void
test_insert_in_front() {
  create_list( &new_list );

  assert_true( insert_in_front( &new_list, alpha ) );
  assert_true( insert_in_front( &new_list, bravo ) );
  assert_true( insert_in_front( &new_list, charlie ) );

  assert_string_equal( new_list->data, "charlie" );
  assert_string_equal( new_list->next->data, "bravo" );
  assert_string_equal( new_list->next->next->data, "alpha" );
  assert_true( new_list->next->next->next == NULL );

  delete_list( new_list );
}


static void
test_insert_in_front_aborts_with_NULL_head() {
  expect_string( mock_die, output, "head must not be NULL" );
  expect_assert_failure( insert_in_front( NULL, alpha ) );
}


static void
test_insert_before() {
  create_list( &new_list );
  char before_bravo[] = "before bravo";

  append_to_tail( &new_list, alpha );
  append_to_tail( &new_list, bravo );
  append_to_tail( &new_list, charlie );

  insert_before( &new_list, bravo, before_bravo );

  assert_string_equal( new_list->data, "alpha" );
  assert_string_equal( new_list->next->data, "before bravo" );
  assert_string_equal( new_list->next->next->data, "bravo" );
  assert_string_equal( new_list->next->next->next->data, "charlie" );
  assert_true( new_list->next->next->next->next == NULL );

  delete_list( new_list );
}


static void
test_insert_before_fails_if_sibling_not_found() {
  create_list( &new_list );
  char no_such_data[] = "NO SUCH DATA";

  append_to_tail( &new_list, alpha );
  append_to_tail( &new_list, bravo );
  append_to_tail( &new_list, charlie );

  assert_false( insert_before( &new_list, no_such_data, ( void * ) 123 ) );

  delete_list( new_list );
}


static void
test_insert_before_aborts_with_NULL_head() {
  expect_string( mock_die, output, "head must not be NULL" );
  expect_assert_failure( insert_before( NULL, alpha, ( void * ) 123 ) );
}


static void
test_append_to_tail() {
  create_list( &new_list );

  assert_true( append_to_tail( &new_list, alpha ) );
  assert_true( append_to_tail( &new_list, bravo ) );
  assert_true( append_to_tail( &new_list, charlie ) );

  assert_string_equal( new_list->data, "alpha" );
  assert_string_equal( new_list->next->data, "bravo" );
  assert_string_equal( new_list->next->next->data, "charlie" );
  assert_true( new_list->next->next->next == NULL );

  delete_list( new_list );
}


static void
test_append_to_tail_aborts_with_NULL_head() {
  expect_string( mock_die, output, "head must not be NULL" );
  expect_assert_failure( append_to_tail( NULL, alpha ) );
}


static void
test_delete_first_element() {
  create_list( &new_list );

  append_to_tail( &new_list, delete_me );
  append_to_tail( &new_list, bravo );
  append_to_tail( &new_list, charlie );

  assert_true( delete_element( &new_list, delete_me ) );
  assert_string_equal( new_list->data, "bravo" );
  assert_string_equal( new_list->next->data, "charlie" );
  assert_true( new_list->next->next == NULL );

  delete_list( new_list );
}


static void
test_delete_middle_element() {
  create_list( &new_list );

  append_to_tail( &new_list, alpha );
  append_to_tail( &new_list, delete_me );
  append_to_tail( &new_list, charlie );

  assert_true( delete_element( &new_list, delete_me ) );
  assert_string_equal( new_list->data, "alpha" );
  assert_string_equal( new_list->next->data, "charlie" );
  assert_true( new_list->next->next == NULL );

  delete_list( new_list );
}


static void
test_delete_last_element() {
  create_list( &new_list );

  append_to_tail( &new_list, alpha );
  append_to_tail( &new_list, bravo );
  append_to_tail( &new_list, delete_me );

  assert_true( delete_element( &new_list, delete_me ) );
  assert_string_equal( new_list->data, "alpha" );
  assert_string_equal( new_list->next->data, "bravo" );
  assert_true( new_list->next->next == NULL );

  delete_list( new_list );
}


static void
test_delete_nonexisting_element() {
  create_list( &new_list );

  append_to_tail( &new_list, alpha );
  append_to_tail( &new_list, bravo );
  append_to_tail( &new_list, charlie );

  assert_false( delete_element( &new_list, delete_me ) );
  assert_string_equal( new_list->data, "alpha" );
  assert_string_equal( new_list->next->data, "bravo" );
  assert_string_equal( new_list->next->next->data, "charlie" );
  assert_true( new_list->next->next->next == NULL );

  delete_list( new_list );
}


static void
test_delete_element_aborts_with_NULL_head() {
  expect_string( mock_die, output, "head must not be NULL" );
  expect_assert_failure( delete_element( NULL, delete_me ) );
}


static void
test_list_length() {
  create_list( &new_list );
  append_to_tail( &new_list, alpha );
  append_to_tail( &new_list, bravo );
  append_to_tail( &new_list, charlie );

  assert_int_equal( 3, ( int ) list_length_of( new_list ) );

  delete_list( new_list );
}


static void
test_list_length_of_empty_list() {
  create_list( &new_list );
  assert_int_equal( 0, ( int ) list_length_of( new_list ) );
}


static void
get_sum_length( void *data, void *user_data ) {
  size_t *sum_length = user_data;
  *sum_length += strlen( data );
}


static void
test_iterate_list() {
  create_list( &new_list );

  append_to_tail( &new_list, &alpha );
  append_to_tail( &new_list, &bravo );
  append_to_tail( &new_list, &charlie );

  size_t sum_length = 0;
  iterate_list( new_list, get_sum_length, &sum_length );
  assert_int_equal( 17, sum_length );

  delete_list( new_list );
}


static void
test_iterate_list_on_empty_list() {
  create_list( &new_list );

  size_t sum_length = 0;
  iterate_list( new_list, get_sum_length, &sum_length );
  assert_int_equal( 0, sum_length );

  delete_list( new_list );
}


static bool
find_bravo( void *data, void *user_data ) {
  UNUSED( user_data );

  if ( strcmp( data, "bravo" ) == 0 ) {
    return true;
  }
  else {
    return false;
  }
}


static bool
find_nothing( void *data, void *user_data ) {
  UNUSED( data );
  UNUSED( user_data );

  return false;
}


static void
test_find_list_custom() {
  create_list( &new_list );

  append_to_tail( &new_list, &alpha );
  append_to_tail( &new_list, &bravo );
  append_to_tail( &new_list, &charlie );

  void *found = find_list_custom( new_list, find_bravo, NULL );
  assert_string_equal( found, "bravo" );

  delete_list( new_list );
}


static void
test_find_list_custom_fail() {
  create_list( &new_list );

  append_to_tail( &new_list, &alpha );
  append_to_tail( &new_list, &bravo );
  append_to_tail( &new_list, &charlie );

  void *found = find_list_custom( new_list, find_nothing, NULL );
  assert_true( found == NULL );

  delete_list( new_list );
}


static void
test_delete_list() {
  create_list( &new_list );

  insert_in_front( &new_list, alpha );
  insert_in_front( &new_list, bravo );
  insert_in_front( &new_list, charlie );

  assert_true( delete_list( new_list ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_create_list ),
    unit_test( test_create_and_delete_empty_list ),
    unit_test_setup_teardown( test_create_list_aborts_with_NULL,
                              setup, teardown ),

    unit_test( test_insert_in_front ),
    unit_test_setup_teardown( test_insert_in_front_aborts_with_NULL_head,
                              setup, teardown ),

    unit_test( test_insert_before ),
    unit_test( test_insert_before_fails_if_sibling_not_found ),
    unit_test_setup_teardown( test_insert_before_aborts_with_NULL_head,
                              setup, teardown ),

    unit_test( test_append_to_tail ),
    unit_test_setup_teardown( test_append_to_tail_aborts_with_NULL_head,
                              setup, teardown ),

    unit_test( test_delete_first_element ),
    unit_test( test_delete_middle_element ),
    unit_test( test_delete_last_element ),
    unit_test( test_delete_nonexisting_element ),
    unit_test_setup_teardown( test_delete_element_aborts_with_NULL_head,
                              setup, teardown ),

    unit_test( test_list_length ),
    unit_test( test_list_length_of_empty_list ),

    unit_test( test_iterate_list ),
    unit_test( test_iterate_list_on_empty_list ),
    unit_test( test_find_list_custom ),
    unit_test( test_find_list_custom_fail ),

    unit_test( test_delete_list ),
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
