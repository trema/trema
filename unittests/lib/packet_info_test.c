/*
 * Unit tests for packet_info functions and macros.
 *
 * Author: Naoyoshi Tada
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


#include <assert.h>
#include <netinet/ip.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "packet_info.h"


/********************************************************************************
 * Tests.
 ********************************************************************************/

void
test_alloc_packet_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );

  alloc_packet( buf );
  assert_true( buf->user_data  != NULL );

  free_buffer( buf );
}


void
test_alloc_packet_fails_if_buffer_is_NULL() {
  expect_assert_failure( alloc_packet( NULL ) ) ;
}


void
test_free_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  alloc_packet( buf );
  assert_true( buf->user_data_free_function != NULL );
  ( *buf->user_data_free_function )( buf );
  assert_true( buf->user_data == NULL );
  assert_true( buf->user_data_free_function == NULL );

  free_buffer( buf );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_alloc_packet_succeeds ),
    unit_test( test_alloc_packet_fails_if_buffer_is_NULL ),

    unit_test( test_free_buffer_succeeds ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
