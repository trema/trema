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
#include <unistd.h>

#include "checks.h"
#include "cmockery_trema.h"
#include "trema.h"

#include "timer.h"
#include "probe_timer_table.h"

#include "discovery_management.h"
#include "service_management.h"


/********************************************************************************
 * Common function.
 ********************************************************************************/

extern dlist_element *probe_timer_table;
extern dlist_element *probe_timer_last;


static bool
is_probe_timer_table_sorted_in_increasing_order() {
  dlist_element *dlist;
  probe_timer_entry *entry_prev = NULL;
  probe_timer_entry *entry = NULL;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = dlist->next ) {
    entry = dlist->data;
    if ( entry_prev == NULL ) {
      entry_prev = entry;
      continue;
    }

    if ( entry_prev->expires.tv_sec > entry->expires.tv_sec ) {
      return false;
    }
    if ( entry_prev->expires.tv_sec == entry->expires.tv_sec &&
         entry_prev->expires.tv_nsec > entry->expires.tv_nsec ){
      return false;
    }
    entry_prev = entry;
  }
  return true;
}


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;

static bool ( *original_add_timer_event_callback )( struct itimerspec *interval, timer_callback callback, void *user_data );
static bool ( *original_delete_timer_event )( timer_callback callback, void *user_data );

static void ( *original_execute_timer_events )( int *next_timeout_usec );

static bool ( *original_send_probe )( const uint8_t *mac, uint64_t dpid, uint16_t port_no );

static uint8_t ( *original_set_discovered_link_status )( topology_update_link_status* link_status );

static void ( *original_warn )( const char *format, ... );


static bool
mock_add_timer_event_callback( struct itimerspec *interval, timer_callback callback, void *user_data ) {
  check_expected( interval );
  check_expected( callback );
  check_expected( user_data );
  return (bool)mock();
}


static bool
mock_delete_timer_event( timer_callback callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );
  return (bool)mock();
}


static void
mock_execute_timer_events( int *next_timeout_usec ) {
  UNUSED( next_timeout_usec );
  // Do nothing.
}


static bool
mock_send_probe( const uint8_t *mac, uint64_t dpid, uint16_t port_no ) {
  check_expected( mac );
  check_expected( dpid );
  check_expected( port_no );

  return (bool) mock();
}


static uint8_t
mock_set_discovered_link_status( topology_update_link_status* link_status ) {
  const uint64_t from_dpid = link_status->from_dpid;
  check_expected( from_dpid );

  const uint64_t to_dpid = link_status->to_dpid;
  check_expected( to_dpid );

  const uint16_t from_portno = link_status->from_portno;
  check_expected( from_portno );

  const uint16_t to_portno = link_status->to_portno;
  check_expected( to_portno );

  const uint8_t status = link_status->status;
  check_expected( status );

  return (uint8_t) mock();
}


static bool check_warn = false;
static void
mock_warn_check( const char *format, va_list args ) {
  char message[ 1000 ];
  vsnprintf( message, 1000, format, args );

  check_expected( message );
}


static void
mock_warn( const char *format, ... ) {
  if( check_warn ) {
    va_list arg;
    va_start( arg, format );
    mock_warn_check( format, arg );
    va_end( arg );
  }
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/


static void
setup() {
  init_timer();
  init_probe_timer_table();
}


static void
teardown() {
  finalize_probe_timer_table();
  finalize_timer();
}


static void
setup_timer_event_off() {
  swap_original( execute_timer_events );
  swap_original( send_probe );
  swap_original( set_discovered_link_status );
  swap_original( warn );
  check_warn = false;

  init_timer();
  init_probe_timer_table();
}


static void
teardown_timer_event_off() {
  finalize_probe_timer_table();
  finalize_timer();
  revert_original( execute_timer_events );
  revert_original( send_probe );
  revert_original( set_discovered_link_status );
  revert_original( warn );
}


/********************************************************************************
 * Tests.
 ********************************************************************************/


//void init_probe_timer_table( void );
//void finalize_probe_timer_table( void );
static void
test_init_and_finalize_probe_timer_table() {
  swap_original( add_timer_event_callback );
  swap_original( delete_timer_event );

  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 500000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;

  expect_memory( mock_add_timer_event_callback, interval, &interval, sizeof(interval) );
  expect_not_value( mock_add_timer_event_callback, callback, NULL );
  expect_value( mock_add_timer_event_callback, user_data, NULL );
  will_return( mock_add_timer_event_callback, true );
  init_probe_timer_table();

  expect_not_value( mock_delete_timer_event, callback, NULL );
  expect_value( mock_delete_timer_event, user_data, NULL );
  will_return( mock_delete_timer_event, true );
  finalize_probe_timer_table();

  revert_original( delete_timer_event );
  revert_original( add_timer_event_callback );
}


static void
test_init_and_finalize_probe_timer_table_should_free_left_over_entry() {
  swap_original( add_timer_event_callback );
  swap_original( delete_timer_event );

  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 500000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;

  expect_memory( mock_add_timer_event_callback, interval, &interval, sizeof(interval) );
  expect_not_value( mock_add_timer_event_callback, callback, NULL );
  expect_value( mock_add_timer_event_callback, user_data, NULL );
  will_return( mock_add_timer_event_callback, true );
  init_probe_timer_table();

  uint64_t dpid1 = 0x1;
  uint8_t mac1[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e1 = allocate_probe_timer_entry( &dpid1, 1, mac1 );
  insert_probe_timer_entry( e1 );

  expect_not_value( mock_delete_timer_event, callback, NULL );
  expect_value( mock_delete_timer_event, user_data, NULL );
  will_return( mock_delete_timer_event, true );
  finalize_probe_timer_table();

  revert_original( delete_timer_event );
  revert_original( add_timer_event_callback );
}


//probe_timer_entry *allocate_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no, const uint8_t *mac );
//void free_probe_timer_entry( probe_timer_entry *free_entry );
static void
test_allocate_and_free_probe_timer_entry() {
  uint64_t dpid = 0x1;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x02,0x03,0x04,0x05,0x06 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );
  assert_true( e != NULL );
  assert_int_equal( e->datapath_id, dpid );
  assert_int_equal( e->port_no, 1 );
  assert_memory_equal( e->mac, mac, ETH_ADDRLEN );
  assert_int_equal( e->retry_count, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );
  assert_false( e->link_up );
  assert_false( e->dirty );

  free_probe_timer_entry( e );
}


// void insert_probe_timer_entry( probe_timer_entry *entry );
// probe_timer_entry *delete_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no );
// probe_timer_entry *lookup_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no );
static void
test_probe_timer_entry_manipulation() {
  uint64_t dpid1 = 0x1;
  uint8_t mac1[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e1 = allocate_probe_timer_entry( &dpid1, 1, mac1 );
  e1->expires.tv_sec = 1;
  e1->expires.tv_nsec = 100;

  uint64_t dpid2 = 0x2;
  uint8_t mac2[ETH_ADDRLEN] = { 0x02,0x02,0x02,0x02,0x02,0x02 };
  probe_timer_entry* e2 = allocate_probe_timer_entry( &dpid2, 2, mac2 );
  e2->expires.tv_sec = 2;
  e2->expires.tv_nsec = 20;

  uint64_t dpid3 = 0x3;
  uint8_t mac3[ETH_ADDRLEN] = { 0x03,0x03,0x03,0x03,0x03,0x03 };
  probe_timer_entry* e3 = allocate_probe_timer_entry( &dpid3, 3, mac3 );
  e3->expires.tv_sec = 2;
  e3->expires.tv_nsec = 300;

  uint64_t dpid4 = 0x4;
  uint8_t mac4[ETH_ADDRLEN] = { 0x04,0x04,0x04,0x04,0x04,0x04 };
  probe_timer_entry* e4 = allocate_probe_timer_entry( &dpid4, 4, mac4 );
  e4->expires.tv_sec = 2;
  e4->expires.tv_nsec = 4;

  uint64_t dpid5 = 0x5;
  uint8_t mac5[ETH_ADDRLEN] = { 0x05,0x05,0x05,0x05,0x05,0x05 };
  probe_timer_entry* e5 = allocate_probe_timer_entry( &dpid5, 5, mac5 );
  e5->expires.tv_sec = 2;
  e5->expires.tv_nsec = 4;

  // expire order
  //  e1, e4, e5, e2, e3


  // [e2]
  insert_probe_timer_entry( e2 );
  assert_true( probe_timer_table->next->data == e2 );
  assert_true( probe_timer_last->data == e2 );
  probe_timer_entry* le2 = lookup_probe_timer_entry( &dpid2, 2 );
  assert_true( le2 == e2 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e2, e3]
  insert_probe_timer_entry( e3 );
  assert_true( probe_timer_table->next->data == e2 );
  assert_true( probe_timer_last->data == e3 );
  probe_timer_entry* le3 = lookup_probe_timer_entry( &dpid3, 3 );
  assert_true( le3 == e3 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e4, e2, e3]
  insert_probe_timer_entry( e4 );
  assert_true( probe_timer_table->next->data == e4 );
  assert_true( probe_timer_last->data == e3 );
  probe_timer_entry* le4 = lookup_probe_timer_entry( &dpid4, 4 );
  assert_true( le4 == e4 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e4, e5, e2, e3]
  insert_probe_timer_entry( e5 );
  assert_true( probe_timer_table->next->data == e4 );
  assert_true( probe_timer_last->data == e3 );
  probe_timer_entry* le5 = lookup_probe_timer_entry( &dpid5, 5 );
  assert_true( le5 == e5 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e1, e4, e5, e2, e3]
  insert_probe_timer_entry( e1 );
  assert_true( probe_timer_table->next->data == e1 );
  assert_true( probe_timer_last->data == e3 );
  probe_timer_entry* le1 = lookup_probe_timer_entry( &dpid1, 1 );
  assert_true( le1 == e1 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  uint64_t dpidE = 0xE;
  assert_true( lookup_probe_timer_entry( &dpidE, 5 ) == NULL );
  assert_true( delete_probe_timer_entry( &dpidE, 5 ) == NULL );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e1, e4, e5, e3]
  probe_timer_entry* de2 = delete_probe_timer_entry( &dpid2, 2 );
  assert_true( probe_timer_table->next->data == e1 );
  assert_true( probe_timer_last->data == e3 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e4, e5, e3]
  probe_timer_entry* de1 = delete_probe_timer_entry( &dpid1, 1 );
  assert_true( probe_timer_table->next->data == e4 );
  assert_true( probe_timer_last->data == e3 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e5, e3]
  probe_timer_entry* de4 = delete_probe_timer_entry( &dpid4, 4 );
  assert_true( probe_timer_table->next->data == e5 );
  assert_true( probe_timer_last->data == e3 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // [e3]
  probe_timer_entry* de5 = delete_probe_timer_entry( &dpid5, 5 );
  assert_true( probe_timer_table->next->data == e3 );
  assert_true( probe_timer_last->data == e3 );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  // []
  probe_timer_entry* de3 = delete_probe_timer_entry( &dpid3, 3 );
  assert_true( probe_timer_table->next == NULL );
  assert_true( probe_timer_last->data == NULL );
  assert_true( is_probe_timer_table_sorted_in_increasing_order() );

  free_probe_timer_entry( de1 );
  free_probe_timer_entry( de2 );
  free_probe_timer_entry( de3 );
  free_probe_timer_entry( de4 );
  free_probe_timer_entry( de5 );
}


//void probe_request( probe_timer_entry *entry, int event, uint64_t *dpid, uint16_t port_no );
static void
test_probe_request_inactive_event_up_then_send_delay() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );

  probe_request( e, PROBE_TIMER_EVENT_UP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );

  // cleanup
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );
  free_probe_timer_entry( e );
}


static void
test_probe_request_inactive_event_other_then_inactive() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );

  probe_request( e, PROBE_TIMER_EVENT_DOWN, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) == NULL );

  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) == NULL );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) == NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_send_delay_event_down_then_inactive() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_SEND_DELAY;
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );

  probe_request( e, PROBE_TIMER_EVENT_DOWN, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );

  // cleanup
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) == NULL );
  free_probe_timer_entry( e );
}


static void
test_probe_request_send_delay_event_timeout_then_wait_and_sendlldp() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  // try to pass set_expires's nsec carry up case
  e->expires.tv_nsec = 1000 * 1000 * 1000 - 1;

  e->state = PROBE_TIMER_STATE_SEND_DELAY;
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );

  // detect lldp
  expect_memory( mock_send_probe, mac, mac, ETH_ADDRLEN );
  expect_value( mock_send_probe, dpid, 0x1234 );
  expect_value( mock_send_probe, port_no, 1 );
  will_return( mock_send_probe, true );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  // cleanup
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );
  free_probe_timer_entry( e );
}


static void
test_probe_request_send_delay_event_timeout_then_wait_and_sendlldp_fail() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_SEND_DELAY;
  e->retry_count = 0;
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );

  // detect lldp
  expect_memory( mock_send_probe, mac, mac, ETH_ADDRLEN );
  expect_value( mock_send_probe, dpid, 0x1234 );
  expect_value( mock_send_probe, port_no, 1 );
  will_return( mock_send_probe, false );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );
  assert_int_equal( e->retry_count, 3 );

  // cleanup
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );
  free_probe_timer_entry( e );
}


static void
test_probe_request_send_delay_event_other_then_send_delay() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_SEND_DELAY;
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );

  probe_request( e, PROBE_TIMER_EVENT_UP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_down_then_inactive() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_WAIT;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  probe_request( e, PROBE_TIMER_EVENT_DOWN, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) == NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_recv_lldp_then_confirmed_and_set_link() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 42, mac );

  e->state = PROBE_TIMER_STATE_WAIT;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UP );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_OK );

  uint64_t dst_dpid = 0x5678;
  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, &dst_dpid, 72 );

  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_int_equal( e->to_datapath_id, 0x5678 );
  assert_int_equal( e->to_port_no, 72 );
  assert_true( e->link_up );
  assert_false( e->dirty );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_recv_lldp_then_confirmed_and_set_link_failed() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 42, mac );

  e->state = PROBE_TIMER_STATE_WAIT;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UP );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_INVALID );

  uint64_t dst_dpid = 0x5678;
  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, &dst_dpid, 72 );

  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_int_equal( e->to_datapath_id, 0x5678 );
  assert_int_equal( e->to_port_no, 72 );
  assert_true( e->link_up );
  assert_false( e->dirty );
  assert_int_equal( e->retry_count, 24 );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_timeout_then_wait_and_send_lldp_if_retry_positive() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->retry_count = 2;
  e->state = PROBE_TIMER_STATE_WAIT;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  expect_memory( mock_send_probe, mac, mac, ETH_ADDRLEN );
  expect_value( mock_send_probe, dpid, 0x1234 );
  expect_value( mock_send_probe, port_no, 1 );
  will_return( mock_send_probe, true );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );
  assert_true( e->retry_count > 0 );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_timeout_then_wait_and_send_lldp_fail_if_retry_positive() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->retry_count = 2;
  e->state = PROBE_TIMER_STATE_WAIT;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  expect_memory( mock_send_probe, mac, mac, ETH_ADDRLEN );
  expect_value( mock_send_probe, dpid, 0x1234 );
  expect_value( mock_send_probe, port_no, 1 );
  will_return( mock_send_probe, false );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );
  assert_true( e->retry_count == 2 );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_timeout_then_confirm_and_set_link_if_retry_zero() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 42, mac );

  e->retry_count = 1;
  e->state = PROBE_TIMER_STATE_WAIT;
  e->link_up =true;
  e->to_datapath_id = 0x5678;
  e->to_port_no = 72;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UNSTABLE );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_OK );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( e->retry_count > 0 );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_timeout_then_confirm_and_set_link_fail_if_retry_zero() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 42, mac );

  e->retry_count = 1;
  e->state = PROBE_TIMER_STATE_WAIT;
  e->link_up = false;
  e->to_datapath_id = 0x5678;
  e->to_port_no = 72;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_DOWN );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_INVALID );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( e->retry_count > 0 );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_wait_event_up_then_wait() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_WAIT;
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );

  probe_request( e, PROBE_TIMER_EVENT_UP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_down_then_inactive() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  probe_request( e, PROBE_TIMER_EVENT_DOWN, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_INACTIVE );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) == NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_timeout_then_confirmed_if_retry_positive() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  e->retry_count = 2;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_timeout_then_send_delay_if_retry_zero() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  e->retry_count = 1;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_timeout_then_send_delay_if_dirty() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  e->retry_count = 2;
  e->dirty = true;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  probe_request( e, PROBE_TIMER_EVENT_TIMEOUT, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_recv_lldp_then_confirmed_if_linkup() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  e->link_up = true;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_recv_lldp_then_confirmed_and_set_link_if_not_linkup() {
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 42, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  e->link_up = false;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UNSTABLE );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_OK );

  uint64_t dst_dpid = 0x5678;
  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, &dst_dpid, 72 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( e->dirty );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_recv_lldp_then_confirmed_and_set_link_but_failed_if_not_linkup() {

  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 42, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  e->link_up = false;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UNSTABLE );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_INVALID );

  check_warn = true;
  expect_string( mock_warn_check, message, "Failed to set (0x1234,42)->(0x5678,72) status to TD_LINK_UNSTABLE." );

  uint64_t dst_dpid = 0x5678;
  probe_request( e, PROBE_TIMER_EVENT_RECV_LLDP, &dst_dpid, 72 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( e->dirty );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_probe_request_confirmed_event_up_then_wait() {
  // other case
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );

  e->state = PROBE_TIMER_STATE_CONFIRMED;
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );

  probe_request( e, PROBE_TIMER_EVENT_UP, NULL, 0 );
  assert_int_equal( e->state, PROBE_TIMER_STATE_CONFIRMED );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_interval_timerevent() {
  // base: test_probe_request_send_delay_event_timeout_then_wait_and_sendlldp
  uint64_t dpid = 0x1234;
  uint8_t mac[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e = allocate_probe_timer_entry( &dpid, 1, mac );
  insert_probe_timer_entry( e );

  e->state = PROBE_TIMER_STATE_SEND_DELAY;
  e->expires.tv_sec = 1;
  e->expires.tv_nsec = 1;

  // detect lldp
  expect_memory( mock_send_probe, mac, mac, ETH_ADDRLEN );
  expect_value( mock_send_probe, dpid, 0x1234 );
  expect_value( mock_send_probe, port_no, 1 );
  will_return( mock_send_probe, true );

  usleep(500*1000);
  // manually kick timer event.
  int next = 1;
  original_execute_timer_events( &next );

  assert_int_equal( e->state, PROBE_TIMER_STATE_WAIT );
  assert_true( delete_probe_timer_entry( &e->datapath_id, e->port_no ) != NULL );

  // cleanup
  free_probe_timer_entry( e );
}


static void
test_transition_to_inactive_marks_peer_dirty() {
  uint64_t dpid1 = 0x1234;
  uint8_t mac1[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e1 = allocate_probe_timer_entry( &dpid1, 1, mac1 );
  e1->state = PROBE_TIMER_STATE_SEND_DELAY;
  e1->to_datapath_id = 0x5678;
  e1->to_port_no = 2;

  uint64_t dpid2 = 0x5678;
  uint8_t mac2[ETH_ADDRLEN] = { 0x02,0x02,0x02,0x02,0x02,0x02 };
  probe_timer_entry* e2 = allocate_probe_timer_entry( &dpid2, 2, mac2 );
  e2->link_up = true;
  insert_probe_timer_entry( e2 );

  probe_request( e1, PROBE_TIMER_EVENT_DOWN, NULL, 0 );
  assert_int_equal( e1->state, PROBE_TIMER_STATE_INACTIVE );

  assert_true( e2->dirty );

  // cleanup
  assert_true( delete_probe_timer_entry( &e1->datapath_id, e1->port_no ) == NULL );
  assert_true( delete_probe_timer_entry( &e2->datapath_id, e2->port_no ) == e2 );
  free_probe_timer_entry( e1 );
  free_probe_timer_entry( e2 );
}


static void
test_linkstate_change_marks_peer_dirty() {
  uint64_t dpid1 = 0x1234;
  uint8_t mac1[ETH_ADDRLEN] = { 0x01,0x01,0x01,0x01,0x01,0x01 };
  probe_timer_entry* e1 = allocate_probe_timer_entry( &dpid1, 42, mac1 );
  e1->state = PROBE_TIMER_STATE_WAIT;
  e1->to_datapath_id = 0x5678;
  e1->to_port_no = 72;

  uint64_t dpid2 = 0x5678;
  uint8_t mac2[ETH_ADDRLEN] = { 0x02,0x02,0x02,0x02,0x02,0x02 };
  probe_timer_entry* e2 = allocate_probe_timer_entry( &dpid2, 72, mac2 );
  e2->link_up = false;
  insert_probe_timer_entry( e2 );

  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UP );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_OK );

  probe_request( e1, PROBE_TIMER_EVENT_RECV_LLDP, &dpid2, 72 );
  assert_int_equal( e1->state, PROBE_TIMER_STATE_CONFIRMED );

  assert_true( e2->dirty );

  // cleanup
  assert_true( delete_probe_timer_entry( &e1->datapath_id, e1->port_no ) == e1 );
  assert_true( delete_probe_timer_entry( &e2->datapath_id, e2->port_no ) == e2 );
  free_probe_timer_entry( e1 );
  free_probe_timer_entry( e2 );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/


int
main() {
  const UnitTest tests[] = {
      unit_test( test_init_and_finalize_probe_timer_table ),
      unit_test( test_init_and_finalize_probe_timer_table_should_free_left_over_entry ),

      unit_test_setup_teardown( test_allocate_and_free_probe_timer_entry, setup, teardown ),
      unit_test_setup_teardown( test_probe_timer_entry_manipulation, setup, teardown ),

      // probe request state transition
      unit_test_setup_teardown( test_probe_request_inactive_event_up_then_send_delay, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_inactive_event_other_then_inactive, setup_timer_event_off, teardown_timer_event_off ),

      unit_test_setup_teardown( test_probe_request_send_delay_event_down_then_inactive, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_send_delay_event_timeout_then_wait_and_sendlldp, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_send_delay_event_timeout_then_wait_and_sendlldp_fail, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_send_delay_event_other_then_send_delay, setup_timer_event_off, teardown_timer_event_off ),

      unit_test_setup_teardown( test_probe_request_wait_event_recv_lldp_then_confirmed_and_set_link, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_recv_lldp_then_confirmed_and_set_link_failed, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_down_then_inactive, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_timeout_then_wait_and_send_lldp_if_retry_positive, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_timeout_then_wait_and_send_lldp_fail_if_retry_positive, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_timeout_then_confirm_and_set_link_if_retry_zero, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_timeout_then_confirm_and_set_link_fail_if_retry_zero, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_wait_event_up_then_wait, setup_timer_event_off, teardown_timer_event_off ),

      unit_test_setup_teardown( test_probe_request_confirmed_event_down_then_inactive, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_timeout_then_confirmed_if_retry_positive, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_timeout_then_send_delay_if_retry_zero, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_timeout_then_send_delay_if_dirty, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_recv_lldp_then_confirmed_if_linkup, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_recv_lldp_then_confirmed_and_set_link_if_not_linkup, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_recv_lldp_then_confirmed_and_set_link_but_failed_if_not_linkup, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_probe_request_confirmed_event_up_then_wait, setup_timer_event_off, teardown_timer_event_off ),

      unit_test_setup_teardown( test_transition_to_inactive_marks_peer_dirty, setup_timer_event_off, teardown_timer_event_off ),
      unit_test_setup_teardown( test_linkstate_change_marks_peer_dirty, setup_timer_event_off, teardown_timer_event_off ),

      // test interval timer call
      unit_test_setup_teardown( test_interval_timerevent, setup_timer_event_off, teardown_timer_event_off ),
  };

  setup_leak_detector();
  return run_tests( tests );
}

