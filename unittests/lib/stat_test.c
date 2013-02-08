/*
 * Unit tests for stat.[ch]
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


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trema.h"
#include "cmockery_trema.h"


/********************************************************************************
 * static variable/functions in stat.c
 ********************************************************************************/

extern hash_table *stats;

void create_stats_table();
void delete_stats_table();


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

void
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


void
mock_info( const char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
}


void
mock_warn( const char *format, ... ) {
  UNUSED( format );
}


void
mock_error( const char *format, ... ) {
  UNUSED( format );
}


void
mock_callback( const char *key, const uint64_t value, void *user_data ) {
  check_expected( key );
  check_expected( value );
  check_expected( user_data );
}


/********************************************************************************
 * Setup and teardown.
 ********************************************************************************/

static void
reset() {
  stats = NULL;
}


/********************************************************************************
 * init_stat() tests.
 ********************************************************************************/

static void
test_init_stat_succeeds() {
  assert_true( init_stat() );
  assert_true( stats != NULL );
  assert_true( finalize_stat() );
}


static void
test_init_stat_reinitializes_if_already_initialized() {
  assert_true( init_stat() );
  assert_true( init_stat() );
  assert_true( stats != NULL );

  assert_true( finalize_stat() );
}


/********************************************************************************
 * finalize_stat() tests.
 ********************************************************************************/

static void
test_finalize_stat_succeeds() {
  assert_true( init_stat() );
  assert_true( finalize_stat() );
  assert_true( stats == NULL );
}


static void
test_finalize_stat_fails_if_not_initialized() {
  expect_assert_failure( finalize_stat() );
}


/********************************************************************************
 * add_stat_entry() tests.
 ********************************************************************************/

static void
test_add_stat_entry_succeeds() {
  assert_true( init_stat() );

  const char *key = "key";
  assert_true( add_stat_entry( key ) );
  stat_entry *entry = lookup_hash_entry( stats, key );
  assert_string_equal( entry->key, key );
  uint64_t expected_value = 0;
  assert_memory_equal( &entry->value, &expected_value, sizeof( uint64_t ) );

  assert_true( finalize_stat() );
}


static void
test_add_stat_entry_fails_with_duplicated_key() {
  assert_true( init_stat() );

  const char *key = "key";
  assert_true( add_stat_entry( key ) );
  assert_false( add_stat_entry( key ) );

  assert_true( finalize_stat() );
}


/********************************************************************************
 * increment_stat() tests.
 ********************************************************************************/

static void
test_increment_stat_succeeds_with_defined_key() {
  assert_true( init_stat() );

  const char *key = "key";
  assert_true( add_stat_entry( key ) );
  increment_stat( key );

  stat_entry *entry = lookup_hash_entry( stats, key );
  assert_string_equal( entry->key, key );
  uint64_t expected_value = 1;
  assert_memory_equal( &entry->value, &expected_value, sizeof( uint64_t ) );

  assert_true( finalize_stat() );
}


static void
test_increment_stat_succeeds_with_undefined_key() {
  assert_true( init_stat() );

  const char *key = "key";
  increment_stat( key );

  stat_entry *entry = lookup_hash_entry( stats, key );
  assert_string_equal( entry->key, key );
  uint64_t expected_value = 1;
  assert_memory_equal( &entry->value, &expected_value, sizeof( uint64_t ) );

  assert_true( finalize_stat() );
}


static void
test_increment_stat_fails_if_key_is_NULL() {
  assert_true( init_stat() );

  expect_assert_failure( increment_stat( NULL ) );

  assert_true( finalize_stat() );
}


static void
test_increment_stat_fails_if_not_initialized() {
  const char *key = "key";
  expect_assert_failure( increment_stat( key ) );
}


/********************************************************************************
 * reset_stats() tests.
 ********************************************************************************/

static void
test_reset_stats_succeeds_with_single_entry() {
  assert_true( init_stat() );

  const char *key = "key";
  increment_stat( key );

  reset_stats();

  hash_iterator iter;
  hash_entry *e = NULL;
  init_hash_iterator( stats, &iter );
  int n_entries = 0;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    n_entries++;
  }
  assert_int_equal( n_entries, 0 );

  assert_true( finalize_stat() );
}


static void
test_reset_stats_succeeds_with_multiple_entries() {
  assert_true( init_stat() );

  const char *keys[] = { "key0", "key1" };
  increment_stat( keys[ 0 ] );
  increment_stat( keys[ 1 ] );

  reset_stats();

  hash_iterator iter;
  hash_entry *e = NULL;
  init_hash_iterator( stats, &iter );
  int n_entries = 0;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    n_entries++;
  }
  assert_int_equal( n_entries, 0 );

  assert_true( finalize_stat() );
}


static void
test_reset_stats_succeeds_without_entries() {
  assert_true( init_stat() );

  reset_stats();

  hash_iterator iter;
  hash_entry *e = NULL;
  init_hash_iterator( stats, &iter );
  int n_entries = 0;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    n_entries++;
  }
  assert_int_equal( n_entries, 0 );

  assert_true( finalize_stat() );
}


static void
test_reset_stats_fails_if_not_initialized() {
  expect_assert_failure( reset_stats() );
}


/********************************************************************************
 * foreach_stat() tests.
 ********************************************************************************/

static void
test_foreach_stat_succeeds() {
  assert_true( init_stat() );

  const char *keys[] = { "key0", "key1" };
  increment_stat( keys[ 0 ] );
  increment_stat( keys[ 1 ] );
  increment_stat( keys[ 1 ] );

  void *user_data = ( void * ) ( intptr_t ) 0x1;

  expect_string( mock_callback, key, keys[ 1 ] );
  expect_value( mock_callback, value, 2 );
  expect_value( mock_callback, user_data, user_data );

  expect_string( mock_callback, key, keys[ 0 ] );
  expect_value( mock_callback, value, 1 );
  expect_value( mock_callback, user_data, user_data );

  foreach_stat( mock_callback, user_data );

  assert_true( finalize_stat() );
}


static void
test_foreach_stat_succeeds_without_entries() {
  assert_true( init_stat() );

  foreach_stat( mock_callback, NULL );

  assert_true( finalize_stat() );
}


static void
test_foreach_stat_fails_if_callback_function_is_NULL() {
  assert_true( init_stat() );

  expect_assert_failure( foreach_stat( NULL, NULL ) );

  assert_true( finalize_stat() );
}


static void
test_foreach_stat_fails_if_not_initialized() {
  expect_assert_failure( foreach_stat( mock_callback, NULL ) );
}


/********************************************************************************
 * dump_stats() tests.
 ********************************************************************************/

static void
test_dump_stats_succeeds() {
  assert_true( init_stat() );

  const char *key = "key";
  increment_stat( key );

  expect_string( mock_info, message, "Statistics:" );
  expect_string( mock_info, message, "key: 1" );
  dump_stats();

  assert_true( finalize_stat() );
}


static void
test_dump_stats_succeeds_without_entries() {
  assert_true( init_stat() );

  expect_string( mock_info, message, "Statistics:" );
  expect_string( mock_info, message, "No statistics found." );
  dump_stats();

  assert_true( finalize_stat() );
}


static void
test_dump_stats_fails_if_not_initialized() {
  expect_assert_failure( dump_stats() );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // init_stat() tests.
    unit_test_setup_teardown( test_init_stat_succeeds, reset, reset ),
    unit_test_setup_teardown( test_init_stat_reinitializes_if_already_initialized, reset, reset ),

    // finalize_stat() tests.
    unit_test_setup_teardown( test_finalize_stat_succeeds, reset, reset ),
    unit_test_setup_teardown( test_finalize_stat_fails_if_not_initialized, reset, reset ),

    // add_stat_entry() tests.
    unit_test_setup_teardown( test_add_stat_entry_succeeds, reset, reset ),
    unit_test_setup_teardown( test_add_stat_entry_fails_with_duplicated_key, reset, reset ),

    // increment_stat() tests.
    unit_test_setup_teardown( test_increment_stat_succeeds_with_defined_key, reset, reset ),
    unit_test_setup_teardown( test_increment_stat_succeeds_with_undefined_key, reset, reset ),
    unit_test_setup_teardown( test_increment_stat_fails_if_key_is_NULL, reset, reset ),
    unit_test_setup_teardown( test_increment_stat_fails_if_not_initialized, reset, reset ),

    // reset_stats() tests.
    unit_test_setup_teardown( test_reset_stats_succeeds_with_single_entry, reset, reset ),
    unit_test_setup_teardown( test_reset_stats_succeeds_with_multiple_entries, reset, reset ),
    unit_test_setup_teardown( test_reset_stats_succeeds_without_entries, reset, reset ),
    unit_test_setup_teardown( test_reset_stats_fails_if_not_initialized, reset, reset ),

    // foreach_stat() tests.
    unit_test_setup_teardown( test_foreach_stat_succeeds, reset, reset ),
    unit_test_setup_teardown( test_foreach_stat_succeeds_without_entries, reset, reset ),
    unit_test_setup_teardown( test_foreach_stat_fails_if_callback_function_is_NULL, reset, reset ),
    unit_test_setup_teardown( test_foreach_stat_fails_if_not_initialized, reset, reset ),

    // dump_stats() tests.
    unit_test_setup_teardown( test_dump_stats_succeeds, reset, reset ),
    unit_test_setup_teardown( test_dump_stats_succeeds_without_entries, reset, reset ),
    unit_test_setup_teardown( test_dump_stats_fails_if_not_initialized, reset, reset ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
