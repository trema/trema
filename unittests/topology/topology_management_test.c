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

#include "topology_management.h"

#include "service_management.h"
#include "topology_table.h"

/********************************************************************************
 * Common function.
 ********************************************************************************/


#define  TEST_TREMA_NAME "test_topo_mgmt"

// defined in trema.c
extern void set_trema_name( const char *name );
extern void _free_trema_name();

/********************************************************************************
 * Mock functions.
 ********************************************************************************/


#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;


static void ( *original_notify_switch_status_for_all_user )( sw_entry *sw );
static void ( *original_notify_port_status_for_all_user )( port_entry *port );
static void ( *original_notify_link_status_for_all_user )( port_entry *port );
static bool ( *original_send_request_message )( const char *to_service_name, const char *from_service_name, const uint16_t tag, const void *data, size_t len, void *user_data );

static int mock_notify_switch_status_for_all_user_calls = 0;
static int mock_notify_switch_status_for_all_user_max = 0;


static void
mock_notify_switch_status_for_all_user( sw_entry *sw ) {
  const uint64_t datapath_id = sw->datapath_id;
  check_expected( datapath_id );

  const bool up = sw->up;
  check_expected( up );

  if( ++mock_notify_switch_status_for_all_user_calls ==  mock_notify_switch_status_for_all_user_max ){
    stop_event_handler();
    stop_messenger();
  }
}


static int mock_notify_port_status_for_all_user_calls = 0;
static int mock_notify_port_status_for_all_user_max = 0;


static void
mock_notify_port_status_for_all_user( port_entry *port ) {
  const uint64_t datapath_id = port->sw->datapath_id;
  check_expected( datapath_id );

  const uint16_t port_no = port->port_no;
  check_expected( port_no );

  const char* name = port->name;
  check_expected( name );

  const bool up = port->up;
  check_expected( up );

  if( ++mock_notify_port_status_for_all_user_calls ==  mock_notify_port_status_for_all_user_max ){
    stop_event_handler();
    stop_messenger();
  }
}


static void
mock_notify_link_status_for_all_user( port_entry *port ) {
  assert_true( port->sw != NULL );
  assert_true( port->link_to != NULL );

  const uint64_t from_dpid = port->sw->datapath_id;
  check_expected( from_dpid );
  const uint16_t from_portno = port->port_no;
  check_expected( from_portno );

  const uint64_t to_dpid = port->link_to->datapath_id;
  check_expected( to_dpid );
  const uint16_t to_portno = port->link_to->port_no;
  check_expected( to_portno );

  const bool up = port->link_to->up;
  check_expected( up );
}


static bool free_user_data_member = false;
struct callback_info {
  void *callback;
  void *user_data;
};
struct event_forward_operation_to_all_request_param {
  bool add;
  enum efi_event_type type;
  char* service_name;
  event_forward_entry_to_all_callback callback;
  void* user_data;
};


static bool
mock_send_request_message( const char *to_service_name, const char *from_service_name,
                           const uint16_t tag, const void *data, size_t len, void *user_data ) {
  uint32_t tag32 = tag;
  struct callback_info *hd = user_data;

  check_expected( to_service_name );
  check_expected( from_service_name );
  check_expected( tag32 );
  check_expected( data );
  check_expected( len );
  check_expected( hd->callback );
  check_expected( hd->user_data );

  bool sent_ok = ( bool ) mock();
  if( sent_ok ) {
    if ( free_user_data_member ) {
      const management_application_request* mgmt = data;
      switch ( htonl(mgmt->application_id) ) {
      case EVENT_FORWARD_ENTRY_ADD:
      case EVENT_FORWARD_ENTRY_DELETE:
      case EVENT_FORWARD_ENTRY_DUMP:
      case EVENT_FORWARD_ENTRY_SET:
      {
        struct event_forward_operation_to_all_request_param *p = hd->user_data;

        xfree( p->service_name );
        xfree( p );
        hd->user_data = NULL;
      }
      break;
      case EFI_GET_SWLIST:
        // nothing to free
        break;
      default:
        xfree( hd->user_data );
        break;
      }
    }
    xfree( hd );
  }
  return sent_ok;
}


static void
expect_port_status_set() {
  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char topology[14+1];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl(sizeof( struct expected_data ) ),
        },
        .application_id = htonl(EVENT_FORWARD_ENTRY_ADD),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PORT_STATUS,
          .n_services = htonl( 1 ),
      },
      .topology = TEST_TREMA_NAME
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_any( mock_send_request_message, from_service_name );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_any( mock_send_request_message, hd->callback );
  expect_any( mock_send_request_message, hd->user_data );
  will_return( mock_send_request_message, true );
}


static void
expect_state_notify_set() {
  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char topology[14+1];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl(sizeof( struct expected_data ) ),
        },
        .application_id = htonl(EVENT_FORWARD_ENTRY_ADD),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_STATE_NOTIFY,
          .n_services = htonl( 1 ),
      },
      .topology = TEST_TREMA_NAME,
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_any( mock_send_request_message, from_service_name );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_any( mock_send_request_message, hd->callback );
  expect_any( mock_send_request_message, hd->user_data );
  will_return( mock_send_request_message, true );
}


static void
expect_switch_list_request() {
  struct expected_data {
    management_application_request mgmt;
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl(sizeof( struct expected_data ) ),
        },
        .application_id = htonl(EFI_GET_SWLIST),
      },
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_any( mock_send_request_message, from_service_name );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_any( mock_send_request_message, hd->callback );
  expect_any( mock_send_request_message, hd->user_data );
  will_return( mock_send_request_message, true );
}

/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/


static void
setup() {
  set_trema_name( TEST_TREMA_NAME );
  init_messenger("/tmp");
  init_timer();
  init_stat();
  init_openflow_application_interface( TEST_TREMA_NAME );
  init_event_forward_interface();

  swap_original( notify_switch_status_for_all_user );
  swap_original( notify_port_status_for_all_user );
  swap_original( notify_link_status_for_all_user );
  swap_original( send_request_message );
  free_user_data_member = false;
  mock_notify_switch_status_for_all_user_calls = 0;
  mock_notify_switch_status_for_all_user_max = 0;
  mock_notify_port_status_for_all_user_calls = 0;
  mock_notify_port_status_for_all_user_max = 0;
}


static void
teardown() {
  finalize_event_forward_interface();
  finalize_openflow_application_interface();
  finalize_timer();
  finalize_stat();
  finalize_messenger();

  revert_original( send_request_message );
  revert_original( notify_switch_status_for_all_user );
  revert_original( notify_port_status_for_all_user );
  revert_original( notify_link_status_for_all_user );
  mock_notify_switch_status_for_all_user_calls = 0;
  mock_notify_switch_status_for_all_user_max = 0;
  mock_notify_port_status_for_all_user_calls = 0;
  mock_notify_port_status_for_all_user_max = 0;
  _free_trema_name();
}


static void
setup_topology_mgmt() {
  setup();
  assert_true( init_topology_management() );
  free_user_data_member = true;
  expect_port_status_set();
  expect_state_notify_set();
  expect_switch_list_request();
  assert_true( start_topology_management() );
}


static void
teardown_topology_mgmt() {
  finalize_topology_management();
  teardown();
}

/********************************************************************************
 * Tests.
 ********************************************************************************/

//bool init_topology_management( void );
//void finalize_topology_management( void );
//bool start_topology_management( void );
static void
test_init_start_finalize_topology_management() {
  assert_true( init_topology_management() );
  free_user_data_member = true;
  expect_port_status_set();
  expect_state_notify_set();
  expect_switch_list_request();
  assert_true( start_topology_management() );
  finalize_topology_management();
}


static void
helper_sw_received_feature_request_end( uint16_t tag, void *data, size_t len ) {
  UNUSED( len );

  check_expected( tag );
  openflow_service_header_t* ofs_header = data;
  const uint64_t datapath_id = ntohll(ofs_header->datapath_id);
  check_expected( datapath_id );
  const size_t hdr_len = sizeof(openflow_service_header_t) + ntohs( ofs_header->service_name_length );
  struct ofp_header* feature_req = ( struct ofp_header * ) ((char*)ofs_header + hdr_len);
  const uint8_t type = feature_req->type;
  check_expected( type );

  stop_event_handler();
  stop_messenger();
}


static void
test_receive_switch_ready_then_notify_sw_status_and_request_features() {
  setup_topology_mgmt();

  // send fake SW ready event to OFA I/F.
  openflow_service_header_t data;
  data.datapath_id = htonll( 0x1234 );
  data.service_name_length = 0;
  const size_t len = sizeof( openflow_service_header_t );
  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_READY, &data, len ) );

  // check notify to service mgmt
  expect_value( mock_notify_switch_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_switch_status_for_all_user, up, true );

  // check listen on fake sw messenger
  const char* SW_MSNGER_NAME = "switch.0x1234";
  assert_true( add_message_received_callback( SW_MSNGER_NAME, helper_sw_received_feature_request_end ) );

  expect_value( helper_sw_received_feature_request_end, tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( helper_sw_received_feature_request_end, datapath_id, 0x1234 );
  expect_value( helper_sw_received_feature_request_end, type, OFPT_FEATURES_REQUEST );

  // start
  start_messenger();
  start_event_handler();

  // cleanup
  assert_true( delete_message_received_callback( SW_MSNGER_NAME, helper_sw_received_feature_request_end ) );
  int next_timeout_usec;
  execute_timer_events( &next_timeout_usec );

  // remove sw?
  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = lookup_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  delete_sw_entry( sw );

  teardown_topology_mgmt();
}


static void
test_feature_reply_then_update_ports() {
  setup_topology_mgmt();

  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = update_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  sw->up = true;
  port_entry *p1 = update_port_entry( sw, 1, "Some port name" );
  p1->up = true;
  port_entry *p2 = update_port_entry( sw, 2, "Port removed" );
  p2->up = true;
  port_entry *p4 = update_port_entry( sw, 4, "Port changed" );
  p4->up = true;

  // send fake feature_reply event to OFA I/F.
  buffer* buf = alloc_buffer();
  openflow_service_header_t* ofs_header = append_back_buffer( buf, sizeof(openflow_service_header_t) );
  ofs_header->datapath_id = htonll( 0x1234 );
  ofs_header->service_name_length = htons( 0 );

  struct ofp_switch_features *features = append_back_buffer( buf, sizeof(struct ofp_switch_features) );
  features->header.length = htons(sizeof(struct ofp_switch_features) + sizeof( struct ofp_phy_port ) * 4 );
  features->header.type = OFPT_FEATURES_REPLY;
  features->header.version = OFP_VERSION;
  features->header.xid = 0xDEADBEEF;
  features->datapath_id = htonll( 0x1234 );

  struct ofp_phy_port *port1 = append_back_buffer( buf, sizeof( struct ofp_phy_port ) );
  memset( port1, 0, sizeof( struct ofp_phy_port ));
  sprintf( port1->name, "Some port name1" );
  port1->port_no = htons( 1 );
  port1->state = htonl( 0 );

  struct ofp_phy_port *port3 = append_back_buffer( buf, sizeof( struct ofp_phy_port ) );
  memset( port3, 0, sizeof( struct ofp_phy_port ));
  sprintf( port3->name, "New port name" );
  port3->port_no = htons( 3 );
  port3->state = htonl( 0 );

  struct ofp_phy_port *port4 = append_back_buffer( buf, sizeof( struct ofp_phy_port ) );
  memset( port4, 0, sizeof( struct ofp_phy_port ));
  sprintf( port4->name, "Port changed" );
  port4->port_no = htons( 4 );
  port4->state = htonl( OFPPS_LINK_DOWN );

  struct ofp_phy_port *port5 = append_back_buffer( buf, sizeof( struct ofp_phy_port ) );
  memset( port5, 0, sizeof( struct ofp_phy_port ));
  sprintf( port5->name, "Port invalid" );
  port5->port_no = htons( OFPP_FLOOD );
  port5->state = htonl( OFPPS_LINK_DOWN );

  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_MESSAGE, buf->data, buf->length ) );
  free_buffer( buf );

  // check notify to service mgmt
  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 3 );
  expect_string( mock_notify_port_status_for_all_user, name, "New port name" );
  expect_value( mock_notify_port_status_for_all_user, up, true );

  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 4 );
  expect_string( mock_notify_port_status_for_all_user, name, "Port changed" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  // Note: Port name change is not Topology change => Port 1 will not be notified.
  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 2 );
  expect_string( mock_notify_port_status_for_all_user, name, "Port removed" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  // stop on last notification call
  mock_notify_port_status_for_all_user_max = 2;

  // pump message
  start_messenger();
  start_event_handler();

  // clean up
  port_entry *p = lookup_port_entry_by_port( sw, 1 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 3 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 4 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  delete_sw_entry( sw );

  teardown_topology_mgmt();
}


static void
test_receive_switch_disconnected_then_notify_sw_status() {
  setup_topology_mgmt();

  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = update_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  sw->up = true;
  port_entry *p1 = update_port_entry( sw, 1, "Some port name1" );
  p1->up = true;
  port_entry *p2 = update_port_entry( sw, 2, "Some port name2" );
  p2->up = true;

  link_to* l = update_link_to(p1, &datapath_id, 2, true );
  assert_true( l != NULL );

  // send fake SW disconnected event to OFA I/F.
  openflow_service_header_t data;
  data.datapath_id = htonll( 0x1234 );
  data.service_name_length = 0;
  const size_t len = sizeof( openflow_service_header_t );
  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_DISCONNECTED, &data, len ) );

  // check notify to service mgmt
  expect_value( mock_notify_link_status_for_all_user, from_dpid, 0x1234 );
  expect_value( mock_notify_link_status_for_all_user, from_portno, 1 );
  expect_value( mock_notify_link_status_for_all_user, to_dpid, 0x1234 );
  expect_value( mock_notify_link_status_for_all_user, to_portno, 2 );
  expect_value( mock_notify_link_status_for_all_user, up, false );

  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 2 );
  expect_string( mock_notify_port_status_for_all_user, name, "Some port name2" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 1 );
  expect_string( mock_notify_port_status_for_all_user, name, "Some port name1" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  // stop messenger on sw notification
  mock_notify_switch_status_for_all_user_max = 1;
  expect_value( mock_notify_switch_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_switch_status_for_all_user, up, false );

  // start
  start_messenger();
  start_event_handler();

  // cleanup
  int next_timeout_usec;
  execute_timer_events( &next_timeout_usec );

  sw = lookup_sw_entry( &datapath_id );
  assert_true( sw == NULL );

  teardown_topology_mgmt();
}


static void
test_receive_port_add_status_then_notify_port_status() {
  setup_topology_mgmt();

  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = update_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  sw->up = true;
  port_entry *p1 = update_port_entry( sw, 1, "Some port name" );
  p1->up = true;
  port_entry *p2 = update_port_entry( sw, 2, "Port removed" );
  p2->up = true;
  port_entry *p4 = update_port_entry( sw, 4, "Port changed" );
  p4->up = true;

  // send fake port_status event to OFA I/F.
  buffer* buf = alloc_buffer();
  openflow_service_header_t* ofs_header = append_back_buffer( buf, sizeof(openflow_service_header_t) );
  ofs_header->datapath_id = htonll( 0x1234 );
  ofs_header->service_name_length = htons( 0 );

  struct ofp_port_status *port_status = append_back_buffer( buf, sizeof(struct ofp_port_status) );
  memset( port_status, 0, sizeof(struct ofp_port_status) );
  port_status->header.length = htons( sizeof(struct ofp_port_status) );
  port_status->header.type = OFPT_PORT_STATUS;
  port_status->header.version = OFP_VERSION;
  port_status->header.xid = 0xDEADBEEF;
  port_status->reason = OFPPR_ADD;

  port_status->desc.port_no = htons( 3 );
  port_status->desc.config = htonl( OFPPC_PORT_DOWN );
  sprintf( port_status->desc.name, "Added port" );

  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_MESSAGE, buf->data, buf->length ) );
  free_buffer( buf );

  // check notify to service mgmt
  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 3 );
  expect_string( mock_notify_port_status_for_all_user, name, "Added port" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  // stop on last notification call
  mock_notify_port_status_for_all_user_max = 1;

  // pump message
  start_messenger();
  start_event_handler();

  // clean up
  port_entry *p = lookup_port_entry_by_port( sw, 1 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 2 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 3 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 4 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  delete_sw_entry( sw );

  teardown_topology_mgmt();
}


static void
test_receive_port_del_status_then_notify_port_status() {
  setup_topology_mgmt();

  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = update_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  sw->up = true;
  port_entry *p1 = update_port_entry( sw, 1, "Some port name" );
  p1->up = true;
  port_entry *p2 = update_port_entry( sw, 2, "Port removed" );
  p2->up = true;
  port_entry *p4 = update_port_entry( sw, 4, "Port changed" );
  p4->up = true;

  // send fake port_status event to OFA I/F.
  buffer* buf = alloc_buffer();
  openflow_service_header_t* ofs_header = append_back_buffer( buf, sizeof(openflow_service_header_t) );
  ofs_header->datapath_id = htonll( 0x1234 );
  ofs_header->service_name_length = htons( 0 );

  struct ofp_port_status *port_status = append_back_buffer( buf, sizeof(struct ofp_port_status) );
  memset( port_status, 0, sizeof(struct ofp_port_status) );
  port_status->header.length = htons( sizeof(struct ofp_port_status) );
  port_status->header.type = OFPT_PORT_STATUS;
  port_status->header.version = OFP_VERSION;
  port_status->header.xid = 0xDEADBEEF;
  port_status->reason = OFPPR_DELETE;

  port_status->desc.port_no = htons( 2 );
  port_status->desc.state = htonl( 0 );
  sprintf( port_status->desc.name, "Port removed" );

  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_MESSAGE, buf->data, buf->length ) );
  free_buffer( buf );

  // check notify to service mgmt
  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 2 );
  expect_string( mock_notify_port_status_for_all_user, name, "Port removed" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  // stop on last notification call
  mock_notify_port_status_for_all_user_max = 1;

  // pump message
  start_messenger();
  start_event_handler();

  // clean up
  port_entry *p = lookup_port_entry_by_port( sw, 1 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 2 );
  assert_true( p == NULL );

  p = lookup_port_entry_by_port( sw, 3 );
  assert_true( p == NULL );

  p = lookup_port_entry_by_port( sw, 4 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  delete_sw_entry( sw );

  teardown_topology_mgmt();
}


static void
test_receive_port_mod_status_then_notify_port_status() {
  setup_topology_mgmt();

  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = update_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  sw->up = true;
  port_entry *p1 = update_port_entry( sw, 1, "Some port name" );
  p1->up = true;
  port_entry *p2 = update_port_entry( sw, 2, "Port removed" );
  p2->up = true;
  port_entry *p4 = update_port_entry( sw, 4, "Port changed" );
  p4->up = true;

  link_to* l = update_link_to( p4, &datapath_id, 2, true );
  assert_true( l != NULL );

  // send fake port_status event to OFA I/F.
  buffer* buf = alloc_buffer();
  openflow_service_header_t* ofs_header = append_back_buffer( buf, sizeof(openflow_service_header_t) );
  ofs_header->datapath_id = htonll( 0x1234 );
  ofs_header->service_name_length = htons( 0 );

  struct ofp_port_status *port_status = append_back_buffer( buf, sizeof(struct ofp_port_status) );
  memset( port_status, 0, sizeof(struct ofp_port_status) );
  port_status->header.length = htons( sizeof(struct ofp_port_status) );
  port_status->header.type = OFPT_PORT_STATUS;
  port_status->header.version = OFP_VERSION;
  port_status->header.xid = 0xDEADBEEF;
  port_status->reason = OFPPR_MODIFY;

  port_status->desc.port_no = htons( 4 );
  port_status->desc.state = htonl( OFPPS_LINK_DOWN );
  sprintf( port_status->desc.name, "Port changed" );

  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_MESSAGE, buf->data, buf->length ) );
  free_buffer( buf );

  // check notify to service mgmt
  expect_value( mock_notify_link_status_for_all_user, from_dpid, 0x1234 );
  expect_value( mock_notify_link_status_for_all_user, from_portno, 4 );
  expect_value( mock_notify_link_status_for_all_user, to_dpid, 0x1234 );
  expect_value( mock_notify_link_status_for_all_user, to_portno, 2 );
  expect_value( mock_notify_link_status_for_all_user, up, false );

  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 4 );
  expect_string( mock_notify_port_status_for_all_user, name, "Port changed" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  // stop on last notification call
  mock_notify_port_status_for_all_user_max = 1;

  // pump message
  start_messenger();
  start_event_handler();

  // clean up
  port_entry *p = lookup_port_entry_by_port( sw, 1 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 2 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 3 );
  assert_true( p == NULL );

  p = lookup_port_entry_by_port( sw, 4 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  delete_sw_entry( sw );

  teardown_topology_mgmt();
}

static void
test_receive_port_mod_status_port_no_then_notify_port_status() {
  setup_topology_mgmt();

  const uint64_t datapath_id = 0x1234;
  sw_entry *sw = update_sw_entry( &datapath_id );
  assert_true( sw != NULL );
  sw->up = true;
  port_entry *p1 = update_port_entry( sw, 1, "Some port name" );
  p1->up = true;
  port_entry *p2 = update_port_entry( sw, 2, "Port removed" );
  p2->up = true;
  port_entry *p4 = update_port_entry( sw, 4, "Port changed" );
  p4->up = true;

  // send fake port_status event to OFA I/F.
  buffer* buf = alloc_buffer();
  openflow_service_header_t* ofs_header = append_back_buffer( buf, sizeof(openflow_service_header_t) );
  ofs_header->datapath_id = htonll( 0x1234 );
  ofs_header->service_name_length = htons( 0 );

  struct ofp_port_status *port_status = append_back_buffer( buf, sizeof(struct ofp_port_status) );
  memset( port_status, 0, sizeof(struct ofp_port_status) );
  port_status->header.length = htons( sizeof(struct ofp_port_status) );
  port_status->header.type = OFPT_PORT_STATUS;
  port_status->header.version = OFP_VERSION;
  port_status->header.xid = 0xDEADBEEF;
  port_status->reason = OFPPR_MODIFY;

  port_status->desc.port_no = htons( 3 );
  sprintf( port_status->desc.name, "Port changed" );

  assert_true( send_message( TEST_TREMA_NAME, MESSENGER_OPENFLOW_MESSAGE, buf->data, buf->length ) );
  free_buffer( buf );

  // check notify to service mgmt
  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 4 );
  expect_string( mock_notify_port_status_for_all_user, name, "Port changed" );
  expect_value( mock_notify_port_status_for_all_user, up, false );

  expect_value( mock_notify_port_status_for_all_user, datapath_id, 0x1234 );
  expect_value( mock_notify_port_status_for_all_user, port_no, 3 );
  expect_string( mock_notify_port_status_for_all_user, name, "Port changed" );
  expect_value( mock_notify_port_status_for_all_user, up, true );

  // stop on last notification call
  mock_notify_port_status_for_all_user_max = 2;

  // pump message
  start_messenger();
  start_event_handler();

  // clean up
  port_entry *p = lookup_port_entry_by_port( sw, 1 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 2 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 3 );
  assert_true( p != NULL );
  delete_port_entry( sw, p );

  p = lookup_port_entry_by_port( sw, 4 );
  assert_true( p == NULL );

  delete_sw_entry( sw );

  teardown_topology_mgmt();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test_setup_teardown( test_init_start_finalize_topology_management, setup, teardown ),
      // test for set_switch_ready_handler( handle_switch_ready, NULL );
      unit_test( test_receive_switch_ready_then_notify_sw_status_and_request_features ),
      // test for set_switch_disconnected_handler( handle_switch_disconnected, NULL );
      unit_test( test_receive_switch_disconnected_then_notify_sw_status ),
      // test for set_features_reply_handler( handle_switch_features_reply, NULL );
      unit_test( test_feature_reply_then_update_ports ),
      // test for set_port_status_handler( handle_port_status, NULL );
      unit_test( test_receive_port_add_status_then_notify_port_status ),
      unit_test( test_receive_port_del_status_then_notify_port_status ),
      unit_test( test_receive_port_mod_status_then_notify_port_status ),
      unit_test( test_receive_port_mod_status_port_no_then_notify_port_status ),
  };

  setup_leak_detector();
  return run_tests( tests );
}

