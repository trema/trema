/*
 * Unit tests for utility functions.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmockery.h"
#include "log.h"
#include "trema_wrapper.h"
#include "utility.h"


/********************************************************************************
 * Setup and teardown
 ********************************************************************************/

static void ( *original_critical )( const char *format, ... );

static void
mock_critical( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );
}


static void ( *original_abort )( void );

static void
stub_abort() {
  // Do nothing.
}


static void
setup() {
  original_critical = critical;
  critical = mock_critical;

  original_abort = abort;
  trema_abort = stub_abort;
}


static void
teardown() {
  critical = original_critical;
  trema_abort = original_abort;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_die() {
  expect_string( mock_critical, output, "Bye!" );
  die( "Bye!" );
}


static void
test_compare_string() {
  char hello1[] = "Hello World";
  char hello2[] = "Hello World";
  char bye[] = "Bye World";

  assert_true( compare_string( hello1, hello2 ) );
  assert_false( compare_string( hello1, bye ) );
}


static void
test_hash_string() {
  char hello1[] = "Hello World";
  char hello2[] = "Hello World";

  assert_true( hash_string( hello1 ) == hash_string( hello2 ) );
}


static void
test_compare_uint32() {
  uint32_t x = 123;
  uint32_t y = 123;
  uint32_t z = 321;

  assert_true( compare_uint32( ( void * ) &x, ( void * ) &y ) );
  assert_false( compare_uint32( ( void * ) &x, ( void * ) &z ) );
}


static void
test_hash_uint32() {
  uint32_t key = 123;

  assert_int_equal( 123, hash_uint32( ( void * ) &key ) );
}


static void
test_compare_datapath_id() {
  uint64_t x = 123;
  uint64_t y = 123;
  uint64_t z = 321;

  assert_true( compare_datapath_id( ( void * ) &x, ( void * ) &y ) );
  assert_false( compare_datapath_id( ( void * ) &x, ( void * ) &z ) );
}


static void
test_hash_datapath_id() {
  uint64_t x = 123;
  uint64_t y = 123;
  uint64_t z = 321;

  assert_true( hash_datapath_id( ( void * ) &x ) == hash_datapath_id( ( void * ) &y ) );
  assert_true( hash_datapath_id( ( void * ) &x ) != hash_datapath_id( ( void * ) &z ) );
}


static void
test_compare_mac() {
  uint8_t mac1[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t mac2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  assert_true( compare_mac( mac1, mac1 ) );
  assert_false( compare_mac( mac1, mac2 ) );
}


static void
test_hash_mac() {
  uint8_t mac1[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t mac2[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  assert_true( hash_mac( mac1 ) == hash_mac( mac2 ) );
}


static void
test_string_to_datapath_id() {
  uint64_t datapath_id;
  uint64_t expected_datapath_id = 18446744073709551615ULL;

  assert_true( string_to_datapath_id( "18446744073709551615", &datapath_id ) );
  assert_memory_equal( &datapath_id, &expected_datapath_id, sizeof( uint64_t ) );

  assert_false( string_to_datapath_id( "INVALID DATAPATH ID", &datapath_id ) );
}


static void
test_match_to_string() {
  char match_str[ 256 ];
  char expected_match_str[] = "wildcards = 0, in_port = 1, dl_src = 01:02:03:04:05:07, dl_dst = 08:09:0a:0b:0c:0d, dl_vlan = 1, dl_vlan_pcp = 1, dl_type = 0x800, nw_tos = 1, nw_proto = 6, nw_src = 10.9.8.7, nw_dst = 6.5.4.3, tp_src = 1024, tp_dst = 2048";
  struct ofp_match match = { 0, 1,
                             { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 },
                             { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d },
                             1, 1, { 0 }, 0x800, 1, 0x6, { 0, 0 },
                             0x0a090807, 0x06050403, 1024, 2048 };

  assert_true( match_to_string( &match, match_str, sizeof( match_str ) ) );
  assert_string_equal( match_str, expected_match_str );
}


static void
test_phy_port_to_string() {
  char phy_port_str[ 256 ];
  char expected_phy_port_str[] = "port_no = 1, hw_addr =  01:02:03:04:05:06, name = GbE 0/1, config = 0x1, state = 0x1, curr = 0x4a0, advertised = 0x6bf, supported = 0x6bf, peer = 0x6bf";
  uint8_t hw_addr[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
  uint32_t port_features = ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD |
                             OFPPF_100MB_FD | OFPPF_1GB_HD | OFPPF_1GB_FD |
                             OFPPF_COPPER |  OFPPF_AUTONEG | OFPPF_PAUSE );
  struct ofp_phy_port phy_port;

  phy_port.port_no = 1;
  memcpy( phy_port.hw_addr, hw_addr, sizeof( phy_port.hw_addr ) );
  memset( phy_port.name, '\0', OFP_MAX_PORT_NAME_LEN );
  strncpy( phy_port.name, "GbE 0/1", OFP_MAX_PORT_NAME_LEN );
  phy_port.config = OFPPC_PORT_DOWN;
  phy_port.state = OFPPS_LINK_DOWN;
  phy_port.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  phy_port.advertised = port_features;
  phy_port.supported = port_features;
  phy_port.peer = port_features;

  assert_true( phy_port_to_string( &phy_port, phy_port_str, sizeof( phy_port_str ) ) );
  assert_string_equal( phy_port_str, expected_phy_port_str );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_die, setup, teardown ),

    unit_test( test_compare_string ),
    unit_test( test_hash_string ),

    unit_test( test_compare_uint32 ),
    unit_test( test_hash_uint32 ),

    unit_test( test_compare_datapath_id ),
    unit_test( test_hash_datapath_id ),

    unit_test( test_compare_mac ),
    unit_test( test_hash_mac ),

    unit_test( test_string_to_datapath_id ),

    unit_test( test_match_to_string ),
    unit_test( test_phy_port_to_string ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
