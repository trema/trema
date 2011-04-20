/*
 * Unit tests for libpathresolver
 * 
 * Author: Kazushi SUGYO, Shuji Ishii
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


#include <sys/types.h>
#include <arpa/inet.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"
#include "cmockery.h"


#include "trema.h"
#include "service_management.h"
#include "topology_management.h"
#include "topology_table.h"
#include "wrapper.h"

#define VOID_FUNCTION (0)

static list_element *sw_table;

static uint64_t datapath_id = 0x1111222233334444ULL;
static uint32_t transaction_id = 12345;
static void *dummy_user_data = &dummy_user_data;

static uint32_t dummy_n_buffers = 0;
static uint8_t dummy_n_tables = 0;
static uint32_t dummy_capabilities = 0;
static uint32_t dummy_actions = 0;

static list_element *phy_ports = NULL;
static uint8_t HW_ADDR[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
static const uint32_t PORT_FEATURES = ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD |
                                        OFPPF_100MB_FD | OFPPF_1GB_HD | OFPPF_1GB_FD |
                                        OFPPF_COPPER |  OFPPF_AUTONEG | OFPPF_PAUSE );

static uint16_t port1_no = 1;
static char port1_name[] = "port1";
static uint16_t port2_no = 2;
static char port2_name[] = "port2";
static uint16_t port3_no = 3;
static char port3_name[] = "port3";
static uint16_t port4_no = 4;
static char port4_name[] = "port4";

static bool kick_switch_features_reply = false;

static sw_entry *sw;
static port_entry *port1;
static port_entry *port2;
static port_entry *port3;
static link_to *link_to_from_1_to_2;
static link_to *link_to_from_2_to_1;

// static functions
void switch_ready( uint64_t datapath_id, void *user_data );
void switch_disconnected( uint64_t datapath_id, void *user_data );
void switch_features_reply( uint64_t datapath_id, uint32_t transaction_id,
                            uint32_t n_buffers, uint8_t n_tables,
                            uint32_t capabilities, uint32_t actions,
                            const list_element *phy_ports, void *user_data );
void port_status( uint64_t datapath_id, uint32_t transaction_id, uint8_t reason,
                  struct ofp_phy_port phy_port, void *user_data );



/******************************************************************************
 * Helper functions.                                                          
 ******************************************************************************/


static void
helper_delete_link_to( port_entry *port ) {
  if ( port->link_to == NULL ) {
    return;
  }
  xfree( port->link_to );
  port->link_to = NULL;
}


static void
helper_free_port_entry( port_entry *free_entry ) {
  if ( free_entry->link_to != NULL ) {
    helper_delete_link_to( free_entry );
  }
  xfree( free_entry );
}


static sw_entry *
helper_allocate_sw_entry( uint64_t *datapath_id ) {
  sw_entry *new_entry;

  new_entry = xmalloc( sizeof( sw_entry ) );
  new_entry->datapath_id = *datapath_id;
  new_entry->id = transaction_id;
  create_list( &( new_entry->port_table ) );

  return new_entry;
}


static void
helper_free_sw_entry( sw_entry *sw ) {
  list_element *e;

  for ( e = sw->port_table; e != NULL; e = e->next ) {
    helper_free_port_entry( e->data );
  }
  delete_list( sw->port_table );
  xfree( sw );
}


static sw_entry *
helper_lookup_sw_entry( uint64_t *datapath_id ) {
  sw_entry *entry;
  list_element *list;

  for ( list = sw_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( entry->datapath_id == *datapath_id ) {
      // found
      return entry;
    }
  }

  return NULL;
}


static sw_entry *
helper_update_sw_entry( uint64_t *datapath_id ) {
  sw_entry *entry;

  entry = helper_lookup_sw_entry( datapath_id );
  if ( entry != NULL ) {
      return entry;
  }
  entry = helper_allocate_sw_entry( datapath_id );
  insert_in_front( &sw_table, entry );

  return entry;
}


static void
init_sw_table() {
  create_list( &sw_table );
}


static void
finalize_sw_table() {
  list_element *e;

  for ( e = sw_table; e != NULL; e = e->next ) {
    helper_free_sw_entry( e->data );
  }
  delete_list( sw_table );
  sw_table = NULL;
}


static port_entry *
allocate_port_entry( sw_entry *sw, uint16_t port_no, const char *name ) {
  port_entry *new_entry;

  new_entry = xmalloc( sizeof( port_entry ) );
  new_entry->sw = sw;
  new_entry->port_no = port_no;
  strncpy( new_entry->name, name, sizeof( new_entry->name ) );
  new_entry->name[ OFP_MAX_PORT_NAME_LEN - 1] = '\0';
  new_entry->up = false;
  new_entry->external = false;
  new_entry->link_to = NULL;

  return new_entry;
}


static port_entry *
helper_lookup_port_entry( sw_entry *sw, uint16_t port_no, const char *name ) {
  port_entry *store = NULL, *entry;
  list_element *list;

  for ( list = sw->port_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( name == NULL ) {
      if ( entry->port_no == port_no ) {
        return entry;
      }
    } else {
      if ( strcmp( entry->name, name ) == 0 ) {
        return entry;
      }
      if ( entry->port_no == port_no ) {
        store = entry;
      }
    }
  }
  if ( store != NULL ) {
    return store;
  }

  return NULL;
}


static port_entry *
helper_update_port_entry( sw_entry *sw, uint16_t port_no, const char *name ) {
  port_entry *entry;

  entry = helper_lookup_port_entry( sw, port_no, name );
  if ( entry != NULL ) {
      return entry;
  }
  entry = allocate_port_entry( sw, port_no, name );
  insert_in_front( &( sw->port_table ), entry );

  return entry;
}


static link_to *
helper_update_link_to( port_entry *port, uint64_t *datapath_id, uint16_t port_no, bool up ) {
  if ( port->link_to != NULL ) {
    xfree( port->link_to );
    port->link_to = NULL;
  }

  port->link_to = xmalloc( sizeof( link_to ) );
  port->link_to->datapath_id = *datapath_id;
  port->link_to->port_no = port_no;
  port->link_to->up = up;

  return port->link_to;
}


static list_element *
setup_test_phy_ports() {
  list_element *ports;

  struct ofp_phy_port *phy_port = xmalloc( sizeof( struct ofp_phy_port ) );
  phy_port->port_no = port1_no;
  memcpy( phy_port->hw_addr, HW_ADDR, sizeof( phy_port->hw_addr ) );
  strcpy( phy_port->name, port1_name );
  phy_port->config = 0; //OFPPC_PORT_DOWN;
  phy_port->state = 0; //OFPPS_LINK_DOWN;
  phy_port->curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port->advertised = PORT_FEATURES;
  phy_port->supported = PORT_FEATURES;
  phy_port->peer = PORT_FEATURES;

  create_list( &ports );
  append_to_tail( &ports, phy_port );
  
  phy_port = xmalloc( sizeof( struct ofp_phy_port ) );
  phy_port->port_no = port2_no;
  memcpy( phy_port->hw_addr, HW_ADDR, sizeof( phy_port->hw_addr ) );
  strcpy( phy_port->name, port2_name );
  phy_port->config = 0; //OFPPC_PORT_DOWN;
  phy_port->state = 0; //OFPPS_LINK_DOWN;
  phy_port->curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port->advertised = PORT_FEATURES;
  phy_port->supported = PORT_FEATURES;
  phy_port->peer = PORT_FEATURES;

  append_to_tail( &ports, phy_port );

  phy_port = xmalloc( sizeof( struct ofp_phy_port ) );
  phy_port->port_no = port3_no;
  memcpy( phy_port->hw_addr, HW_ADDR, sizeof( phy_port->hw_addr ) );
  strcpy( phy_port->name, port3_name );
  phy_port->config = 0; //OFPPC_PORT_DOWN;
  phy_port->state = 0; //OFPPS_LINK_DOWN;
  phy_port->curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port->advertised = PORT_FEATURES;
  phy_port->supported = PORT_FEATURES;
  phy_port->peer = PORT_FEATURES;

  append_to_tail( &ports, phy_port );

  return ports;
}
  

static void
teardown_test_phy_ports( list_element *ports ) {
  list_element *e;

  for ( e = ports; e != NULL; e = e->next ) {
    xfree( e->data );
  }

  delete_list( ports );
}


static void
setup_test_ports() {
  // add datapath_id
  sw = helper_update_sw_entry( &datapath_id );

  // add ports
  port3 = helper_update_port_entry( sw, port3_no, port3_name );
  memcpy( port3->mac, HW_ADDR, ETH_ADDRLEN );

  port2 = helper_update_port_entry( sw, port2_no, port2_name );
  memcpy( port2->mac, HW_ADDR, ETH_ADDRLEN );

  port1 = helper_update_port_entry( sw, port1_no, port1_name );
  memcpy( port1->mac, HW_ADDR, ETH_ADDRLEN );

  // add link (port1 <-> port2 )
  link_to_from_1_to_2 = helper_update_link_to( port1, &datapath_id, port2_no, true );
  link_to_from_2_to_1 = helper_update_link_to( port2, &datapath_id, port1_no, true );
}


static void
teardown_test_ports() {
  sw = NULL;
  port1 = port2 = port3 = NULL;
  link_to_from_1_to_2 = link_to_from_2_to_1 = NULL;
}


/*************************************************************************
 * Setup and teardown function.                                          
 *************************************************************************/


static void
setup() {
  init_log( "topology_management_test", false );
  assert_true( sw_table == NULL );
  init_sw_table();
}


static void
teardown() {
  finalize_sw_table();
}


/******************************************************************************
 * Mock                                                                       
 ******************************************************************************/

void
mock_die( char *format, ... ) {
  UNUSED( format );
}


uint32_t
mock_get_transaction_id() {
  return ( uint32_t ) mock();
}


buffer *
mock_create_features_request( /*const*/ uint32_t transaction_id ) {
  check_expected( transaction_id );

  return ( buffer * ) mock();
}


bool
mock_send_openflow_message( /*const*/ uint64_t datapath_id64, buffer *message ) {
  uint64_t *datapath_id = &datapath_id64;
  check_expected( datapath_id );
  check_expected( message );

  bool ret = ( bool ) mock();

  if ( kick_switch_features_reply ) {
    kick_switch_features_reply = false;
    switch_features_reply( datapath_id64, transaction_id, dummy_n_buffers, dummy_n_tables, dummy_capabilities, dummy_actions, phy_ports, dummy_user_data );
  }

  return ret;
}


port_entry *
mock_update_port_entry( sw_entry *sw, uint16_t port_no16, /*const*/ char *name ) {
  check_expected( sw );
  uint32_t port_no = port_no16;
  check_expected( port_no );
  check_expected( name );

  return ( port_entry * ) mock();
}


void
mock_notify_port_status_for_all_user( port_entry *port ) {
  sw_entry *sw = port->sw;
  check_expected( sw );
  uint32_t port_no = port->port_no;
  check_expected( port_no );
  char *name = port->name;
  check_expected( name );
  uint8_t *mac = port->mac;
  check_expected( mac );
  bool up = port->up;
  check_expected( up );
  bool external = port->external;
  check_expected( external );
  link_to *link_to = port->link_to;
  check_expected( link_to );
  uint32_t id = port->id;
  check_expected( id );

  ( void ) mock();
}


void
mock_notify_link_status_for_all_user( port_entry *port ) {
  check_expected( port );

  ( void ) mock();
}


void
mock_delete_link_to( port_entry *port ) {
  check_expected( port );

  ( void ) mock();
}


void
mock_delete_port_entry( sw_entry *sw, port_entry *port ) {
  check_expected( sw );
  check_expected( port );

  ( void ) mock();
}


sw_entry *
mock_update_sw_entry( uint64_t *datapath_id ) {
  check_expected( datapath_id );

  return ( sw_entry * ) mock();
}


sw_entry *
mock_lookup_sw_entry( uint64_t *datapath_id ) {
  check_expected( datapath_id );

  return ( sw_entry * ) mock();
}


void
mock_delete_sw_entry( sw_entry *sw ) {
  check_expected( sw );

  ( void ) mock();
}


port_entry *
mock_lookup_port_entry( sw_entry *sw, uint16_t port_no16, /*const*/ char *name ) {
  check_expected( sw );
  uint32_t port_no = port_no16;
  check_expected( port_no );
  check_expected( name );

  return ( port_entry * ) mock();
}


const char *
mock_get_trema_name() {
  return ( const char * ) mock();
}


bool
mock_init_openflow_application_interface( /*const*/ char *custom_service_name ) {
  check_expected( custom_service_name );

  return ( bool ) mock();
}


bool
mock_set_switch_ready_handler( switch_ready_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}


bool
mock_set_switch_disconnected_handler( switch_disconnected_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}


bool
mock_set_features_reply_handler( features_reply_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}


bool
mock_set_port_status_handler( port_status_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}



/******************************************************************************
 * Tests.                                                                     
 ******************************************************************************/


static void
test_start_topology_management() {
  // get_trema_name()
  char my_name[] = "topology_management";
  will_return( mock_get_trema_name, my_name );

  // init_openflow_application_interface()
  expect_string( mock_init_openflow_application_interface, custom_service_name, my_name );
  will_return( mock_init_openflow_application_interface, true );

  // set_switch_ready_handler()
  expect_value( mock_set_switch_ready_handler, callback, switch_ready );
  expect_value( mock_set_switch_ready_handler, user_data, NULL );
  will_return( mock_set_switch_ready_handler, true );

  // set_switch_disconnected_handler()
  expect_value( mock_set_switch_disconnected_handler, callback, switch_disconnected );
  expect_value( mock_set_switch_disconnected_handler, user_data, NULL );
  will_return( mock_set_switch_disconnected_handler, true );

  // set_features_reply_handler()
  expect_value( mock_set_features_reply_handler, callback, switch_features_reply );
  expect_value( mock_set_features_reply_handler, user_data, NULL );
  will_return( mock_set_features_reply_handler, true );

  // set_port_status_handler()
  expect_value( mock_set_port_status_handler, callback, port_status );
  expect_value( mock_set_port_status_handler, user_data, NULL );
  will_return( mock_set_port_status_handler, true );

  // target
  start_topology_management();
}


static void
test_stop_topology_management() {
  // target
  stop_topology_management();
}


static void
test_switch_is_ready() {
  setup();

  // update_sw_entry()
  expect_memory( mock_update_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  sw_entry *new_entry = helper_allocate_sw_entry( &datapath_id );
  will_return( mock_update_sw_entry, new_entry );

  // get_transaction_id()
  will_return( mock_get_transaction_id, transaction_id );

  // create_features_request()
  expect_value( mock_create_features_request, transaction_id, transaction_id );
  buffer *dummy_buffer = alloc_buffer();
  will_return( mock_create_features_request, dummy_buffer );

  // send_openflow_message()
  kick_switch_features_reply = true;
  phy_ports = setup_test_phy_ports(); // port 1, 2, 3
  expect_memory( mock_send_openflow_message, datapath_id, &datapath_id, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, message, dummy_buffer );
  will_return( mock_send_openflow_message, true );

  // lookup_sw_entry()
  expect_memory( mock_lookup_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  will_return( mock_lookup_sw_entry, new_entry );

  // port1
  // update_port_entry()
  expect_value( mock_update_port_entry, sw, new_entry );
  expect_value( mock_update_port_entry, port_no, ( uint32_t ) port1_no );
  expect_string( mock_update_port_entry, name, port1_name );
  port_entry *port1 = helper_update_port_entry( new_entry, port1_no, port1_name );
  will_return( mock_update_port_entry, port1 );

  // nofity_port_status_for_all_user()
  expect_value( mock_notify_port_status_for_all_user, sw, new_entry );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port1_no );
  expect_string( mock_notify_port_status_for_all_user, name, port1_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, true );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, NULL );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  // port2
  // update_port_entry()
  expect_value( mock_update_port_entry, sw, new_entry );
  expect_value( mock_update_port_entry, port_no, ( uint32_t ) port2_no );
  expect_string( mock_update_port_entry, name, port2_name );
  port_entry *port2 = helper_update_port_entry( new_entry, port2_no, port2_name );
  will_return( mock_update_port_entry, port2 );

  // nofity_port_status_for_all_user()
  expect_value( mock_notify_port_status_for_all_user, sw, new_entry );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port2_no );
  expect_string( mock_notify_port_status_for_all_user, name, port2_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, true );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, NULL );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  // port3
  // update_port_entry()
  expect_value( mock_update_port_entry, sw, new_entry );
  expect_value( mock_update_port_entry, port_no, ( uint32_t ) port3_no );
  expect_string( mock_update_port_entry, name, port3_name );
  port_entry *port3 = helper_update_port_entry( new_entry, port3_no, port3_name );
  will_return( mock_update_port_entry, port3 );

  // nofity_port_status_for_all_user()
  expect_value( mock_notify_port_status_for_all_user, sw, new_entry );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port3_no );
  expect_string( mock_notify_port_status_for_all_user, name, port3_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, true );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, NULL );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  // target
  switch_ready( datapath_id, dummy_user_data );

  teardown_test_phy_ports( phy_ports );
  helper_free_sw_entry( new_entry );
  teardown();
}


static void
test_switch_is_disconnected() {
  setup();
  setup_test_ports();

  // lookup_sw_entry()
  expect_memory( mock_lookup_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  will_return( mock_lookup_sw_entry, sw );

  // port1: delete_notification()-> notify_link_status_for_all_user(), delete_link_to(), notify_port_status_for_all_user(), delete_port_entry()
  expect_value( mock_notify_link_status_for_all_user, port, port1 );
  will_return( mock_notify_link_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_link_to, port, port1 );
  will_return( mock_delete_link_to, VOID_FUNCTION );

  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port1_no );
  expect_string( mock_notify_port_status_for_all_user, name, port1_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, false );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, link_to_from_1_to_2 );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_port_entry, sw, sw );
  expect_value( mock_delete_port_entry, port, port1 );
  will_return( mock_delete_port_entry, VOID_FUNCTION );

  // port2: delete_notification()-> notify_link_status_for_all_user(), delete_link_to(), notify_port_status_for_all_user(), delete_port_entry()
  expect_value( mock_notify_link_status_for_all_user, port, port2 );
  will_return( mock_notify_link_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_link_to, port, port2 );
  will_return( mock_delete_link_to, VOID_FUNCTION );

  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port2_no );
  expect_string( mock_notify_port_status_for_all_user, name, port2_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, false );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, link_to_from_2_to_1 );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_port_entry, sw, sw );
  expect_value( mock_delete_port_entry, port, port2 );
  will_return( mock_delete_port_entry, VOID_FUNCTION );

  // port3: delete_notification()-> notify_port_status_for_all_user(), delete_port_entry()
  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port3_no );
  expect_string( mock_notify_port_status_for_all_user, name, port3_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, false );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, NULL );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_port_entry, sw, sw );
  expect_value( mock_delete_port_entry, port, port3 );
  will_return( mock_delete_port_entry, VOID_FUNCTION );

  // delete_sw_entry()
  expect_value( mock_delete_sw_entry, sw, sw );
  will_return( mock_delete_sw_entry, VOID_FUNCTION );

  // target
  switch_disconnected( datapath_id, dummy_user_data );

  teardown_test_ports();
  teardown();
}


static void
test_port_is_added() {
  setup();

  // lookup_sw_entry()
  sw = helper_update_sw_entry( &datapath_id );
  expect_memory( mock_lookup_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  will_return( mock_lookup_sw_entry, sw );

  // lookup_port_entry()
  expect_value( mock_lookup_port_entry, sw, sw );
  expect_value( mock_lookup_port_entry, port_no, ( uint32_t ) port4_no );
  expect_string( mock_lookup_port_entry, name, port4_name );
  will_return( mock_lookup_port_entry, NULL );

  // update_port_entry()
  port_entry *port = helper_update_port_entry( sw, port4_no, port4_name );
  expect_value( mock_update_port_entry, sw, sw );
  expect_value( mock_update_port_entry, port_no, ( uint32_t ) port4_no );
  expect_string( mock_update_port_entry, name, port4_name ); 
  will_return( mock_update_port_entry, port );

  // notify_port_status_for_all_user()
  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port4_no );
  expect_string( mock_notify_port_status_for_all_user, name, port4_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, true );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, NULL );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  // target
  struct ofp_phy_port new_port;
  new_port.port_no = port4_no;
  memcpy( new_port.hw_addr, HW_ADDR, ETH_ADDRLEN );
  strcpy( new_port.name, port4_name );
  new_port.config = 0; // up
  new_port.state = 0; // up

  port_status( datapath_id, transaction_id, OFPPR_ADD, new_port, dummy_user_data );

  teardown();
}


static void
test_port_is_deleted() {
  setup();
  setup_test_ports();

  // lookup_sw_entry()
  expect_memory( mock_lookup_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  will_return( mock_lookup_sw_entry, sw );

  // lookup_port_entry()
  expect_value( mock_lookup_port_entry, sw, sw );
  expect_value( mock_lookup_port_entry, port_no, ( uint32_t ) port1_no );
  expect_string( mock_lookup_port_entry, name, port1_name );
  will_return( mock_lookup_port_entry,  port1 );

  // port1: delete_notification()-> notify_link_status_for_all_user(), delete_link_to(), notify_port_status_for_all_user(), delete_port_entry()
  expect_value( mock_notify_link_status_for_all_user, port, port1 );
  will_return( mock_notify_link_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_link_to, port, port1 );
  will_return( mock_delete_link_to, VOID_FUNCTION );

  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port1_no );
  expect_string( mock_notify_port_status_for_all_user, name, port1_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, false );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, link_to_from_1_to_2 );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_port_entry, sw, sw );
  expect_value( mock_delete_port_entry, port, port1 );
  will_return( mock_delete_port_entry, VOID_FUNCTION );

  // target
  struct ofp_phy_port deleted_port;
  deleted_port.port_no = port1_no;
  memcpy( deleted_port.hw_addr, HW_ADDR, ETH_ADDRLEN );
  strcpy( deleted_port.name, port1_name );
  deleted_port.config = OFPPC_PORT_DOWN;
  deleted_port.state = OFPPS_LINK_DOWN;

  port_status( datapath_id, transaction_id, OFPPR_DELETE, deleted_port, dummy_user_data );

  teardown_test_ports();
  teardown();
}


static void
test_port_no_is_changed() {
  setup();
  setup_test_ports();

  // port_no: from 1 to 4

  // lookup_sw_entry()
  expect_memory( mock_lookup_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  will_return( mock_lookup_sw_entry, sw );

  // lookup_port_entry()
  expect_value( mock_lookup_port_entry, sw, sw );
  expect_value( mock_lookup_port_entry, port_no, ( uint32_t ) port4_no );
  expect_string( mock_lookup_port_entry, name, port1_name );
  will_return( mock_lookup_port_entry, port1 );

  // port1: delete_notification()-> notify_link_status_for_all_user(), delete_link_to(), notify_port_status_for_all_user(), delete_port_entry()
  expect_value( mock_notify_link_status_for_all_user, port, port1 );
  will_return( mock_notify_link_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_link_to, port, port1 );
  will_return( mock_delete_link_to, VOID_FUNCTION );

  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port1_no );
  expect_string( mock_notify_port_status_for_all_user, name, port1_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, false );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, link_to_from_1_to_2 );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  expect_value( mock_delete_port_entry, sw, sw );
  expect_value( mock_delete_port_entry, port, port1 );
  will_return( mock_delete_port_entry, VOID_FUNCTION );

  // port1: add_notification()-> update_port_entry(), notify_port_status_for_all_user()
  expect_value( mock_update_port_entry, sw, sw );
  expect_value( mock_update_port_entry, port_no, ( uint32_t ) port4_no );
  expect_string( mock_update_port_entry, name, port1_name ); 
  will_return( mock_update_port_entry, port1 );

  expect_value( mock_notify_port_status_for_all_user, sw, sw );
  expect_value( mock_notify_port_status_for_all_user, port_no, ( uint32_t ) port1_no );
  expect_string( mock_notify_port_status_for_all_user, name, port1_name );
  expect_memory( mock_notify_port_status_for_all_user, mac, HW_ADDR, ETH_ADDRLEN );
  expect_value( mock_notify_port_status_for_all_user, up, true );
  expect_value( mock_notify_port_status_for_all_user, external, false );
  expect_value( mock_notify_port_status_for_all_user, link_to, link_to_from_1_to_2 );
  expect_value( mock_notify_port_status_for_all_user, id, transaction_id );
  will_return( mock_notify_port_status_for_all_user, VOID_FUNCTION );

  // target
  struct ofp_phy_port port_no_is_changed; // port_no: 1 -> 4
  port_no_is_changed.port_no = port4_no;
  memcpy( port_no_is_changed.hw_addr, HW_ADDR, ETH_ADDRLEN );
  strcpy( port_no_is_changed.name, port1_name );
  port_no_is_changed.config = 0; // up
  port_no_is_changed.state = 0; // up

  port_status( datapath_id, transaction_id, OFPPR_MODIFY, port_no_is_changed, dummy_user_data );

  teardown_test_ports();
  teardown();
}


static void
test_port_name_is_changed() {
  setup();
  setup_test_ports();

  // name: from "port1" to "port4"

  // lookup_sw_entry()
  expect_memory( mock_lookup_sw_entry, datapath_id, &datapath_id, sizeof( uint64_t ) );
  will_return( mock_lookup_sw_entry, sw );

  // lookup_port_entry()
  expect_value( mock_lookup_port_entry, sw, sw );
  expect_value( mock_lookup_port_entry, port_no, ( uint32_t ) port1_no );
  expect_string( mock_lookup_port_entry, name, port4_name );
  will_return( mock_lookup_port_entry, port1 );

  // port1: update_notification()->return

  // target
  struct ofp_phy_port port_name_is_changed; // name: "port1" -> "port4"
  port_name_is_changed.port_no = port1_no;
  memcpy( port_name_is_changed.hw_addr, HW_ADDR, ETH_ADDRLEN );
  strcpy( port_name_is_changed.name, port4_name );
  port_name_is_changed.config = OFPPC_PORT_DOWN;
  port_name_is_changed.state = OFPPS_LINK_DOWN;

  port_status( datapath_id, transaction_id, OFPPR_MODIFY, port_name_is_changed, dummy_user_data );

  teardown_test_ports();
  teardown();
}


/******************************************************************************
 * Run tests.                                                                 
 ******************************************************************************/


int
main() {
  const UnitTest tests[] = {
    unit_test( test_start_topology_management ),
    unit_test( test_stop_topology_management ),

    unit_test( test_switch_is_ready ),
    unit_test( test_switch_is_disconnected ),

    unit_test( test_port_is_added ),
    unit_test( test_port_is_deleted ),
    unit_test( test_port_no_is_changed ),
    unit_test( test_port_name_is_changed ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
