/*
 * Unit tests for x-wrappers.
 * 
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmockery.h"
#include "wrapper.h"


/********************************************************************************
 * Mock function.
 ********************************************************************************/

int
mock_vasprintf( char **strp, const char *fmt, va_list ap ) {
  const size_t buffer_length = 1024;
  *strp = xmalloc( buffer_length );
  memset( *strp, '\0', buffer_length );
  return vsprintf( *strp, fmt, ap );
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
test_xcalloc_and_xfree() {
  void *mem = xcalloc( 1, 1 );
  assert_int_equal( 0, ( ( char * ) mem )[ 0 ] );
  xfree( mem );
}


static void
test_xstrdup() {
  char hello[] = "Hello";
  char *dup_hello = xstrdup( hello );
  assert_string_equal( hello, dup_hello );
  xfree( dup_hello );
}


static void
test_xasprintf() {
  char hello[] = "Hello";
  char *printed_hello = xasprintf( "%s", hello );
  assert_string_equal( hello, printed_hello );
  xfree( printed_hello );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_xmalloc_and_xfree ),
    unit_test( test_xcalloc_and_xfree ),
    unit_test( test_xstrdup ),
    unit_test( test_xasprintf ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
