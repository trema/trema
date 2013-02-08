/*
 * Unit tests for ether functions and macros.
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


#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "utility.h"
#include "packet_info.h"


/******************************************************************************
 * Mock functions.
 ******************************************************************************/

static void ( *original_die )( const char *format, ... );

static void
mock_die( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );

  mock_assert( false, "mock_die", __FILE__, __LINE__ );
}


/******************************************************************************
 * Setup and teardown function.
 ******************************************************************************/

static void
setup() {
  original_die = die;
  die = mock_die;
}


static void
teardown() {
  die = original_die;
}


/******************************************************************************
 * Test functions.
 ******************************************************************************/

void
test_fill_ether_padding_succeeds_if_length_is_ETH_MINIMUM_LENGTH() {
  buffer *buffer = alloc_buffer_with_length( ( size_t ) ETH_MINIMUM_LENGTH );
  append_back_buffer( buffer, ETH_MINIMUM_LENGTH );

  fill_ether_padding( buffer );

  assert_int_equal ( ( int ) buffer->length, ETH_MINIMUM_LENGTH );

  free_buffer( buffer );
}


void
test_fill_ether_padding_succeeds_if_length_is_less_than_ETH_MINIMUM_LENGTH() {
  buffer *buffer = alloc_buffer_with_length( ( size_t ) ETH_MINIMUM_LENGTH - ETH_FCS_LENGTH - 1 );
  append_back_buffer( buffer, ETH_MINIMUM_LENGTH - ETH_FCS_LENGTH - 1 );

  fill_ether_padding( buffer );

  assert_int_equal ( ( int ) buffer->length, ETH_MINIMUM_LENGTH - ETH_FCS_LENGTH );

  free_buffer( buffer );
}


void
test_fill_ether_padding_fails_if_buffer_is_NULL() {
  expect_string( mock_die, output,
                 "Argument of fill_ether_padding must not be NULL." );
  expect_assert_failure( fill_ether_padding( NULL ) );
}


/******************************************************************************
 * Run tests.
 ******************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_fill_ether_padding_succeeds_if_length_is_ETH_MINIMUM_LENGTH ),
    unit_test( test_fill_ether_padding_succeeds_if_length_is_less_than_ETH_MINIMUM_LENGTH ),
    unit_test_setup_teardown( test_fill_ether_padding_fails_if_buffer_is_NULL,
                              setup, teardown ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
