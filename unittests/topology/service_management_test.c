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

#include "service_management.h"

#include "subscriber_table.h"
#include "topology_table.h"

/********************************************************************************
 * Common function.
 ********************************************************************************/

#define TEST_TREMA_NAME "test_service_mgmt"
#define TEST_TOPOLOGY_MESSENGER_NAME "test_service_mgmt.t"
#define TEST_SUBSCRIBER_NAME "test_service_mgmt-c-12345"

#define TEST_CONTROL_NAME "test_service_mgmt-control"

// defined in trema.c
extern void set_trema_name( const char *name );
extern void _free_trema_name();


// defined in service_management.c
extern void ping_all_subscriber(void* user_data );

//defined in discovery_manager.h
extern void _enable_discovery( void );
extern void _disable_discovery( void );
extern void (* enable_discovery )( void );
extern void (* disable_discovery )( void );

/********************************************************************************
 * Mock functions.
 ********************************************************************************/

#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;

static bool ( *original_add_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
static bool ( *original_add_message_replied_callback )( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) );

static bool ( *original_add_periodic_event_callback )( const time_t seconds, timer_callback callback, void *user_data );

static uint8_t ( *original_set_discovered_link_status )( topology_update_link_status* link_status );


static bool
mock_add_message_requested_callback( const char *service_name,
                                     void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  check_expected( service_name );
  UNUSED( callback );
  return ( bool ) mock();
}

static bool
mock_add_message_replied_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  check_expected( service_name );
//  check_expected( callback );
  UNUSED( callback );
  return (bool)mock();
}

static bool
mock_add_periodic_event_callback( const time_t seconds, timer_callback callback, void *user_data ) {
  check_expected( seconds );
//  check_expected( callback );
  UNUSED( callback );
  check_expected( user_data );
  return ( bool ) mock();
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


static bool
mock_link_status_notification( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  topology_link_status *const link_status = data;
  const int number_of_links = ( int ) ( len / sizeof( topology_link_status ) );
  int i;

  // (re)build topology db
  for ( i = 0; i < number_of_links; i++ ) {
    topology_link_status *s = &link_status[ i ];
    s->from_dpid = ntohll( s->from_dpid );
    s->from_portno = ntohs( s->from_portno );
    s->to_dpid = ntohll( s->to_dpid );
    s->to_portno = ntohs( s->to_portno );

    const uint64_t from_dpid = s->from_dpid;
    check_expected( from_dpid );

    const uint16_t from_portno = s->from_portno;
    check_expected( from_portno );

    const uint64_t to_dpid = s->to_dpid;
    check_expected( to_dpid );

    const uint16_t to_portno = s->to_portno;
    check_expected( to_portno );

    const uint8_t status = s->status;
    check_expected( status );
  }

  return ( bool ) mock();
}


// handle asynchronous notification from topology
static bool
mock_port_status_notification( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  UNUSED( len );
  topology_port_status *const port_status = data;


  // arrange byte order
  topology_port_status *s = port_status;
  s->dpid = ntohll( s->dpid );
  s->port_no = ntohs( s->port_no );

  const uint64_t dpid = s->dpid;
  check_expected( dpid );

  const uint16_t port_no = s->port_no;
  check_expected( port_no );

  const char* name = s->name;
  check_expected( name );

  const uint8_t* mac = s->mac;
  check_expected( mac );

  const uint8_t external = s->external;
  check_expected( external );

  const uint8_t status = s->status;
  check_expected( status );

  return ( bool ) mock();
}

static bool
mock_switch_status_notification( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  UNUSED( len );
  topology_switch_status* switch_status = data;

  // arrange byte order
  switch_status->dpid = ntohll( switch_status->dpid );

  const uint64_t dpid = switch_status->dpid;
  check_expected( dpid );

  const uint8_t status = switch_status->status;
  check_expected( status );

  return ( bool ) mock();
}

static bool END_ON_RETURN = true;

static void
callback_fake_libtopology_client_notification_end( uint16_t tag, void *data, size_t len ) {

  bool end = true;
  switch( tag ){
  case TD_MSGTYPE_LINK_STATUS_NOTIFICATION:
    end = mock_link_status_notification( tag, data, len );
    break;

  case TD_MSGTYPE_PORT_STATUS_NOTIFICATION:
    end = mock_port_status_notification( tag, data, len );
    break;

  case TD_MSGTYPE_SWITCH_STATUS_NOTIFICATION:
    end = mock_switch_status_notification( tag, data, len );
    break;

  default:
    // not reachable
    assert_int_equal(tag, NULL);
  }

  if ( end ) {
    stop_event_handler();
    stop_messenger();
  }
}


#define TESTHELPER_STOP_MESSENGER 0
static void testhelper_stop_messenger();

#define TESTHELPER_SUBSCRIBER_LAST_SEEN_UPDATED_END 1
static void testhelper_subscriber_last_seen_updated_end();


static void
callback_test_control_notification_handler( uint16_t tag, void *data, size_t len ) {
  UNUSED( data );
  UNUSED( len );
  switch( tag ){
  case TESTHELPER_STOP_MESSENGER:
    testhelper_stop_messenger();
    break;
  case TESTHELPER_SUBSCRIBER_LAST_SEEN_UPDATED_END:
    testhelper_subscriber_last_seen_updated_end();
    break;
  }
}


static void
mock_ping_request( const messenger_context_handle *handle, void *data, size_t len ) {

  assert_true( len > 0 );
  topology_request *req = data;
  const char* name = req->name;
  check_expected( name );


  // respond to topology ping
  const size_t name_bytes = strlen( name ) + 1;
  const size_t res_len = sizeof(topology_ping_response) + name_bytes;

  buffer *buf = alloc_buffer_with_length( res_len );
  topology_ping_response *res = append_back_buffer( buf, res_len );
  strncpy( res->name, name, name_bytes );

  bool ret = send_reply_message( handle, TD_MSGTYPE_PING_RESPONSE,
                                 buf->data, buf->length );
  assert_true( ret );
  free_buffer( buf );

  flush_messenger();
  // below is purely for testing purpose.
  send_message( TEST_CONTROL_NAME, ( uint16_t ) mock(), NULL, 0 );
}


static void
callback_fake_libtopology_client_request( const messenger_context_handle *handle,
                                          uint16_t tag, void *data, size_t len ) {
  switch ( tag ) {
  case TD_MSGTYPE_PING_REQUEST:
    // ping: topology service -> libtopology
    mock_ping_request( handle, data, len );
    break;

  default:
    // not reachable
    assert_int_equal(tag, NULL);
  }
}


static void
mock_topology_reply( uint16_t tag, void *data, size_t len, void *user_data ) {
  UNUSED( tag );
  UNUSED( len );
  UNUSED( user_data );
  topology_response *res = data;

  uint8_t status = res->status;
  check_expected( status );
}


static void
mock_query_link_status_reply( uint16_t tag,
                              void *data, size_t len,
                              void *param0 ) {
  UNUSED( tag );
  UNUSED( param0 );
  topology_link_status *const link_status = data;
  const int number_of_links =( int ) ( len / sizeof( topology_link_status ) );
  check_expected( number_of_links );

  // rearrange byte order
  for (int  i = 0; i < number_of_links; i++ ) {
    topology_link_status *s = &link_status[ i ];
    s->from_dpid = ntohll( s->from_dpid );
    uint64_t from_dpid = s->from_dpid;
    check_expected( from_dpid );
    s->from_portno = ntohs( s->from_portno );
    uint16_t from_portno = s->from_portno;
    check_expected( from_portno );
    s->to_dpid = ntohll( s->to_dpid );
    uint64_t to_dpid = s->to_dpid;
    check_expected( to_dpid );
    s->to_portno = ntohs( s->to_portno );
    uint16_t to_portno = s->to_portno;
    check_expected( to_portno );

    uint8_t status = s->status;
    check_expected( status );
  }
}


// handle reply from topology
static void
mock_query_port_status_reply( uint16_t tag,
                              void *data, size_t len,
                              void *param0 ) {
  UNUSED( tag );
  UNUSED( param0 );
  topology_port_status *const port_status = data;
  const int number_of_ports =( int ) ( len / sizeof( topology_port_status ) );
  check_expected( number_of_ports );

  // rearrange byte order
  for ( int i = 0; i < number_of_ports; i++ ) {
    topology_port_status *s = &port_status[ i ];
    s->dpid = ntohll( s->dpid );
    uint64_t dpid = s->dpid;
    check_expected( dpid );
    s->port_no = ntohs( s->port_no );
    uint16_t port_no = s->port_no;
    check_expected( port_no );

    uint8_t status = s->status;
    check_expected( status );
    uint8_t external = s->external;
    check_expected( external );

    const char* name = s->name;
    check_expected( name );

    uint8_t* mac = s->mac;
    check_expected( mac );
  }
}


// handle reply from topology
static void
mock_query_switch_status_reply( uint16_t tag,
                                void *data, size_t len, void *param0 ) {
  UNUSED( tag );
  UNUSED( param0 );
  topology_switch_status *const switch_status = data;
  const int number_of_switches = ( int ) ( len / sizeof( topology_switch_status ) );
  check_expected( number_of_switches );

  // rearrange byte order
  for ( int i = 0; i < number_of_switches; i++ ) {
    topology_switch_status *s = &switch_status[ i ];
    s->dpid = ntohll( s->dpid );
    uint64_t dpid = s->dpid;
    check_expected( dpid );

    uint8_t status = s->status;
    check_expected( status );
  }
}


static void
callback_fake_libtopology_client_reply_end( uint16_t tag, void *data, size_t len, void *user_data ) {
  switch ( tag ) {
  case TD_MSGTYPE_SUBSCRIBE_RESPONSE:
  case TD_MSGTYPE_UNSUBSCRIBE_RESPONSE:
  case TD_MSGTYPE_ENABLE_DISCOVERY_RESPONSE:
  case TD_MSGTYPE_DISABLE_DISCOVERY_RESPONSE:
  case TD_MSGTYPE_UPDATE_LINK_STATUS_RESPONSE:
    mock_topology_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_QUERY_LINK_STATUS_RESPONSE:
    mock_query_link_status_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_QUERY_PORT_STATUS_RESPONSE:
    mock_query_port_status_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_QUERY_SWITCH_STATUS_RESPONSE:
    mock_query_switch_status_reply( tag, data, len, user_data );
    break;



  default:
    // not reachable
    assert_int_equal(tag, NULL);
  }
  stop_event_handler();
  stop_messenger();
}


static void
mock_execute_timer_events( int *next_timeout_usec ) {
  UNUSED( next_timeout_usec );
  // Do nothing.
}


static void
mock_enable_discovery() {

}


static void
mock_disable_discovery() {

}

/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/


static void
setup_fake_messenger() {
  set_trema_name( TEST_TREMA_NAME );

  swap_original( add_message_requested_callback );
  swap_original( add_message_replied_callback );

  swap_original( add_periodic_event_callback );
}


static void
teardown_fake_messenger() {
  revert_original( add_periodic_event_callback );

  revert_original( add_message_replied_callback );
  revert_original( add_message_requested_callback );

  _free_trema_name();
}


static void
setup_service_management() {
  enable_discovery = _enable_discovery;
  disable_discovery = _disable_discovery;
  set_trema_name( TEST_TREMA_NAME );

  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );
}


static void
teardown_service_management() {
  finalize_service_management();

  _free_trema_name();
}


static void
setup_fake_subscriber() {
  setup_service_management();

  insert_subscriber_entry( TEST_SUBSCRIBER_NAME );
}


static void
teardown_fake_subscriber() {
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  delete_subscriber_entry( e );

  teardown_service_management();
}


/********************************************************************************
 * Tests.
 ********************************************************************************/


//bool init_service_management( service_management_options new_options );
//void finalize_service_management();
static void
test_init_finalize_service_management() {
  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );
  finalize_service_management();
}


//bool start_service_management( void );
static void
test_init_start_finalize_service_management() {
  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );


  expect_string( mock_add_message_requested_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_requested_callback, true);
  expect_string( mock_add_message_replied_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_replied_callback, true);

  expect_value( mock_add_periodic_event_callback, seconds, 60 );
  expect_value( mock_add_periodic_event_callback, user_data, NULL );
  will_return( mock_add_periodic_event_callback, true);

  assert_true( start_service_management() );

  finalize_service_management();
}


static void
test_start_service_management_fail_on_periodic_event_add_fail() {
  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );


  expect_string( mock_add_message_requested_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_requested_callback, true );
  expect_string( mock_add_message_replied_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_replied_callback, true );

  expect_value( mock_add_periodic_event_callback, seconds, 60 );
  expect_value( mock_add_periodic_event_callback, user_data, NULL );
  will_return( mock_add_periodic_event_callback, false );

  assert_false( start_service_management() );

  finalize_service_management();
}


static void
test_start_service_management_fail_on_replied_callback_add_fail() {
  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );


  expect_string( mock_add_message_requested_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_requested_callback, true );
  expect_string( mock_add_message_replied_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_replied_callback, false );


  assert_false( start_service_management() );

  finalize_service_management();
}


static void
test_start_service_management_fail_on_requested_callback_add_fail() {
  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );


  expect_string( mock_add_message_requested_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_requested_callback, false );

  assert_false( start_service_management() );

  finalize_service_management();
}


//void notify_switch_status_for_all_user( sw_entry *sw );
static void
test_notify_switch_status_for_all_user() {

  init_messenger( "/tmp" );
  init_timer();
  assert_true( add_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;


  notify_switch_status_for_all_user( &sw );

  expect_value( mock_switch_status_notification, dpid, 0x1234);
  expect_value( mock_switch_status_notification, status, TD_SWITCH_UP );
  will_return( mock_switch_status_notification, END_ON_RETURN );


  start_event_handler();
  start_messenger();

  assert_true( delete_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  finalize_timer();
  finalize_messenger();
}


//void notify_port_status_for_all_user( port_entry *port );
static void
test_notify_port_status_for_all_user() {

  init_messenger( "/tmp" );
  init_timer();
  assert_true( add_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;
  port_entry port = {
      .sw = &sw,
      .port_no = 42,
      .name = "Some port name",
      .mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
      .up = true,
      .external = false,
  };

  notify_port_status_for_all_user( &port );

  expect_value( mock_port_status_notification, dpid, 0x1234 );
  expect_value( mock_port_status_notification, port_no, 42 );

  expect_string( mock_port_status_notification, name, "Some port name" );
  expect_memory( mock_port_status_notification, mac, port.mac, ETH_ADDRLEN );
  expect_value( mock_port_status_notification, external, TD_PORT_INACTIVE );

  expect_value( mock_port_status_notification, status, TD_PORT_UP );
  will_return( mock_port_status_notification, END_ON_RETURN );


  start_event_handler();
  start_messenger();

  assert_true( delete_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  finalize_timer();
  finalize_messenger();
}


//void notify_link_status_for_all_user( port_entry *port );
static void
test_notify_link_status_for_all_user() {

  init_messenger( "/tmp" );
  init_timer();
  assert_true( add_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;

  link_to link = {
      .datapath_id = 0x5678,
      .port_no = 72,
      .up = true
  };

  port_entry port = {
      .sw = &sw,
      .port_no = 42,
      .name = "Some port name",
      .mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
      .up = true,
      .external = false,
      .link_to = &link,
  };

  notify_link_status_for_all_user( &port );

  expect_value( mock_link_status_notification, from_dpid, 0x1234 );
  expect_value( mock_link_status_notification, from_portno, 42 );

  expect_value( mock_link_status_notification, to_dpid, 0x5678 );
  expect_value( mock_link_status_notification, to_portno, 72 );

  expect_value( mock_link_status_notification, status, TD_LINK_UP );
  will_return( mock_link_status_notification, END_ON_RETURN );


  start_event_handler();
  start_messenger();

  assert_true( delete_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  finalize_timer();
  finalize_messenger();
}


//bool set_link_status_updated_hook( link_status_updated_hook, void *user_data );
static void
local_link_status_updated_handler( void *user_data, const port_entry *port ) {
  check_expected( user_data );

  const sw_entry* sw = port->sw;
  check_expected( sw );

  const uint16_t port_no = port->port_no;
  check_expected( port_no );

  const char* name = port->name;
  check_expected( name );

  const uint8_t* mac = port->mac;
  check_expected( mac );

  const bool up = port->up;
  check_expected( up );

  const bool external = port->external;
  check_expected( external );

  assert( port->link_to != NULL );

  const uint64_t link_dpid = port->link_to->datapath_id;
  check_expected( link_dpid );

  const uint16_t link_port_no = port->link_to->port_no;
  check_expected( link_port_no );

  const bool link_up = port->link_to->up;
  check_expected( link_up );
}


static void
test_set_link_status_updated_hook() {
  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;

  link_to link = {
      .datapath_id = 0x5678,
      .port_no = 72,
      .up = true
  };

  port_entry port = {
      .sw = &sw,
      .port_no = 42,
      .name = "Some port name",
      .mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
      .up = true,
      .external = false,
      .link_to = &link,
  };

  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );

  expect_value( local_link_status_updated_handler, user_data, NULL );

  expect_not_value( local_link_status_updated_handler, sw, NULL );
  expect_value( local_link_status_updated_handler, port_no, 42 );
  expect_string( local_link_status_updated_handler, name, "Some port name" );
  expect_memory( local_link_status_updated_handler, mac, port.mac, ETH_ADDRLEN );
  expect_value( local_link_status_updated_handler, up, true );
  expect_value( local_link_status_updated_handler, external, false );

  expect_value( local_link_status_updated_handler, link_dpid, 0x5678 );
  expect_value( local_link_status_updated_handler, link_port_no, 72 );
  expect_value( local_link_status_updated_handler, link_up, true );

  notify_link_status_for_all_user( &port );

  assert_true( set_link_status_updated_hook( NULL, NULL ) );

}


//bool set_port_status_updated_hook( port_status_updated_hook, void *user_data );
static void
local_port_status_updated_handler( void *user_data, const port_entry *port ) {
  check_expected( user_data );

  const sw_entry* sw = port->sw;
  check_expected( sw );

  const uint16_t port_no = port->port_no;
  check_expected( port_no );

  const char* name = port->name;
  check_expected( name );

  const uint8_t* mac = port->mac;
  check_expected( mac );

  const bool up = port->up;
  check_expected( up );

  const bool external = port->external;
  check_expected( external );
}


static void
test_set_port_status_updated_hook() {
  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;
  port_entry port = {
      .sw = &sw,
      .port_no = 42,
      .name = "Some port name",
      .mac = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 },
      .up = true,
      .external = false,
  };

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );

  expect_value( local_port_status_updated_handler, user_data, NULL );

  expect_not_value( local_port_status_updated_handler, sw, NULL );
  expect_value( local_port_status_updated_handler, port_no, 42 );
  expect_string( local_port_status_updated_handler, name, "Some port name" );
  expect_memory( local_port_status_updated_handler, mac, port.mac, ETH_ADDRLEN );
  expect_value( local_port_status_updated_handler, up, true );
  expect_value( local_port_status_updated_handler, external, false );

  notify_port_status_for_all_user( &port );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );

}


//bool set_switch_status_updated_hook( switch_status_updated_hook, void *user_data );
static void
local_switch_status_updated_handler( void *user_data, const sw_entry *sw ) {
  check_expected( user_data );

  const uint64_t datapath_id = sw->datapath_id;
  check_expected( datapath_id );

  const bool up = sw->up;
  check_expected( up );
}


static void
test_set_switch_status_updated_hook() {
  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;

  assert_true( set_switch_status_updated_hook( local_switch_status_updated_handler, NULL ) );

  expect_value( local_switch_status_updated_handler, user_data, NULL );
  expect_value( local_switch_status_updated_handler, datapath_id, 0x1234 );
  expect_value( local_switch_status_updated_handler, up, true );
  notify_switch_status_for_all_user( &sw );

  assert_true( set_switch_status_updated_hook( NULL, NULL ) );

}


static void
testhelper_stop_messenger() {
  stop_event_handler();
  stop_messenger();
}


static time_t g_last_seen;


static void
testhelper_subscriber_last_seen_updated_end() {
  // test subscriber
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );

  assert_true( e->last_seen > g_last_seen );

  stop_event_handler();
  stop_messenger();
}


static void
test_ping_subscriber() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_requested_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_request ) );
  assert_true( add_message_received_callback( TEST_CONTROL_NAME, callback_test_control_notification_handler ) );


  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  // -1 sec to assure time stamp change observable.
  e->last_seen--;
  g_last_seen = e->last_seen;


  start_service_management();

  ping_all_subscriber( NULL );

  expect_string( mock_ping_request, name, TEST_SUBSCRIBER_NAME );
  will_return( mock_ping_request, TESTHELPER_SUBSCRIBER_LAST_SEEN_UPDATED_END );

  start_event_handler();
  start_messenger();

  assert_true( delete_message_requested_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_request ) );
  assert_true( delete_message_received_callback( TEST_CONTROL_NAME, callback_test_control_notification_handler ) );


  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_ping_ageout_subscriber() {
  service_management_options options = {
      .ping_interval_sec = 60,
      .ping_ageout_cycles = 5,
  };
  assert_true( init_service_management( options ) );

  expect_string( mock_add_message_requested_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_requested_callback, true);
  expect_string( mock_add_message_replied_callback, service_name, TEST_TOPOLOGY_MESSENGER_NAME );
  will_return( mock_add_message_replied_callback, true);
  expect_value( mock_add_periodic_event_callback, seconds, 60 );
  expect_value( mock_add_periodic_event_callback, user_data, NULL );
  will_return( mock_add_periodic_event_callback, true);

  assert_true( start_service_management() );

  assert_true( insert_subscriber_entry( TEST_SUBSCRIBER_NAME ) );

  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  // -1 sec to assure time stamp change observable.
  e->last_seen = time( NULL ) - ( 60 * 5 + 1 );

  ping_all_subscriber( NULL );

  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea == NULL );

  finalize_service_management();
}


static void
test_recv_subscribe_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  // send subscribe request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_SUBSCRIBE_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  assert_string_equal( e->name, TEST_SUBSCRIBER_NAME );

  // clean up
  delete_subscriber_entry( e );

  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_subscribe_from_subscribed_client_return_already_subscribed() {
  // note: setup/teardown differ from test_subscribe_from_client()

  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  // send subscribe request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_SUBSCRIBE_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_ALREADY_SUBSCRIBED );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  assert_string_equal( e->name, TEST_SUBSCRIBER_NAME );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_unsubscribe_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  // prepare subscriber table
  assert_true( insert_subscriber_entry( TEST_SUBSCRIBER_NAME ) );
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  assert_string_equal( e->name, TEST_SUBSCRIBER_NAME );

  // send unsubscribe request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_UNSUBSCRIBE_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();


  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea == NULL );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_unsubscribe_from_unsubscribed_client_return_no_such_subscriber() {
  // note: setup/teardown differ from test_subscribe_from_client()

  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e == NULL );

  // send unsubscribe request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_UNSUBSCRIBE_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_NO_SUCH_SUBSCRIBER );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea == NULL );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_enable_discovery_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  enable_discovery = mock_enable_discovery;

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  assert_string_equal( e->name, TEST_SUBSCRIBER_NAME );
  e->use_discovery = false;

  // send enable_discovery request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_ENABLE_DISCOVERY_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea != NULL );
  assert_string_equal( ea->name, TEST_SUBSCRIBER_NAME );
  assert_true( ea->use_discovery );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_enable_discovery_from_client_when_already_enabled() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  enable_discovery = mock_enable_discovery;

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  assert_string_equal( e->name, TEST_SUBSCRIBER_NAME );
  e->use_discovery = true;

  // send enable_discovery request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_ENABLE_DISCOVERY_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea != NULL );
  assert_string_equal( ea->name, TEST_SUBSCRIBER_NAME );
  assert_true( ea->use_discovery );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_enable_discovery_from_unsubscribed_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  enable_discovery = mock_enable_discovery;

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e == NULL );

  // send enable_discovery request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_ENABLE_DISCOVERY_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea != NULL );
  assert_string_equal( ea->name, TEST_SUBSCRIBER_NAME );
  assert_true( ea->use_discovery );

  // clean up
  delete_subscriber_entry( ea );
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_disable_discovery_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  disable_discovery = mock_disable_discovery;

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e != NULL );
  assert_string_equal( e->name, TEST_SUBSCRIBER_NAME );
  e->use_discovery = true;

  // send disable_discovery request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_DISABLE_DISCOVERY_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea != NULL );
  assert_string_equal( ea->name, TEST_SUBSCRIBER_NAME );
  assert_false( ea->use_discovery );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_disable_discovery_from_unsubscribed_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  disable_discovery = mock_disable_discovery;

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();
  subscriber_entry* e = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( e == NULL );

  // send disable_discovery request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_DISABLE_DISCOVERY_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  expect_value( mock_topology_reply, status, TD_RESPONSE_NO_SUCH_SUBSCRIBER );

  start_event_handler();
  start_messenger();

  // check subscriber table
  subscriber_entry* ea = lookup_subscriber_entry( TEST_SUBSCRIBER_NAME );
  assert_true( ea == NULL );

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_query_switch_status_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = false;

  // send query_switch_status request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_QUERY_SWITCH_STATUS_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  // check reply
  expect_value( mock_query_switch_status_reply, number_of_switches, 1 );
  expect_value( mock_query_switch_status_reply, status, TD_SWITCH_DOWN );
  expect_value( mock_query_switch_status_reply, dpid, 0x1234 );

  start_event_handler();
  start_messenger();

  // clean up
  delete_sw_entry( sw );

  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_query_port_status_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = false;
  port_entry* port = update_port_entry( sw, 42, "Some port name" );
  port->external = true;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port->mac[i] = (uint8_t)i;
  }
  port->up = false;

  // send query_port_status request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_QUERY_PORT_STATUS_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  // check reply
  expect_value( mock_query_port_status_reply, number_of_ports, 1 );
  expect_value( mock_query_port_status_reply, status, TD_PORT_DOWN );
  expect_value( mock_query_port_status_reply, dpid, 0x1234 );
  expect_value( mock_query_port_status_reply, port_no, 42 );
  expect_value( mock_query_port_status_reply, external, TD_PORT_EXTERNAL );
  expect_string( mock_query_port_status_reply, name, "Some port name" );
  expect_memory( mock_query_port_status_reply, mac, "\0\1\2\3\4\5", ETH_ADDRLEN );

  start_event_handler();
  start_messenger();

  // clean up
  delete_port_entry( sw, port );
  delete_sw_entry( sw );

  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


static void
test_recv_query_link_status_from_client() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;
  dpid = 0x5678;
  sw_entry* sw2 = update_sw_entry( &dpid );
  sw2->up = true;

  port_entry* port = update_port_entry( sw, 42, "Some port name1" );
  port->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port->mac[i] = (uint8_t)i;
  }
  port->up = true;

  port_entry* port2 = update_port_entry( sw2, 72, "Some port name2" );
  port2->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port2->mac[i] = (uint8_t)i;
  }
  port2->up = true;

  port_entry* port3 = update_port_entry( sw2, 102, "Some port name3" );
  port3->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port3->mac[i] = (uint8_t)i;
  }
  port3->up = true;


  dpid = 0x5678;
  update_link_to( port, &dpid, 72, false );

  dpid = 0x1234;
  update_link_to( port2, &dpid, 42, true );

  // send query_link_status request
  const size_t req_len = strlen( TEST_SUBSCRIBER_NAME ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, TEST_SUBSCRIBER_NAME );
  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_QUERY_LINK_STATUS_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  // check reply
  expect_value( mock_query_link_status_reply, number_of_links, 3 );

  expect_value( mock_query_link_status_reply, status, TD_LINK_DOWN );
  expect_value( mock_query_link_status_reply, from_dpid, 0x5678 );
  expect_value( mock_query_link_status_reply, from_portno, 102 );
  expect_value( mock_query_link_status_reply, to_dpid, 0x0 );
  expect_value( mock_query_link_status_reply, to_portno, 0 );

  expect_value( mock_query_link_status_reply, status, TD_LINK_UP );
  expect_value( mock_query_link_status_reply, from_dpid, 0x5678 );
  expect_value( mock_query_link_status_reply, from_portno, 72 );
  expect_value( mock_query_link_status_reply, to_dpid, 0x1234 );
  expect_value( mock_query_link_status_reply, to_portno, 42 );

  expect_value( mock_query_link_status_reply, status, TD_LINK_DOWN );
  expect_value( mock_query_link_status_reply, from_dpid, 0x1234 );
  expect_value( mock_query_link_status_reply, from_portno, 42 );
  expect_value( mock_query_link_status_reply, to_dpid, 0x5678 );
  expect_value( mock_query_link_status_reply, to_portno, 72 );


  start_event_handler();
  start_messenger();

  // clean up
  delete_link_to( port );
  delete_link_to( port2 );
  delete_port_entry( sw, port );
  delete_port_entry( sw2, port2 );
  delete_port_entry( sw2, port3 );
  delete_sw_entry( sw );
  delete_sw_entry( sw2 );

  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( execute_timer_events );
}


//uint8_t set_discovered_link_status( topology_update_link_status* link_status );
static void
test_set_discovered_link_status() {

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );
  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );


  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;

  dpid = 0x5678;
  sw_entry* sw2 = update_sw_entry( &dpid );
  sw2->up = true;

  port_entry* port = update_port_entry( sw, 42, "Some port name1" );
  port->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port->mac[i] = (uint8_t)i;
  }
  port->up = true;

  port_entry* port2 = update_port_entry( sw2, 72, "Some port name2" );
  port2->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port2->mac[i] = (uint8_t)i;
  }
  port2->up = true;


  topology_update_link_status link_status;
  link_status.from_dpid = 0x1234;
  link_status.from_portno = 42;
  link_status.to_dpid = 0x5678;
  link_status.to_portno = 72;
  link_status.status = TD_LINK_UP;

  // check called handlers
  expect_value( mock_link_status_notification, from_dpid, 0x1234 );
  expect_value( mock_link_status_notification, from_portno, 42 );
  expect_value( mock_link_status_notification, to_dpid, 0x5678 );
  expect_value( mock_link_status_notification, to_portno, 72 );
  expect_value( mock_link_status_notification, status, TD_LINK_UP );
  will_return( mock_link_status_notification, END_ON_RETURN );

  expect_value( local_link_status_updated_handler, user_data, NULL );
  expect_not_value( local_link_status_updated_handler, sw, NULL );
  expect_value( local_link_status_updated_handler, port_no, 42 );
  expect_string( local_link_status_updated_handler, name, "Some port name1" );
  expect_memory( local_link_status_updated_handler, mac, "\0\1\2\3\4\5", ETH_ADDRLEN );
  expect_value( local_link_status_updated_handler, up, true );
  expect_value( local_link_status_updated_handler, external, false );
  expect_value( local_link_status_updated_handler, link_dpid, 0x5678 );
  expect_value( local_link_status_updated_handler, link_port_no, 72 );
  expect_value( local_link_status_updated_handler, link_up, true );

  // set links
  uint8_t result = set_discovered_link_status( &link_status );
  assert_int_equal( result, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // clean up
  delete_link_to( port );
  delete_link_to( port2 );
  delete_port_entry( sw, port );
  delete_port_entry( sw2, port2 );
  delete_sw_entry( sw );
  delete_sw_entry( sw2 );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );
  assert_true( set_link_status_updated_hook( NULL, NULL ) );
  assert_true( delete_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  finalize_timer();
  finalize_messenger();
}


static void
test_set_discovered_link_status_port_external() {

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );
  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );


  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;

  dpid = 0x5678;
  sw_entry* sw2 = update_sw_entry( &dpid );
  sw2->up = true;

  port_entry* port = update_port_entry( sw, 42, "Some port name1" );
  port->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port->mac[i] = (uint8_t)i;
  }
  port->up = true;
  port->external = false;

  port_entry* port2 = update_port_entry( sw2, 72, "Some port name2" );
  port2->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port2->mac[i] = (uint8_t)i;
  }
  port2->up = true;


  topology_update_link_status link_status;
  link_status.from_dpid = 0x1234;
  link_status.from_portno = 42;
  link_status.to_dpid = 0x5678;
  link_status.to_portno = 72;
  link_status.status = TD_LINK_DOWN;

  // check called handlers
  expect_value( mock_link_status_notification, from_dpid, 0x1234 );
  expect_value( mock_link_status_notification, from_portno, 42 );
  expect_value( mock_link_status_notification, to_dpid, 0x5678 );
  expect_value( mock_link_status_notification, to_portno, 72 );
  expect_value( mock_link_status_notification, status, TD_LINK_DOWN );
  will_return( mock_link_status_notification, false );

  expect_value( mock_port_status_notification, dpid, 0x1234 );
  expect_value( mock_port_status_notification, port_no, 42 );
  expect_string( mock_port_status_notification, name, "Some port name1" );
  expect_memory( mock_port_status_notification, mac, port->mac, ETH_ADDRLEN );
  expect_value( mock_port_status_notification, external, TD_PORT_EXTERNAL );
  expect_value( mock_port_status_notification, status, TD_PORT_UP );
  will_return( mock_port_status_notification, END_ON_RETURN );

  expect_value( local_link_status_updated_handler, user_data, NULL );
  expect_not_value( local_link_status_updated_handler, sw, NULL );
  expect_value( local_link_status_updated_handler, port_no, 42 );
  expect_string( local_link_status_updated_handler, name, "Some port name1" );
  expect_memory( local_link_status_updated_handler, mac, "\0\1\2\3\4\5", ETH_ADDRLEN );
  expect_value( local_link_status_updated_handler, up, true );
  expect_value( local_link_status_updated_handler, external, false );
  expect_value( local_link_status_updated_handler, link_dpid, 0x5678 );
  expect_value( local_link_status_updated_handler, link_port_no, 72 );
  expect_value( local_link_status_updated_handler, link_up, false );

  expect_value( local_port_status_updated_handler, user_data, NULL );
  expect_not_value( local_port_status_updated_handler, sw, NULL );
  expect_value( local_port_status_updated_handler, port_no, 42 );
  expect_string( local_port_status_updated_handler, name, "Some port name1" );
  expect_memory( local_port_status_updated_handler, mac, port->mac, ETH_ADDRLEN );
  expect_value( local_port_status_updated_handler, up, true );
  expect_value( local_port_status_updated_handler, external, true );

  // set links
  uint8_t result = set_discovered_link_status( &link_status );
  assert_int_equal( result, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // clean up
  delete_link_to( port );
  delete_link_to( port2 );
  delete_port_entry( sw, port );
  delete_port_entry( sw2, port2 );
  delete_sw_entry( sw );
  delete_sw_entry( sw2 );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );
  assert_true( set_link_status_updated_hook( NULL, NULL ) );
  assert_true( delete_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  finalize_timer();
  finalize_messenger();
}


static void
test_set_discovered_link_status_linkchange() {

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );
  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );


  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;

  dpid = 0x5678;
  sw_entry* sw2 = update_sw_entry( &dpid );
  sw2->up = true;

  port_entry* port = update_port_entry( sw, 42, "Some port name1" );
  port->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port->mac[i] = (uint8_t)i;
  }
  port->up = true;

  port_entry* port2 = update_port_entry( sw2, 72, "Some port name2" );
  port2->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port2->mac[i] = (uint8_t)i;
  }
  port2->up = true;

  assert_true( update_link_to( port, &dpid, 102, true ) );

  topology_update_link_status link_status;
  link_status.from_dpid = 0x1234;
  link_status.from_portno = 42;
  link_status.to_dpid = 0x5678;
  link_status.to_portno = 72;
  link_status.status = TD_LINK_UP;

  // check called handlers
  expect_value( mock_link_status_notification, from_dpid, 0x1234 );
  expect_value( mock_link_status_notification, from_portno, 42 );
  expect_value( mock_link_status_notification, to_dpid, 0x5678 );
  expect_value( mock_link_status_notification, to_portno, 72 );
  expect_value( mock_link_status_notification, status, TD_LINK_UP );
  will_return( mock_link_status_notification, END_ON_RETURN );

  expect_value( local_link_status_updated_handler, user_data, NULL );
  expect_not_value( local_link_status_updated_handler, sw, NULL );
  expect_value( local_link_status_updated_handler, port_no, 42 );
  expect_string( local_link_status_updated_handler, name, "Some port name1" );
  expect_memory( local_link_status_updated_handler, mac, "\0\1\2\3\4\5", ETH_ADDRLEN );
  expect_value( local_link_status_updated_handler, up, true );
  expect_value( local_link_status_updated_handler, external, false );
  expect_value( local_link_status_updated_handler, link_dpid, 0x5678 );
  expect_value( local_link_status_updated_handler, link_port_no, 72 );
  expect_value( local_link_status_updated_handler, link_up, true );

  // set links
  uint8_t result = set_discovered_link_status( &link_status );
  assert_int_equal( result, TD_RESPONSE_OK );

  start_event_handler();
  start_messenger();

  // clean up
  delete_link_to( port );
  delete_link_to( port2 );
  delete_port_entry( sw, port );
  delete_port_entry( sw2, port2 );
  delete_sw_entry( sw );
  delete_sw_entry( sw2 );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );
  assert_true( set_link_status_updated_hook( NULL, NULL ) );
  assert_true( delete_message_received_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_notification_end ) );

  finalize_timer();
  finalize_messenger();
}


static void
test_set_discovered_link_status_on_down_port_fail() {

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );
  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );


  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;

  dpid = 0x5678;
  sw_entry* sw2 = update_sw_entry( &dpid );
  sw2->up = true;

  port_entry* port = update_port_entry( sw, 42, "Some port name1" );
  port->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port->mac[i] = (uint8_t)i;
  }
  port->up = false;

  port_entry* port2 = update_port_entry( sw2, 72, "Some port name2" );
  port2->external = false;
  for( int i = 0 ; i < ETH_ADDRLEN ; ++i ){
    port2->mac[i] = (uint8_t)i;
  }
  port2->up = true;


  topology_update_link_status link_status;
  link_status.from_dpid = 0x1234;
  link_status.from_portno = 42;
  link_status.to_dpid = 0x5678;
  link_status.to_portno = 72;
  link_status.status = TD_LINK_UP;

  // set links
  uint8_t result = set_discovered_link_status( &link_status );
  // check call to fail
  assert_int_equal( result, TD_RESPONSE_INVALID );

  // clean up
  delete_link_to( port );
  delete_link_to( port2 );
  delete_port_entry( sw, port );
  delete_port_entry( sw2, port2 );
  delete_sw_entry( sw );
  delete_sw_entry( sw2 );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );
  assert_true( set_link_status_updated_hook( NULL, NULL ) );
}


static void
test_set_discovered_link_status_on_invalid_port_fail() {

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );
  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );


  uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;


  topology_update_link_status link_status;
  link_status.from_dpid = 0x1234;
  link_status.from_portno = 42;
  link_status.to_dpid = 0x5678;
  link_status.to_portno = 72;
  link_status.status = TD_LINK_UP;

  // set links
  uint8_t result = set_discovered_link_status( &link_status );
  // check call to fail
  assert_int_equal( result, TD_RESPONSE_INVALID );

  // clean up
  delete_sw_entry( sw );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );
  assert_true( set_link_status_updated_hook( NULL, NULL ) );
}


static void
test_set_discovered_link_status_on_invalid_switch_fail() {

  assert_true( set_port_status_updated_hook( local_port_status_updated_handler, NULL ) );
  assert_true( set_link_status_updated_hook( local_link_status_updated_handler, NULL ) );


  topology_update_link_status link_status;
  link_status.from_dpid = 0x1234;
  link_status.from_portno = 42;
  link_status.to_dpid = 0x5678;
  link_status.to_portno = 72;
  link_status.status = TD_LINK_UP;

  // set link
  uint8_t result = set_discovered_link_status( &link_status );
  // check call to fail
  assert_int_equal( result, TD_RESPONSE_INVALID );

  assert_true( set_port_status_updated_hook( NULL, NULL ) );
  assert_true( set_link_status_updated_hook( NULL, NULL ) );
}


static void
test_recv_update_link_status_request() {
  // avoid periodic ping event from running.
  void ( *original_execute_timer_events )( int *next_timeout_usec );
  swap_original( execute_timer_events );

  swap_original( set_discovered_link_status );

  init_messenger( "/tmp" );
  init_timer();

  assert_true( add_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  start_service_management();

  // send update_link_status request
  buffer *buf = alloc_buffer_with_length( sizeof( topology_update_link_status ) );
  topology_update_link_status *req = append_back_buffer( buf, sizeof( topology_update_link_status ) );
  req->from_dpid = htonll( 0x1234 );
  req->from_portno = htons( 42 );
  req->to_dpid = htonll( 0x5678 );
  req->to_portno = htons( 72 );
  req->status = TD_LINK_UP;

  // check internal API call
  expect_value( mock_set_discovered_link_status, from_dpid, 0x1234 );
  expect_value( mock_set_discovered_link_status, from_portno, 42 );
  expect_value( mock_set_discovered_link_status, to_dpid, 0x5678 );
  expect_value( mock_set_discovered_link_status, to_portno, 72 );
  expect_value( mock_set_discovered_link_status, status, TD_LINK_UP );
  will_return( mock_set_discovered_link_status, TD_RESPONSE_OK );

  expect_value( mock_topology_reply, status, TD_RESPONSE_OK );

  send_request_message( TEST_TOPOLOGY_MESSENGER_NAME, TEST_SUBSCRIBER_NAME, TD_MSGTYPE_UPDATE_LINK_STATUS_REQUEST,
                        buf->data, buf->length, NULL );
  free_buffer( buf );

  start_event_handler();
  start_messenger();

  // clean up
  assert_true( delete_message_replied_callback( TEST_SUBSCRIBER_NAME, callback_fake_libtopology_client_reply_end ) );

  finalize_timer();
  finalize_messenger();

  revert_original( set_discovered_link_status );
  revert_original( execute_timer_events );
}


static void
test_get_topology_messenger_name() {
  set_trema_name( TEST_TREMA_NAME );

  assert_string_equal( get_topology_messenger_name(), TEST_TOPOLOGY_MESSENGER_NAME );
  assert_string_equal( get_topology_messenger_name(), TEST_TOPOLOGY_MESSENGER_NAME );

  _free_trema_name();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test( test_get_topology_messenger_name ),
      unit_test( test_init_finalize_service_management ),

      unit_test_setup_teardown( test_init_start_finalize_service_management, setup_fake_messenger, teardown_fake_messenger ),
      unit_test_setup_teardown( test_start_service_management_fail_on_requested_callback_add_fail, setup_fake_messenger, teardown_fake_messenger ),
      unit_test_setup_teardown( test_start_service_management_fail_on_replied_callback_add_fail, setup_fake_messenger, teardown_fake_messenger ),
      unit_test_setup_teardown( test_start_service_management_fail_on_periodic_event_add_fail, setup_fake_messenger, teardown_fake_messenger ),

      unit_test_setup_teardown( test_notify_switch_status_for_all_user, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_notify_port_status_for_all_user, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_notify_link_status_for_all_user, setup_fake_subscriber, teardown_fake_subscriber ),

      unit_test( test_set_switch_status_updated_hook ),
      unit_test( test_set_port_status_updated_hook ),
      unit_test( test_set_link_status_updated_hook ),

      unit_test_setup_teardown( test_ping_subscriber, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_ping_ageout_subscriber, setup_fake_messenger, teardown_fake_messenger ),

      unit_test_setup_teardown( test_recv_subscribe_from_client, setup_service_management, teardown_service_management ),
      unit_test_setup_teardown( test_recv_subscribe_from_subscribed_client_return_already_subscribed, setup_fake_subscriber, teardown_fake_subscriber ),

      unit_test_setup_teardown( test_recv_unsubscribe_from_client, setup_service_management, teardown_service_management ),
      unit_test_setup_teardown( test_recv_unsubscribe_from_unsubscribed_client_return_no_such_subscriber, setup_service_management, teardown_service_management ),

      unit_test_setup_teardown( test_recv_enable_discovery_from_client, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_recv_enable_discovery_from_client_when_already_enabled, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_recv_enable_discovery_from_unsubscribed_client, setup_service_management, teardown_service_management ),

      unit_test_setup_teardown( test_recv_disable_discovery_from_client, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_recv_disable_discovery_from_unsubscribed_client, setup_service_management, teardown_service_management ),

      unit_test_setup_teardown( test_recv_query_switch_status_from_client, setup_service_management, teardown_service_management ),
      unit_test_setup_teardown( test_recv_query_port_status_from_client, setup_service_management, teardown_service_management ),
      unit_test_setup_teardown( test_recv_query_link_status_from_client, setup_service_management, teardown_service_management ),

      unit_test_setup_teardown( test_set_discovered_link_status, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_set_discovered_link_status_port_external, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_set_discovered_link_status_linkchange, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_set_discovered_link_status_on_down_port_fail, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_set_discovered_link_status_on_invalid_port_fail, setup_fake_subscriber, teardown_fake_subscriber ),
      unit_test_setup_teardown( test_set_discovered_link_status_on_invalid_switch_fail, setup_fake_subscriber, teardown_fake_subscriber ),

      unit_test_setup_teardown( test_recv_update_link_status_request, setup_service_management, teardown_service_management ),

  };

  setup_leak_detector();
  return run_tests( tests );
}

