/*
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

#include "checks.h"
#include "cmockery_trema.h"
#include "trema.h"
#include "topology_table.h"


/********************************************************************************
 * Common function.
 ********************************************************************************/


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

static void
setup() {
  init_topology_table();
}

static void
teardown() {
  finalize_topology_table();
}

/********************************************************************************
 * Tests.
 ********************************************************************************/

//void init_topology_table( void );
//void finalize_topology_table( void );
static void
test_init_and_finalize_topology_table() {
  init_topology_table();
  finalize_topology_table();
}


static void
test_finalize_topology_table_frees_sw_and_port_left() {
  init_topology_table();

  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  const uint16_t portno = 1;
  const char* portname = "Port No.1";
  port_entry* p = update_port_entry( sw, portno, portname );
  assert_true( p != NULL );
  finalize_topology_table();
}


//sw_entry *update_sw_entry( uint64_t *datapath_id );
//void delete_sw_entry( sw_entry *sw );
//sw_entry *lookup_sw_entry( uint64_t *datapath_id );
static void
test_insert_lookup_and_delete_sw_entry() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  sw_entry* sw2 = lookup_sw_entry( &dpid );
  assert_true( sw2 != NULL );
  assert_memory_equal( sw, sw2, sizeof(sw_entry) );

  delete_sw_entry( sw2 );

  sw_entry* sw3 = lookup_sw_entry( &dpid );
  assert_true( sw3 == NULL );
}

// double insert
static void
test_double_insert_returns_prev_sw_entry() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  sw_entry* sw2 = update_sw_entry( &dpid );
  assert_true( sw2 != NULL );
  assert_true( sw2->datapath_id = dpid );
  assert_true( sw == sw2 );

  delete_sw_entry( sw );
}

static void
test_sw_lookup_failure_returns_NULL() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = lookup_sw_entry( &dpid );
  assert_true( sw == NULL );
}

//void foreach_sw_entry( void function( sw_entry *entry, void *user_data ), void *user_data );
static void
helper_sw_entry_walker( sw_entry* entry, void* user ) {
  const uint64_t datapath_id = entry->datapath_id;
  check_expected( datapath_id );
  check_expected( user );
}
static void
helper_delete_sw_entry( sw_entry* entry, void* user ) {
  UNUSED( user );
  delete_sw_entry( entry );
}
static void
test_foreach_sw_entry() {
  const int NUM_SW = 10;
  const uint64_t dpid_base = 0x1234;
  uint64_t dpid;
  for( int i = 0 ; i < NUM_SW ; ++i ){
    dpid = dpid_base + ( uint64_t ) i;
    assert_true( update_sw_entry( &dpid ) != NULL );
  }

  void* user = (void*)0x5678;

  expect_value_count( helper_sw_entry_walker, user, user, NUM_SW );
  expect_in_range_count( helper_sw_entry_walker, datapath_id, dpid_base, dpid_base+( uint64_t )NUM_SW, NUM_SW);
  foreach_sw_entry( helper_sw_entry_walker, user );

  foreach_sw_entry( helper_delete_sw_entry, NULL );
}

//port_entry *update_port_entry( sw_entry *sw, uint16_t port_no, const char *name );
//void delete_port_entry( sw_entry *sw, port_entry *port );
//port_entry *lookup_port_entry_by_port( sw_entry *sw, uint16_t port_no );
//port_entry *lookup_port_entry_by_name( sw_entry *sw, const char *name );
static void
test_update_lookup_and_delete_port_entry() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  const uint16_t portno = 1;
  const char* portname = "Port No.1";
  port_entry* p = update_port_entry( sw, portno, portname );
  assert_true( p != NULL );
  assert_int_equal( p->port_no, portno );
  assert_string_equal( p->name, portname );
  assert_true( p->sw == sw );

  port_entry* p2 = lookup_port_entry_by_port( sw, portno );
  assert_true( p2 != NULL );
  assert_memory_equal( p, p2, sizeof(port_entry) );

  delete_port_entry( sw, p2 );

  port_entry* p3 = lookup_port_entry_by_name( sw, portname );
  assert_true( p3 == NULL );

  delete_sw_entry( sw );
}

static void
test_update_lookup_against_existing_port_entry_will_update_existing_port() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  const uint16_t portno = 1;
  const char* old_name = "Port No.1";
  const char* new_name = "Port No.A";
  port_entry* p = update_port_entry( sw, portno, old_name );
  assert_true( p != NULL );
  assert_int_equal( p->port_no, portno );
  assert_string_equal( p->name, old_name );
  assert_true( p->sw == sw );

  port_entry* pn = update_port_entry( sw, portno, new_name );
  assert( pn == p );
  assert_int_equal( pn->port_no, portno );
  assert_string_equal( pn->name, new_name );
  assert_true( pn->sw == sw );


  port_entry* p2 = lookup_port_entry_by_port( sw, portno );
  assert_true( p2 == pn );
  assert_memory_equal( p2, pn, sizeof(port_entry) );

  port_entry* p3 = lookup_port_entry_by_name( sw, old_name );
  assert_true( p3 == NULL );

  port_entry* p4 = lookup_port_entry_by_name( sw, new_name );
  assert_true( p4 == pn );

  delete_port_entry( sw, pn );

  port_entry* p5 = lookup_port_entry_by_name( sw, new_name );
  assert_true( p5 == NULL );

  delete_sw_entry( sw );
}

static void
test_lookup_port_entry_by_portno_only() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  port_entry* p = update_port_entry( sw, 1, "Port No.1" );
  assert_true( p != NULL );
  assert_int_equal( p->port_no, 1 );
  assert_string_equal( p->name, "Port No.1" );
  assert_true( p->sw == sw );

  port_entry* p2 = update_port_entry( sw, 2, "Port No.2" );
  assert_true( p2 != NULL );
  port_entry* p3 = update_port_entry( sw, 3, "Port No.3" );
  assert_true( p3 != NULL );

  port_entry* pl = lookup_port_entry_by_port( sw, 1 ) ;
  assert_true( pl == p );

  delete_port_entry( sw, p2 );
  delete_port_entry( sw, p3 );
  delete_port_entry( sw, p );
  delete_sw_entry( sw );
}

static void
test_lookup_port_entry_by_portname_only() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  port_entry* p = update_port_entry( sw, 1, "Port No.1" );
  assert_true( p != NULL );
  assert_int_equal( p->port_no, 1 );
  assert_string_equal( p->name, "Port No.1" );
  assert_true( p->sw == sw );
  port_entry* p2 = update_port_entry( sw, 2, "Port No.2" );
  assert_true( p2 != NULL );
  port_entry* p3 = update_port_entry( sw, 3, "Port No.3" );
  assert_true( p3 != NULL );

  port_entry* pl = lookup_port_entry_by_name( sw, "Port No.1" );
  assert_true( pl == p );

  delete_port_entry( sw, p2 );
  delete_port_entry( sw, p3 );
  delete_port_entry( sw, p );
  delete_sw_entry( sw );
}

static void
test_lookup_port_entry_by_portno_when_dup_name_exist_should_return_exact_match() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  port_entry* p1 = update_port_entry( sw, 1, "Port No.X" );
  assert_true( p1 != NULL );
  port_entry* p2 = update_port_entry( sw, 2, "Port No.X" );
  assert_true( p2 != NULL );
  assert_true( p2 != p1 );
  port_entry* p3 = update_port_entry( sw, 3, "Port No.X" );
  assert_true( p3 != NULL );
  assert_true( p3 != p1 );
  assert_true( p3 != p2 );

  port_entry* pl = lookup_port_entry_by_port( sw, 2 );
  assert_true( pl == p2 );

  delete_port_entry( sw, p1 );
  delete_port_entry( sw, p2 );
  delete_port_entry( sw, p3 );
  delete_sw_entry( sw );
}

static void
test_lookup_port_entry_by_name_when_dup_name_exist_may_return_any_match() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );
  assert_true( sw->datapath_id = dpid );

  port_entry* p1 = update_port_entry( sw, 1, "Port No.Y" );
  assert_true( p1 != NULL );

  port_entry* p2 = update_port_entry( sw, 2, "Port No.X" );
  assert_true( p2 != NULL );
  assert_true( p2 != p1 );
  port_entry* p3 = update_port_entry( sw, 3, "Port No.X" );
  assert_true( p3 != NULL );
  assert_true( p3 != p1 );
  assert_true( p3 != p2 );
  port_entry* p4 = update_port_entry( sw, 4, "Port No.X" );
  assert_true( p4 != NULL );
  assert_true( p4 != p1 );
  assert_true( p4 != p2 );
  assert_true( p4 != p3 );

  port_entry* p5 = update_port_entry( sw, 5, "Port No.Y" );
  assert_true( p5 != NULL );
  assert_true( p5 != p1 );
  assert_true( p5 != p2 );
  assert_true( p5 != p3 );
  assert_true( p5 != p4 );


  port_entry* pl = lookup_port_entry_by_name( sw, "Port No.X" );
  assert_true( (pl == p2) || (pl == p3) || (pl == p4) );

  delete_port_entry( sw, p1 );
  delete_port_entry( sw, p2 );
  delete_port_entry( sw, p3 );
  delete_port_entry( sw, p4 );
  delete_port_entry( sw, p5 );
  delete_sw_entry( sw );
}

static void
test_lookup_port_by_portno_failure_returns_NULL() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );

  port_entry* p = lookup_port_entry_by_port(sw, 42 );
  assert_true( p == NULL );

  delete_sw_entry( sw );
}


static void
test_lookup_port_by_name_failure_returns_NULL() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );

  port_entry* p = lookup_port_entry_by_name(sw, "does not exist" );
  assert_true( p == NULL );

  delete_sw_entry( sw );
}

// void foreach_port_entry( void function( port_entry *entry, void *user_data ), void *user_data );
static void
helper_port_entry_walker( port_entry* entry, void* user ) {
  const uint16_t port_no = entry->port_no;
  const char* name = entry->name;

  check_expected( port_no );
  check_expected( name );
  check_expected( user );
}

static void
test_foreach_port_entry() {
  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  assert_true( sw != NULL );

  char name[OFP_MAX_PORT_NAME_LEN] = { "Port No.X" };
  const int NUM_PORT = 100;

  for( int i = 0 ; i < NUM_PORT ; ++i ){
    snprintf(name, OFP_MAX_PORT_NAME_LEN, "Port No.%d", 1+i );
    update_port_entry( sw, ( uint16_t ) (1+i), name );
  }

  void* user = (void*)0x5678;

  expect_value_count( helper_port_entry_walker, user, user, NUM_PORT );
  expect_in_range_count( helper_port_entry_walker, port_no, 1, (unsigned) NUM_PORT, NUM_PORT);
  expect_memory_count( helper_port_entry_walker, name, "Port No.", strlen("Port No."), NUM_PORT );
  foreach_port_entry( helper_port_entry_walker, user );

  for( int i = 0 ; i < NUM_PORT ; ++i ){
    port_entry* p = lookup_port_entry_by_port( sw, ( uint16_t ) (1+i) );
    delete_port_entry( sw, p );
  }
  delete_sw_entry( sw );
}
// link_to *update_link_to( port_entry *port, uint64_t *datapath_id, uint16_t port_no, bool up );
// void delete_link_to( port_entry *port );
static void
test_update_link_to_and_delete_link_to() {
  uint64_t dpid;

  dpid = 0x1;
  sw_entry* sw1 = update_sw_entry( &dpid );
  assert_true( sw1 != NULL );
  port_entry* p1_1 = update_port_entry( sw1, 1, "SW1-1");
  assert_true( p1_1 != NULL );
  dpid = 0x2;
  sw_entry* sw2 = update_sw_entry( &dpid );
  assert_true( sw2 != NULL );
  assert_true( sw2 != sw1 );
  port_entry* p2_1 = update_port_entry( sw2, 1, "SW2-1");
  assert_true( p2_1 != NULL );
  dpid = 0x3;
  sw_entry* sw3 = update_sw_entry( &dpid );
  assert_true( sw3 != NULL );
  port_entry* p3_1 = update_port_entry( sw3, 1, "SW3-1");
  assert_true( p3_1 != NULL );

  dpid = 0x2;
  link_to* l = update_link_to( p1_1, &dpid, 1, true );
  assert_true( l != NULL );
  assert_int_equal( l->datapath_id, 0x2 );
  assert_int_equal( l->port_no, 1);
  assert_true( l->up );

  dpid = 0x3;
  link_to* l2 = update_link_to( p1_1, &dpid, 1, true );
  assert_true( l2 != NULL );
  assert_int_equal( l2->datapath_id, 0x3 );
  assert_int_equal( l2->port_no, 1);
  assert_true( l2->up );

  delete_link_to( p1_1 );

  delete_port_entry( sw1, p1_1 );
  delete_sw_entry( sw1 );

  delete_port_entry( sw2, p2_1 );
  delete_sw_entry( sw2 );

  delete_port_entry( sw3, p3_1 );
  delete_sw_entry( sw3 );
}



/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test( test_init_and_finalize_topology_table ),
      unit_test( test_finalize_topology_table_frees_sw_and_port_left ),
      unit_test_setup_teardown( test_insert_lookup_and_delete_sw_entry, setup, teardown ),
      unit_test_setup_teardown( test_double_insert_returns_prev_sw_entry, setup, teardown ),
      unit_test_setup_teardown( test_sw_lookup_failure_returns_NULL, setup, teardown ),
      unit_test_setup_teardown( test_foreach_sw_entry, setup, teardown ),

      unit_test_setup_teardown( test_update_lookup_and_delete_port_entry, setup, teardown ),
      unit_test_setup_teardown( test_update_lookup_against_existing_port_entry_will_update_existing_port, setup, teardown ),
      unit_test_setup_teardown( test_lookup_port_entry_by_portno_only, setup, teardown ),
      unit_test_setup_teardown( test_lookup_port_entry_by_portname_only, setup, teardown ),
      unit_test_setup_teardown( test_lookup_port_entry_by_portno_when_dup_name_exist_should_return_exact_match, setup, teardown ),
      unit_test_setup_teardown( test_lookup_port_entry_by_name_when_dup_name_exist_may_return_any_match, setup, teardown ),
      unit_test_setup_teardown( test_lookup_port_by_portno_failure_returns_NULL, setup, teardown ),
      unit_test_setup_teardown( test_lookup_port_by_name_failure_returns_NULL, setup, teardown ),
      unit_test_setup_teardown( test_foreach_port_entry, setup, teardown ),
      unit_test_setup_teardown( test_update_link_to_and_delete_link_to, setup, teardown ),
  };
  setup_leak_detector();
  return run_tests( tests );
}

