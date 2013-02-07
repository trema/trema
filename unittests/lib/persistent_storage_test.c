/*
 * Unit tests for persistent_storage.[ch]
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
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "cmockery_trema.h"


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static int ( *original_unlink )( const char *pathname );
static void ( *original_error )( const char *format, ... );

static int
mock_unlink( const char *pathname ) {
  unlink( pathname );
  errno = ENOENT;
  return ( int ) mock();
}


static int
mock_sqlite3_open( const char *filename, sqlite3 **ppDb ) {
  sqlite3_open( filename, ppDb );
  return ( int ) mock();
}


static int
mock_sqlite3_close( sqlite3 *db ) {
  sqlite3_close( db );
  return ( int ) mock();
}


static char error_message[] = "ERROR";

static int
mock_sqlite3_exec( sqlite3 *db, const char *sql, int ( *callback )( void *, int, char **, char ** ), void *argument, char **errmsg ) {
  sqlite3_exec( db, sql, callback, argument, errmsg );
  *errmsg = error_message;
  return ( int ) mock();
}


static int
mock_sqlite3_changes( sqlite3 *db ) {
  UNUSED( db );
  return ( int ) mock();
}


static const char *
mock_sqlite3_errmsg( sqlite3 *db ) {
  UNUSED( db );
  return error_message;
}


static void
mock_sqlite3_free( void *freed ) {
  UNUSED( freed );
}


static void
mock_error( const char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

static void
setup() {
  setenv( "TREMA_TMP", "/tmp", 1 );
  set_trema_tmp();
  unlink( "/tmp/.trema.db" );
  original_error = error;
  error = mock_error;
  original_unlink = trema_unlink;
  trema_unlink = mock_unlink;
}


static void
setup_and_init() {
  setup();
  init_persistent_storage();
}


static void
teardown() {
  unsetenv( "TREMA_TMP" );
  unlink( "/tmp/.trema.db" );
  error = original_error;
  trema_unlink = original_unlink;
}


static void
teardown_and_finalize() {
  teardown();
  finalize_persistent_storage();
}


/********************************************************************************
 * init_persistent_storage() tests.
 ********************************************************************************/

static void
test_init_persistent_storage_succeeds() {
  assert_true( init_persistent_storage() );
  assert_true( _get_backend_initialized() );
  assert_true( _get_db_file() != NULL );
  assert_true( _get_db_handle() != NULL );

  finalize_persistent_storage();
}


static void
test_init_persistent_storage_fails_if_already_initialized() {
  init_persistent_storage();

  expect_string( mock_error, message, "Backend is already initialized." );
  assert_false( init_persistent_storage() );

  finalize_persistent_storage();
}


static void
test_init_persistent_storage_fails_if_backend_is_not_opened() {
  void *original_sqlite3_open = trema_sqlite3_open;
  trema_sqlite3_open = mock_sqlite3_open;
  void *original_sqlite3_errmsg = trema_sqlite3_errmsg;
  trema_sqlite3_errmsg = mock_sqlite3_errmsg;

  will_return( mock_sqlite3_open, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to open backend database ( ERROR )." );
  assert_false( init_persistent_storage() );
  assert_false( _get_backend_initialized() );
  assert_true( _get_db_file() == NULL );
  assert_true( _get_db_handle() == NULL );

  trema_sqlite3_open = original_sqlite3_open;
  trema_sqlite3_errmsg = original_sqlite3_errmsg;
}


static void
test_init_persistent_storage_fails_if_backend_db_is_not_created() {
  void *original_sqlite3_exec = trema_sqlite3_exec;
  trema_sqlite3_exec = mock_sqlite3_exec;
  void *original_sqlite3_free = trema_sqlite3_free;
  trema_sqlite3_free = mock_sqlite3_free;

  will_return( mock_sqlite3_exec, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to create a table ( error = ERROR )." );
  assert_false( init_persistent_storage() );
  assert_false( _get_backend_initialized() );
  assert_true( _get_db_file() == NULL );
  assert_true( _get_db_handle() == NULL );

  trema_sqlite3_exec = original_sqlite3_exec;
  trema_sqlite3_free = original_sqlite3_free;
}


/********************************************************************************
 * finalize_persistent_storage() tests.
 ********************************************************************************/

static void
test_finalize_persistent_storage_succeeds() {
  init_persistent_storage();

  assert_true( finalize_persistent_storage() );
  assert_false( _get_backend_initialized() );
  assert_true( _get_db_file() == NULL );
  assert_true( _get_db_handle() == NULL );
}


static void
test_finalize_persistent_storage_fails_if_not_initialized() {
  expect_string( mock_error, message, "Backend is not initialized yet." );
  assert_false( finalize_persistent_storage() );
}


static void
test_finalize_persistent_storage_fails_if_backend_is_not_closed() {
  init_persistent_storage();

  void *original_sqlite3_close = trema_sqlite3_close;
  trema_sqlite3_close = mock_sqlite3_close;
  void *original_sqlite3_errmsg = trema_sqlite3_errmsg;
  trema_sqlite3_errmsg = mock_sqlite3_errmsg;

  will_return( mock_sqlite3_close, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to destroy a sqlite3 object ( ERROR )." );
  assert_false( finalize_persistent_storage() );
  assert_false( _get_backend_initialized() );
  assert_true( _get_db_file() == NULL );
  assert_true( _get_db_handle() == NULL );

  trema_sqlite3_close = original_sqlite3_close;
  trema_sqlite3_errmsg = original_sqlite3_errmsg;
}


/********************************************************************************
 * clear_persistent_storage() tests.
 ********************************************************************************/

static void
test_clear_persistent_storage_succeeds() {
  init_persistent_storage();

  set_value( "KEY", "VALUE" );

  will_return( mock_unlink, 0 );
  assert_true( clear_persistent_storage() );
  assert_true( set_value( "NEW_KEY", "NEW_VALUE" ) );
  char buf[ 256 ];
  expect_string( mock_error, message, "Failed to retrieve a value for 'KEY'." );
  assert_false( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_true( get_value( "NEW_KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "NEW_VALUE", buf );

  finalize_persistent_storage();
}


static void
test_clear_persistent_storage_fails_if_not_initialized() {
  expect_string( mock_error, message, "Backend is not initialized yet." );
  assert_false( clear_persistent_storage() );
}


static void
test_clear_persistent_storage_fails_if_unlink_fails() {
  init_persistent_storage();

  set_value( "KEY", "VALUE" );

  will_return( mock_unlink, -1 );
  expect_string( mock_error, message, "Failed to delete database file. ( db_file = /tmp/.trema.db, errno = No such file or directory [2] )." );
  assert_false( clear_persistent_storage() );
}


/********************************************************************************
 * set_value() tests.
 ********************************************************************************/

static void
test_set_value_succeeds() {
  assert_true( set_value( "KEY", "VALUE" ) );
  assert_true( set_value( "KEY2", "VALUE2" ) );
}


static void
test_set_value_updates_value() {
  set_value( "KEY", "VALUE" );

  assert_true( set_value( "KEY", "NEW_VALUE" ) );
  char buf[ 256 ];
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "NEW_VALUE", buf );
}


static void
test_set_value_clears_value() {
  set_value( "KEY", "VALUE" );

  assert_true( set_value( "KEY", "" ) );
  char buf[ 256 ];
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "", buf );
}


static void
test_set_value_fails_with_too_long_key() {
  size_t key_length = _get_max_key_length() + 1;
  char *key = xmalloc( key_length + 1 );
  memset( key, 'a', key_length );
  key[ key_length ] = '\0';
  char *expected_error = xasprintf( "Too long key ( length = %u ) specified. Maximum length is %u.",
                                    key_length, _get_max_key_length() );

  expect_string( mock_error, message, expected_error );
  assert_false( set_value( key, "VALUE" ) );

  xfree( key );
  xfree( expected_error );
}


static void
test_set_value_fails_with_too_long_value() {
  size_t value_length = _get_max_value_length() + 1;
  char *value = xmalloc( value_length + 1 );
  memset( value, 'a', value_length );
  value[ value_length ] = '\0';
  char *expected_error = xasprintf( "Too long value ( length = %u ) specified. Maximum length is %u.",
                                    value_length, _get_max_value_length() );

  expect_string( mock_error, message, expected_error );
  assert_false( set_value( "KEY", value ) );

  xfree( value );
  xfree( expected_error );
}


static void
test_set_value_fails_with_NULL_key() {
  expect_string( mock_error, message, "'key' must be specified." );
  assert_false( set_value( NULL, "VALUE" ) );
}


static void
test_set_value_fails_with_NULL_value() {
  expect_string( mock_error, message, "'value' must be specified." );
  assert_false( set_value( "KEY", NULL ) );
}


static void
test_set_value_fails_with_invalid_characters() {
  const char invalid_characters[] = { ( char ) 0x7f, ( char ) 0x1f, ( char ) 0xff, ( char ) 0x00 };
  expect_string( mock_error, message, "'key' must only have printable ASCII characters." );
  assert_false( set_value( invalid_characters, "VALUE" ) );
  expect_string( mock_error, message, "'value' must only have printable ASCII characters." );
  assert_false( set_value( "KEY", invalid_characters ) );
}


static void
test_set_value_fails_if_not_initialized() {
  expect_string( mock_error, message, "Backend is not initialized yet." );
  assert_false( set_value( "KEY", "VALUE" ) );
}


static void
test_set_value_escapes_string() {
  assert_true( set_value( "THIS \"IS\", A, 'KEY'", "HERE \"IS\", A, 'VALULE'" ) );
  char buf[ 256 ];
  assert_true( get_value( "THIS \"IS\", A, 'KEY'", buf, sizeof( buf ) ) );
  assert_string_equal( "HERE \"IS\", A, 'VALULE'", buf );

  assert_true( set_value( "THIS \"IS\", A, 'KEY'", "HERE \"IS\", A, 'NEW_VALULE'" ) );
  assert_true( get_value( "THIS \"IS\", A, 'KEY'", buf, sizeof( buf ) ) );
  assert_string_equal( "HERE \"IS\", A, 'NEW_VALULE'", buf );
}


static void
test_set_value_avoids_injection_attacks() {
  assert_true( set_value( "KEY', 'VALUE'); DROP TABLE trema;", "VALUE" ) );
  char buf[ 256 ];
  assert_true( get_value( "KEY', 'VALUE'); DROP TABLE trema;", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE", buf );

  assert_true( set_value( "KEY", "VALUE'); DROP TABLE trema;" ) );
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE'); DROP TABLE trema;", buf );

  assert_true( set_value( "KEY'; DROP TABLE trema;", "NEW_VALUE" ) );
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE'); DROP TABLE trema;", buf );

  assert_true( set_value( "KEY", "NEW_VALUE' WHERE KEY = 'KEY'; DROP TABLE trema;" ) );
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "NEW_VALUE' WHERE KEY = 'KEY'; DROP TABLE trema;", buf );
}


static void
test_set_value_fails_if_backend_fails_to_store_key_value() {
  void *original_sqlite3_exec = trema_sqlite3_exec;
  trema_sqlite3_exec = mock_sqlite3_exec;
  void *original_sqlite3_free = trema_sqlite3_free;
  trema_sqlite3_free = mock_sqlite3_free;
  void *original_sqlite3_changes = trema_sqlite3_changes;
  trema_sqlite3_changes = mock_sqlite3_changes;

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to execute a SQL statement ( statement = INSERT INTO trema (key,value) VALUES ('KEY','VALUE'), error = ERROR )." );
  assert_false( set_value( "KEY", "VALUE" ) );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_changes, 1 );
  delete_key_value( "KEY" );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_changes, 2 );
  expect_string( mock_error, message, "Multiple entries are created ( n_changes = 2 )." );
  assert_false( set_value( "KEY", "VALUE" ) );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_changes, 1 );
  delete_key_value( "KEY" );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_changes, 0 );
  assert_false( set_value( "KEY", "VALUE" ) );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to execute a SQL statement ( statement = UPDATE trema SET value = 'NEW_VALUE' WHERE key = 'KEY', error = ERROR )." );
  assert_false( set_value( "KEY", "NEW_VALUE" ) );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_changes, 0 );
  assert_false( set_value( "KEY", "NEW_VALUE" ) );

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_changes, 2 );
  expect_string( mock_error, message, "Multiple entries are updated ( n_changes = 2 )." );
  assert_false( set_value( "KEY", "NEW_VALUE" ) );

  trema_sqlite3_exec = original_sqlite3_exec;
  trema_sqlite3_free = original_sqlite3_free;
  trema_sqlite3_changes = original_sqlite3_changes;
}


/********************************************************************************
 * get_value() tests.
 ********************************************************************************/

static void
test_get_value_succeeds() {
  set_value( "KEY1", "VALUE1" );
  set_value( "KEY2", "VALUE2" );

  char buf[ 256 ];
  assert_true( get_value( "KEY1", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE1", buf );
  assert_true( get_value( "KEY2", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE2", buf );
}


static void
test_get_value_fails_with_undefined_key() {
  char buf[ 256 ];
  expect_string( mock_error, message, "Failed to retrieve a value for 'UNDEFINED_KEY'." );
  assert_false( get_value( "UNDEFINED_KEY", buf, sizeof( buf ) ) );
}


static void
test_get_value_fails_with_too_long_key() {
  size_t key_length = _get_max_key_length() + 1;
  char *key = xmalloc( key_length + 1 );
  memset( key, 'a', key_length );
  key[ key_length ] = '\0';
  char *expected_error = xasprintf( "Too long key ( length = %u ) specified. Maximum length is %u.",
                                    key_length, _get_max_key_length() );

  expect_string( mock_error, message, expected_error );
  char buf[ 256 ];
  assert_false( get_value( key, buf, sizeof( buf ) ) );

  xfree( key );
  xfree( expected_error );
}


static void
test_get_value_fails_with_insufficient_buffer() {
  set_value( "KEY", "VALUE" );

  expect_string( mock_error, message, "'length' must be greater than zero." );
  char buf[ 1 ];
  assert_false( get_value( "KEY", buf, 0 ) );
  expect_string( mock_error, message, "Insufficient buffer space ( 6 [bytes] > 1 [bytes] )." );
  assert_false( get_value( "KEY", buf, sizeof( buf ) ) );
}


static void
test_get_value_fails_with_NULL_key() {
  char buf[ 256 ];
  expect_string( mock_error, message, "'key' must be specified." );
  assert_false( get_value( NULL, buf, sizeof( buf ) ) );
}


static void
test_get_value_fails_with_NULL_value() {
  expect_string( mock_error, message, "'value' must be specified." );
  assert_false( get_value( "KEY", NULL, 128 ) );
}


static void
test_get_value_fails_with_invalid_characters() {
  const char invalid_characters[] = { ( char ) 0x7f, ( char ) 0x1f, ( char ) 0xff, ( char ) 0x00 };
  expect_string( mock_error, message, "'key' must only have printable ASCII characters." );
  char buf[ 256 ];
  assert_false( get_value( invalid_characters, buf, sizeof( buf ) ) );
}


static void
test_get_value_fails_if_not_initialized() {
  char buf[ 256 ];
  expect_string( mock_error, message, "Backend is not initialized yet." );
  assert_false( get_value( "KEY", buf, sizeof( buf ) ) );
}


static void
test_get_value_escapes_string() {
  set_value( "THIS \"IS\", A, 'KEY'", "VALUE" );

  char buf[ 256 ];
  assert_true( get_value( "THIS \"IS\", A, 'KEY'", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE", buf );
}


static void
test_get_value_avoids_injection_attacks() {
  set_value( "KEY", "VALUE" );

  char buf[ 256 ];
  expect_string( mock_error, message, "Failed to retrieve a value for 'KEY'; DROP TABLE trema;'." );
  assert_false( get_value( "KEY'; DROP TABLE trema;", buf, sizeof( buf ) ) );
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE", buf );
}


static void
test_get_value_fails_if_backend_fails_to_lookup_value() {
  void *original_sqlite3_exec = trema_sqlite3_exec;
  trema_sqlite3_exec = mock_sqlite3_exec;
  void *original_sqlite3_free = trema_sqlite3_free;
  trema_sqlite3_free = mock_sqlite3_free;

  will_return( mock_sqlite3_exec, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to execute a SQL statement ( statement = SELECT value FROM trema WHERE key = 'KEY', error = ERROR )." );
  expect_string( mock_error, message, "Failed to retrieve a value for 'KEY'." );
  char buf[ 256 ];
  assert_false( get_value( "KEY", buf, sizeof( buf ) ) );

  trema_sqlite3_exec = original_sqlite3_exec;
  trema_sqlite3_free = original_sqlite3_free;
}



/********************************************************************************
 * delete_key_value() tests.
 ********************************************************************************/

static void
test_delete_key_value_succeeds() {
  const char *key = "KEY";
  const char *value = "VALUE";

  set_value( key, value );

  assert_true( delete_key_value( key ) );
  expect_string( mock_error, message, "Failed to retrieve a value for 'KEY'." );
  char buf[ 256 ];
  assert_false( get_value( key, buf, sizeof( buf ) ) );
}


static void
test_delete_key_value_fails_with_undefined_key() {
  expect_string( mock_error, message, "An entry for 'UNDEFINED_KEY' does not exist." );
  assert_false( delete_key_value( "UNDEFINED_KEY" ) );
}


static void
test_delete_key_value_fails_with_too_long_key() {
  size_t key_length = _get_max_key_length() + 1;
  char *key = xmalloc( key_length + 1 );
  memset( key, 'a', key_length );
  key[ key_length ] = '\0';
  char *expected_error = xasprintf( "Too long key ( length = %u ) specified. Maximum length is %u.",
                                    key_length, _get_max_key_length() );

  expect_string( mock_error, message, expected_error );
  assert_false( delete_key_value( key ) );

  xfree( key );
  xfree( expected_error );
}


static void
test_delete_key_value_fails_with_NULL_key() {
  expect_string( mock_error, message, "'key' must be specified." );
  assert_false( delete_key_value( NULL ) );
}


static void
test_delete_key_value_fails_with_invalid_characters() {
  const char invalid_characters[] = { ( char ) 0x7f, ( char ) 0x1f, ( char ) 0xff, ( char ) 0x00 };
  expect_string( mock_error, message, "'key' must only have printable ASCII characters." );
  assert_false( delete_key_value( invalid_characters ) );
}


static void
test_delete_key_value_fails_if_not_initialized() {
  expect_string( mock_error, message, "Backend is not initialized yet." );
  assert_false( delete_key_value( "KEY" ) );
}


static void
test_delete_key_value_escapes_string() {
  set_value( "THIS \"IS\", A, 'KEY'", "VALUE" );

  assert_true( delete_key_value( "THIS \"IS\", A, 'KEY'" ) );
  expect_string( mock_error, message, "Failed to retrieve a value for 'THIS \"IS\", A, 'KEY''." );
  char buf[ 256 ];
  assert_false( get_value( "THIS \"IS\", A, 'KEY'", buf, sizeof( buf ) ) );
}


static void
test_delete_key_value_avoids_injection_attacks() {
  set_value( "KEY", "VALUE" );

  expect_string( mock_error, message, "An entry for 'KEY'; DROP TABLE trema;' does not exist." );
  assert_false( delete_key_value( "KEY'; DROP TABLE trema;" ) );
  char buf[ 256 ];
  assert_true( get_value( "KEY", buf, sizeof( buf ) ) );
  assert_string_equal( "VALUE", buf );
}


static void
test_delete_key_value_fails_if_backend_fails_to_delete_key_value() {
  set_value( "KEY", "VALUE" );
  void *original_sqlite3_exec = trema_sqlite3_exec;
  trema_sqlite3_exec = mock_sqlite3_exec;
  void *original_sqlite3_free = trema_sqlite3_free;
  trema_sqlite3_free = mock_sqlite3_free;

  will_return( mock_sqlite3_exec, SQLITE_OK );
  will_return( mock_sqlite3_exec, SQLITE_ERROR );
  expect_string( mock_error, message, "Failed to execute a SQL statement ( statement = DELETE FROM trema WHERE key = 'KEY', error = ERROR )." );
  assert_false( delete_key_value( "KEY" ) );

  trema_sqlite3_exec = original_sqlite3_exec;
  trema_sqlite3_free = original_sqlite3_free;
}


static void
test_delete_key_value_fails_if_backend_deletes_multiple_key_values() {
  set_value( "KEY", "VALUE" );
  void *original_sqlite3_changes = trema_sqlite3_changes;
  trema_sqlite3_changes = mock_sqlite3_changes;

  will_return( mock_sqlite3_changes, 2 );
  expect_string( mock_error, message, "Multiple entries are deleted ( n_changes = 2 )." );
  assert_false( delete_key_value( "KEY" ) );

  trema_sqlite3_changes = original_sqlite3_changes;
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // init_persistent_storage() tests.
    unit_test_setup_teardown( test_init_persistent_storage_succeeds, setup, teardown ),
    unit_test_setup_teardown( test_init_persistent_storage_fails_if_already_initialized, setup, teardown ),
    unit_test_setup_teardown( test_init_persistent_storage_fails_if_backend_is_not_opened, setup, teardown ),
    unit_test_setup_teardown( test_init_persistent_storage_fails_if_backend_db_is_not_created, setup, teardown ),

    // finalize_persistent_storage() tests.
    unit_test_setup_teardown( test_finalize_persistent_storage_succeeds, setup, teardown ),
    unit_test_setup_teardown( test_finalize_persistent_storage_fails_if_not_initialized, setup, teardown ),
    unit_test_setup_teardown( test_finalize_persistent_storage_fails_if_backend_is_not_closed, setup, teardown ),

    // clear_persistent_storage() tests.
    unit_test_setup_teardown( test_clear_persistent_storage_succeeds, setup, teardown ),
    unit_test_setup_teardown( test_clear_persistent_storage_fails_if_not_initialized, setup, teardown ),
    unit_test_setup_teardown( test_clear_persistent_storage_fails_if_unlink_fails, setup, teardown ),

    // set_value() tests.
    unit_test_setup_teardown( test_set_value_succeeds, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_updates_value, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_clears_value, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_with_too_long_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_with_too_long_value, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_with_NULL_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_with_NULL_value, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_with_invalid_characters, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_if_not_initialized, setup, teardown ),
    unit_test_setup_teardown( test_set_value_escapes_string, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_avoids_injection_attacks, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_set_value_fails_if_backend_fails_to_store_key_value, setup_and_init, teardown_and_finalize ),

    // get_value() tests.
    unit_test_setup_teardown( test_get_value_succeeds, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_with_undefined_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_with_too_long_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_with_insufficient_buffer, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_with_NULL_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_with_NULL_value, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_with_invalid_characters, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_if_not_initialized, setup, teardown ),
    unit_test_setup_teardown( test_get_value_escapes_string, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_avoids_injection_attacks, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_get_value_fails_if_backend_fails_to_lookup_value, setup_and_init, teardown_and_finalize ),

    // delete_key_value() tests.
    unit_test_setup_teardown( test_delete_key_value_succeeds, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_with_undefined_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_with_too_long_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_with_NULL_key, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_with_invalid_characters, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_if_not_initialized, setup, teardown ),
    unit_test_setup_teardown( test_delete_key_value_escapes_string, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_avoids_injection_attacks, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_if_backend_fails_to_delete_key_value, setup_and_init, teardown_and_finalize ),
    unit_test_setup_teardown( test_delete_key_value_fails_if_backend_deletes_multiple_key_values, setup_and_init, teardown_and_finalize ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
