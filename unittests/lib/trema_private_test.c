/*
 * Unit tests for trema_private.[ch]
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "cmockery_trema.h"
#include "trema.h"
#include "trema_private.h"
#include "trema_wrapper.h"


/********************************************************************************
 * Mock and stub functions.
 ********************************************************************************/

static void
mock_die( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );

  mock_assert( false, "mock_die", __FILE__, __LINE__ ); } // Hoaxes gcov.


static int
mock_fprintf( FILE *stream, const char *format, ... ) {
  UNUSED( stream );

  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );

  check_expected( output );

  return 0;
}


/********************************************************************************
 * Setup and teardown.
 ********************************************************************************/

static void
reset() {
  unsetenv( "TREMA_HOME" );
  unsetenv( "TREMA_TMP" );

  die = mock_die;
  trema_fprintf = mock_fprintf;
}


/********************************************************************************
 * set_trema_home() tests.
 *******************************************************************************/

static void
test_set_trema_home() {
  setenv( "TREMA_HOME", "/var", 1 );

  set_trema_home();

  assert_string_equal( "/var", _get_trema_home() );

  unset_trema_home();
}


static void
test_set_trema_home_when_TREMA_HOME_is_NOT_set() {
  set_trema_home();

  assert_string_equal( "/", _get_trema_home() );

  unset_trema_home();
}


static void
test_set_trema_home_falls_back_to_ROOT_if_TREMA_HOME_is_invalid() {
  setenv( "TREMA_HOME", "NO_SUCH_DIRECTORY", 1 );

  expect_string( mock_fprintf, output, "Could not get the absolute path of NO_SUCH_DIRECTORY: No such file or directory.\n" );
  expect_string( mock_fprintf, output, "Falling back TREMA_HOME to \"/\".\n" );

  set_trema_home();

  assert_string_equal( "/", _get_trema_home() );

  unset_trema_home();
}


/********************************************************************************
 * get_trema_home() tests.
 *******************************************************************************/

static void
test_get_trema_home() {
  setenv( "TREMA_HOME", "/var", 1 );
  set_trema_home();

  assert_string_equal( "/var", get_trema_home() );

  unset_trema_home();
}


static void
test_get_trema_home_if_not_initilized() {
  setenv( "TREMA_HOME", "/var", 1 );

  assert_string_equal( "/var", get_trema_home() );

  unset_trema_home();
}


/********************************************************************************
 * unset_trema_home() test.
 *******************************************************************************/

static void
test_unset_trema_home() {
  setenv( "TREMA_HOME", "/var", 1 );
  set_trema_home();

  unset_trema_home();

  assert_true( _get_trema_home() == NULL );
}


/********************************************************************************
 * set_trema_tmp() tests.
 *******************************************************************************/

static void
test_set_trema_tmp() {
  setenv( "TREMA_TMP", "/var/tmp", 1 );

  set_trema_tmp();

  assert_string_equal( "/var/tmp", _get_trema_tmp() );

  unset_trema_tmp();
}


static void
test_set_trema_tmp_when_TREMA_TMP_is_NOT_set_and_TREMA_HOME_is_ROOT() {
  setenv( "TREMA_HOME", "/", 1 );
  set_trema_home();

  set_trema_tmp();

  assert_string_equal( "/tmp", _get_trema_tmp() );

  unset_trema_home();
  unset_trema_tmp();
}


static void
test_set_trema_tmp_when_TREMA_TMP_is_NOT_set_and_TREMA_HOME_is_NOT_ROOT() {
  setenv( "TREMA_HOME", "/var", 1 );
  set_trema_home();

  set_trema_tmp();

  assert_string_equal( "/var/tmp", _get_trema_tmp() );

  unset_trema_home();
  unset_trema_tmp();
}


static void
test_set_trema_tmp_falls_back_to_default_if_TREMA_TMP_is_invalid() {
  setenv( "TREMA_TMP", "NO_SUCH_DIRECTORY", 1 );

  expect_string( mock_fprintf, output, "Could not get the absolute path of NO_SUCH_DIRECTORY: No such file or directory.\n" );
  expect_string( mock_fprintf, output, "Falling back TREMA_TMP to \"/tmp\".\n" );

  set_trema_tmp();

  assert_string_equal( "/tmp", _get_trema_tmp() );

  unset_trema_home();
  unset_trema_tmp();
}


static void
test_set_trema_tmp_when_TREMA_HOME_and_TREMA_TMP_are_set() {
  setenv( "TREMA_HOME", "/var", 1 );
  set_trema_home();

  setenv( "TREMA_TMP", "/", 1 );
  set_trema_tmp();

  assert_string_equal( "/var", _get_trema_home() );
  assert_string_equal( "/", _get_trema_tmp() );

  unset_trema_home();
  unset_trema_tmp();
}


/********************************************************************************
 * get_trema_tmp() tests.
 *******************************************************************************/

static void
test_get_trema_tmp() {
  setenv( "TREMA_TMP", "/var/tmp", 1 );

  set_trema_tmp();

  assert_string_equal( "/var/tmp", get_trema_tmp() );

  unset_trema_home();
  unset_trema_tmp();
}


static void
test_get_trema_tmp_if_not_initialized() {
  assert_string_equal( "/tmp", get_trema_tmp() );

  unset_trema_home();
  unset_trema_tmp();
}


/********************************************************************************
 * unset_trema_tmp() test.
 *******************************************************************************/

static void
test_unset_trema_tmp() {
  setenv( "TREMA_TMP", "/var/tmp", 1 );
  set_trema_tmp();

  unset_trema_tmp();

  assert_true( _get_trema_tmp() == NULL );

  unset_trema_home();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // set_trema_home() tests.
    unit_test_setup_teardown( test_set_trema_home, reset, reset ),
    unit_test_setup_teardown( test_set_trema_home_when_TREMA_HOME_is_NOT_set, reset, reset ),
    unit_test_setup_teardown( test_set_trema_home_falls_back_to_ROOT_if_TREMA_HOME_is_invalid, reset, reset ),

    // get_trema_home() tests.
    unit_test_setup_teardown( test_get_trema_home, reset, reset ),
    unit_test_setup_teardown( test_get_trema_home_if_not_initilized, reset, reset ),

    // unset_trema_home() test.
    unit_test_setup_teardown( test_unset_trema_home, reset, reset ),

    // set_trema_tmp() tests.
    unit_test_setup_teardown( test_set_trema_tmp, reset, reset ),
    unit_test_setup_teardown( test_set_trema_tmp_when_TREMA_TMP_is_NOT_set_and_TREMA_HOME_is_ROOT, reset, reset ),
    unit_test_setup_teardown( test_set_trema_tmp_when_TREMA_TMP_is_NOT_set_and_TREMA_HOME_is_NOT_ROOT, reset, reset ),
    unit_test_setup_teardown( test_set_trema_tmp_falls_back_to_default_if_TREMA_TMP_is_invalid, reset, reset ),
    unit_test_setup_teardown( test_set_trema_tmp_when_TREMA_HOME_and_TREMA_TMP_are_set, reset, reset ),

    // get_trema_tmp() tests.
    unit_test_setup_teardown( test_get_trema_tmp, reset, reset ),
    unit_test_setup_teardown( test_get_trema_tmp_if_not_initialized, reset, reset ),

    // unset_trema_tmp() test.
    unit_test_setup_teardown( test_unset_trema_tmp, reset, reset ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
