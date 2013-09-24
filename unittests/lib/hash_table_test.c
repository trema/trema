/*
 * Unit tests for hash table.
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


#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "hash_table.h"
#include "utility.h"


/********************************************************************************
 * Test functions.
 ********************************************************************************/

static hash_table *table;

static char alpha[] = "alpha";
static char bravo[] = "bravo";
static char charlie[] = "charlie";


static void
test_lookup_empty_table_returns_NULL() {
  table = create_hash( compare_string, hash_string );

  assert_true( lookup_hash_entry( table, alpha ) == NULL );

  delete_hash( table );
}


static void
test_insert_and_lookup() {
  table = create_hash( compare_string, hash_string );

  insert_hash_entry( table, alpha, alpha );
  assert_string_equal( lookup_hash_entry( table, alpha ), "alpha" );

  delete_hash( table );
}


typedef struct {
  const char *key;
} key;

static void
test_insert_and_lookup_by_atom_hash() {
  key mykey = { "alpha" };
  table = create_hash( compare_atom, hash_atom );

  insert_hash_entry( table, &mykey, alpha );
  assert_string_equal( lookup_hash_entry( table, &mykey ), "alpha" );

  delete_hash( table );
}


static void
test_insert_twice_overwrites_old_value() {
  char *prev;
  char key[] = "key";
  table = create_hash( compare_string, hash_string );

  char old_value[] = "old value";
  char new_value[] = "new value";

  insert_hash_entry( table, key, old_value );
  prev = insert_hash_entry( table, key, new_value );

  assert_string_equal( lookup_hash_entry( table, key ), "new value" );
  assert_string_equal( prev, "old value" );

  delete_hash( table );
}


static void
test_delete_entry() {
  table = create_hash( compare_string, hash_string );

  insert_hash_entry( table, alpha, alpha );
  insert_hash_entry( table, bravo, bravo );
  insert_hash_entry( table, charlie, charlie );

  delete_hash_entry( table, bravo );
  assert_string_equal( lookup_hash_entry( table, alpha ), "alpha" );
  assert_true( lookup_hash_entry( table, bravo ) == NULL );
  assert_string_equal( lookup_hash_entry( table, charlie ), "charlie" );

  delete_hash( table );
}


static void
test_nonexistent_entry_returns_NULL() {
  table = create_hash( compare_string, hash_string );

  assert_true( delete_hash_entry( table, "NO SUCH KEY" ) == NULL );

  delete_hash( table );
}


static char *abc[ 3 ] = { NULL, NULL, NULL };

static void
append_back_foreach( void *key, void *value, void *user_data ) {
  assert_true( strcmp( key, value ) == 0 );
  assert_true( user_data == NULL );

  int i;
  for ( i = 0; i < 3; i++ ) {
    if ( abc[ i ] == NULL ) {
      abc[ i ] = value;
      return;
    }
  }
}


static void
test_foreach() {
  table = create_hash( compare_string, hash_string );
  insert_hash_entry( table, alpha, alpha );
  insert_hash_entry( table, bravo, bravo );
  insert_hash_entry( table, charlie, charlie );
  delete_hash_entry( table, bravo );
  delete_hash_entry( table, charlie );

  foreach_hash( table, append_back_foreach, NULL );

  assert_string_equal( abc[ 0 ], "alpha" );
  assert_true( abc[ 1 ] == NULL );
  assert_true( abc[ 2 ] == NULL );

  delete_hash( table );
}


static void
test_iterator() {
  table = create_hash( compare_string, hash_string );

  char one[] = "one";
  insert_hash_entry( table, one, ( void * ) 1 );

  char two[] = "two";
  insert_hash_entry( table, two, ( void * ) 2 );

  char three[] = "three";
  insert_hash_entry( table, three, ( void * ) 3 );

  char four[] = "four";
  insert_hash_entry( table, four, ( void * ) 4 );

  char five[] = "five";
  insert_hash_entry( table, five, ( void * ) 5 );

  char six[] = "six";
  insert_hash_entry( table, six, ( void * ) 6 );

  char seven[] = "seven";
  insert_hash_entry( table, seven, ( void * ) 7 );

  char eight[] = "eight";
  insert_hash_entry( table, eight, ( void * ) 8 );

  char nine[] = "nine";
  insert_hash_entry( table, nine, ( void * ) 9 );

  char ten[] = "ten";
  insert_hash_entry( table, ten, ( void * ) 10 );

  int sum = 0;
  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( table, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    sum += ( int ) ( uintptr_t ) e->value;
    delete_hash_entry( table, e->key );
  }
  assert_true( sum == 55 );

  delete_hash( table );
}


static void
test_multiple_inserts_and_deletes_then_iterate() {
  table = create_hash( compare_string, hash_string );

  char one[] = "one";
  insert_hash_entry( table, one, ( void * ) 1 );
  delete_hash_entry( table, one );
  insert_hash_entry( table, one, ( void * ) 1 );
  delete_hash_entry( table, one );
  insert_hash_entry( table, one, ( void * ) 1 );

  int sum = 0;
  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( table, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    sum += ( int ) ( uintptr_t ) e->value;
    delete_hash_entry( table, e->key );
  }
  assert_true( sum == 1 );

  delete_hash( table );
}


static void
test_iterate_empty_hash() {
  hash_iterator iter;
  table = create_hash( compare_atom, hash_atom );
  init_hash_iterator( table, &iter );

  while ( iterate_hash_next( &iter ) != NULL ) {
    UNREACHABLE_CODE();
  }

  delete_hash( table );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_lookup_empty_table_returns_NULL ),
    unit_test( test_insert_and_lookup ),
    unit_test( test_insert_and_lookup_by_atom_hash ),
    unit_test( test_insert_twice_overwrites_old_value ),
    unit_test( test_delete_entry ),
    unit_test( test_nonexistent_entry_returns_NULL ),
    unit_test( test_foreach ),
    unit_test( test_iterator ),
    unit_test( test_multiple_inserts_and_deletes_then_iterate ),
    unit_test( test_iterate_empty_hash ),
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
