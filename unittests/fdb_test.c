/*
 * Unit tests for routing switch
 * 
 * Author: Shuji Ishii
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
#include <string.h>
#include "checks.h"
#include "cmockery.h"


#include "trema.h"
#include "ether.h"
#include "fdb.h"


#define VOID_FUNCTION (0)
#define FDB_ENTRY_TIMEOUT 300
#define FDB_AGING_INTERVAL 5
static const int HOST_MOVE_GUARD_SEC = 5;


typedef struct mac_db_entry {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint64_t dpid;
  uint16_t port;
  time_t updated_at;
  time_t created_at;
} fdb_entry;


// static functions
void age_fdb_entry( void *key, void *value, void *user_data );
void age_fdb( void *user_data );


/**********************************************************************
 * Helper functions.                                                  
 **********************************************************************/

// switch A, host a, host aa
static uint64_t dpid_switch_A = 0x1111ULL;
__attribute__((unused)) static uint16_t port_no_from_A_to_B = 1;
__attribute__((unused)) static uint16_t port_no_from_A_to_C = 2;
static uint16_t port_no_from_A_to_host_a = 3;
static uint16_t port_no_from_A_to_host_aa = 4;
__attribute__((unused)) static uint16_t new_port_no_of_switch_A = 5;
__attribute__((unused)) static uint16_t unknown_port_of_switch_A = 6;
static uint8_t host_a_mac[ ETH_ADDRLEN ] = { 0x10, 0x22, 0x33, 0xaa, 0xbb, 0xcc };
static time_t create_time_of_host_a = 100;
static time_t update_time_of_host_a = 110;
static uint8_t host_aa_mac[ ETH_ADDRLEN ] = { 0x10, 0x33, 0x55, 0x88, 0x99, 0xff };
static time_t create_time_of_host_aa = 200;
static time_t update_time_of_host_aa = 210;

// switch B, host b
static uint64_t dpid_switch_B = 0x2222ULL;
__attribute__((unused)) static uint16_t port_no_from_B_to_C = 1;
__attribute__((unused)) static uint16_t port_no_from_B_to_A = 2;
static uint16_t port_no_from_B_to_host_b = 3;
static uint8_t host_b_mac[ ETH_ADDRLEN ] = { 0xbc, 0xcc, 0xaa, 0x00, 0x88, 0x99 };
static time_t create_time_of_host_b = 300;
static time_t update_time_of_host_b = 310;

// switch C, host c
static uint64_t dpid_switch_C = 0x3333ULL;
__attribute__((unused)) static uint16_t port_no_from_C_to_A = 1;
__attribute__((unused)) static uint16_t port_no_from_C_to_B = 2;
static uint16_t port_no_from_C_to_host_c = 3;
static uint8_t host_c_mac[ ETH_ADDRLEN ] = { 0xcc, 0x77, 0x11, 0x55, 0xaa, 0xff };
static time_t create_time_of_host_c = 400;
static time_t update_time_of_host_c = 410;

// switch D (not registered)
static uint64_t dpid_switch_D = 0x4444ULL;
static uint16_t new_port_of_switch_D = 3333;

// new host
static uint8_t new_host_mac[ ETH_ADDRLEN ] = { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54 };
static time_t create_time_of_new_host = 500;
//static time_t update_time_of_new_host = 510;

// multicast/broadcast frame
static uint8_t multicast_ipv6_mac[ ETH_ADDRLEN ] = { 0x33, 0x33, 0x00, 0x01, 0x00, 0x03};
static uint8_t multicast_stp_mac[ ETH_ADDRLEN ] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
static uint8_t multicast_ipv4_mac[ ETH_ADDRLEN ] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0xfc };
static uint8_t broadcast_mac[ ETH_ADDRLEN ] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };

// cookie, transaction_id
static uint64_t cookie = 0x1111222233334444ULL;
static const uint32_t transaction_id = 0x3456789aU;

static fdb_entry *
helper_lookup_fdb( hash_table *fdb, uint8_t mac[ OFP_ETH_ALEN ] ) {
  return ( fdb_entry * ) lookup_hash_entry( fdb, ( void * ) mac );
}


/*************************************************************************
 * Setup and teardown function.                                          
 *************************************************************************/

static hash_table *
setup() {
  assert_true( create_time_of_host_a + HOST_MOVE_GUARD_SEC < update_time_of_host_a );
  assert_true( create_time_of_host_aa + HOST_MOVE_GUARD_SEC < update_time_of_host_aa );
  assert_true( create_time_of_host_b + HOST_MOVE_GUARD_SEC < update_time_of_host_b );
  assert_true( create_time_of_host_c + HOST_MOVE_GUARD_SEC < update_time_of_host_c );

  init_log( "fdb_test", false );

  hash_table *fdb = create_hash( compare_mac, hash_mac );

  fdb_entry *new_entry = xmalloc( sizeof( fdb_entry ) );
  new_entry->dpid = dpid_switch_A;
  memcpy( new_entry->mac, host_a_mac, ETH_ADDRLEN );
  new_entry->port = port_no_from_A_to_host_a;
  new_entry->updated_at = update_time_of_host_a;
  new_entry->created_at = create_time_of_host_a;
  insert_hash_entry( fdb, new_entry->mac, new_entry );

  new_entry = xmalloc( sizeof( fdb_entry ) );
  new_entry->dpid = dpid_switch_A;
  memcpy( new_entry->mac, host_aa_mac, ETH_ADDRLEN );
  new_entry->port = port_no_from_A_to_host_aa;
  new_entry->updated_at = update_time_of_host_aa;
  new_entry->created_at = create_time_of_host_aa;
  insert_hash_entry( fdb, new_entry->mac, new_entry );

  new_entry = xmalloc( sizeof( fdb_entry ) );
  new_entry->dpid = dpid_switch_B;
  memcpy( new_entry->mac, host_b_mac, ETH_ADDRLEN );
  new_entry->port = port_no_from_B_to_host_b;
  new_entry->updated_at = update_time_of_host_b;
  new_entry->created_at = create_time_of_host_b;
  insert_hash_entry( fdb, new_entry->mac, new_entry );

  new_entry = xmalloc( sizeof( fdb_entry ) );
  new_entry->dpid = dpid_switch_C;
  memcpy( new_entry->mac, host_c_mac, ETH_ADDRLEN );
  new_entry->port = port_no_from_C_to_host_c;
  new_entry->updated_at = update_time_of_host_c;
  new_entry->created_at = create_time_of_host_c;
  insert_hash_entry( fdb, new_entry->mac, new_entry );

  return fdb;
}


static void
teardown( hash_table *fdb ) {
  if ( fdb != NULL ) {
    hash_iterator iter;
    hash_entry *e;
    init_hash_iterator( fdb, &iter );
    while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
      xfree( e->value );
    }
    delete_hash( fdb );
  }
}


/*************************************************************************
 * Mock                                                                  
 *************************************************************************/

void
mock_die( char *format, ... ) {
  UNUSED( format );
}


time_t
mock_time( time_t *t ) {
  UNUSED( t );
  return ( time_t )mock();
}


bool
mock_add_periodic_event_callback( /*const*/ time_t seconds, void ( *callback )( void *user_data ), void *user_data ) {
  check_expected( seconds );
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}


uint64_t
mock_get_cookie() {
  return cookie;
}


uint32_t
mock_get_transaction_id() {
  return transaction_id;
}


buffer *
mock_create_flow_mod( /*const*/ uint32_t transaction_id, /*const*/ struct ofp_match match0,
                      /*const*/ uint64_t cookie64, /*const*/ uint16_t command16,
                      /*const*/ uint16_t idle_timeout16, /*const*/ uint16_t hard_timeout16,
                      /*const*/ uint16_t priority16, /*const*/ uint32_t buffer_id,
                      /*const*/ uint16_t out_port16, /*const*/ uint16_t flags16,
                      /*const*/ openflow_actions_t *actions ) {
  check_expected( transaction_id );
  struct ofp_match *match = &match0;
  check_expected( match );
  uint64_t *cookie = &cookie64;
  check_expected( cookie );
  uint32_t command = command16;
  check_expected( command );
  uint32_t idle_timeout = idle_timeout16;
  check_expected( idle_timeout );
  uint32_t hard_timeout = hard_timeout16;
  check_expected( hard_timeout );
  uint32_t priority = priority16;
  check_expected( priority );
  check_expected( buffer_id );
  uint32_t out_port = out_port16;
  check_expected( out_port );
  uint32_t flags = flags16;
  check_expected( flags );
  check_expected( actions );

  return ( buffer * ) mock();
}


bool
mock_send_openflow_message( /*const*/ uint64_t datapath_id64, buffer *message ) {
  uint64_t *datapath_id = &datapath_id64;
  check_expected( datapath_id );
  check_expected( message );

  return ( bool ) mock();
}


void
mock_free_buffer( buffer *buf ) {
  check_expected( buf );

  ( void ) mock();
}


/************************************************************************
 * Tests.                                                               
 ************************************************************************/

static void
test_mock_die() {
  char message[] = "die";
  mock_die( message );
}


static void
test_create_and_delete_fdb() {
  hash_table *fdb = create_fdb();

  assert_true( lookup_hash_entry( fdb, host_a_mac ) == NULL );
  assert_true( lookup_hash_entry( fdb, host_aa_mac ) == NULL );
  assert_true( lookup_hash_entry( fdb, host_b_mac ) == NULL );
  assert_true( lookup_hash_entry( fdb, host_c_mac ) == NULL );

  fdb_entry *host_a_entry = xmalloc( sizeof( fdb_entry ) );
  host_a_entry->dpid = dpid_switch_A;
  memcpy( host_a_entry->mac, host_a_mac, ETH_ADDRLEN );
  host_a_entry->port = port_no_from_A_to_host_a;
  host_a_entry->updated_at = update_time_of_host_a;
  insert_hash_entry( fdb, host_a_entry->mac, host_a_entry );

  assert_true( lookup_hash_entry( fdb, host_a_mac ) == host_a_entry );

  delete_fdb( fdb );
}


static void
test_init_age_fdb() {
  hash_table *fdb = setup();

  // add_periodic_event_callback()
  expect_value( mock_add_periodic_event_callback, seconds, FDB_AGING_INTERVAL );
  expect_value( mock_add_periodic_event_callback, callback, age_fdb );
  expect_value( mock_add_periodic_event_callback, user_data, fdb );
  will_return( mock_add_periodic_event_callback, true );

  // target
  init_age_fdb( fdb );

  teardown( fdb );
}


static void
test_add_new_fdb_successed() {
  hash_table *fdb = setup();

  // time()
  will_return( mock_time, 100 );

  // target
  assert_true( update_fdb( fdb, new_host_mac, dpid_switch_D, new_port_of_switch_D ) );

  // check
  fdb_entry *result = lookup_hash_entry( fdb, new_host_mac );
  assert_true( result != NULL );
  assert_memory_equal( result->mac, new_host_mac, OFP_ETH_ALEN );
  assert_true( result->dpid == dpid_switch_D );
  assert_true( result->port == new_port_of_switch_D );
  assert_true( result->updated_at == 100 );
  assert_true( result->created_at == 100 );

  teardown( fdb );
}


static void
test_update_fdb_for_new_host() {
  hash_table *fdb = setup();

  // time()
  will_return( mock_time, create_time_of_new_host );

  // target
  assert_true( update_fdb( fdb, new_host_mac, dpid_switch_D, new_port_of_switch_D ) );

  // check
  fdb_entry *entry = lookup_hash_entry( fdb, new_host_mac );
  assert_true( entry != NULL );
  assert_true( entry->dpid == dpid_switch_D );
  assert_memory_equal( entry->mac, new_host_mac, ETH_ADDRLEN );
  assert_true( entry->port == new_port_of_switch_D );
  assert_true( entry->updated_at == create_time_of_new_host );
  assert_true( entry->created_at == create_time_of_new_host );

  teardown( fdb );
}


static void
test_update_fdb_for_already_registered_host() {
  hash_table *fdb = setup();
  
  // time()
  will_return( mock_time, ( uint32_t ) update_time_of_host_a );

  // target
  assert_true( update_fdb( fdb, host_a_mac, dpid_switch_A, port_no_from_A_to_host_a ) );

  // check
  fdb_entry *entry = lookup_hash_entry( fdb, host_a_mac );
  assert_true( entry != NULL );
  assert_true( entry->dpid == dpid_switch_A );
  assert_memory_equal( entry->mac, host_a_mac, ETH_ADDRLEN );
  assert_true( entry->port == port_no_from_A_to_host_a );
  assert_true( entry->updated_at == update_time_of_host_a );
  assert_true( entry->created_at == create_time_of_host_a );

  teardown( fdb );
}


static void
test_update_fdb_for_host_moved() {
  hash_table *fdb = setup();
  
  // time()
  will_return( mock_time, update_time_of_host_a );

  // time()
  will_return( mock_time, update_time_of_host_a );

  // poisoning()->
  // create_flow_mod()
  expect_value( mock_create_flow_mod, transaction_id, transaction_id );
  struct ofp_match expected_match_dst;
  memset( &expected_match_dst, 0, sizeof( struct ofp_match ) );
  memcpy( expected_match_dst.dl_dst, host_a_mac, OFP_ETH_ALEN );
  expected_match_dst.wildcards = ( OFPFW_ALL & ~OFPFW_DL_DST );
  expect_memory( mock_create_flow_mod, match, &expected_match_dst, sizeof( struct ofp_match ) );
  expect_memory( mock_create_flow_mod, cookie, &cookie, sizeof( uint64_t ) );
  expect_value( mock_create_flow_mod, command, OFPFC_DELETE );
  expect_value( mock_create_flow_mod, idle_timeout, 0 );
  expect_value( mock_create_flow_mod, hard_timeout, 0 );
  expect_value( mock_create_flow_mod, priority, 0 );
  expect_value( mock_create_flow_mod, buffer_id, 0 );
  expect_value( mock_create_flow_mod, out_port, OFPP_NONE );
  expect_value( mock_create_flow_mod, flags, 0 );
  expect_value( mock_create_flow_mod, actions, NULL );
  buffer dummy_flow_mod;
  will_return( mock_create_flow_mod, &dummy_flow_mod );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, datapath_id, &dpid_switch_A, sizeof( uint64_t )  );
  expect_value( mock_send_openflow_message, message, &dummy_flow_mod );
  will_return( mock_send_openflow_message, true );
 
  // free_buffer()
  expect_value( mock_free_buffer, buf, &dummy_flow_mod );
  will_return( mock_free_buffer, VOID_FUNCTION );

  
  // create_flow_mod()
  expect_value( mock_create_flow_mod, transaction_id, transaction_id );
  struct ofp_match expected_match_src;
  memset( &expected_match_src, 0, sizeof( struct ofp_match ) );
  memcpy( expected_match_src.dl_src, host_a_mac, OFP_ETH_ALEN );
  expected_match_src.wildcards = ( OFPFW_ALL & ~OFPFW_DL_SRC );
  expect_memory( mock_create_flow_mod, match, &expected_match_src, sizeof( struct ofp_match ) );
  expect_memory( mock_create_flow_mod, cookie, &cookie, sizeof( uint64_t ) );
  expect_value( mock_create_flow_mod, command, OFPFC_DELETE );
  expect_value( mock_create_flow_mod, idle_timeout, 0 );
  expect_value( mock_create_flow_mod, hard_timeout, 0 );
  expect_value( mock_create_flow_mod, priority, 0 );
  expect_value( mock_create_flow_mod, buffer_id, 0 );
  expect_value( mock_create_flow_mod, out_port, OFPP_NONE );
  expect_value( mock_create_flow_mod, flags, 0 );
  expect_value( mock_create_flow_mod, actions, NULL );
  will_return( mock_create_flow_mod, &dummy_flow_mod );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, datapath_id, &dpid_switch_A, sizeof( uint64_t )  );
  expect_value( mock_send_openflow_message, message, &dummy_flow_mod );
  will_return( mock_send_openflow_message, true );
 
  // free_buffer()
  expect_value( mock_free_buffer, buf, &dummy_flow_mod );
  will_return( mock_free_buffer, VOID_FUNCTION );

  // target (from switch A to switch C)
  assert_true( update_fdb( fdb, host_a_mac, dpid_switch_C, port_no_from_C_to_host_c ) );

  // check
  fdb_entry *entry = lookup_hash_entry( fdb, host_a_mac );
  assert_true( entry != NULL );
  assert_true( entry->dpid == dpid_switch_C );
  assert_memory_equal( entry->mac, host_a_mac, ETH_ADDRLEN );
  assert_true( entry->port == port_no_from_C_to_host_c );
  assert_true( entry->updated_at == update_time_of_host_a );
  assert_true( entry->created_at == create_time_of_host_a );

  teardown( fdb );
}


static void
test_update_fdb_for_host_moved_too_fast() {
  hash_table *fdb = setup();
  
  // time()
  will_return( mock_time, ( uint32_t ) ( create_time_of_host_a + HOST_MOVE_GUARD_SEC - 1 ) );

  // target (from switch A to switch C)
  assert_false( update_fdb( fdb, host_a_mac, dpid_switch_C, port_no_from_C_to_host_c ) );


  teardown( fdb );
}


static void
test_lookup_fdb_successed() {
  hash_table *fdb = setup();

  uint64_t dpid = 0xdeadbeefULL;
  uint16_t port_no = 0xcafeU;

  // target
  assert_true( lookup_fdb( fdb, host_a_mac, &dpid, &port_no ) ); // lookup host_a

  assert_true( dpid == dpid_switch_A );
  assert_true( port_no = port_no_from_A_to_host_a );

  teardown( fdb );
}
  

static void
test_lookup_fdb_failed() {
  hash_table *fdb = setup();

  uint64_t dpid = 0xdeadbeefULL;
  uint16_t port_no = 0xcafeU;

  // target
  assert_false( lookup_fdb( fdb, new_host_mac, &dpid, &port_no ) ); // lookup new_host

  // check if dpid and port_no are untouched
  assert_true( dpid == 0xdeadbeefULL );
  assert_true( port_no = 0xcafeU );

  teardown( fdb );
}


static void
test_lookup_fdb_multicast_frame() {
  hash_table *fdb = setup();

  uint64_t dpid = 0xdeadbeefULL;
  uint16_t port_no = 0xcafeU;

  uint8_t *macs[] = { multicast_ipv6_mac, multicast_stp_mac, multicast_ipv4_mac, broadcast_mac, NULL };

  for ( int i = 0; macs[ i ] != NULL; i++ ) {
    // target
    assert_false( lookup_fdb( fdb, macs[ i ], &dpid, &port_no ) ); // lookup new_host

    // check if dpid and port_no are untouched
    assert_true( dpid == 0xdeadbeefULL );
    assert_true( port_no = 0xcafeU );
  }

  teardown( fdb );
}


static void
test_age_fdb_successed() {
  hash_table *fdb = setup();

  // time() x 4, value 550 causes expire fdb entries of host_a and host_aa
  will_return_count( mock_time, 550, 4 );

  // target
  age_fdb( fdb );

  // check if host_a and host_aa are expired, host_b and host_c are not.
  assert_true( helper_lookup_fdb( fdb, host_a_mac ) == NULL );
  assert_true( helper_lookup_fdb( fdb, host_aa_mac ) == NULL );
  assert_true( helper_lookup_fdb( fdb, host_b_mac ) != NULL );
  assert_true( helper_lookup_fdb( fdb, host_c_mac ) != NULL );

  teardown( fdb );
}


int
main() {
  const UnitTest tests[] = {
    unit_test( test_mock_die ),

    unit_test( test_create_and_delete_fdb ),
    unit_test( test_init_age_fdb ),
    unit_test( test_add_new_fdb_successed ),

    unit_test( test_update_fdb_for_already_registered_host ),
    unit_test( test_update_fdb_for_new_host ),
    unit_test( test_update_fdb_for_host_moved ),
    unit_test( test_update_fdb_for_host_moved_too_fast ),

    unit_test( test_lookup_fdb_successed ),
    unit_test( test_lookup_fdb_failed ),
    unit_test( test_lookup_fdb_multicast_frame ),
    unit_test( test_age_fdb_successed ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

