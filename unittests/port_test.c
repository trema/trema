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
#include "port.h"


#define VOID_FUNCTION (0)

// static functions
void delete_outbound_switch( list_element **switches, switch_info *delete_switch );


/**********************************************************************
 * Helper functions.                                                  
 **********************************************************************/

static uint64_t dpid_switch_A = 0x12345678ULL;
static uint16_t port_no_a = 0x33;
static uint16_t port_no_aa = 0x34;
static uint64_t dpid_switch_B = 0x9abcdef0ULL;
static uint16_t port_no_b = 0x44;
static uint16_t port_no_bb = 0x45;
static uint16_t port_no_bbb = 0x46;
static uint64_t dpid_switch_C = 0xcafebabeULL;
static uint16_t port_no_c = 0xdead;
static buffer dummy_packet;
static openflow_actions_t dummy_actions;


static switch_info *
helper_alloc_switch( uint64_t dpid ) {
  switch_info *sw = xmalloc( sizeof( switch_info ) );
  sw->dpid = dpid;
  create_list( &sw->ports );

  return sw;
}


static switch_info *
helper_lookup_switch( list_element *switches, uint64_t dpid ) {
  for ( list_element *e = switches; e != NULL; e = e->next ) {
    switch_info *sw = e->data;
    if ( sw->dpid == dpid ) {
      return sw;
    }
  }

  return NULL;
}


static port_info *
helper_lookup_port( list_element *switches, uint64_t dpid, uint16_t port_no ) {
  switch_info *sw = helper_lookup_switch( switches, dpid );
  if ( sw == NULL ) {
    return NULL;
  }
  
  for ( list_element *e = sw->ports; e != NULL; e = e->next ) {
    port_info *p = e->data;
    if ( p->dpid == dpid && p->port_no == port_no ) {
      return p;
    }
  }

  return NULL;
}


static void
helper_add_outbound_port( list_element **switches, uint64_t dpid, uint16_t port_no ) {
  // lookup switch
  switch_info *sw = helper_lookup_switch( *switches, dpid );
  if ( sw == NULL ) {
    sw = helper_alloc_switch( dpid );
    append_to_tail( switches, sw );
  }

  port_info *new_port = xmalloc( sizeof( port_info ) );
  new_port->dpid = dpid;
  new_port->port_no = port_no;
  append_to_tail( &sw->ports, new_port );
}


/*************************************************************************
 * Setup and teardown function.                                          
 *************************************************************************/

static void
setup( list_element **switches ) {
  init_log( "port_test", false );

  create_list( switches );
}


static void
setup_test_ports_data( list_element **switches ) {
  assert( switches != NULL );

  helper_add_outbound_port( switches, dpid_switch_A, port_no_a );
  helper_add_outbound_port( switches, dpid_switch_A, port_no_aa );
  helper_add_outbound_port( switches, dpid_switch_B, port_no_b );
  helper_add_outbound_port( switches, dpid_switch_B, port_no_bb );
  helper_add_outbound_port( switches, dpid_switch_B, port_no_bbb );
}


static void
teardown( list_element *switches ) {
  for ( list_element *e = switches; e != NULL; e = e->next ) {
    switch_info *sw = e->data;
    for ( list_element *ee = sw->ports; ee != NULL; ee = ee->next ) {
      xfree( ee->data );
    }
    delete_list( sw->ports );
    xfree( sw );
  }
  delete_list( switches );
}


/*************************************************************************
 * Mock                                                                  
 *************************************************************************/

void
mock_die( char *format, ... ) {
  UNUSED( format );
}


static void
mock_foreach_switch_callback( switch_info *sw, buffer *packet, uint64_t dpid64, uint16_t in_port16 ) {
  check_expected( sw );
  check_expected( packet );
  uint64_t *dpid = &dpid64;
  check_expected( dpid );
  uint32_t in_port = in_port16;
  check_expected( in_port );

  ( void ) mock();
}


static int
mock_foreach_port_callback( port_info *port, openflow_actions_t *actions,
                            uint64_t dpid64, uint16_t in_port16 ) {
  check_expected( port );
  check_expected( actions );
  uint64_t *dpid = &dpid64;
  check_expected( dpid );
  uint32_t in_port = in_port16;
  check_expected( in_port );

  return ( int ) mock();
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
test_create_outbound_ports() {
  list_element *switches = ( list_element * ) &switches; // fill non-NULL value

  // target
  create_outbound_ports( &switches );

  assert_true( switches == NULL );
}


static void
test_add_outbound_port_new_port() {
  list_element *switches;

  setup( &switches );

  // target
  add_outbound_port( &switches, dpid_switch_A, port_no_a );

  // check
  assert_true( switches != NULL );
  switch_info *sw = switches->data;
  list_element *ports = sw->ports;
  port_info *p = ports->data;
  assert_true( p != NULL );
  assert_true( p->dpid == dpid_switch_A );
  assert_true( p->port_no == port_no_a );

  // target
  add_outbound_port( &switches, dpid_switch_B, port_no_b );

  // check
  list_element *next_sw = switches->next; // next switch
  sw = next_sw->data;
  assert_true( next_sw->next == NULL );
  assert_true( sw->ports != NULL );
  p = sw->ports->data;
  assert_true( p != NULL );
  assert_true( p->dpid == dpid_switch_B );
  assert_true( p->port_no == port_no_b );

  teardown( switches );
}


static void
test_lookup_outbound_port_for_null_ports() {
  list_element *switches = NULL;

  // target
  port_info *result = lookup_outbound_port( switches, dpid_switch_A, port_no_a );
  assert_true( result == NULL );

  result = lookup_outbound_port( switches, dpid_switch_B, port_no_b );
  assert_true( result == NULL );

  result = lookup_outbound_port( switches, dpid_switch_A, port_no_b ); // not found
  assert_true( result == NULL );

  result = lookup_outbound_port( switches, dpid_switch_B, port_no_a ); // not found
  assert_true( result == NULL );

  result = lookup_outbound_port( switches, dpid_switch_C, port_no_c ); // not found
  assert_true( result == NULL );

  teardown( switches );
}


static void
test_lookup_outbound_port() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  // target
  port_info *result = lookup_outbound_port( switches, dpid_switch_A, port_no_a );
  assert_true( result != NULL );
  assert_true( result->dpid == dpid_switch_A );
  assert_true( result->port_no == port_no_a );

  result = lookup_outbound_port( switches, dpid_switch_B, port_no_b );
  assert_true( result != NULL );
  assert_true( result->dpid == dpid_switch_B );
  assert_true( result->port_no == port_no_b );

  result = lookup_outbound_port( switches, dpid_switch_A, port_no_b ); // not found
  assert_true( result == NULL );

  result = lookup_outbound_port( switches, dpid_switch_B, port_no_a ); // not found
  assert_true( result == NULL );

  result = lookup_outbound_port( switches, dpid_switch_C, port_no_c ); // not found
  assert_true( result == NULL );

  teardown( switches );
}


static void
test_delete_outbound_port() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  port_info *delete_me = helper_lookup_port( switches, dpid_switch_A, port_no_a );

  // target
  delete_outbound_port( &switches, delete_me );

  // check
  assert_true( helper_lookup_port( switches, dpid_switch_A, port_no_a ) == NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_B, port_no_b ) != NULL );

  teardown( switches );
}


static void
test_delete_outbound_port_for_unknown_dpid() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  port_info delete_me_but_unknown;
  delete_me_but_unknown.dpid = dpid_switch_C; // not registered

  // target
  delete_outbound_port( &switches, &delete_me_but_unknown );

  // check
  assert_true( helper_lookup_port( switches, dpid_switch_A, port_no_a ) != NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_B, port_no_b ) != NULL );

  teardown( switches );
}


static void
test_delete_outbound_port_for_last_one() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  // lookup target
  port_info *victim1 = helper_lookup_port( switches, dpid_switch_A, port_no_a );
  assert_true( victim1 != NULL );
  port_info *victim2 = helper_lookup_port( switches, dpid_switch_A, port_no_aa );
  assert_true( victim2 != NULL );

  // target
  delete_outbound_port( &switches, victim1 );
  delete_outbound_port( &switches, victim2 );

  // check
  assert_true( helper_lookup_switch( switches, dpid_switch_A ) == NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_A, port_no_a ) == NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_A, port_no_aa ) == NULL );
  assert_true( helper_lookup_switch( switches, dpid_switch_B ) != NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_B, port_no_b ) != NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_B, port_no_bb ) != NULL );
  assert_true( helper_lookup_port( switches, dpid_switch_B, port_no_bbb ) != NULL );

  teardown( switches );
}


static void
test_delete_outbound_all_ports() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  // target
  delete_outbound_all_ports( &switches );

  assert_true( switches == NULL );
}


static void
test_delete_outbound_switch_success() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  // target switch
  switch_info *delete_me = switches->data;

  // target
  delete_outbound_switch( &switches, delete_me );

  assert_true( switches != NULL );
  assert_true( switches->data != NULL );
  assert_true( switches->next == NULL );

  // next switch
  delete_me = switches->data;

  delete_outbound_switch( &switches, delete_me );
  assert_true( switches == NULL );
}


static void
test_foreach_switch_success() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  // mock_foreach_switch_callback()
  expect_value( mock_foreach_switch_callback, sw, switches->data );
  expect_value( mock_foreach_switch_callback, packet, &dummy_packet );
  expect_memory( mock_foreach_switch_callback, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_foreach_switch_callback, in_port, ( uint32_t ) port_no_c );
  will_return( mock_foreach_switch_callback, VOID_FUNCTION );

  // mock_foreach_switch_callback()
  expect_value( mock_foreach_switch_callback, sw, switches->next->data );
  expect_value( mock_foreach_switch_callback, packet, &dummy_packet );
  expect_memory( mock_foreach_switch_callback, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_foreach_switch_callback, in_port, ( uint32_t ) port_no_c );
  will_return( mock_foreach_switch_callback, VOID_FUNCTION );

  // target
  foreach_switch( switches, mock_foreach_switch_callback, &dummy_packet, dpid_switch_C, port_no_c );

  teardown( switches );
}


static void
test_foreach_port_success() {
  list_element *switches;

  setup( &switches );
  setup_test_ports_data( &switches );

  // mock_foreach_port_callback()
  switch_info *sw = switches->data; // switch A
  port_info *p = sw->ports->data;
  expect_value( mock_foreach_port_callback, port, p );
  expect_value( mock_foreach_port_callback, actions, &dummy_actions );
  expect_memory( mock_foreach_port_callback, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_foreach_port_callback, in_port, ( uint32_t ) port_no_c );
  will_return( mock_foreach_port_callback, 1 );

  // mock_foreach_port_callback()
  p = sw->ports->next->data;
  expect_value( mock_foreach_port_callback, port, p );
  expect_value( mock_foreach_port_callback, actions, &dummy_actions );
  expect_memory( mock_foreach_port_callback, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_foreach_port_callback, in_port, ( uint32_t ) port_no_c );
  will_return( mock_foreach_port_callback, 2 );

  // target
  int r = foreach_port( sw->ports, mock_foreach_port_callback, &dummy_actions, dpid_switch_C, port_no_c );

  assert_true( r == ( 1 + 2 ) );

  teardown( switches );
}


int
main() {
  const UnitTest tests[] = {
    unit_test( test_mock_die ),

    unit_test( test_create_outbound_ports ),
    unit_test( test_add_outbound_port_new_port ),
    unit_test( test_lookup_outbound_port_for_null_ports ),
    unit_test( test_lookup_outbound_port ),
    unit_test( test_delete_outbound_port ),
    unit_test( test_delete_outbound_port_for_unknown_dpid ),
    unit_test( test_delete_outbound_port_for_last_one ),
    unit_test( test_delete_outbound_all_ports ),
    unit_test( test_delete_outbound_switch_success ),
    unit_test( test_foreach_switch_success ),
    unit_test( test_foreach_port_success ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

