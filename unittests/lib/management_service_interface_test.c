/*
 * Unit tests for management_service_interface.[ch]
 * 
 * Author: Yasunobu Chiba
 *
 * Copyright (C) 2012 NEC Corporation
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"
#include "trema_wrapper.h"
#include "cmockery_trema.h"


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static void ( *original_die )( const char *format, ... );

static void
mock_die( const char *format, ... ) {
  UNUSED( format );
  mock_assert( false, "mock_die", __FILE__, __LINE__ ); } // Hoaxes gcov.


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

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
 * get_management_service_name() tests.
 ********************************************************************************/

static void
test_get_management_service_name_succeeds() {
  assert_string_equal( get_management_service_name( "tetris" ), "tetris.m" );
}


static void
test_get_management_service_name_fails_if_service_name_is_NULL() {
  expect_assert_failure( get_management_service_name( NULL ) );
}


static void
test_get_management_service_name_fails_if_service_name_length_is_zero() {
  expect_assert_failure( get_management_service_name( "" ) );
}


static void
test_get_management_service_name_fails_with_too_long_service_name() {
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  memset( service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  memset( service_name, 'a', MESSENGER_SERVICE_NAME_LENGTH - 1 );

  expect_assert_failure( get_management_service_name( service_name ) );
}



/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // get_management_service_name() tests.
    unit_test_setup_teardown( test_get_management_service_name_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_get_management_service_name_fails_if_service_name_is_NULL,
                              setup, teardown ),
    unit_test_setup_teardown( test_get_management_service_name_fails_if_service_name_length_is_zero,
                              setup, teardown ),
    unit_test_setup_teardown( test_get_management_service_name_fails_with_too_long_service_name,
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
