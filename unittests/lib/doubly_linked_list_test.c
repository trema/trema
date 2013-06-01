/*
 * Unit tests for doubly linked list library.
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
#include "cmockery_trema.h"
#include "doubly_linked_list.h"
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
 * Tests.
 ********************************************************************************/

static void
test_create_dlist() {
  dlist_element *new_element = create_dlist();

  assert_true( new_element != NULL );
  assert_true( new_element->data == NULL );
  assert_true( new_element->next == NULL );
  assert_true( new_element->prev == NULL );

  delete_dlist( new_element );
}


// NULL <- "alpha" <-> "bravo" <-> "charlie" <-> new_element
static void
test_insert_before_dlist() {
  char element_data1[] = "alpha";
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *new_element = create_dlist();

  assert_true( insert_before_dlist( new_element, element_data1 ) );
  assert_true( insert_before_dlist( new_element, element_data2 ) );
  assert_true( insert_before_dlist( new_element, element_data3 ) );

  assert_string_equal( new_element->prev->data, "charlie" );
  assert_string_equal( new_element->prev->prev->data, "bravo" );
  assert_string_equal( new_element->prev->prev->prev->data, "alpha" );
  assert_true( new_element->prev->prev->prev->prev == NULL );

  assert_true( new_element->prev->next == new_element );
  assert_true( new_element->prev->prev->next->next == new_element );
  assert_true( new_element->prev->prev->prev->next->next->next == new_element );

  delete_dlist( new_element );
}


static void
test_insert_before_dlist_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( insert_before_dlist( NULL, NULL ) );
}


// new_element <-> "charlie" <-> "bravo" <-> "alpha" -> NULL
static void
test_insert_after_dlist() {
  char element_data1[] = "alpha";
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *new_element = create_dlist();

  assert_true( insert_after_dlist( new_element, element_data1 ) );
  assert_true( insert_after_dlist( new_element, element_data2 ) );
  assert_true( insert_after_dlist( new_element, element_data3 ) );

  assert_string_equal( new_element->next->data, "charlie" );
  assert_string_equal( new_element->next->next->data, "bravo" );
  assert_string_equal( new_element->next->next->next->data, "alpha" );
  assert_true( new_element->next->next->next->next == NULL );

  assert_true( new_element->next->prev == new_element );
  assert_true( new_element->next->next->prev->prev == new_element );
  assert_true( new_element->next->next->next->prev->prev->prev == new_element );

  delete_dlist( new_element );
}


static void
test_insert_after_dlist_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( insert_after_dlist( NULL, NULL ) );
}


// "alpha" <-> "bravo" <-> "charlie"
static void
test_get_first_element() {
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *alpha = create_dlist();
  dlist_element *bravo = insert_after_dlist( alpha, element_data2 );
  dlist_element *charlie = insert_after_dlist( bravo, element_data3 );

  assert_true( get_first_element( alpha ) == alpha );
  assert_true( get_first_element( bravo ) == alpha );
  assert_true( get_first_element( charlie ) == alpha );

  delete_dlist( alpha );
}


static void
test_get_first_element_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( get_first_element( NULL ) );
}


// "alpha" <-> "bravo" <-> "charlie"
static void
test_get_last_element() {
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *alpha = create_dlist();
  dlist_element *bravo = insert_after_dlist( alpha, element_data2 );
  dlist_element *charlie = insert_after_dlist( bravo, element_data3 );

  assert_true( get_last_element( alpha ) == charlie );
  assert_true( get_last_element( bravo ) == charlie );
  assert_true( get_last_element( charlie ) == charlie );

  delete_dlist( alpha );
}


static void
test_get_last_element_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( get_last_element( NULL ) );
}


// Remove "alpha" of "alpha" <-> "bravo" <-> "charlie"
static void
test_remove_first_element() {
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *alpha = create_dlist();
  dlist_element *bravo = insert_after_dlist( alpha, element_data2 );
  dlist_element *charlie = insert_after_dlist( bravo, element_data3 );

  assert_true( delete_dlist_element( alpha ) );
  assert_true( bravo->prev == NULL );
  assert_true( bravo->next == charlie );

  delete_dlist( bravo );
}


// Remove "bravo" of "alpha" <-> "bravo" <-> "charlie"
static void
test_remove_middle_element() {
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *alpha = create_dlist();
  dlist_element *bravo = insert_after_dlist( alpha, element_data2 );
  dlist_element *charlie = insert_after_dlist( bravo, element_data3 );

  assert_true( delete_dlist_element( bravo ) );
  assert_true( alpha->next == charlie );
  assert_true( charlie->prev == alpha );

  delete_dlist( alpha );
}


// Remove "charlie" of "alpha" <-> "bravo" <-> "charlie"
static void
test_remove_last_element() {
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";

  dlist_element *alpha = create_dlist();
  dlist_element *bravo = insert_after_dlist( alpha, element_data2 );
  dlist_element *charlie = insert_after_dlist( bravo, element_data3 );

  assert_true( delete_dlist_element( charlie ) );
  assert_true( bravo->next == NULL );
  assert_true( bravo->prev == alpha );

  delete_dlist( alpha );
}


static void
test_delete_dlist_element_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( delete_dlist_element( NULL ) );
}


// find elements of "charlie" <-> "bravo" <-> "alpha"
static void
test_find_element() {
  char element_data1[] = "alpha";
  char element_data2[] = "bravo";
  char element_data3[] = "charlie";
  char no_such_element[] = "NO SUCH ELEMENT";

  dlist_element *new_element = create_dlist();

  insert_after_dlist( new_element, element_data1 );
  insert_after_dlist( new_element, element_data2 );
  insert_after_dlist( new_element, element_data3 );

  dlist_element *e = NULL;

  e = find_element( new_element, element_data3 );
  assert_true( new_element->next == e );
  e = find_element( new_element, element_data2 );
  assert_true( new_element->next->next == e );
  e = find_element( new_element, element_data1 );
  assert_true( new_element->next->next->next == e );

  e = find_element( new_element->next->next->next, element_data2 );
  assert_true( new_element->next->next == e );

  e = find_element( new_element, no_such_element );
  assert_true( e == NULL );

  delete_dlist( new_element );
}


static void
test_find_element_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( find_element( NULL, NULL ) );
}


static void
test_delete_dlist_aborts_with_NULL_dlist() {
  expect_string( mock_die, output, "element must not be NULL" );
  expect_assert_failure( delete_dlist( NULL ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_create_dlist ),

    unit_test( test_insert_before_dlist ),
    unit_test_setup_teardown( test_insert_before_dlist_aborts_with_NULL_dlist,
                              setup, teardown ),

    unit_test( test_insert_after_dlist ),
    unit_test_setup_teardown( test_insert_after_dlist_aborts_with_NULL_dlist,
                              setup, teardown ),

    unit_test( test_get_first_element ),
    unit_test_setup_teardown( test_get_first_element_aborts_with_NULL_dlist,
                              setup, teardown ),

    unit_test( test_get_last_element ),
    unit_test_setup_teardown( test_get_last_element_aborts_with_NULL_dlist,
                              setup, teardown ),

    unit_test( test_remove_middle_element ),
    unit_test( test_remove_first_element ),
    unit_test( test_remove_last_element ),
    unit_test_setup_teardown( test_delete_dlist_element_aborts_with_NULL_dlist,
                              setup, teardown ),

    unit_test( test_find_element ),
    unit_test_setup_teardown( test_find_element_aborts_with_NULL_dlist,
                              setup, teardown ),

    unit_test_setup_teardown( test_delete_dlist_aborts_with_NULL_dlist,
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
