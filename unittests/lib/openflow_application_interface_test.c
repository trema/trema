/*
 * Unit tests for OpenFlow Application Interface.
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


#include <openflow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "checks.h"
#include "cmockery_trema.h"
#include "hash_table.h"
#include "linked_list.h"
#include "log.h"
#include "messenger.h"
#include "openflow_application_interface.h"
#include "openflow_message.h"
#include "stat.h"
#include "wrapper.h"


/********************************************************************************
 * Helpers.
 ********************************************************************************/

extern bool openflow_application_interface_initialized;
extern openflow_event_handlers_t event_handlers;
extern char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
extern hash_table *stats;

extern void assert_if_not_initialized();
extern void handle_error( const uint64_t datapath_id, buffer *data );
extern void handle_echo_reply( const uint64_t datapath_id, buffer *data );
extern void handle_vendor( const uint64_t datapath_id, buffer *data );
extern void handle_features_reply( const uint64_t datapath_id, buffer *data );
extern void handle_get_config_reply( const uint64_t datapath_id, buffer *data );
extern void handle_packet_in( const uint64_t datapath_id, buffer *data );
extern void handle_flow_removed( const uint64_t datapath_id, buffer *data );
extern void handle_port_status( const uint64_t datapath_id, buffer *data );
extern void handle_stats_reply( const uint64_t datapath_id, buffer *data );
extern void handle_barrier_reply( const uint64_t datapath_id, buffer *data );
extern void handle_queue_get_config_reply( const uint64_t datapath_id, buffer *data );
extern void dump_buf( const buffer *data );
extern void handle_switch_events( uint16_t type, void *data, size_t length );
extern void handle_openflow_message( void *data, size_t length );
extern void handle_message( uint16_t type, void *data, size_t length );
extern void insert_dpid( list_element **head, uint64_t *dpid );
extern void handle_list_switches_reply( uint16_t message_type, void *data, size_t length, void *user_data );


#define SWITCH_READY_HANDLER ( ( void * ) 0x00020001 )
#define SWITCH_READY_USER_DATA ( ( void * ) 0x00020011 )
#define SWITCH_DISCONNECTED_HANDLER ( ( void * ) 0x00020002 )
#define SWITCH_DISCONNECTED_USER_DATA ( ( void * ) 0x00020021 )
#define ERROR_HANDLER ( ( void * ) 0x00010001 )
#define ERROR_USER_DATA ( ( void * ) 0x00010011 )
#define ECHO_REPLY_HANDLER ( ( void * ) 0x00010002 )
#define ECHO_REPLY_USER_DATA ( ( void * ) 0x00010021 )
#define VENDOR_HANDLER ( ( void * ) 0x00010003 )
#define VENDOR_USER_DATA ( ( void * ) 0x00010031 )
#define FEATURES_REPLY_HANDLER ( ( void * ) 0x00010004 )
#define FEATURES_REPLY_USER_DATA ( ( void * ) 0x00010041 )
#define GET_CONFIG_REPLY_HANDLER ( ( void * ) 0x00010005 )
#define GET_CONFIG_REPLY_USER_DATA ( ( void * ) 0x00010051 )
#define PACKET_IN_HANDLER ( ( void * ) 0x00010006 )
#define PACKET_IN_USER_DATA ( ( void * ) 0x00010061 )
#define FLOW_REMOVED_HANDLER ( ( void * ) 0x00010007 )
#define FLOW_REMOVED_USER_DATA ( ( void * ) 0x00010071 )
#define PORT_STATUS_HANDLER ( ( void * ) 0x00010008 )
#define PORT_STATUS_USER_DATA ( ( void * ) 0x00010081 )
#define STATS_REPLY_HANDLER ( ( void * ) 0x00010009 )
#define STATS_REPLY_USER_DATA ( ( void * ) 0x00010091 )
#define BARRIER_REPLY_HANDLER ( ( void * ) 0x0001000a )
#define BARRIER_REPLY_USER_DATA ( ( void * ) 0x000100a1 )
#define QUEUE_GET_CONFIG_REPLY_HANDLER ( ( void * ) 0x0001000b )
#define QUEUE_GET_CONFIG_REPLY_USER_DATA ( ( void * ) 0x000100b1 )
#define LIST_SWITCHES_REPLY_HANDLER ( ( void * ) 0x0001000c )
#define LIST_SWITCHES_REPLY_USER_DATA ( ( void * ) 0x000100c1 )

static const pid_t PID = 12345;
static char SERVICE_NAME[] = "learning switch application 0";
static openflow_event_handlers_t NULL_EVENT_HANDLERS = { false, ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         false, ( void * ) 0, ( void * ) 0,
                                                         false, ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0, ( void * ) 0,
                                                         ( void * ) 0 };
static openflow_event_handlers_t EVENT_HANDLERS = {
  false, SWITCH_READY_HANDLER, SWITCH_READY_USER_DATA,
  SWITCH_DISCONNECTED_HANDLER, SWITCH_DISCONNECTED_USER_DATA,
  ERROR_HANDLER, ERROR_USER_DATA,
  ECHO_REPLY_HANDLER, ECHO_REPLY_USER_DATA,
  VENDOR_HANDLER, VENDOR_USER_DATA,
  FEATURES_REPLY_HANDLER, FEATURES_REPLY_USER_DATA,
  GET_CONFIG_REPLY_HANDLER, GET_CONFIG_REPLY_USER_DATA,
  false, PACKET_IN_HANDLER, PACKET_IN_USER_DATA,
  false, FLOW_REMOVED_HANDLER, FLOW_REMOVED_USER_DATA,
  PORT_STATUS_HANDLER, PORT_STATUS_USER_DATA,
  STATS_REPLY_HANDLER, STATS_REPLY_USER_DATA,
  BARRIER_REPLY_HANDLER, BARRIER_REPLY_USER_DATA,
  QUEUE_GET_CONFIG_REPLY_HANDLER, QUEUE_GET_CONFIG_REPLY_USER_DATA,
  LIST_SWITCHES_REPLY_HANDLER
};
static uint64_t DATAPATH_ID = 0x0102030405060708ULL;
static char REMOTE_SERVICE_NAME[] = "switch.0x102030405060708";
static const uint32_t TRANSACTION_ID = 0x04030201;
static const uint32_t VENDOR_ID = 0xccddeeff;
static const uint8_t MAC_ADDR_X[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 };
static const uint8_t MAC_ADDR_Y[ OFP_ETH_ALEN ] = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d };
static const char *PORT_NAME = "port 1";
static const uint32_t PORT_FEATURES = ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD |
                                        OFPPF_100MB_FD | OFPPF_1GB_HD | OFPPF_1GB_FD |
                                        OFPPF_COPPER |  OFPPF_AUTONEG | OFPPF_PAUSE );
static struct ofp_match MATCH = { OFPFW_ALL, 1,
                                  { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 },
                                  { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d },
                                  1, 1, { 0 }, 0x800, 0xfc, 0x6, { 0, 0 },
                                  0x0a090807, 0x0a090807, 1024, 2048 };
#define USER_DATA_LEN 64
static uint8_t USER_DATA[ USER_DATA_LEN ];


static bool packet_in_handler_called = false;


/********************************************************************************
 * Mocks.
 ********************************************************************************/

const char *
mock_get_trema_name() {
  return "TEST_SERVICE_NAME";
}


pid_t
mock_getpid() {
  return PID;
}


bool
mock_init_openflow_message() {
  return ( bool ) mock();
}


bool
mock_add_message_received_callback( char *service_name,
                                    void ( *callback )( uint16_t tag, void *data, size_t len ) ) {
  check_expected( service_name );
  check_expected( callback );

  return ( bool ) mock();
}


bool
mock_add_message_replied_callback( char *service_name,
                                   void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  check_expected( service_name );
  check_expected( callback );

  return ( bool ) mock();
}


bool
mock_send_message( char *service_name, uint16_t tag, void *data, size_t len ) {
  uint32_t tag32 = tag;

  check_expected( service_name );
  check_expected( tag32 );
  check_expected( data );
  check_expected( len );

  return ( bool ) mock();
}


bool
mock_send_request_message( char *to_service_name, char *from_service_name, uint16_t tag,
                           void *data, size_t len, void *user_data ) {
  uint32_t tag32 = tag;

  check_expected( to_service_name );
  check_expected( from_service_name );
  check_expected( tag32 );
  check_expected( data );
  check_expected( len );
  check_expected( user_data );

  return ( bool ) mock();
}


bool
mock_delete_message_received_callback( char *service_name,
                                       void ( *callback )( uint16_t tag, void *data, size_t len ) ) {
  check_expected( service_name );
  check_expected( callback );

  return ( bool ) mock();
}


bool
mock_delete_message_replied_callback( char *service_name,
                                      void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  check_expected( service_name );
  check_expected( callback );

  return ( bool ) mock();
}


bool
mock_clear_send_queue( const char *service_name ) {
  check_expected( service_name );

  return ( bool ) mock();
}


bool
mock_parse_packet( buffer *buf ) {
  calloc_packet_info( buf );
  return ( bool ) mock();
}


static void
mock_switch_disconnected_handler( uint64_t datapath_id, void *user_data ) {
  check_expected( &datapath_id );
  check_expected( user_data );
}


static void
mock_error_handler( uint64_t datapath_id, uint32_t transaction_id, uint16_t type, uint16_t code,
                    const buffer *data, void *user_data ) {
  uint32_t type32 = type;
  uint32_t code32 = code;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( type32 );
  check_expected( code32 );
  check_expected( data->length );
  check_expected( data->data );
  check_expected( user_data );
}


static void
mock_echo_reply_handler( uint64_t datapath_id, uint32_t transaction_id, const buffer *data,
                         void *user_data ) {
  void *data_uc;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  if ( data != NULL ) {
    check_expected( data->length );
    check_expected( data->data );
  }
  else {
    data_uc = ( void * ) ( unsigned long ) data;
    check_expected( data_uc );
  }
  check_expected( user_data );
}


static void
mock_vendor_handler( uint64_t datapath_id, uint32_t transaction_id, uint32_t vendor,
                     const buffer *data, void *user_data ) {
  void *data_uc;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( vendor );
  if ( data != NULL ) {
    check_expected( data->length );
    check_expected( data->data );
  }
  else {
    data_uc = ( void * ) ( unsigned long ) data;
    check_expected( data_uc );
  }
  check_expected( user_data );
}


static void
mock_features_reply_handler( uint64_t datapath_id, uint32_t transaction_id,
                             uint32_t n_buffers, uint8_t n_tables, uint32_t capabilities,
                             uint32_t actions, const list_element *phy_ports,
                             void *user_data ) {
  uint32_t n_tables32 = n_tables;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( n_buffers );
  check_expected( n_tables32 );
  check_expected( capabilities );
  check_expected( actions );
  if ( phy_ports != NULL ) {
    struct ofp_phy_port *port1 = phy_ports->data;
    struct ofp_phy_port *port2 = phy_ports->next->data;
    check_expected( port1 );
    check_expected( port2 );
  }
  else {
    void *phy_ports_uc = ( void * ) ( unsigned long ) phy_ports;
    check_expected( phy_ports_uc );
  }
  check_expected( user_data );
}


static void
mock_get_config_reply_handler( uint64_t datapath_id, uint32_t transaction_id,
                               uint16_t flags, uint16_t miss_send_len, void *user_data ) {
  uint32_t flags32 = flags;
  uint32_t miss_send_len32 = miss_send_len;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( flags32 );
  check_expected( miss_send_len32 );
  check_expected( user_data );
}


static void
mock_flow_removed_handler( uint64_t datapath_id, uint32_t transaction_id, struct ofp_match match,
                           uint64_t cookie, uint16_t priority, uint8_t reason, uint32_t duration_sec,
                           uint32_t duration_nsec, uint16_t idle_timeout, uint64_t packet_count,
                           uint64_t byte_count, void *user_data ) {
  uint32_t priority32 = priority;
  uint32_t reason32 = reason;
  uint32_t idle_timeout32 = idle_timeout;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( &match );
  check_expected( &cookie );
  check_expected( priority32 );
  check_expected( reason32 );
  check_expected( duration_sec );
  check_expected( duration_nsec );
  check_expected( idle_timeout32 );
  check_expected( &packet_count );
  check_expected( &byte_count );
  check_expected( user_data );
}


static void
mock_port_status_handler( uint64_t datapath_id, uint32_t transaction_id, uint8_t reason,
                          struct ofp_phy_port phy_port, void *user_data ) {
  uint32_t reason32 = reason;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( reason32 );
  check_expected( &phy_port );
  check_expected( user_data );
}


static void
mock_stats_reply_handler( uint64_t datapath_id, uint32_t transaction_id, uint16_t type,
                          uint16_t flags, const buffer *data, void *user_data ) {
  uint32_t type32 = type;
  uint32_t flags32 = flags;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( type32 );
  check_expected( flags32 );
  check_expected( data->length );
  check_expected( data->data );
  check_expected( user_data );
}


static void
mock_barrier_reply_handler( uint64_t datapath_id, uint32_t transaction_id, void *user_data ) {
  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( user_data );
}


static void
mock_queue_get_config_reply_handler( uint64_t datapath_id, uint32_t transaction_id,
                                     uint16_t port, const list_element *queues, void *user_data ) {
  uint32_t port32 = port;
  struct ofp_packet_queue *queue1, *queue2;

  queue1 = queues->data;
  queue2 = queues->next->data;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( port32 );
  check_expected( queue1 );
  check_expected( queue2 );
  check_expected( user_data );
}


static void
mock_handle_list_switches_reply( const list_element *switches, void *user_data ) {
  uint64_t *dpid1, *dpid2, *dpid3;

  if ( switches != NULL ) {
    dpid1 = switches->data;
    check_expected( *dpid1 );
    if ( switches->next != NULL ) {
      dpid2 = switches->next->data;
      check_expected( *dpid2 );
      if ( switches->next->next != NULL ) {
        dpid3 = switches->next->next->data;
        check_expected( *dpid3 );
      }
    }
  }
  check_expected( user_data );
}


void
mock_die( char *format, ... ) {
  check_expected( format );
  mock_assert( false, "die", __FILE__, __LINE__ );
}


void
mock_debug( char *format, ... ) {
  UNUSED( format );
}


void
mock_info( char *format, ... ) {
  UNUSED( format );
}


void
mock_warn( char *format, ... ) {
  UNUSED( format );
}


void
mock_error( char *format, ... ) {
  UNUSED( format );
}


void
mock_critical( char *format, ... ) {
  UNUSED( format );
}


static int
mock_get_logging_level() {
  return LOG_DEBUG;
}


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

static void
cleanup() {
  openflow_application_interface_initialized = false;
  packet_in_handler_called = false;

  memset( service_name, 0, sizeof( service_name ) );
  memset( &event_handlers, 0, sizeof( event_handlers ) );
  memset( USER_DATA, 'Z', sizeof( USER_DATA ) );
  if ( stats != NULL ) {
    delete_hash( stats );
    stats = NULL;
  }
}


static void
init() {
  bool ret;

  get_logging_level = mock_get_logging_level;

  cleanup();

  will_return( mock_init_openflow_message, true );

  expect_string( mock_add_message_received_callback, service_name, SERVICE_NAME );
  expect_value( mock_add_message_received_callback, callback, handle_message );
  will_return( mock_add_message_received_callback, true );

  expect_string( mock_add_message_replied_callback, service_name, SERVICE_NAME );
  expect_value( mock_add_message_replied_callback, callback, handle_list_switches_reply );
  will_return( mock_add_message_replied_callback, true );

  init_stat();

  ret = init_openflow_application_interface( SERVICE_NAME );

  assert_true( ret );
  assert_true( openflow_application_interface_initialized );
  assert_string_equal( service_name, SERVICE_NAME );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * init_openflow_application_interface() tests.
 ********************************************************************************/

static void
test_init_openflow_application_interface_with_valid_custom_service_name() {
  bool ret;

  will_return( mock_init_openflow_message, true );

  expect_string( mock_add_message_received_callback, service_name, SERVICE_NAME );
  expect_value( mock_add_message_received_callback, callback, handle_message );
  will_return( mock_add_message_received_callback, true );

  expect_string( mock_add_message_replied_callback, service_name, SERVICE_NAME );
  expect_value( mock_add_message_replied_callback, callback, handle_list_switches_reply );
  will_return( mock_add_message_replied_callback, true );

  ret = init_openflow_application_interface( SERVICE_NAME );

  assert_true( ret );
  assert_true( openflow_application_interface_initialized );
  assert_string_equal( service_name, SERVICE_NAME );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


static void
test_init_openflow_application_interface_with_too_long_custom_service_name() {
  bool ret;
  char too_long_service_name[ MESSENGER_SERVICE_NAME_LENGTH + 1 ];
  char expected_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];

  memset( too_long_service_name, 'a', sizeof( too_long_service_name ) );
  too_long_service_name[ MESSENGER_SERVICE_NAME_LENGTH ] = '\0';

  memset( expected_service_name, '\0', sizeof( expected_service_name ) );

  ret = init_openflow_application_interface( too_long_service_name );

  assert_true( ret == false );
  assert_true( openflow_application_interface_initialized == false );
  assert_string_equal( service_name, expected_service_name );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


static void
test_init_openflow_application_interface_if_already_initialized() {
  bool ret;

  ret = set_openflow_event_handlers( EVENT_HANDLERS );

  assert_true( ret );
  assert_memory_equal( &event_handlers, &EVENT_HANDLERS, sizeof( event_handlers ) );

  ret = init_openflow_application_interface( SERVICE_NAME );

  assert_true( ret == false );
  assert_true( openflow_application_interface_initialized == true );
  assert_string_equal( service_name, SERVICE_NAME );
  assert_memory_equal( &event_handlers, &EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * init_openflow_application_interface() tests.
 ********************************************************************************/

static void
test_finalize_openflow_application_interface() {
  bool ret;
  char expected_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];

  memset( expected_service_name, '\0', sizeof( expected_service_name ) );

  expect_string( mock_delete_message_received_callback, service_name, SERVICE_NAME );
  expect_value( mock_delete_message_received_callback, callback, handle_message );
  will_return( mock_delete_message_received_callback, true );

  expect_string( mock_delete_message_replied_callback, service_name, SERVICE_NAME );
  expect_value( mock_delete_message_replied_callback, callback, handle_list_switches_reply );
  will_return( mock_delete_message_replied_callback, true );

  ret = finalize_openflow_application_interface();

  assert_true( ret );
  assert_true( openflow_application_interface_initialized == false );
  assert_string_equal( service_name, expected_service_name );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


static void
test_finalize_openflow_application_interface_if_not_initialized() {
  char expected_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];

  memset( expected_service_name, '\0', sizeof( expected_service_name ) );

  expect_assert_failure( finalize_openflow_application_interface() );

  assert_true( openflow_application_interface_initialized == false );
  assert_string_equal( service_name, expected_service_name );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_openflow_event_handlers() tests.
 ********************************************************************************/

static void
test_set_openflow_event_handlers() {
  bool ret;

  ret = set_openflow_event_handlers( EVENT_HANDLERS );

  assert_true( ret );
  assert_memory_equal( &event_handlers, &EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * Switch ready handler tests.
 ********************************************************************************/

static void
mock_switch_ready_handler( uint64_t datapath_id, void *user_data ) {
  check_expected( &datapath_id );
  check_expected( user_data );
}


static void
mock_simple_switch_ready_handler( switch_ready event ) {
  uint64_t datapath_id = event.datapath_id;
  void *user_data = event.user_data;

  check_expected( &datapath_id );
  check_expected( user_data );
}


static void
test_set_switch_ready_handler() {
  char user_data[] = "Ready!";
  set_switch_ready_handler( mock_switch_ready_handler, user_data );
  assert_true( event_handlers.switch_ready_callback == mock_switch_ready_handler );
  assert_string_equal( event_handlers.switch_ready_user_data, user_data );
}


static void
test_set_simple_switch_ready_handler() {
  char user_data[] = "Ready!";
  set_switch_ready_handler( mock_simple_switch_ready_handler, user_data );
  assert_true( event_handlers.switch_ready_callback == mock_simple_switch_ready_handler );
  assert_string_equal( event_handlers.switch_ready_user_data, user_data );
}


static void
test_set_switch_ready_handler_should_die_if_handler_is_NULL() {
  char user_data[] = "Ready!";
  expect_string( mock_die, format, "Invalid callback function for switch_ready event." );
  expect_assert_failure( set_switch_ready_handler( NULL, user_data ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


static void
test_handle_switch_ready() {
  char user_data[] = "Ready!";
  buffer *data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  uint64_t *datapath_id = append_back_buffer( data, sizeof( openflow_service_header_t ) );
  *datapath_id = htonll( DATAPATH_ID );

  expect_memory( mock_switch_ready_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_string( mock_switch_ready_handler, user_data, user_data );

  set_switch_ready_handler( mock_switch_ready_handler, user_data );
  handle_message( MESSENGER_OPENFLOW_READY, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.switch_ready_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.switch_ready_receive_succeeded" ) );
}


static void
test_handle_switch_ready_with_simple_handler() {
  char user_data[] = "Ready!";
  buffer *data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  uint64_t *datapath_id = append_back_buffer( data, sizeof( openflow_service_header_t ) );
  *datapath_id = htonll( DATAPATH_ID );

  expect_memory( mock_simple_switch_ready_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_string( mock_simple_switch_ready_handler, user_data, user_data );

  set_switch_ready_handler( mock_simple_switch_ready_handler, user_data );
  handle_message( MESSENGER_OPENFLOW_READY, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.switch_ready_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.switch_ready_receive_succeeded" ) );
}


/********************************************************************************
 * set_switch_disconnected_handler() tests.
 ********************************************************************************/

static void
test_set_switch_disconnected_handler() {
  assert_true( set_switch_disconnected_handler( SWITCH_DISCONNECTED_HANDLER, SWITCH_DISCONNECTED_USER_DATA ) );
  assert_int_equal( event_handlers.switch_disconnected_callback, SWITCH_DISCONNECTED_HANDLER );
  assert_int_equal( event_handlers.switch_disconnected_user_data, SWITCH_DISCONNECTED_USER_DATA );
}


static void
test_set_switch_disconnected_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( switch_disconnected_handler ) must not be NULL." );
  expect_assert_failure( set_switch_disconnected_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_error_handler() tests.
 ********************************************************************************/

static void
test_set_error_handler() {
  assert_true( set_error_handler( ERROR_HANDLER, ERROR_USER_DATA ) );
  assert_int_equal( event_handlers.error_callback, ERROR_HANDLER );
  assert_int_equal( event_handlers.error_user_data, ERROR_USER_DATA );
}


static void
test_set_error_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( error_handler ) must not be NULL." );
  expect_assert_failure( set_error_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_echo_reply_handler() tests.
 ********************************************************************************/

static void
test_set_echo_reply_handler() {
  assert_true( set_echo_reply_handler( ECHO_REPLY_HANDLER, ECHO_REPLY_USER_DATA ) );
  assert_int_equal( event_handlers.echo_reply_callback, ECHO_REPLY_HANDLER );
  assert_int_equal( event_handlers.echo_reply_user_data, ECHO_REPLY_USER_DATA );
}


static void
test_set_echo_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( echo_reply_handler ) must not be NULL." );
  expect_assert_failure( set_echo_reply_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_vendor_handler() tests.
 ********************************************************************************/

static void
test_set_vendor_handler() {
  assert_true( set_vendor_handler( VENDOR_HANDLER, VENDOR_USER_DATA ) );
  assert_int_equal( event_handlers.vendor_callback, VENDOR_HANDLER );
  assert_int_equal( event_handlers.vendor_user_data, VENDOR_USER_DATA );
}


static void
test_set_vendor_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( vendor_handler ) must not be NULL." );
  expect_assert_failure( set_vendor_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_features_reply_handler() tests.
 ********************************************************************************/

static void
test_set_features_reply_handler() {
  assert_true( set_features_reply_handler( FEATURES_REPLY_HANDLER, FEATURES_REPLY_USER_DATA ) );
  assert_int_equal( event_handlers.features_reply_callback, FEATURES_REPLY_HANDLER );
  assert_int_equal( event_handlers.features_reply_user_data, FEATURES_REPLY_USER_DATA );
}


static void
test_set_features_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( features_reply_handler ) must not be NULL." );
  expect_assert_failure( set_features_reply_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_get_config_reply_handler() tests.
 ********************************************************************************/

static void
test_set_get_config_reply_handler() {
  assert_true( set_get_config_reply_handler( GET_CONFIG_REPLY_HANDLER, GET_CONFIG_REPLY_USER_DATA ) );
  assert_int_equal( event_handlers.get_config_reply_callback, GET_CONFIG_REPLY_HANDLER );
  assert_int_equal( event_handlers.get_config_reply_user_data, GET_CONFIG_REPLY_USER_DATA );
}


static void
test_set_get_config_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( get_config_reply_handler ) must not be NULL." );
  expect_assert_failure( set_get_config_reply_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * Packet in handler tests.
 ********************************************************************************/

static void
mock_packet_in_handler(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t buffer_id,
  uint16_t total_len,
  uint16_t in_port,
  uint8_t reason,
  const buffer *data,
  void *user_data
) {
  uint32_t total_len32 = total_len;
  uint32_t in_port32 = in_port;
  uint32_t reason32 = reason;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( buffer_id );
  check_expected( total_len32 );
  check_expected( in_port32 );
  check_expected( reason32 );
  if ( data != NULL ) {
    check_expected( data->length );
    check_expected( data->data );
  }
  else {
    void *data_uc = ( void * ) ( unsigned long ) data;
    check_expected( data_uc );
  }
  check_expected( user_data );

  packet_in_handler_called = true;
}


static void
mock_simple_packet_in_handler( uint64_t dpid, packet_in event ) {
  uint64_t datapath_id = dpid;
  uint32_t transaction_id = event.transaction_id;
  uint32_t buffer_id = event.buffer_id;
  uint32_t total_len32 = event.total_len;
  uint32_t in_port32 = event.in_port;
  uint32_t reason32 = event.reason;
  const buffer *data = event.data;
  void *user_data = event.user_data;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( buffer_id );
  check_expected( total_len32 );
  check_expected( in_port32 );
  check_expected( reason32 );
  check_expected( data->length );
  check_expected( user_data );

  packet_in_handler_called = true;
}


static void
test_set_packet_in_handler() {
  set_packet_in_handler( mock_packet_in_handler, PACKET_IN_USER_DATA );
  assert_true( event_handlers.packet_in_callback == mock_packet_in_handler );
  assert_true( event_handlers.packet_in_user_data == PACKET_IN_USER_DATA );
}


static void
test_set_simple_packet_in_handler() {
  set_packet_in_handler( mock_simple_packet_in_handler, PACKET_IN_USER_DATA );
  assert_true( event_handlers.packet_in_callback == mock_simple_packet_in_handler );
  assert_true( event_handlers.packet_in_user_data == PACKET_IN_USER_DATA );
}


static void
test_set_packet_in_handler_should_die_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function (packet_in_handler) must not be NULL." );
  expect_assert_failure( set_packet_in_handler( NULL, PACKET_IN_USER_DATA ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


static void
test_handle_packet_in() {
  uint8_t reason = OFPR_NO_MATCH;
  uint16_t in_port = 1;
  uint32_t buffer_id = 0x01020304;
  buffer *data = alloc_buffer_with_length( 64 );
  calloc_packet_info( data );
  append_back_buffer( data, 64 );
  memset( data->data, 0x01, 64 );
  uint16_t total_len = ( uint16_t ) data->length;

  will_return( mock_parse_packet, true );
  expect_memory( mock_packet_in_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_packet_in_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_packet_in_handler, buffer_id, buffer_id );
  expect_value( mock_packet_in_handler, total_len32, ( uint32_t ) total_len );
  expect_value( mock_packet_in_handler, in_port32, ( uint32_t ) in_port );
  expect_value( mock_packet_in_handler, reason32, ( uint32_t ) reason );
  expect_value( mock_packet_in_handler, data->length, data->length );
  expect_memory( mock_packet_in_handler, data->data, data->data, data->length );
  expect_memory( mock_packet_in_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_packet_in_handler( mock_packet_in_handler, USER_DATA );

  buffer *buffer = create_packet_in( TRANSACTION_ID, buffer_id, total_len, in_port, reason, data );
  handle_packet_in( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_packet_in_with_simple_handler() {
  uint8_t reason = OFPR_NO_MATCH;
  uint16_t in_port = 1;
  uint32_t buffer_id = 0x01020304;
  buffer *data = alloc_buffer_with_length( 64 );
  calloc_packet_info( data );
  append_back_buffer( data, 64 );
  memset( data->data, 0x01, 64 );
  uint16_t total_len = ( uint16_t ) data->length;

  will_return( mock_parse_packet, true );
  expect_memory( mock_simple_packet_in_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_simple_packet_in_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_simple_packet_in_handler, buffer_id, buffer_id );
  expect_value( mock_simple_packet_in_handler, total_len32, ( uint32_t ) total_len );
  expect_value( mock_simple_packet_in_handler, in_port32, ( uint32_t ) in_port );
  expect_value( mock_simple_packet_in_handler, reason32, ( uint32_t ) reason );
  expect_value( mock_simple_packet_in_handler, data->length, data->length );
  expect_memory( mock_simple_packet_in_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_packet_in_handler( mock_simple_packet_in_handler, USER_DATA );

  buffer *buffer = create_packet_in( TRANSACTION_ID, buffer_id, total_len, in_port, reason, data );
  handle_packet_in( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_packet_in_with_malformed_packet() {
  uint8_t reason = OFPR_NO_MATCH;
  uint16_t in_port = 1;
  uint32_t buffer_id = 0x01020304;
  buffer *data = alloc_buffer_with_length( 64 );
  calloc_packet_info( data );
  append_back_buffer( data, 64 );
  memset( data->data, 0x01, 64 );
  uint16_t total_len = ( uint16_t ) data->length;
  buffer *buffer = create_packet_in( TRANSACTION_ID, buffer_id, total_len, in_port, reason, data );

  will_return( mock_parse_packet, false );

  set_packet_in_handler( mock_packet_in_handler, USER_DATA );
  handle_packet_in( DATAPATH_ID, buffer );

  assert_false( packet_in_handler_called );

  free_buffer( buffer );
}


static void
test_handle_packet_in_without_data() {
  uint8_t reason = OFPR_NO_MATCH;
  uint16_t in_port = 1;
  uint32_t buffer_id = 0x01020304;
  buffer *data = alloc_buffer_with_length( 64 );
  append_back_buffer( data, 64 );
  memset( data->data, 0x01, 64 );
  uint16_t total_len = ( uint16_t ) data->length;

  expect_memory( mock_packet_in_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_packet_in_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_packet_in_handler, buffer_id, buffer_id );
  expect_value( mock_packet_in_handler, total_len32, ( uint32_t ) total_len );
  expect_value( mock_packet_in_handler, in_port32, ( uint32_t ) in_port );
  expect_value( mock_packet_in_handler, reason32, ( uint32_t ) reason );
  expect_value( mock_packet_in_handler, data_uc, NULL );
  expect_memory( mock_packet_in_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_packet_in_handler( mock_packet_in_handler, USER_DATA );

  buffer *buffer = create_packet_in( TRANSACTION_ID, buffer_id, total_len, in_port, reason, NULL );
  handle_packet_in( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_packet_in_without_handler() {
  uint8_t reason = OFPR_NO_MATCH;
  uint16_t in_port = 1;
  uint32_t buffer_id = 0x01020304;
  buffer *data = alloc_buffer_with_length( 64 );
  append_back_buffer( data, 64 );
  memset( data->data, 0x01, 64 );
  uint16_t total_len = ( uint16_t ) data->length;
  buffer *buffer = create_packet_in( TRANSACTION_ID, buffer_id, total_len, in_port, reason, data );

  handle_packet_in( DATAPATH_ID, buffer );
  assert_false( packet_in_handler_called );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_packet_in_should_die_if_message_is_NULL() {
  expect_string( mock_die, format, "handle_packet_in(): packet_in message should not be empty." );
  set_packet_in_handler( mock_packet_in_handler, USER_DATA );
  expect_assert_failure( handle_packet_in( DATAPATH_ID, NULL ) );
}


static void
test_handle_packet_in_should_die_if_message_length_is_zero() {
  buffer *buffer = alloc_buffer_with_length( 32 );

  expect_string( mock_die, format, "handle_packet_in(): packet_in message should not be empty." );
  set_packet_in_handler( mock_packet_in_handler, USER_DATA );
  expect_assert_failure( handle_packet_in( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * set_flow_removed_handler() tests.
 ********************************************************************************/

static void
mock_simple_flow_removed_handler( uint64_t datapath_id, flow_removed message ) {
  uint32_t transaction_id = message.transaction_id;
  struct ofp_match match = message.match;
  uint64_t cookie = message.cookie;
  uint32_t priority32 = message.priority;
  uint32_t reason32 = message.reason;
  uint32_t duration_sec = message.duration_sec;
  uint32_t duration_nsec = message.duration_nsec;
  uint32_t idle_timeout32 = message.idle_timeout;
  uint64_t packet_count = message.packet_count;
  uint64_t byte_count = message.byte_count;
  void *user_data = message.user_data;

  check_expected( &datapath_id );
  check_expected( transaction_id );
  check_expected( &match );
  check_expected( &cookie );
  check_expected( priority32 );
  check_expected( reason32 );
  check_expected( duration_sec );
  check_expected( duration_nsec );
  check_expected( idle_timeout32 );
  check_expected( &packet_count );
  check_expected( &byte_count );
  check_expected( user_data );
}


static void
test_set_flow_removed_handler() {
  set_flow_removed_handler( mock_flow_removed_handler, FLOW_REMOVED_USER_DATA );
  assert_true( event_handlers.flow_removed_callback == mock_flow_removed_handler );
  assert_true( event_handlers.flow_removed_user_data == FLOW_REMOVED_USER_DATA );
}


static void
test_set_simple_flow_removed_handler() {
  set_flow_removed_handler( mock_simple_flow_removed_handler, FLOW_REMOVED_USER_DATA );
  assert_true( event_handlers.flow_removed_callback == mock_simple_flow_removed_handler );
  assert_true( event_handlers.flow_removed_user_data == FLOW_REMOVED_USER_DATA );
}


static void
test_set_flow_removed_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function (flow_removed_handler) must not be NULL." );
  expect_assert_failure( set_flow_removed_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_port_status_handler() tests.
 ********************************************************************************/

static void
test_set_port_status_handler() {
  assert_true( set_port_status_handler( PORT_STATUS_HANDLER, PORT_STATUS_USER_DATA ) );
  assert_int_equal( event_handlers.port_status_callback, PORT_STATUS_HANDLER );
  assert_int_equal( event_handlers.port_status_user_data, PORT_STATUS_USER_DATA );
}


static void
test_set_port_status_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( port_status_handler ) must not be NULL." );
  expect_assert_failure( set_port_status_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_stats_reply_handler() tests.
 ********************************************************************************/

static void
test_set_stats_reply_handler() {
  assert_true( set_stats_reply_handler( STATS_REPLY_HANDLER, STATS_REPLY_USER_DATA ) );
  assert_int_equal( event_handlers.stats_reply_callback, STATS_REPLY_HANDLER );
  assert_int_equal( event_handlers.stats_reply_user_data, STATS_REPLY_USER_DATA );
}


static void
test_set_stats_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( stats_reply_handler ) must not be NULL." );
  expect_assert_failure( set_stats_reply_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_barrier_reply_handler() tests.
 ********************************************************************************/

static void
test_set_barrier_reply_handler() {
  assert_true( set_barrier_reply_handler( BARRIER_REPLY_HANDLER, BARRIER_REPLY_USER_DATA ) );
  assert_int_equal( event_handlers.barrier_reply_callback, BARRIER_REPLY_HANDLER );
  assert_int_equal( event_handlers.barrier_reply_user_data, BARRIER_REPLY_USER_DATA );
}


static void
test_set_barrier_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( barrier_reply_handler ) must not be NULL." );
  expect_assert_failure( set_barrier_reply_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_queue_get_config_reply_handler() tests.
 ********************************************************************************/

static void
test_set_queue_get_config_reply_handler() {
  assert_true( set_queue_get_config_reply_handler( QUEUE_GET_CONFIG_REPLY_HANDLER, QUEUE_GET_CONFIG_REPLY_USER_DATA ) );
  assert_int_equal( event_handlers.queue_get_config_reply_callback, QUEUE_GET_CONFIG_REPLY_HANDLER );
  assert_int_equal( event_handlers.queue_get_config_reply_user_data, QUEUE_GET_CONFIG_REPLY_USER_DATA );
}


static void
test_set_queue_get_config_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( queue_get_config_reply_handler ) must not be NULL." );
  expect_assert_failure( set_queue_get_config_reply_handler( NULL, NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * set_list_switches_reply_handler() tests.
 ********************************************************************************/


static void
test_set_list_switches_reply_handler() {
  assert_true( set_list_switches_reply_handler( LIST_SWITCHES_REPLY_HANDLER ) );
  assert_int_equal( event_handlers.list_switches_reply_callback, LIST_SWITCHES_REPLY_HANDLER );
}


static void
test_set_list_switches_reply_handler_if_handler_is_NULL() {
  expect_string( mock_die, format, "Callback function ( list_switches_reply_handler ) must not be NULL." );
  expect_assert_failure( set_list_switches_reply_handler( NULL ) );
  assert_memory_equal( &event_handlers, &NULL_EVENT_HANDLERS, sizeof( event_handlers ) );
}


/********************************************************************************
 * send_openflow_message() tests.
 ********************************************************************************/

static void
test_send_openflow_message() {
  void *expected_data;
  bool ret;
  size_t expected_length, header_length;
  buffer *buffer;
  openflow_service_header_t *header;

  buffer = create_hello( TRANSACTION_ID );

  assert_true( buffer != NULL );

  header_length = ( size_t ) ( sizeof( openflow_service_header_t ) +
                               strlen( SERVICE_NAME ) + 1 );
  expected_length = ( size_t ) ( header_length + sizeof( struct ofp_header ) );

  expected_data = xcalloc( 1, expected_length );

  header = expected_data;
  header->datapath_id = htonll( DATAPATH_ID );
  header->service_name_length = htons( ( uint16_t ) ( strlen( SERVICE_NAME ) + 1 ) );

  memcpy( ( char * ) expected_data + sizeof( openflow_service_header_t ),
          SERVICE_NAME, strlen( SERVICE_NAME ) + 1 );
  memcpy( ( char * ) expected_data + header_length, buffer->data, buffer->length );

  expect_string( mock_send_message, service_name, REMOTE_SERVICE_NAME );
  expect_value( mock_send_message, tag32, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message, len, expected_length );
  expect_memory( mock_send_message, data, expected_data, expected_length );
  will_return( mock_send_message, true );

  ret = send_openflow_message( DATAPATH_ID, buffer );

  assert_true( ret );
  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.hello_send_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( buffer );
  xfree( expected_data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.hello_send_succeeded" ) );
}


static void
test_send_openflow_message_if_message_is_NULL() {
  expect_assert_failure( send_openflow_message( DATAPATH_ID, NULL ) );
}


static void
test_send_openflow_message_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 128 );

  assert_true( buffer != NULL );

  expect_assert_failure( send_openflow_message( DATAPATH_ID, NULL ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_error() tests.
 ********************************************************************************/

static void
test_handle_error() {
  buffer *buffer, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );

  buffer = create_error( TRANSACTION_ID, OFPET_HELLO_FAILED, OFPHFC_INCOMPATIBLE, data );

  expect_memory( mock_error_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_error_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_error_handler, type32, OFPET_HELLO_FAILED );
  expect_value( mock_error_handler, code32, OFPHFC_INCOMPATIBLE );
  expect_value( mock_error_handler, data->length, data->length );
  expect_memory( mock_error_handler, data->data, data->data, data->length );
  expect_memory( mock_error_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_error_handler( mock_error_handler, USER_DATA );
  handle_error( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_error_if_handler_is_not_registered() {
  buffer *buffer, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );

  buffer = create_error( TRANSACTION_ID, OFPET_HELLO_FAILED, OFPHFC_INCOMPATIBLE, data );

  // FIXME

  handle_error( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_error_if_message_is_NULL() {
  set_error_handler( mock_error_handler, USER_DATA );
  expect_assert_failure( handle_error( DATAPATH_ID, NULL ) );
}


static void
test_handle_error_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_error_handler( mock_error_handler, USER_DATA );
  expect_assert_failure( handle_error( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_echo_request() tests.
 ********************************************************************************/

static void
test_handle_echo_reply() {
  buffer *buffer, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );

  buffer = create_echo_reply( TRANSACTION_ID, data );

  expect_memory( mock_echo_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_echo_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_echo_reply_handler, data->length, data->length );
  expect_memory( mock_echo_reply_handler, data->data, data->data, data->length );
  expect_memory( mock_echo_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_echo_reply_handler( mock_echo_reply_handler, USER_DATA );
  handle_echo_reply( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_echo_reply_without_data() {
  buffer *buffer;

  buffer = create_echo_reply( TRANSACTION_ID, NULL );

  expect_memory( mock_echo_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_echo_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_echo_reply_handler, data_uc, NULL );
  expect_memory( mock_echo_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_echo_reply_handler( mock_echo_reply_handler, USER_DATA );
  handle_echo_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_echo_reply_if_handler_is_not_registered() {
  buffer *buffer, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );

  buffer = create_echo_reply( TRANSACTION_ID, data );

  // FIXME

  handle_echo_reply( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_echo_reply_if_message_is_NULL() {
  set_echo_reply_handler( mock_echo_reply_handler, USER_DATA );
  expect_assert_failure( handle_echo_reply( DATAPATH_ID, NULL ) );
}


static void
test_handle_echo_reply_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_echo_reply_handler( mock_echo_reply_handler, USER_DATA );
  expect_assert_failure( handle_echo_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_vendor() tests.
 ********************************************************************************/

static void
test_handle_vendor() {
  buffer *buffer, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );

  buffer = create_vendor( TRANSACTION_ID, VENDOR_ID, data );

  expect_memory( mock_vendor_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_vendor_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_vendor_handler, vendor, VENDOR_ID );
  expect_value( mock_vendor_handler, data->length, data->length );
  expect_memory( mock_vendor_handler, data->data, data->data, data->length );
  expect_memory( mock_vendor_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_vendor_handler( mock_vendor_handler, USER_DATA );
  handle_vendor( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_vendor_without_data() {
  buffer *buffer;

  buffer = create_vendor( TRANSACTION_ID, VENDOR_ID, NULL );

  expect_memory( mock_vendor_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_vendor_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_vendor_handler, vendor, VENDOR_ID );
  expect_value( mock_vendor_handler, data_uc, NULL );
  expect_memory( mock_vendor_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_vendor_handler( mock_vendor_handler, USER_DATA );
  handle_vendor( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_vendor_if_handler_is_not_registered() {
  buffer *buffer, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );

  buffer = create_vendor( TRANSACTION_ID, VENDOR_ID, data );

  // FIXME

  handle_vendor( DATAPATH_ID, buffer );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_handle_vendor_if_message_is_NULL() {
  set_vendor_handler( mock_vendor_handler, USER_DATA );
  expect_assert_failure( handle_vendor( DATAPATH_ID, NULL ) );
}


static void
test_handle_vendor_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_vendor_handler( mock_vendor_handler, USER_DATA );
  expect_assert_failure( handle_vendor( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_features_reply() tests.
 ********************************************************************************/

static void
test_handle_features_reply() {
  uint8_t n_tables = 2;
  uint32_t n_buffers = 1024;
  uint32_t capabilities;
  uint32_t actions;
  struct ofp_phy_port *phy_port[ 2 ];
  buffer *buffer;
  list_element *ports;

  capabilities = ( OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS |
                   OFPC_QUEUE_STATS | OFPC_ARP_MATCH_IP );
  actions = ( ( 1 << OFPAT_OUTPUT ) | ( 1 << OFPAT_SET_VLAN_VID ) |
              ( 1 << OFPAT_SET_TP_SRC ) | ( 1 << OFPAT_SET_TP_DST ) );

  phy_port[ 0 ] = xcalloc( 1, sizeof( struct ofp_phy_port ) );
  phy_port[ 1 ] = xcalloc( 1, sizeof( struct ofp_phy_port ) );

  phy_port[ 0 ]->port_no = 1;
  memcpy( phy_port[ 0 ]->hw_addr, MAC_ADDR_X, sizeof( phy_port[ 0 ]->hw_addr ) );
  memset( phy_port[ 0 ]->name, '\0', OFP_MAX_PORT_NAME_LEN );
  memcpy( phy_port[ 0 ]->name, PORT_NAME, strlen( PORT_NAME ) );
  phy_port[ 0 ]->config = OFPPC_PORT_DOWN;
  phy_port[ 0 ]->state = OFPPS_LINK_DOWN;
  phy_port[ 0 ]->curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  phy_port[ 0 ]->advertised = PORT_FEATURES;
  phy_port[ 0 ]->supported = PORT_FEATURES;
  phy_port[ 0 ]->peer = PORT_FEATURES;

  memcpy( phy_port[ 1 ], phy_port [ 0 ], sizeof( struct ofp_phy_port ) );

  phy_port[ 1 ]->port_no = 2;
  memcpy( phy_port[ 1 ]->hw_addr, MAC_ADDR_Y, sizeof( phy_port[ 1 ]->hw_addr ) );

  create_list( &ports );
  append_to_tail( &ports, phy_port[ 0 ] );
  append_to_tail( &ports, phy_port[ 1 ] );

  buffer = create_features_reply( TRANSACTION_ID, DATAPATH_ID, n_buffers, n_tables,
                                  capabilities, actions, ports );

  expect_memory( mock_features_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_features_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_features_reply_handler, n_buffers, n_buffers );
  expect_value( mock_features_reply_handler, n_tables32, ( uint32_t ) n_tables );
  expect_value( mock_features_reply_handler, capabilities, capabilities );
  expect_value( mock_features_reply_handler, actions, actions );
  expect_memory( mock_features_reply_handler, port1, ports->data, sizeof( struct ofp_phy_port ) );
  expect_memory( mock_features_reply_handler, port2, ports->next->data, sizeof( struct ofp_phy_port ) );
  expect_memory( mock_features_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_features_reply_handler( mock_features_reply_handler, USER_DATA );
  handle_features_reply( DATAPATH_ID, buffer );

  xfree( phy_port[ 0 ] );
  xfree( phy_port[ 1 ] );
  delete_list( ports );
  free_buffer( buffer );
}


static void
test_handle_features_reply_without_phy_port() {
  uint8_t n_tables = 2;
  uint32_t n_buffers = 1024;
  uint32_t capabilities;
  uint32_t actions;
  buffer *buffer;

  capabilities = ( OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS |
                   OFPC_QUEUE_STATS | OFPC_ARP_MATCH_IP );
  actions = ( ( 1 << OFPAT_OUTPUT ) | ( 1 << OFPAT_SET_VLAN_VID ) |
              ( 1 << OFPAT_SET_TP_SRC ) | ( 1 << OFPAT_SET_TP_DST ) );

  buffer = create_features_reply( TRANSACTION_ID, DATAPATH_ID, n_buffers, n_tables,
                                  capabilities, actions, NULL );

  expect_memory( mock_features_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_features_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_features_reply_handler, n_buffers, n_buffers );
  expect_value( mock_features_reply_handler, n_tables32, ( uint32_t ) n_tables );
  expect_value( mock_features_reply_handler, capabilities, capabilities );
  expect_value( mock_features_reply_handler, actions, actions );
  expect_value( mock_features_reply_handler, phy_ports_uc, NULL );
  expect_memory( mock_features_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_features_reply_handler( mock_features_reply_handler, USER_DATA );
  handle_features_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_features_reply_if_handler_is_not_registered() {
  uint8_t n_tables = 2;
  uint32_t n_buffers = 1024;
  uint32_t capabilities;
  uint32_t actions;
  struct ofp_phy_port *phy_port[ 2 ];
  buffer *buffer;
  list_element *ports;

  capabilities = ( OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS |
                   OFPC_QUEUE_STATS | OFPC_ARP_MATCH_IP );
  actions = ( ( 1 << OFPAT_OUTPUT ) | ( 1 << OFPAT_SET_VLAN_VID ) |
              ( 1 << OFPAT_SET_TP_SRC ) | ( 1 << OFPAT_SET_TP_DST ) );

  phy_port[ 0 ] = xcalloc( 1, sizeof( struct ofp_phy_port ) );
  phy_port[ 1 ] = xcalloc( 1, sizeof( struct ofp_phy_port ) );

  phy_port[ 0 ]->port_no = 1;
  memcpy( phy_port[ 0 ]->hw_addr, MAC_ADDR_X, sizeof( phy_port[ 0 ]->hw_addr ) );
  memset( phy_port[ 0 ]->name, '\0', OFP_MAX_PORT_NAME_LEN );
  memcpy( phy_port[ 0 ]->name, PORT_NAME, strlen( PORT_NAME ) );
  phy_port[ 0 ]->config = OFPPC_PORT_DOWN;
  phy_port[ 0 ]->state = OFPPS_LINK_DOWN;
  phy_port[ 0 ]->curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  phy_port[ 0 ]->advertised = PORT_FEATURES;
  phy_port[ 0 ]->supported = PORT_FEATURES;
  phy_port[ 0 ]->peer = PORT_FEATURES;

  memcpy( phy_port[ 1 ], phy_port [ 0 ], sizeof( struct ofp_phy_port ) );

  phy_port[ 1 ]->port_no = 2;
  memcpy( phy_port[ 1 ]->hw_addr, MAC_ADDR_Y, sizeof( phy_port[ 1 ]->hw_addr ) );

  create_list( &ports );
  append_to_tail( &ports, phy_port[ 0 ] );
  append_to_tail( &ports, phy_port[ 1 ] );

  buffer = create_features_reply( TRANSACTION_ID, DATAPATH_ID, n_buffers, n_tables,
                                  capabilities, actions, ports );

  // FIXME

  handle_features_reply( DATAPATH_ID, buffer );

  xfree( phy_port[ 0 ] );
  xfree( phy_port[ 1 ] );
  delete_list( ports );
  free_buffer( buffer );
}


static void
test_handle_features_reply_if_message_is_NULL() {
  set_features_reply_handler( mock_features_reply_handler, USER_DATA );
  expect_assert_failure( handle_features_reply( DATAPATH_ID, NULL ) );
}


static void
test_handle_features_reply_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_features_reply_handler( mock_features_reply_handler, USER_DATA );
  expect_assert_failure( handle_features_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_get_config_reply() tests.
 ********************************************************************************/

static void
test_handle_get_config_reply() {
  uint16_t flags = OFPC_FRAG_NORMAL;
  uint16_t miss_send_len = 128;
  buffer *buffer;

  buffer = create_get_config_reply( TRANSACTION_ID, flags, miss_send_len );

  expect_memory( mock_get_config_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_get_config_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_get_config_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_get_config_reply_handler, miss_send_len32, ( uint32_t ) miss_send_len );
  expect_memory( mock_get_config_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_get_config_reply_handler( mock_get_config_reply_handler, USER_DATA );
  handle_get_config_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_get_config_reply_if_handler_is_not_registered() {
  uint16_t flags = OFPC_FRAG_NORMAL;
  uint16_t miss_send_len = 128;
  buffer *buffer;

  buffer = create_get_config_reply( TRANSACTION_ID, flags, miss_send_len );

  // FIXME

  handle_get_config_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_get_config_reply_if_message_is_NULL() {
  set_get_config_reply_handler( mock_get_config_reply_handler, USER_DATA );
  expect_assert_failure( handle_get_config_reply( DATAPATH_ID, NULL ) );
}


static void
test_handle_get_config_reply_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_get_config_reply_handler( mock_get_config_reply_handler, USER_DATA );
  expect_assert_failure( handle_get_config_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_flow_removed() tests.
 ********************************************************************************/

static void
test_handle_flow_removed() {
  uint8_t reason =  OFPRR_IDLE_TIMEOUT;
  uint16_t idle_timeout = 60;
  uint16_t priority = UINT16_MAX;
  uint32_t duration_sec = 180;
  uint32_t duration_nsec = 10000;
  uint64_t cookie = 0x0102030405060708ULL;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 100000;
  buffer *buffer = create_flow_removed(
    TRANSACTION_ID,
    MATCH,
    cookie,
    priority,
    reason,
    duration_sec,
    duration_nsec,
    idle_timeout,
    packet_count,
    byte_count
  );

  expect_memory( mock_flow_removed_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_flow_removed_handler, transaction_id, TRANSACTION_ID );
  expect_memory( mock_flow_removed_handler, &match, &MATCH, sizeof( struct ofp_match ) );
  expect_memory( mock_flow_removed_handler, &cookie, &cookie, sizeof( uint64_t ) );
  expect_value( mock_flow_removed_handler, priority32, ( uint32_t ) priority );
  expect_value( mock_flow_removed_handler, reason32, ( uint32_t ) reason );
  expect_value( mock_flow_removed_handler, duration_sec, duration_sec );
  expect_value( mock_flow_removed_handler, duration_nsec, duration_nsec );
  expect_value( mock_flow_removed_handler, idle_timeout32, ( uint32_t ) idle_timeout );
  expect_memory( mock_flow_removed_handler, &packet_count, &packet_count, sizeof( uint64_t ) );
  expect_memory( mock_flow_removed_handler, &byte_count, &byte_count, sizeof( uint64_t ) );
  expect_memory( mock_flow_removed_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_flow_removed_handler( mock_flow_removed_handler, USER_DATA );
  handle_flow_removed( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_flow_removed_with_simple_handler() {
  uint8_t reason = OFPRR_IDLE_TIMEOUT;
  uint16_t idle_timeout = 60;
  uint16_t priority = UINT16_MAX;
  uint32_t duration_sec = 180;
  uint32_t duration_nsec = 10000;
  uint64_t cookie = 0x0102030405060708ULL;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 100000;
  buffer *buffer = create_flow_removed(
    TRANSACTION_ID,
    MATCH,
    cookie,
    priority,
    reason,
    duration_sec,
    duration_nsec,
    idle_timeout,
    packet_count,
    byte_count
  );

  expect_memory( mock_simple_flow_removed_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_simple_flow_removed_handler, transaction_id, TRANSACTION_ID );
  expect_memory( mock_simple_flow_removed_handler, &match, &MATCH, sizeof( struct ofp_match ) );
  expect_memory( mock_simple_flow_removed_handler, &cookie, &cookie, sizeof( uint64_t ) );
  expect_value( mock_simple_flow_removed_handler, priority32, ( uint32_t ) priority );
  expect_value( mock_simple_flow_removed_handler, reason32, ( uint32_t ) reason );
  expect_value( mock_simple_flow_removed_handler, duration_sec, duration_sec );
  expect_value( mock_simple_flow_removed_handler, duration_nsec, duration_nsec );
  expect_value( mock_simple_flow_removed_handler, idle_timeout32, ( uint32_t ) idle_timeout );
  expect_memory( mock_simple_flow_removed_handler, &packet_count, &packet_count, sizeof( uint64_t ) );
  expect_memory( mock_simple_flow_removed_handler, &byte_count, &byte_count, sizeof( uint64_t ) );
  expect_memory( mock_simple_flow_removed_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_flow_removed_handler( mock_simple_flow_removed_handler, USER_DATA );
  handle_flow_removed( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_flow_removed_if_handler_is_not_registered() {
  uint8_t reason =  OFPRR_IDLE_TIMEOUT;
  uint16_t idle_timeout = 60;
  uint16_t priority = UINT16_MAX;
  uint32_t duration_sec = 180;
  uint32_t duration_nsec = 10000;
  uint64_t cookie = 0x0102030405060708ULL;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 100000;
  buffer *buffer;

  buffer = create_flow_removed( TRANSACTION_ID, MATCH, cookie, priority,
                                reason, duration_sec, duration_nsec,
                                idle_timeout, packet_count, byte_count );

  // FIXME
  handle_flow_removed( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_flow_removed_if_message_is_NULL() {
  set_flow_removed_handler( mock_flow_removed_handler, USER_DATA );
  expect_assert_failure( handle_flow_removed( DATAPATH_ID, NULL ) );
}


static void
test_handle_flow_removed_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_flow_removed_handler( mock_flow_removed_handler, USER_DATA );
  expect_assert_failure( handle_flow_removed( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_port_status() tests.
 ********************************************************************************/

static void
test_handle_port_status() {
  uint8_t reason = OFPPR_MODIFY;
  buffer *buffer;
  struct ofp_phy_port desc;

  desc.port_no = 1;
  memcpy( desc.hw_addr, MAC_ADDR_X, sizeof( desc.hw_addr ) );
  memset( desc.name, '\0', OFP_MAX_PORT_NAME_LEN );
  memcpy( desc.name, PORT_NAME, strlen( PORT_NAME ) );
  desc.config = OFPPC_PORT_DOWN;
  desc.state = OFPPS_LINK_DOWN;
  desc.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  desc.advertised = PORT_FEATURES;
  desc.supported = PORT_FEATURES;
  desc.peer = PORT_FEATURES;

  buffer = create_port_status( TRANSACTION_ID, reason, desc );

  expect_memory( mock_port_status_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_port_status_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_port_status_handler, reason32, ( uint32_t ) reason );
  expect_memory( mock_port_status_handler, &phy_port, &desc, sizeof( struct ofp_phy_port ) );
  expect_memory( mock_port_status_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_port_status_handler( mock_port_status_handler, USER_DATA );
  handle_port_status( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_port_status_if_handler_is_not_registered() {
  uint8_t reason = OFPPR_MODIFY;
  buffer *buffer;
  struct ofp_phy_port desc;

  desc.port_no = 1;
  memcpy( desc.hw_addr, MAC_ADDR_X, sizeof( desc.hw_addr ) );
  memset( desc.name, '\0', OFP_MAX_PORT_NAME_LEN );
  memcpy( desc.name, PORT_NAME, strlen( PORT_NAME ) );
  desc.config = OFPPC_PORT_DOWN;
  desc.state = OFPPS_LINK_DOWN;
  desc.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  desc.advertised = PORT_FEATURES;
  desc.supported = PORT_FEATURES;
  desc.peer = PORT_FEATURES;

  buffer = create_port_status( TRANSACTION_ID, reason, desc );

  // FIXME
  handle_port_status( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_port_status_if_message_is_NULL() {
  set_port_status_handler( mock_port_status_handler, USER_DATA );
  expect_assert_failure( handle_port_status( DATAPATH_ID, NULL ) );
}


static void
test_handle_port_status_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_port_status_handler( mock_port_status_handler, USER_DATA );
  expect_assert_failure( handle_port_status( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_stats_reply() tests.
 ********************************************************************************/

static void
test_handle_stats_reply_if_type_is_OFPST_DESC() {
  char mfr_desc[ DESC_STR_LEN ];
  char hw_desc[ DESC_STR_LEN ];
  char sw_desc[ DESC_STR_LEN ];
  char serial_num[ SERIAL_NUM_LEN ];
  char dp_desc[ DESC_STR_LEN ];
  uint16_t flags = 0;
  uint32_t body_len;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;

  memset( mfr_desc, '\0', DESC_STR_LEN );
  memset( hw_desc, '\0', DESC_STR_LEN );
  memset( sw_desc, '\0', DESC_STR_LEN );
  memset( serial_num, '\0', SERIAL_NUM_LEN );
  memset( dp_desc, '\0', DESC_STR_LEN );
  sprintf( mfr_desc, "NEC Coporation" );
  sprintf( hw_desc, "OpenFlow Switch Hardware" );
  sprintf( sw_desc, "OpenFlow Switch Software" );
  sprintf( serial_num, "123456" );
  sprintf( dp_desc, "Datapath 0" );

  buffer = create_desc_stats_reply( TRANSACTION_ID, flags, mfr_desc, hw_desc,
                                    sw_desc, serial_num, dp_desc );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_DESC );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  expect_memory( mock_stats_reply_handler, data->data, stats_reply->body, body_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_type_is_OFPST_FLOW() {
  void *expected_data;
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  uint32_t body_len;
  buffer *buffer;
  list_element *flow_stats;
  struct ofp_stats_reply *stats_reply;
  struct ofp_flow_stats *stats[ 2 ];
  struct ofp_action_output *action;

  stats_len = offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output );

  stats[ 0 ] = xcalloc( 1, stats_len );
  stats[ 1 ] = xcalloc( 1, stats_len );

  stats[ 0 ]->length = stats_len;
  stats[ 0 ]->table_id = 1;
  stats[ 0 ]->pad = 0;
  stats[ 0 ]->match = MATCH;
  stats[ 0 ]->duration_sec = 60;
  stats[ 0 ]->duration_nsec = 10000;
  stats[ 0 ]->priority = 1024;
  stats[ 0 ]->idle_timeout = 60;
  stats[ 0 ]->hard_timeout = 3600;
  stats[ 0 ]->cookie = 0x0102030405060708ULL;
  stats[ 0 ]->packet_count = 1000;
  stats[ 0 ]->byte_count = 100000;
  action = ( struct ofp_action_output * ) stats[ 0 ]->actions;
  action->type = OFPAT_OUTPUT;
  action->len = 8;
  action->port = 1;
  action->max_len = 2048;

  memcpy( stats[ 1 ], stats[ 0 ], stats_len );
  stats[ 1 ]->cookie = 0x0203040506070809ULL;
  action = ( struct ofp_action_output * ) stats[ 1 ]->actions;
  action->port = 2;

  create_list( &flow_stats );
  append_to_tail( &flow_stats, stats[ 0 ] );
  append_to_tail( &flow_stats, stats[ 1 ] );

  expected_data = xcalloc( 1, ( size_t ) ( stats_len * 2 ) );
  memcpy( expected_data, stats[ 0 ], stats_len );
  memcpy( ( char * ) expected_data + stats_len, stats[ 1 ], stats_len );

  buffer = create_flow_stats_reply( TRANSACTION_ID, flags, flow_stats );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_FLOW );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  expect_memory( mock_stats_reply_handler, data->data, expected_data, stats_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( flow_stats );
  xfree( expected_data );
  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_type_is_OFPST_AGGREGATE() {
  uint16_t flags = 0;
  uint32_t body_len;
  uint32_t flow_count = 1000;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 10000;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;
  struct ofp_aggregate_stats_reply aggregate_stats_reply;

  buffer = create_aggregate_stats_reply( TRANSACTION_ID, flags, packet_count,
                                         byte_count, flow_count );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_AGGREGATE );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  ntoh_aggregate_stats( &aggregate_stats_reply,
                        ( struct ofp_aggregate_stats_reply * ) stats_reply->body );
  expect_memory( mock_stats_reply_handler, data->data, &aggregate_stats_reply, body_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_type_is_OFPST_TABLE() {
  void *expected_data;
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  uint32_t body_len;
  buffer *buffer;
  list_element *table_stats;
  struct ofp_stats_reply *stats_reply;
  struct ofp_table_stats *stats[ 2 ];

  stats_len = sizeof( struct ofp_table_stats );

  stats[ 0 ] = xcalloc( 1, stats_len );
  stats[ 1 ] = xcalloc( 1, stats_len );

  stats[ 0 ]->table_id = 1;
  sprintf( stats[ 0 ]->name, "Table 1" );
  stats[ 0 ]->wildcards =  OFPFW_ALL;
  stats[ 0 ]->max_entries = 10000;
  stats[ 0 ]->active_count = 1000;
  stats[ 0 ]->lookup_count = 100000;
  stats[ 0 ]->matched_count = 10000;

  memcpy( stats[ 1 ], stats[ 0 ], stats_len );
  stats[ 1 ]->table_id = 2;
  sprintf( stats[ 1 ]->name, "Table 2" );

  create_list( &table_stats );
  append_to_tail( &table_stats, stats[ 0 ] );
  append_to_tail( &table_stats, stats[ 1 ] );

  expected_data = xcalloc( 1, ( size_t ) ( stats_len * 2 ) );
  memcpy( expected_data, stats[ 0 ], stats_len );
  memcpy( ( char * ) expected_data + stats_len, stats[ 1 ], stats_len );

  buffer = create_table_stats_reply( TRANSACTION_ID, flags, table_stats );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_TABLE );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  expect_memory( mock_stats_reply_handler, data->data, expected_data, stats_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( table_stats );
  xfree( expected_data );
  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_type_is_OFPST_PORT() {
  void *expected_data;
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  uint32_t body_len;
  buffer *buffer;
  list_element *port_stats;
  struct ofp_stats_reply *stats_reply;
  struct ofp_port_stats *stats[ 2 ];

  stats_len = sizeof( struct ofp_port_stats );

  stats[ 0 ] = xcalloc( 1, stats_len );
  stats[ 1 ] = xcalloc( 1, stats_len );

  stats[ 0 ]->port_no = 1;
  stats[ 0 ]->rx_packets = 10000;
  stats[ 0 ]->tx_packets = 20000;
  stats[ 0 ]->rx_bytes = 30000;
  stats[ 0 ]->tx_bytes = 40000;
  stats[ 0 ]->rx_dropped = 50000;
  stats[ 0 ]->tx_dropped = 60000;
  stats[ 0 ]->rx_errors = 70000;
  stats[ 0 ]->tx_errors = 80000;
  stats[ 0 ]->rx_frame_err = 1;
  stats[ 0 ]->rx_over_err = 2;
  stats[ 0 ]->rx_crc_err = 1;
  stats[ 0 ]->collisions = 3;

  memcpy( stats[ 1 ], stats[ 0 ], stats_len );
  stats[ 1 ]->port_no = 2;

  create_list( &port_stats );
  append_to_tail( &port_stats, stats[ 0 ] );
  append_to_tail( &port_stats, stats[ 1 ] );

  expected_data = xcalloc( 1, ( size_t ) ( stats_len * 2 ) );
  memcpy( expected_data, stats[ 0 ], stats_len );
  memcpy( ( char * ) expected_data + stats_len, stats[ 1 ], stats_len );

  buffer = create_port_stats_reply( TRANSACTION_ID, flags, port_stats );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_PORT );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  expect_memory( mock_stats_reply_handler, data->data, expected_data, stats_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( port_stats );
  xfree( expected_data );
  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_type_is_OFPST_QUEUE() {
  void *expected_data;
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  uint32_t body_len;
  buffer *buffer;
  list_element *queue_stats;
  struct ofp_stats_reply *stats_reply;
  struct ofp_queue_stats *stats[ 2 ];

  stats_len = sizeof( struct ofp_queue_stats );

  stats[ 0 ] = xcalloc( 1, stats_len );
  stats[ 1 ] = xcalloc( 1, stats_len );

  stats[ 0 ]->port_no = 1;
  stats[ 0 ]->queue_id = 2;
  stats[ 0 ]->tx_bytes = 100000;
  stats[ 0 ]->tx_packets = 60000;
  stats[ 0 ]->tx_errors = 80;

  memcpy( stats[ 1 ], stats[ 0 ], stats_len );
  stats[ 1 ]->queue_id = 3;

  create_list( &queue_stats );
  append_to_tail( &queue_stats, stats[ 0 ] );
  append_to_tail( &queue_stats, stats[ 1 ] );

  expected_data = xcalloc( 1, ( size_t ) ( stats_len * 2 ) );
  memcpy( expected_data, stats[ 0 ], stats_len );
  memcpy( ( char * ) expected_data + stats_len, stats[ 1 ], stats_len );

  buffer = create_queue_stats_reply( TRANSACTION_ID, flags, queue_stats );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_QUEUE );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  expect_memory( mock_stats_reply_handler, data->data, expected_data, stats_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( queue_stats );
  xfree( expected_data );
  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_type_is_OFPST_VENDOR() {
  void *expected_data;
  uint16_t flags = 0;
  uint32_t body_len;
  uint32_t vendor = VENDOR_ID;
  buffer *buffer, *body;
  struct ofp_stats_reply *stats_reply;

  body = alloc_buffer_with_length( 128 );
  append_back_buffer( body, 128 );
  memset( body->data, 0xa1, body->length );

  expected_data = xcalloc( 1, ( size_t ) ( body->length + sizeof( uint32_t ) ) );
  memcpy( expected_data, &vendor, sizeof( uint32_t ) );
  memcpy( ( char * ) expected_data + sizeof( uint32_t ), body->data, body->length );

  buffer = create_vendor_stats_reply( TRANSACTION_ID, flags, vendor, body );

  stats_reply = buffer->data;
  body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                            offsetof( struct ofp_stats_reply, body ) );

  expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_stats_reply_handler, type32, OFPST_VENDOR );
  expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
  expect_value( mock_stats_reply_handler, data->length, body_len );
  expect_memory( mock_stats_reply_handler, data->data, expected_data, body_len );
  expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  handle_stats_reply( DATAPATH_ID, buffer );

  xfree( expected_data );
  free_buffer( body );
  free_buffer( buffer );
}


static void
test_handle_stats_reply_with_undefined_type() {
  uint16_t flags = 0;
  uint32_t vendor = VENDOR_ID;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;

  buffer = create_vendor_stats_reply( TRANSACTION_ID, flags, vendor, NULL );

  stats_reply = buffer->data;
  stats_reply->type = htons( 0xfffe );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  expect_assert_failure( handle_stats_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_handler_is_not_registered() {
  char mfr_desc[ DESC_STR_LEN ];
  char hw_desc[ DESC_STR_LEN ];
  char sw_desc[ DESC_STR_LEN ];
  char serial_num[ SERIAL_NUM_LEN ];
  char dp_desc[ DESC_STR_LEN ];
  uint16_t flags = 0;
  buffer *buffer;

  memset( mfr_desc, '\0', DESC_STR_LEN );
  memset( hw_desc, '\0', DESC_STR_LEN );
  memset( sw_desc, '\0', DESC_STR_LEN );
  memset( serial_num, '\0', SERIAL_NUM_LEN );
  memset( dp_desc, '\0', DESC_STR_LEN );
  sprintf( mfr_desc, "NEC Coporation" );
  sprintf( hw_desc, "OpenFlow Switch Hardware" );
  sprintf( sw_desc, "OpenFlow Switch Software" );
  sprintf( serial_num, "123456" );
  sprintf( dp_desc, "Datapath 0" );

  buffer = create_desc_stats_reply( TRANSACTION_ID, flags, mfr_desc, hw_desc,
                                    sw_desc, serial_num, dp_desc );

  // FIXME
  handle_stats_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_stats_reply_if_message_is_NULL() {
  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  expect_assert_failure( handle_stats_reply( DATAPATH_ID, NULL ) );
}


static void
test_handle_stats_reply_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
  expect_assert_failure( handle_stats_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_barrier_reply() tests.
 ********************************************************************************/

static void
test_handle_barrier_reply() {
  buffer *buffer;

  buffer = create_barrier_reply( TRANSACTION_ID );

  expect_memory( mock_barrier_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_barrier_reply_handler, transaction_id, TRANSACTION_ID );
  expect_memory( mock_barrier_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_barrier_reply_handler( mock_barrier_reply_handler, USER_DATA );
  handle_barrier_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_barrier_reply_if_handler_is_not_registered() {
  buffer *buffer;

  buffer = create_barrier_reply( TRANSACTION_ID );

  // FIXME
  handle_barrier_reply( DATAPATH_ID, buffer );

  free_buffer( buffer );
}


static void
test_handle_barrier_reply_if_message_is_NULL() {
  set_barrier_reply_handler( mock_barrier_reply_handler, USER_DATA );
  expect_assert_failure( handle_barrier_reply( DATAPATH_ID, NULL ) );
}


static void
test_handle_barrier_reply_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_barrier_reply_handler( mock_barrier_reply_handler, USER_DATA );
  expect_assert_failure( handle_barrier_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_queue_get_config_reply() tests.
 ********************************************************************************/

static void
test_handle_queue_get_config_reply() {
  size_t queue_len;
  uint16_t port = 1;
  list_element *queues;
  buffer *buffer;
  struct ofp_packet_queue *queue[ 2 ];
  struct ofp_queue_prop_header *prop_header;

  queue_len = offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header );
  queue[ 0 ] = xcalloc( 1, queue_len );
  queue[ 1 ] = xcalloc( 1, queue_len );

  queue[ 0 ]->queue_id = 1;
  queue[ 0 ]->len = 16;
  prop_header = queue[ 0 ]->properties;
  prop_header->property = OFPQT_NONE;
  prop_header->len = 8;

  queue[ 1 ]->queue_id = 2;
  queue[ 1 ]->len = 16;
  prop_header = queue[ 1 ]->properties;
  prop_header->property = OFPQT_NONE;
  prop_header->len = 8;

  create_list( &queues );
  append_to_tail( &queues, queue[ 0 ] );
  append_to_tail( &queues, queue[ 1 ] );

  buffer = create_queue_get_config_reply( TRANSACTION_ID, port, queues );

  expect_memory( mock_queue_get_config_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_queue_get_config_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_queue_get_config_reply_handler, port32, ( uint32_t ) port );
  expect_memory( mock_queue_get_config_reply_handler, queue1, queue[ 0 ], queue_len );
  expect_memory( mock_queue_get_config_reply_handler, queue2, queue[ 1 ], queue_len );
  expect_memory( mock_queue_get_config_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

  set_queue_get_config_reply_handler( mock_queue_get_config_reply_handler, USER_DATA );
  handle_queue_get_config_reply( DATAPATH_ID, buffer );

  xfree( queue[ 0 ] );
  xfree( queue[ 1 ] );
  delete_list( queues );
  free_buffer( buffer );
}


static void
test_handle_queue_get_config_reply_without_queues() {
  uint16_t port = 1;
  buffer *buffer;

  buffer = create_queue_get_config_reply( TRANSACTION_ID, port, NULL );

  set_queue_get_config_reply_handler( mock_queue_get_config_reply_handler, USER_DATA );
  expect_assert_failure( handle_queue_get_config_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


static void
test_handle_queue_get_config_reply_if_handler_is_not_registered() {
  size_t queue_len;
  uint16_t port = 1;
  list_element *queues;
  buffer *buffer;
  struct ofp_packet_queue *queue[ 2 ];
  struct ofp_queue_prop_header *prop_header;

  queue_len = offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header );
  queue[ 0 ] = xcalloc( 1, queue_len );
  queue[ 1 ] = xcalloc( 1, queue_len );

  queue[ 0 ]->queue_id = 1;
  queue[ 0 ]->len = 16;
  prop_header = queue[ 0 ]->properties;
  prop_header->property = OFPQT_NONE;
  prop_header->len = 8;

  queue[ 1 ]->queue_id = 2;
  queue[ 1 ]->len = 16;
  prop_header = queue[ 1 ]->properties;
  prop_header->property = OFPQT_NONE;
  prop_header->len = 8;

  create_list( &queues );
  append_to_tail( &queues, queue[ 0 ] );
  append_to_tail( &queues, queue[ 1 ] );

  buffer = create_queue_get_config_reply( TRANSACTION_ID, port, queues );

  // FIXME
  handle_queue_get_config_reply( DATAPATH_ID, buffer );

  xfree( queue[ 0 ] );
  xfree( queue[ 1 ] );
  delete_list( queues );
  free_buffer( buffer );
}


static void
test_handle_queue_get_config_reply_if_message_is_NULL() {
  set_queue_get_config_reply_handler( mock_queue_get_config_reply_handler, USER_DATA );
  expect_assert_failure( handle_queue_get_config_reply( DATAPATH_ID, NULL ) );
}


static void
test_handle_queue_get_config_reply_if_message_length_is_zero() {
  buffer *buffer;

  buffer = alloc_buffer_with_length( 32 );

  set_queue_get_config_reply_handler( mock_queue_get_config_reply_handler, USER_DATA );
  expect_assert_failure( handle_queue_get_config_reply( DATAPATH_ID, buffer ) );

  free_buffer( buffer );
}


/********************************************************************************
 * handle_list_switches_reply() tests.
 ********************************************************************************/

static void
test_insert_dpid() {
  list_element *head;
  create_list( &head );
  uint64_t alice = 0x1;
  uint64_t bob = 0x2;
  uint64_t carol = 0x3;

  insert_dpid( &head, &carol );
  insert_dpid( &head, &alice );
  insert_dpid( &head, &bob );

  list_element *element = head;
  assert_true( element != NULL );
  assert_true( element->data != NULL );
  assert_true( alice == *( uint64_t * ) element->data );

  element = element->next;
  assert_true( element != NULL );
  assert_true( element->data != NULL );
  assert_true( bob == *( uint64_t * ) element->data );

  element = element->next;
  assert_true( element != NULL );
  assert_true( element->data != NULL );
  assert_true( carol == *( uint64_t * ) element->data );

  element = element->next;
  assert_true( element == NULL );

  delete_list( head );
}


static void
test_insert_dpid_if_head_is_NULL() {
  uint64_t dpid = 0x1;

  expect_assert_failure( insert_dpid( NULL, &dpid ) );
}


static void
test_insert_dpid_if_dpid_is_NULL() {
  list_element *head;
  create_list( &head );

  expect_assert_failure( insert_dpid( &head, NULL ) );

  delete_list( head );
}


static void
test_handle_list_switches_reply() {
  uint16_t message_type = 0;
  uint64_t alice = 0x1;
  uint64_t bob = 0x2;
  uint64_t carol = 0x3;
  uint64_t dpid[] = { htonll( bob ), htonll( carol ), htonll( alice ) };
  size_t length = sizeof( dpid );
  void *user_data = LIST_SWITCHES_REPLY_USER_DATA;

  expect_value( mock_handle_list_switches_reply, *dpid1, alice );
  expect_value( mock_handle_list_switches_reply, *dpid2, bob );
  expect_value( mock_handle_list_switches_reply, *dpid3, carol );
  expect_value( mock_handle_list_switches_reply, user_data, LIST_SWITCHES_REPLY_USER_DATA );

  set_list_switches_reply_handler( mock_handle_list_switches_reply );
  handle_list_switches_reply( message_type, dpid, length, user_data );
}


static void
test_handle_list_switches_reply_if_data_is_NULL() {
  uint16_t message_type = 0;
  size_t length = 64;
  void *user_data = LIST_SWITCHES_REPLY_USER_DATA;

  set_list_switches_reply_handler( mock_handle_list_switches_reply );
  expect_assert_failure( handle_list_switches_reply( message_type, NULL, length, user_data ) );
}


static void
test_handle_list_switches_reply_if_length_is_zero() {
  uint16_t message_type = 0;
  uint64_t dpid[] = { 0 };
  void *user_data = LIST_SWITCHES_REPLY_USER_DATA;

  expect_value( mock_handle_list_switches_reply, user_data, LIST_SWITCHES_REPLY_USER_DATA );

  set_list_switches_reply_handler( mock_handle_list_switches_reply );
  handle_list_switches_reply( message_type, dpid, 0, user_data );
}


/********************************************************************************
 * handle_switch_events() tests.
 ********************************************************************************/

static void
test_handle_switch_events_if_type_is_MESSENGER_OPENFLOW_CONNECTED() {
  buffer *data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  append_back_buffer( data, sizeof( openflow_service_header_t ) );

  handle_switch_events( MESSENGER_OPENFLOW_CONNECTED, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.switch_connected_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.switch_connected_receive_succeeded" ) );
}


static void
test_handle_switch_events_if_type_is_MESSENGER_OPENFLOW_DISCONNECTED() {
  uint64_t *datapath_id;
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  datapath_id = append_back_buffer( data, sizeof( openflow_service_header_t ) );

  *datapath_id = htonll( DATAPATH_ID );

  expect_memory( mock_switch_disconnected_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_switch_disconnected_handler, user_data, SWITCH_DISCONNECTED_USER_DATA );

  expect_string( mock_clear_send_queue, service_name, REMOTE_SERVICE_NAME );
  will_return( mock_clear_send_queue, true );

  set_switch_disconnected_handler( mock_switch_disconnected_handler, SWITCH_DISCONNECTED_USER_DATA );
  handle_switch_events( MESSENGER_OPENFLOW_DISCONNECTED, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.switch_disconnected_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.switch_disconnected_receive_succeeded" ) );
}


static void
test_handle_switch_events_if_message_is_NULL() {
  expect_assert_failure( handle_switch_events( MESSENGER_OPENFLOW_READY, NULL, 1 ) );
}


static void
test_handle_switch_events_if_message_length_is_zero() {
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );

  expect_assert_failure( handle_switch_events( MESSENGER_OPENFLOW_READY, data->data, 0 ) );

  free_buffer( data );
}


static void
test_handle_switch_events_if_message_length_is_too_big() {
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );

  expect_assert_failure( handle_switch_events( MESSENGER_OPENFLOW_READY, data->data,
                                               data->length + 1 ) );

  free_buffer( data );
}


static void
test_handle_switch_events_if_unhandled_message_type() {
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  append_back_buffer( data, sizeof( openflow_service_header_t ) );

  // FIXME
  handle_switch_events( MESSENGER_OPENFLOW_MESSAGE, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.undefined_switch_event_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.undefined_switch_event_receive_succeeded" ) );
}


/********************************************************************************
 * handle_openflow_message() tests.
 ********************************************************************************/

static void
test_handle_openflow_message() {
  openflow_service_header_t messenger_header;
  stat_entry *stat;

  messenger_header.datapath_id = htonll( DATAPATH_ID );
  messenger_header.service_name_length = 0;

  // error
  {
    buffer *buffer, *data;

    data = alloc_buffer_with_length( 16 );
    append_back_buffer( data, 16 );
    memset( data->data, 'a', 16 );

    buffer = create_error( TRANSACTION_ID, OFPET_HELLO_FAILED, OFPHFC_INCOMPATIBLE, data );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );
    expect_memory( mock_error_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_error_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_error_handler, type32, OFPET_HELLO_FAILED );
    expect_value( mock_error_handler, code32, OFPHFC_INCOMPATIBLE );
    expect_value( mock_error_handler, data->length, data->length );
    expect_memory( mock_error_handler, data->data, data->data, data->length );
    expect_memory( mock_error_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_error_handler( mock_error_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.error_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( data );
    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.error_receive_succeeded" ) );
  }

  // vendor
  {
    buffer *buffer, *data;

    data = alloc_buffer_with_length( 16 );
    append_back_buffer( data, 16 );
    memset( data->data, 'a', 16 );

    buffer = create_vendor( TRANSACTION_ID, VENDOR_ID, data );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_vendor_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_vendor_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_vendor_handler, vendor, VENDOR_ID );
    expect_value( mock_vendor_handler, data->length, data->length );
    expect_memory( mock_vendor_handler, data->data, data->data, data->length );
    expect_memory( mock_vendor_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_vendor_handler( mock_vendor_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.vendor_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( data );
    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.vendor_receive_succeeded" ) );
  }

  // features_reply
  {
    uint8_t n_tables = 2;
    uint32_t n_buffers = 1024;
    uint32_t capabilities;
    uint32_t actions;
    struct ofp_phy_port *phy_port[ 2 ];
    buffer *buffer;
    list_element *ports;

    capabilities = ( OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS |
                     OFPC_QUEUE_STATS | OFPC_ARP_MATCH_IP );
    actions = ( ( 1 << OFPAT_OUTPUT ) | ( 1 << OFPAT_SET_VLAN_VID ) |
                ( 1 << OFPAT_SET_TP_SRC ) | ( 1 << OFPAT_SET_TP_DST ) );

    phy_port[ 0 ] = xcalloc( 1, sizeof( struct ofp_phy_port ) );
    phy_port[ 1 ] = xcalloc( 1, sizeof( struct ofp_phy_port ) );

    phy_port[ 0 ]->port_no = 1;
    memcpy( phy_port[ 0 ]->hw_addr, MAC_ADDR_X, sizeof( phy_port[ 0 ]->hw_addr ) );
    memset( phy_port[ 0 ]->name, '\0', OFP_MAX_PORT_NAME_LEN );
    memcpy( phy_port[ 0 ]->name, PORT_NAME, strlen( PORT_NAME ) );
    phy_port[ 0 ]->config = OFPPC_PORT_DOWN;
    phy_port[ 0 ]->state = OFPPS_LINK_DOWN;
    phy_port[ 0 ]->curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
    phy_port[ 0 ]->advertised = PORT_FEATURES;
    phy_port[ 0 ]->supported = PORT_FEATURES;
    phy_port[ 0 ]->peer = PORT_FEATURES;

    memcpy( phy_port[ 1 ], phy_port [ 0 ], sizeof( struct ofp_phy_port ) );

    phy_port[ 1 ]->port_no = 2;
    memcpy( phy_port[ 1 ]->hw_addr, MAC_ADDR_Y, sizeof( phy_port[ 1 ]->hw_addr ) );

    create_list( &ports );
    append_to_tail( &ports, phy_port[ 0 ] );
    append_to_tail( &ports, phy_port[ 1 ] );

    buffer = create_features_reply( TRANSACTION_ID, DATAPATH_ID, n_buffers, n_tables,
                                    capabilities, actions, ports );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_features_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_features_reply_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_features_reply_handler, n_buffers, n_buffers );
    expect_value( mock_features_reply_handler, n_tables32, ( uint32_t ) n_tables );
    expect_value( mock_features_reply_handler, capabilities, capabilities );
    expect_value( mock_features_reply_handler, actions, actions );
    expect_memory( mock_features_reply_handler, port1, ports->data, sizeof( struct ofp_phy_port ) );
    expect_memory( mock_features_reply_handler, port2, ports->next->data, sizeof( struct ofp_phy_port ) );
    expect_memory( mock_features_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_features_reply_handler( mock_features_reply_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.features_reply_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    xfree( phy_port[ 0 ] );
    xfree( phy_port[ 1 ] );
    delete_list( ports );
    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.features_reply_receive_succeeded" ) );
  }

  // get_config_reply
  {
    uint16_t flags = OFPC_FRAG_NORMAL;
    uint16_t miss_send_len = 128;
    buffer *buffer;

    buffer = create_get_config_reply( TRANSACTION_ID, flags, miss_send_len );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_get_config_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_get_config_reply_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_get_config_reply_handler, flags32, ( uint32_t ) flags );
    expect_value( mock_get_config_reply_handler, miss_send_len32, ( uint32_t ) miss_send_len );
    expect_memory( mock_get_config_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_get_config_reply_handler( mock_get_config_reply_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.get_config_reply_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.get_config_reply_receive_succeeded" ) );
  }

  // packet_in
  {
    uint8_t reason =  OFPR_NO_MATCH;
    uint16_t total_len;
    uint16_t in_port = 1;
    uint32_t buffer_id = 0x01020304;
    buffer *buffer, *data;

    data = alloc_buffer_with_length( 64 );
    append_back_buffer( data, 64 );
    memset( data->data, 0x01, 64 );

    total_len = ( uint16_t ) data->length;

    buffer = create_packet_in( TRANSACTION_ID, buffer_id, total_len, in_port,
                               reason, data );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    will_return( mock_parse_packet, true );

    expect_memory( mock_packet_in_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_packet_in_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_packet_in_handler, buffer_id, buffer_id );
    expect_value( mock_packet_in_handler, total_len32, ( uint32_t ) total_len );
    expect_value( mock_packet_in_handler, in_port32, ( uint32_t ) in_port );
    expect_value( mock_packet_in_handler, reason32, ( uint32_t ) reason );
    expect_value( mock_packet_in_handler, data->length, data->length );
    expect_memory( mock_packet_in_handler, data->data, data->data, data->length );
    expect_memory( mock_packet_in_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_packet_in_handler( mock_packet_in_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.packet_in_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( data );
    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.packet_in_receive_succeeded" ) );
  }

  // flow_removed
  {
    uint8_t reason =  OFPRR_IDLE_TIMEOUT;
    uint16_t idle_timeout = 60;
    uint16_t priority = UINT16_MAX;
    uint32_t duration_sec = 180;
    uint32_t duration_nsec = 10000;
    uint64_t cookie = 0x0102030405060708ULL;
    uint64_t packet_count = 1000;
    uint64_t byte_count = 100000;
    buffer *buffer;

    buffer = create_flow_removed( TRANSACTION_ID, MATCH, cookie, priority,
                                  reason, duration_sec, duration_nsec,
                                  idle_timeout, packet_count, byte_count );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_flow_removed_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_flow_removed_handler, transaction_id, TRANSACTION_ID );
    expect_memory( mock_flow_removed_handler, &match, &MATCH, sizeof( struct ofp_match ) );
    expect_memory( mock_flow_removed_handler, &cookie, &cookie, sizeof( uint64_t ) );
    expect_value( mock_flow_removed_handler, priority32, ( uint32_t ) priority );
    expect_value( mock_flow_removed_handler, reason32, ( uint32_t ) reason );
    expect_value( mock_flow_removed_handler, duration_sec, duration_sec );
    expect_value( mock_flow_removed_handler, duration_nsec, duration_nsec );
    expect_value( mock_flow_removed_handler, idle_timeout32, ( uint32_t ) idle_timeout );
    expect_memory( mock_flow_removed_handler, &packet_count, &packet_count, sizeof( uint64_t ) );
    expect_memory( mock_flow_removed_handler, &byte_count, &byte_count, sizeof( uint64_t ) );
    expect_memory( mock_flow_removed_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_flow_removed_handler( mock_flow_removed_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.flow_removed_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.flow_removed_receive_succeeded" ) );
  }

  // port_status
  {
    uint8_t reason = OFPPR_MODIFY;
    buffer *buffer;
    struct ofp_phy_port desc;

    desc.port_no = 1;
    memcpy( desc.hw_addr, MAC_ADDR_X, sizeof( desc.hw_addr ) );
    memset( desc.name, '\0', OFP_MAX_PORT_NAME_LEN );
    memcpy( desc.name, PORT_NAME, strlen( PORT_NAME ) );
    desc.config = OFPPC_PORT_DOWN;
    desc.state = OFPPS_LINK_DOWN;
    desc.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
    desc.advertised = PORT_FEATURES;
    desc.supported = PORT_FEATURES;
    desc.peer = PORT_FEATURES;

    buffer = create_port_status( TRANSACTION_ID, reason, desc );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_port_status_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_port_status_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_port_status_handler, reason32, ( uint32_t ) reason );
    expect_memory( mock_port_status_handler, &phy_port, &desc, sizeof( struct ofp_phy_port ) );
    expect_memory( mock_port_status_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_port_status_handler( mock_port_status_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.port_status_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.port_status_receive_succeeded" ) );
  }

  // stats_reply
  {
    char mfr_desc[ DESC_STR_LEN ];
    char hw_desc[ DESC_STR_LEN ];
    char sw_desc[ DESC_STR_LEN ];
    char serial_num[ SERIAL_NUM_LEN ];
    char dp_desc[ DESC_STR_LEN ];
    uint16_t flags = 0;
    uint32_t body_len;
    buffer *buffer;
    struct ofp_stats_reply *stats_reply;

    memset( mfr_desc, '\0', DESC_STR_LEN );
    memset( hw_desc, '\0', DESC_STR_LEN );
    memset( sw_desc, '\0', DESC_STR_LEN );
    memset( serial_num, '\0', SERIAL_NUM_LEN );
    memset( dp_desc, '\0', DESC_STR_LEN );
    sprintf( mfr_desc, "NEC Coporation" );
    sprintf( hw_desc, "OpenFlow Switch Hardware" );
    sprintf( sw_desc, "OpenFlow Switch Software" );
    sprintf( serial_num, "123456" );
    sprintf( dp_desc, "Datapath 0" );

    buffer = create_desc_stats_reply( TRANSACTION_ID, flags, mfr_desc, hw_desc,
                                      sw_desc, serial_num, dp_desc );

    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    stats_reply = ( struct ofp_stats_reply * ) ( ( char * ) buffer->data +
                                                 sizeof( openflow_service_header_t ) );
    body_len = ( uint32_t ) ( ntohs( stats_reply->header.length ) -
                              offsetof( struct ofp_stats_reply, body ) );


    expect_memory( mock_stats_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_stats_reply_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_stats_reply_handler, type32, OFPST_DESC );
    expect_value( mock_stats_reply_handler, flags32, ( uint32_t ) flags );
    expect_value( mock_stats_reply_handler, data->length, body_len );
    expect_memory( mock_stats_reply_handler, data->data, stats_reply->body, body_len );
    expect_memory( mock_stats_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_stats_reply_handler( mock_stats_reply_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.stats_reply_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.stats_reply_receive_succeeded" ) );
  }

  // barrier_reply
  {
    buffer *buffer;

    buffer = create_barrier_reply( TRANSACTION_ID );
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_barrier_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_barrier_reply_handler, transaction_id, TRANSACTION_ID );
    expect_memory( mock_barrier_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_barrier_reply_handler( mock_barrier_reply_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.barrier_reply_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.barrier_reply_receive_succeeded" ) );
  }

  // queue_get_config_reply
  {
    size_t queue_len;
    uint16_t port = 1;
    list_element *queues;
    buffer *buffer;
    struct ofp_packet_queue *queue[ 2 ];
    struct ofp_queue_prop_header *prop_header;

    queue_len = offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header );
    queue[ 0 ] = xcalloc( 1, queue_len );
    queue[ 1 ] = xcalloc( 1, queue_len );

    queue[ 0 ]->queue_id = 1;
    queue[ 0 ]->len = 16;
    prop_header = queue[ 0 ]->properties;
    prop_header->property = OFPQT_NONE;
    prop_header->len = 8;

    queue[ 1 ]->queue_id = 2;
    queue[ 1 ]->len = 16;
    prop_header = queue[ 1 ]->properties;
    prop_header->property = OFPQT_NONE;
    prop_header->len = 8;

    create_list( &queues );
    append_to_tail( &queues, queue[ 0 ] );
    append_to_tail( &queues, queue[ 1 ] );

    buffer = create_queue_get_config_reply( TRANSACTION_ID, port, queues );

    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    expect_memory( mock_queue_get_config_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
    expect_value( mock_queue_get_config_reply_handler, transaction_id, TRANSACTION_ID );
    expect_value( mock_queue_get_config_reply_handler, port32, ( uint32_t ) port );
    expect_memory( mock_queue_get_config_reply_handler, queue1, queue[ 0 ], queue_len );
    expect_memory( mock_queue_get_config_reply_handler, queue2, queue[ 1 ], queue_len );
    expect_memory( mock_queue_get_config_reply_handler, user_data, USER_DATA, USER_DATA_LEN );

    set_queue_get_config_reply_handler( mock_queue_get_config_reply_handler, USER_DATA );
    handle_openflow_message( buffer->data, buffer->length );

    stat = lookup_hash_entry( stats, "openflow_application_interface.queue_get_config_reply_receive_succeeded" );
    assert_int_equal( ( int ) stat->value, 1 );

    xfree( queue[ 0 ] );
    xfree( queue[ 1 ] );
    delete_list( queues );
    free_buffer( buffer );
    xfree( delete_hash_entry( stats, "openflow_application_interface.queue_get_config_reply_receive_succeeded" ) );
  }

  // unhandled message
  {
    buffer *buffer;

    buffer = create_hello( TRANSACTION_ID );
    struct ofp_header *header = buffer->data;
    header->type = OFPT_QUEUE_GET_CONFIG_REPLY + 1;
    append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
    memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

    // FIXME
    handle_openflow_message( buffer->data, buffer->length );

    free_buffer( buffer );
  }
}


static void
test_handle_openflow_message_with_malformed_message() {
  buffer *buffer;
  openflow_service_header_t messenger_header;
  struct ofp_header *header;

  messenger_header.datapath_id = htonll( DATAPATH_ID );
  messenger_header.service_name_length = 0;

  buffer = create_hello( TRANSACTION_ID );
  header = buffer->data;
  header->length = htons( UINT16_MAX );
  append_front_buffer( buffer, sizeof( openflow_service_header_t ) );
  memcpy( buffer->data, &messenger_header, sizeof( openflow_service_header_t ) );

  handle_openflow_message( buffer->data, buffer->length );

  free_buffer( buffer );
}


static void
test_handle_openflow_message_if_message_is_NULL() {
  expect_assert_failure( handle_openflow_message( NULL, 1 ) );
}


static void
test_handle_openflow_message_if_message_length_is_zero() {
  buffer *data;

  data = alloc_buffer_with_length( 32 );

  expect_assert_failure( handle_openflow_message( data, 0 ) );

  free_buffer( data );
}


static void
test_handle_openflow_message_if_unhandled_message_type() {
  buffer *data;

  data = alloc_buffer_with_length( 32 );
  append_back_buffer( data, 32 );

  // FIXME
  handle_openflow_message( data, data->length );

  free_buffer( data );
}


/********************************************************************************
 * handle_message() tests.
 ********************************************************************************/

static void
test_handle_message_if_type_is_MESSENGER_OPENFLOW_MESSAGE() {
  buffer *data;
  openflow_service_header_t *header;

  data = create_barrier_reply( TRANSACTION_ID );

  assert_true( data != NULL );

  append_front_buffer( data, sizeof( openflow_service_header_t ) );

  header = data->data;
  header->datapath_id = htonll( DATAPATH_ID );
  header->service_name_length = 0;

  expect_memory( mock_barrier_reply_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_barrier_reply_handler, transaction_id, TRANSACTION_ID );
  expect_value( mock_barrier_reply_handler, user_data, BARRIER_REPLY_USER_DATA );

  set_barrier_reply_handler( mock_barrier_reply_handler, BARRIER_REPLY_USER_DATA );
  handle_message( MESSENGER_OPENFLOW_MESSAGE, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.barrier_reply_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );


  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.barrier_reply_receive_succeeded" ) );
}


static void
test_handle_message_if_type_is_MESSENGER_OPENFLOW_CONNECTED() {
  buffer *data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  append_back_buffer( data, sizeof( openflow_service_header_t ) );

  handle_message( MESSENGER_OPENFLOW_CONNECTED, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.switch_connected_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.switch_connected_receive_succeeded" ) );
}


static void
test_handle_message_if_type_is_MESSENGER_OPENFLOW_DISCONNECTED() {
  uint64_t *datapath_id;
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  datapath_id = append_back_buffer( data, sizeof( openflow_service_header_t ) );

  *datapath_id = htonll( DATAPATH_ID );

  expect_memory( mock_switch_disconnected_handler, &datapath_id, &DATAPATH_ID, sizeof( uint64_t ) );
  expect_value( mock_switch_disconnected_handler, user_data, SWITCH_DISCONNECTED_USER_DATA );

  expect_string( mock_clear_send_queue, service_name, REMOTE_SERVICE_NAME );
  will_return( mock_clear_send_queue, true );

  set_switch_disconnected_handler( mock_switch_disconnected_handler, SWITCH_DISCONNECTED_USER_DATA );
  handle_message( MESSENGER_OPENFLOW_DISCONNECTED, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.switch_disconnected_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.switch_disconnected_receive_succeeded" ) );
}


static void
test_handle_message_if_message_is_NULL() {
  expect_assert_failure( handle_message( MESSENGER_OPENFLOW_MESSAGE, NULL, 1 ) );
}


static void
test_handle_message_if_message_length_is_zero() {
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );

  expect_assert_failure( handle_message( MESSENGER_OPENFLOW_MESSAGE, data->data, 0 ) );

  free_buffer( data );
}


static void
test_handle_message_if_unhandled_message_type() {
  buffer *data;

  data = alloc_buffer_with_length( sizeof( openflow_service_header_t ) );
  append_back_buffer( data, sizeof( openflow_service_header_t ) );

  // FIXME
  handle_message( MESSENGER_OPENFLOW_DISCONNECTED + 1, data->data, data->length );

  stat_entry *stat = lookup_hash_entry( stats, "openflow_application_interface.undefined_switch_event_receive_succeeded" );
  assert_int_equal( ( int ) stat->value, 1 );

  free_buffer( data );
  xfree( delete_hash_entry( stats, "openflow_application_interface.undefined_switch_event_receive_succeeded" ) );
}


/********************************************************************************
 * delete_openflow_messages() tests.
 ********************************************************************************/

static void
test_delete_openflow_messages() {
  expect_string( mock_clear_send_queue, service_name, REMOTE_SERVICE_NAME );
  will_return( mock_clear_send_queue, true );

  assert_true( delete_openflow_messages( DATAPATH_ID ) );
}


static void
test_delete_openflow_messages_if_clear_send_queue_fails() {
  expect_string( mock_clear_send_queue, service_name, REMOTE_SERVICE_NAME );
  will_return( mock_clear_send_queue, false );

  assert_false( delete_openflow_messages( DATAPATH_ID ) );
}


/********************************************************************************
 * disconnect_switch() tests.
 ********************************************************************************/

static void
test_disconnect_switch() {
  size_t expected_length = ( size_t ) ( sizeof( openflow_service_header_t ) + strlen( SERVICE_NAME ) + 1 );
  void *expected_data = xcalloc( 1, expected_length );
  openflow_service_header_t *header = expected_data;
  header->datapath_id = htonll( DATAPATH_ID );
  header->service_name_length = htons( ( uint16_t ) ( strlen( SERVICE_NAME ) + 1 ) );
  memcpy( ( char * ) expected_data + sizeof( openflow_service_header_t ), SERVICE_NAME, strlen( SERVICE_NAME ) + 1 );

  expect_string( mock_send_message, service_name, REMOTE_SERVICE_NAME );
  expect_value( mock_send_message, tag32, MESSENGER_OPENFLOW_DISCONNECT_REQUEST );
  expect_value( mock_send_message, len, expected_length );
  expect_memory( mock_send_message, data, expected_data, expected_length );
  will_return( mock_send_message, true );

  assert_true( disconnect_switch( DATAPATH_ID ) );

  xfree( expected_data );
}


static void
test_disconnect_switch_if_send_message_fails() {
  size_t expected_length = ( size_t ) ( sizeof( openflow_service_header_t ) + strlen( SERVICE_NAME ) + 1 );
  void *expected_data = xcalloc( 1, expected_length );
  openflow_service_header_t *header = expected_data;
  header->datapath_id = htonll( DATAPATH_ID );
  header->service_name_length = htons( ( uint16_t ) ( strlen( SERVICE_NAME ) + 1 ) );
  memcpy( ( char * ) expected_data + sizeof( openflow_service_header_t ), SERVICE_NAME, strlen( SERVICE_NAME ) + 1 );

  expect_string( mock_send_message, service_name, REMOTE_SERVICE_NAME );
  expect_value( mock_send_message, tag32, MESSENGER_OPENFLOW_DISCONNECT_REQUEST );
  expect_value( mock_send_message, len, expected_length );
  expect_memory( mock_send_message, data, expected_data, expected_length );
  will_return( mock_send_message, false );

  assert_false( disconnect_switch( DATAPATH_ID ) );

  xfree( expected_data );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // initialization and finalization tests.
    unit_test_setup_teardown( test_init_openflow_application_interface_with_valid_custom_service_name, cleanup, cleanup ),
    unit_test_setup_teardown( test_init_openflow_application_interface_with_too_long_custom_service_name, cleanup, cleanup ),
    unit_test_setup_teardown( test_init_openflow_application_interface_if_already_initialized, init, cleanup ),

    unit_test_setup_teardown( test_finalize_openflow_application_interface, init, cleanup ),
    unit_test_setup_teardown( test_finalize_openflow_application_interface_if_not_initialized, cleanup, cleanup ),

    unit_test_setup_teardown( test_set_openflow_event_handlers, init, cleanup ),

    // switch ready handler tests.
    unit_test_setup_teardown( test_set_switch_ready_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_simple_switch_ready_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_switch_ready_handler_should_die_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_ready, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_ready_with_simple_handler, init, cleanup ),

    // switch disconnected handler tests.
    unit_test_setup_teardown( test_set_switch_disconnected_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_switch_disconnected_handler_if_handler_is_NULL, init, cleanup ),

    // error handler tests.
    unit_test_setup_teardown( test_set_error_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_error_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_error, init, cleanup ),
    unit_test_setup_teardown( test_handle_error_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_error_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_error_if_message_length_is_zero, init, cleanup ),

    // echo_reply handler tests.
    unit_test_setup_teardown( test_set_echo_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_echo_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_echo_reply, init, cleanup ),
    unit_test_setup_teardown( test_handle_echo_reply_without_data, init, cleanup ),
    unit_test_setup_teardown( test_handle_echo_reply_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_echo_reply_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_echo_reply_if_message_length_is_zero, init, cleanup ),

    // vendor handler tests.
    unit_test_setup_teardown( test_set_vendor_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_vendor_handler_if_handler_is_NULL, init, cleanup ),

    unit_test_setup_teardown( test_handle_vendor, init, cleanup ),
    unit_test_setup_teardown( test_handle_vendor_without_data, init, cleanup ),
    unit_test_setup_teardown( test_handle_vendor_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_vendor_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_vendor_if_message_length_is_zero, init, cleanup ),

    // features reply handler tests.
    unit_test_setup_teardown( test_set_features_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_features_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_features_reply, init, cleanup ),
    unit_test_setup_teardown( test_handle_features_reply_without_phy_port, init, cleanup ),
    unit_test_setup_teardown( test_handle_features_reply_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_features_reply_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_features_reply_if_message_length_is_zero, init, cleanup ),

    // get config reply handler tests.
    unit_test_setup_teardown( test_set_get_config_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_get_config_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_get_config_reply, init, cleanup ),
    unit_test_setup_teardown( test_handle_get_config_reply_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_get_config_reply_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_get_config_reply_if_message_length_is_zero, init, cleanup ),

    // flow removed handler tests.
    unit_test_setup_teardown( test_set_flow_removed_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_flow_removed_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_flow_removed, init, cleanup ),
    unit_test_setup_teardown( test_handle_flow_removed_with_simple_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_simple_flow_removed_handler, init, cleanup ),
    unit_test_setup_teardown( test_handle_flow_removed_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_flow_removed_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_flow_removed_if_message_length_is_zero, init, cleanup ),

    // port status handler tests.
    unit_test_setup_teardown( test_set_port_status_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_port_status_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_port_status, init, cleanup ),
    unit_test_setup_teardown( test_handle_port_status_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_port_status_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_port_status_if_message_length_is_zero, init, cleanup ),

    // stats reply handler tests.
    unit_test_setup_teardown( test_set_stats_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_stats_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_DESC, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_FLOW, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_AGGREGATE, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_TABLE, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_PORT, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_QUEUE, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_type_is_OFPST_VENDOR, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_with_undefined_type, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_stats_reply_if_message_length_is_zero, init, cleanup ),

    // barrier reply handler tests.
    unit_test_setup_teardown( test_set_barrier_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_barrier_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_barrier_reply, init, cleanup ),
    unit_test_setup_teardown( test_handle_barrier_reply_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_barrier_reply_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_barrier_reply_if_message_length_is_zero, init, cleanup ),

    // queue get config reply handler tests.
    unit_test_setup_teardown( test_set_queue_get_config_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_queue_get_config_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_queue_get_config_reply, init, cleanup ),
    unit_test_setup_teardown( test_handle_queue_get_config_reply_without_queues, init, cleanup ),
    unit_test_setup_teardown( test_handle_queue_get_config_reply_if_handler_is_not_registered, init, cleanup ),
    unit_test_setup_teardown( test_handle_queue_get_config_reply_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_queue_get_config_reply_if_message_length_is_zero, init, cleanup ),

    // list switches reply handler tests.
    unit_test_setup_teardown( test_set_list_switches_reply_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_list_switches_reply_handler_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_list_switches_reply, init, cleanup ),
    unit_test_setup_teardown( test_handle_list_switches_reply_if_data_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_list_switches_reply_if_length_is_zero, init, cleanup ),

    // packet-in handler tests.
    unit_test_setup_teardown( test_set_packet_in_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_simple_packet_in_handler, init, cleanup ),
    unit_test_setup_teardown( test_set_packet_in_handler_should_die_if_handler_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in_with_simple_handler, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in_with_malformed_packet, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in_without_data, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in_without_handler, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in_should_die_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_packet_in_should_die_if_message_length_is_zero, init, cleanup ),

    // miscellaneous tests.
    unit_test_setup_teardown( test_insert_dpid, init, cleanup ),
    unit_test_setup_teardown( test_insert_dpid_if_head_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_insert_dpid_if_dpid_is_NULL, init, cleanup ),

    unit_test_setup_teardown( test_handle_switch_events_if_type_is_MESSENGER_OPENFLOW_CONNECTED, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_events_if_type_is_MESSENGER_OPENFLOW_DISCONNECTED, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_events_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_events_if_message_length_is_zero, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_events_if_message_length_is_too_big, init, cleanup ),
    unit_test_setup_teardown( test_handle_switch_events_if_unhandled_message_type, init, cleanup ),

    unit_test_setup_teardown( test_handle_openflow_message, init, cleanup ),
    unit_test_setup_teardown( test_handle_openflow_message_with_malformed_message, init, cleanup ),
    unit_test_setup_teardown( test_handle_openflow_message_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_openflow_message_if_message_length_is_zero, init, cleanup ),
    unit_test_setup_teardown( test_handle_openflow_message_if_unhandled_message_type, init, cleanup ),

    unit_test_setup_teardown( test_handle_message_if_type_is_MESSENGER_OPENFLOW_MESSAGE, init, cleanup ),
    unit_test_setup_teardown( test_handle_message_if_type_is_MESSENGER_OPENFLOW_CONNECTED, init, cleanup ),
    unit_test_setup_teardown( test_handle_message_if_type_is_MESSENGER_OPENFLOW_DISCONNECTED, init, cleanup ),
    unit_test_setup_teardown( test_handle_message_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_handle_message_if_message_length_is_zero, init, cleanup ),
    unit_test_setup_teardown( test_handle_message_if_unhandled_message_type, init, cleanup ),

    // send_openflow_message() tests.
    unit_test_setup_teardown( test_send_openflow_message, init, cleanup ),
    unit_test_setup_teardown( test_send_openflow_message_if_message_is_NULL, init, cleanup ),
    unit_test_setup_teardown( test_send_openflow_message_if_message_length_is_zero, init, cleanup ),

    // delete_openflow_messages() tests.
    unit_test_setup_teardown( test_delete_openflow_messages, init, cleanup ),
    unit_test_setup_teardown( test_delete_openflow_messages_if_clear_send_queue_fails, init, cleanup ),

    // disconnect_switch() tests.
    unit_test_setup_teardown( test_disconnect_switch, init, cleanup ),
    unit_test_setup_teardown( test_disconnect_switch_if_send_message_fails, init, cleanup ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
