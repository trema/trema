/*
 * Unit tests for x-wrappers.
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
#include "checks.h"
#include "cmockery_trema.h"
#include "trema_wrapper.h"
#include "wrapper.h"


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static void *
fail_malloc( size_t size ) {
  UNUSED( size );
  return NULL;
}


static void *
fail_calloc( size_t nmemb, size_t size ) {
  UNUSED( nmemb );
  UNUSED( size );
  return NULL;
}


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


static int
fail_vasprintf( char **strp, const char *fmt, va_list ap ) {
  UNUSED( strp );
  UNUSED( fmt );
  UNUSED( ap );
  return -1;
}


/********************************************************************************
 * Setup and teardown
 ********************************************************************************/

static void
setup() {
  setup_leak_detector();

  original_die = die;
  die = mock_die;
}


static void
setup_fail_allocators() {
  setup();

  trema_malloc = fail_malloc;
  trema_calloc = fail_calloc;
  trema_vasprintf = fail_vasprintf;
}


static void
teardown() {
  teardown_leak_detector();

  trema_vasprintf = vasprintf;
  die = original_die;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_xmalloc_and_xfree() {
  void *mem = xmalloc( 1 );
  xfree( mem );
}


static void
test_xmalloc_fail() {
  expect_string( mock_die, output, "Out of memory, xmalloc failed" );
  expect_assert_failure( xmalloc( 1 ) );
}


static void
test_xcalloc_and_xfree() {
  void *mem = xcalloc( 1, 1 );
  assert_int_equal( 0, ( ( char * ) mem )[ 0 ] );
  xfree( mem );
}


static void
test_xcalloc_fail() {
  expect_string( mock_die, output, "Out of memory, xcalloc failed" );
  expect_assert_failure( xcalloc( 1, 1 ) );
}


static void
test_xstrdup() {
  char hello[] = "Hello";
  char *dup_hello = xstrdup( hello );
  assert_string_equal( hello, dup_hello );
  xfree( dup_hello );
}


static void
test_xstrdup_fail() {
  expect_string( mock_die, output, "Out of memory, xstrdup failed" );
  expect_assert_failure( xstrdup( "FAIL" ) );
}


static char *
test_xvasprintf_helper( const char *fmt, ... ) {
  va_list ap;
  va_start(ap, fmt );
  char *str = xvasprintf( fmt, ap );
  va_end( ap );
  return str;
}


static void
test_xvasprintf() {
  char hello[] = "Hello";
  char *printed_hello = test_xvasprintf_helper( "%s", hello );
  assert_string_equal( hello, printed_hello );
  xfree( printed_hello );
}


static void
test_xvasprintf_fail() {
  expect_string( mock_die, output, "Out of memory, xvasprintf failed" );
  expect_assert_failure( test_xvasprintf_helper( "FAIL" ) );
}


static void
test_xasprintf() {
  char hello[] = "Hello";
  char *printed_hello = xasprintf( "%s", hello );
  assert_string_equal( hello, printed_hello );
  xfree( printed_hello );
}


static void
test_xasprintf_fail() {
  expect_string( mock_die, output, "Out of memory, xasprintf failed" );
  expect_assert_failure( xasprintf( "FAIL" ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_xmalloc_and_xfree, setup, teardown ),
    unit_test_setup_teardown( test_xmalloc_fail, setup_fail_allocators, teardown ),

    unit_test_setup_teardown( test_xcalloc_and_xfree, setup, teardown ),
    unit_test_setup_teardown( test_xcalloc_fail, setup_fail_allocators, teardown ),

    unit_test_setup_teardown( test_xstrdup, setup, teardown ),
    unit_test_setup_teardown( test_xstrdup_fail, setup_fail_allocators, teardown ),

    unit_test_setup_teardown( test_xvasprintf, setup, teardown ),
    unit_test_setup_teardown( test_xvasprintf_fail, setup_fail_allocators, teardown ),

    unit_test_setup_teardown( test_xasprintf, setup, teardown ),
    unit_test_setup_teardown( test_xasprintf_fail, setup_fail_allocators, teardown ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
