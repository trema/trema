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

#include "discovery_management.h"
#include "service_management.h"

#include "topology_table.h"
#include "probe_timer_table.h"

/********************************************************************************
 * Common function.
 ********************************************************************************/

#define TEST_TREMA_NAME "disc_mgmt_test"
// defined in trema.c
extern void set_trema_name( const char *name );
extern void _free_trema_name();

/********************************************************************************
 * Mock functions.
 ********************************************************************************/

static void ( *original_notice )( const char *format, ... );
static void ( *original_warn )( const char *format, ... );

static bool ( *original_set_port_status_updated_hook )( port_status_updated_hook, void *user_data );
static bool ( *original_set_switch_status_updated_hook )( switch_status_updated_hook, void *user_data );

static void ( *original_execute_timer_events )( int *next_timeout_usec );

static bool ( *original_send_request_message )( const char *to_service_name, const char *from_service_name, const uint16_t tag, const void *data, size_t len, void *user_data );

static bool ( *original_send_message )( const char *service_name, const uint16_t tag, const void *data, size_t len );


#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;

static bool check_notice = false;
static void
mock_notice_check( const char *format, va_list args ) {
  char message[ 1000 ];
  vsnprintf( message, 1000, format, args );

  check_expected( message );
}


static void
mock_notice( const char *format, ... ) {
  if ( check_notice ) {
    va_list arg;
    va_start( arg, format );
    mock_notice_check( format, arg );
    va_end( arg );
  }
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
  if ( check_warn ) {
    va_list arg;
    va_start( arg, format );
    mock_warn_check( format, arg );
    va_end( arg );
  }
}


static port_status_updated_hook port_status_updated_hook_callback;
static bool
mock_set_port_status_updated_hook( port_status_updated_hook callback, void *user_data ) {
  port_status_updated_hook_callback = callback;
  check_expected( callback );
  check_expected( user_data );
  return ( bool ) mock();
}


static switch_status_updated_hook handle_switch_status_updated_callback;
static bool
mock_set_switch_status_updated_hook( switch_status_updated_hook callback, void *user_data ) {
  handle_switch_status_updated_callback = callback;
  check_expected( callback );
  check_expected( user_data );
  return ( bool ) mock();
}


static void
expect_switch_and_port_status_hook_set() {
  expect_not_value( mock_set_switch_status_updated_hook, callback, NULL );
  expect_value( mock_set_switch_status_updated_hook, user_data, NULL );
  will_return( mock_set_switch_status_updated_hook, true );
  expect_not_value( mock_set_port_status_updated_hook, callback, NULL );
  expect_value( mock_set_port_status_updated_hook, user_data, NULL );
  will_return( mock_set_port_status_updated_hook, true );
}


static void
expect_switch_and_port_status_hook_clear() {
  expect_value( mock_set_switch_status_updated_hook, callback, NULL );
  expect_value( mock_set_switch_status_updated_hook, user_data, NULL );
  will_return( mock_set_switch_status_updated_hook, true );
  expect_value( mock_set_port_status_updated_hook, callback, NULL );
  expect_value( mock_set_port_status_updated_hook, user_data, NULL );
  will_return( mock_set_port_status_updated_hook, true );
}


static void
mock_execute_timer_events( int *next_timeout_usec ) {
  UNUSED( next_timeout_usec );
  // Do nothing.
}


static bool free_user_data_member = false;
static bool free_event_forward_operation_to_all_request_param = false;
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
    if ( free_event_forward_operation_to_all_request_param ) {
      struct event_forward_operation_to_all_request_param *p = hd->user_data;

      xfree( p->service_name );
      xfree( p );
      hd->user_data = NULL;
    }
    if ( free_user_data_member ) {
      xfree( hd->user_data );
    }
    xfree( hd );
  }
  return sent_ok;
}


static bool
mock_send_message_flow_mod_for_lldp( const char* service_name, const uint16_t tag, const void *data, size_t len ) {
  check_expected( service_name );
  check_expected( tag );

  buffer* buffer = alloc_buffer_with_length( len );
  void* raw_data = append_back_buffer( buffer, len );
  memcpy( raw_data, data, len );

  const openflow_service_header_t* of_s_h = data;
  const uint64_t datapath_id = ntohll( of_s_h->datapath_id );
  check_expected( datapath_id );
  const size_t ofs_header_length = sizeof(openflow_service_header_t)
      + ntohs( of_s_h->service_name_length );

  struct ofp_flow_mod* ofp_flow_mod = remove_front_buffer( buffer, ofs_header_length );
  assert_int_equal( ofp_flow_mod->header.type, OFPT_FLOW_MOD );
  assert_int_equal( ntohs(ofp_flow_mod->priority), UINT16_MAX );
  uint16_t command = ntohs(ofp_flow_mod->command);
  check_expected( command );
  assert_int_equal( ntohs(ofp_flow_mod->idle_timeout), 0 );
  assert_int_equal( ntohs(ofp_flow_mod->hard_timeout), 0 );

  struct ofp_match match;
  ntoh_match( &match, &ofp_flow_mod->match );
  assert_int_equal( match.wildcards, OFPFW_ALL & ~OFPFW_DL_TYPE );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_LLDP );

  struct ofp_action_output* act_out = remove_front_buffer( buffer, offsetof( struct ofp_flow_mod, actions ) );
  assert_int_equal( ntohs(act_out->type), OFPAT_OUTPUT );
  assert_int_equal( ntohs(act_out->port), OFPP_CONTROLLER );

  free_buffer( buffer );

  return ( bool ) mock();
}


static bool
mock_send_message_flow_mod_for_lldp_over_ip( const char* service_name, const uint16_t tag, const void *data, size_t len ) {
  check_expected( service_name );
  check_expected( tag );

  buffer* buffer = alloc_buffer_with_length( len );
  void* raw_data = append_back_buffer( buffer, len );
  memcpy( raw_data, data, len );

  const openflow_service_header_t* of_s_h = data;
  const uint64_t datapath_id = ntohll( of_s_h->datapath_id );
  check_expected( datapath_id );
  const size_t ofs_header_length = sizeof(openflow_service_header_t)
      + ntohs( of_s_h->service_name_length );

  struct ofp_flow_mod* ofp_flow_mod = remove_front_buffer( buffer,
                                                           ofs_header_length );
  assert_int_equal( ofp_flow_mod->header.type, OFPT_FLOW_MOD );
  assert_int_equal( ntohs(ofp_flow_mod->priority), UINT16_MAX );
  uint16_t command = ntohs(ofp_flow_mod->command);
  check_expected( command );
  assert_int_equal( ntohs(ofp_flow_mod->idle_timeout), 0 );
  assert_int_equal( ntohs(ofp_flow_mod->hard_timeout), 0 );

  struct ofp_match match;
  ntoh_match( &match, &ofp_flow_mod->match );
  assert_int_equal(
      match.wildcards,
      OFPFW_ALL & ~( OFPFW_DL_TYPE | OFPFW_NW_PROTO | OFPFW_NW_SRC_MASK | OFPFW_NW_DST_MASK ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_proto, IPPROTO_ETHERIP );
  const uint32_t nw_src = match.nw_src;
  const uint32_t nw_dst = match.nw_dst;
  check_expected( nw_src );
  check_expected( nw_dst );

  struct ofp_action_output* act_out = remove_front_buffer( buffer, offsetof( struct ofp_flow_mod, actions ) );
  assert_int_equal( ntohs(act_out->type), OFPAT_OUTPUT );
  assert_int_equal( ntohs(act_out->port), OFPP_CONTROLLER );

  free_buffer( buffer );

  return ( bool ) mock();
}


static void
expect_enable_discovery() {
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
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
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
expect_disable_discovery() {
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
        .application_id = htonl(EVENT_FORWARD_ENTRY_DELETE),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
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


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/


static void
setup() {
  free_user_data_member = false;
  free_event_forward_operation_to_all_request_param = false;
  set_trema_name( TEST_TREMA_NAME );
  init_messenger( "/tmp" );
  init_timer();
  init_stat();
  init_openflow_application_interface( TEST_TREMA_NAME );
  init_event_forward_interface();

  swap_original( notice );
  swap_original( warn );
  swap_original( set_switch_status_updated_hook );
  swap_original( set_port_status_updated_hook );
  swap_original( send_request_message );
}


static void
teardown() {
  revert_original( notice );
  revert_original( warn );
  check_notice = false;
  check_warn = false;
  revert_original( set_switch_status_updated_hook );
  revert_original( set_port_status_updated_hook );
  revert_original( send_request_message );

  finalize_event_forward_interface();
  finalize_openflow_application_interface();
  finalize_stat();
  finalize_timer();
  finalize_messenger();
  _free_trema_name();
}


static void
setup_discovery_mgmt() {
  setup();
  discovery_management_options options;
  options.always_enabled = false;

  assert_true( init_discovery_management( options ) );
  assert_true( start_discovery_management() );

  original_send_message = send_message;
  send_message = mock_send_message_flow_mod_for_lldp;
}


static void
teardown_discovery_mgmt() {
  send_message = original_send_message;
  finalize_discovery_management();

  // deleted timer event struct will not be freed until next timer event
  // fake timer event;
  int next = 100000;
  execute_timer_events( &next );

  teardown();
}


static void
setup_discovery_mgmt_over_ip() {
  setup();
  discovery_management_options options;
  options.always_enabled = false;
  options.lldp.lldp_over_ip = true;
  options.lldp.lldp_ip_src = 0x01234567;
  options.lldp.lldp_ip_dst = 0x89ABCDEF;

  assert_true( init_discovery_management( options ) );
  assert_true( start_discovery_management() );

  original_send_message = send_message;
  send_message = mock_send_message_flow_mod_for_lldp_over_ip;
}


static void
teardown_discovery_mgmt_over_ip() {
  teardown_discovery_mgmt();
}


/********************************************************************************
 * Tests.
 ********************************************************************************/


static void
test_init_finalize() {
  discovery_management_options options;
  options.always_enabled = false;

  assert_true( init_discovery_management( options ) );

  assert_true( start_discovery_management() );

  finalize_discovery_management();

  // deleted timer event struct will not be freed until next timer event
  // fake timer event;
  int next = 100000;
  execute_timer_events( &next );
}


static void
test_init_finalize_with_always_discovery() {
  discovery_management_options options;
  options.always_enabled = true;

  assert_true( init_discovery_management( options ) );

  expect_switch_and_port_status_hook_set();

  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  assert_true( start_discovery_management() );

  finalize_discovery_management();

  // deleted timer event struct will not be freed until next timer event
  // fake timer event;
  int next = 100000;
  execute_timer_events( &next );
}


static void
test_enable_discovery_twice_prints_message() {

  check_warn = true;

  expect_switch_and_port_status_hook_set();

  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  expect_string( mock_warn_check, message, "Topology Discovery is already enabled." );

  expect_switch_and_port_status_hook_set();

  expect_enable_discovery();
  enable_discovery();

  check_warn = false;

  expect_switch_and_port_status_hook_clear();

  expect_disable_discovery();
  disable_discovery();
}


static void
test_disable_discovery_twice_prints_message() {

  check_warn = true;
  check_notice = true;

  expect_switch_and_port_status_hook_set();

  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  expect_switch_and_port_status_hook_clear();

  expect_disable_discovery();
  disable_discovery();

  expect_string( mock_warn_check, message, "Topology Discovery was not enabled." );

  expect_switch_and_port_status_hook_clear();

  expect_disable_discovery();
  disable_discovery();

  check_warn = false;
  check_notice = false;
}


static void
test_flow_mod_lldp_when_switch_status_event_up() {
  setup_discovery_mgmt();

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  // handle_switch_status_updated_callback should point to
  // handle_switch_status_updated_callback @discovery_management.c
  // as a result of enable_discovery();
  assert_true( handle_switch_status_updated_callback != NULL );

  // Test: do nothing if sw down
  sw_entry sw;
  sw.datapath_id = 0x1234;
  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";
  sw.up = false;

  handle_switch_status_updated_callback( NULL, &sw );

  // Test: send LLDP flow mod if sw up
  expect_string( mock_send_message_flow_mod_for_lldp,
                 service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_flow_mod_for_lldp,
                tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_flow_mod_for_lldp,
                datapath_id, 0x1234 );
  expect_value( mock_send_message_flow_mod_for_lldp,
                command, OFPFC_ADD);
  will_return( mock_send_message_flow_mod_for_lldp, true );

  sw.up = true;
  handle_switch_status_updated_callback( NULL, &sw );


  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  teardown_discovery_mgmt();
}


static void
test_do_nothing_when_switch_status_event_down() {
  setup_discovery_mgmt_over_ip();

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  // handle_switch_status_updated_callback should point to
  // handle_switch_status_updated_callback @discovery_management.c
  // as a result of enable_discovery();
  assert_true( handle_switch_status_updated_callback != NULL );

  // Test: do nothing if sw down
  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = false;

  handle_switch_status_updated_callback( NULL, &sw );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  teardown_discovery_mgmt_over_ip();
}


static void
test_flow_mod_lldp_over_ip_when_switch_status_event_up() {
  setup_discovery_mgmt_over_ip();

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  // handle_switch_status_updated_callback should point to
  // handle_switch_status_updated_callback @discovery_management.c
  // as a result of enable_discovery();
  assert_true( handle_switch_status_updated_callback != NULL );

  sw_entry sw;
  sw.datapath_id = 0x1234;
  sw.up = true;
  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";

  // Test: send LLDP flow mod if sw up
  expect_string( mock_send_message_flow_mod_for_lldp_over_ip,
                service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                datapath_id, 0x1234 );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                command, OFPFC_ADD );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                nw_src, 0x01234567 );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                nw_dst, 0x89ABCDEF );
  will_return( mock_send_message_flow_mod_for_lldp_over_ip, true );

  handle_switch_status_updated_callback( NULL, &sw );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  teardown_discovery_mgmt_over_ip();
}


static void
test_port_status_event() {
  // disable timer events
  swap_original( execute_timer_events );

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  assert_true( port_status_updated_hook_callback != NULL );

  const uint64_t dpid = 0x1234;
  sw_entry* sw = update_sw_entry( &dpid );
  sw->up = true;
  port_entry* p = update_port_entry( sw, 42, "Some Portname" );
  p->up = true;

  // port up for the 1st time => probe_timer_entry created with UP event
  port_status_updated_hook_callback( NULL, p );

  probe_timer_entry* e = lookup_probe_timer_entry( &dpid, 42 );
  assert_true( e != NULL );
  assert_int_equal( e->state, PROBE_TIMER_STATE_SEND_DELAY );

  // port down => probe_timer_entry removed
  p->up = false;
  port_status_updated_hook_callback( NULL, p );
  e = lookup_probe_timer_entry( &dpid, 42 );
  assert_true( e == NULL );

  delete_port_entry( sw, p );
  delete_sw_entry( sw );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  revert_original( execute_timer_events );
}


static void
test_enable_discovery_when_sw_exist_then_flow_mod_add_lldp() {
  setup_discovery_mgmt();

  const uint64_t datapath_id = 0x1234;
  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";
  sw_entry* sw = update_sw_entry( &datapath_id );
  sw->up = true;

  expect_string( mock_send_message_flow_mod_for_lldp,
                 service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_flow_mod_for_lldp,
                tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_flow_mod_for_lldp,
                datapath_id, 0x1234 );
  expect_value( mock_send_message_flow_mod_for_lldp,
                command, OFPFC_ADD);
  will_return( mock_send_message_flow_mod_for_lldp, true );

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  delete_sw_entry( sw );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  teardown_discovery_mgmt();
}


static void
test_disable_discovery_when_sw_exist_then_flow_mod_del_lldp() {
  setup_discovery_mgmt();

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  const uint64_t datapath_id = 0x1234;
  sw_entry* sw = update_sw_entry( &datapath_id );
  sw->up = true;
  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";

  expect_string( mock_send_message_flow_mod_for_lldp,
                 service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_flow_mod_for_lldp,
                tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_flow_mod_for_lldp,
                datapath_id, 0x1234 );
  expect_value( mock_send_message_flow_mod_for_lldp,
                command, OFPFC_DELETE );
  will_return( mock_send_message_flow_mod_for_lldp, true );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  delete_sw_entry( sw );

  teardown_discovery_mgmt();
}


static void
test_enable_discovery_when_sw_exist_then_flow_mod_add_lldp_over_ip() {
  setup_discovery_mgmt_over_ip();

  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";
  const uint64_t datapath_id = 0x1234;
  sw_entry* sw = update_sw_entry( &datapath_id );
  sw->up = true;

  expect_string( mock_send_message_flow_mod_for_lldp_over_ip,
                service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                datapath_id, 0x1234 );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                command, OFPFC_ADD );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                nw_src, 0x01234567 );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                nw_dst, 0x89ABCDEF );
  will_return( mock_send_message_flow_mod_for_lldp_over_ip, true );

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  delete_sw_entry( sw );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  teardown_discovery_mgmt_over_ip();
}


static void
test_disable_discovery_when_sw_exist_then_flow_mod_del_lldp_over_ip() {
  setup_discovery_mgmt_over_ip();

  expect_switch_and_port_status_hook_set();
  free_event_forward_operation_to_all_request_param = true;
  expect_enable_discovery();
  enable_discovery();

  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";
  const uint64_t datapath_id = 0x1234;
  sw_entry* sw = update_sw_entry( &datapath_id );
  sw->up = true;

  expect_string( mock_send_message_flow_mod_for_lldp_over_ip,
                service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                datapath_id, 0x1234 );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                command, OFPFC_DELETE );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                nw_src, 0x01234567 );
  expect_value( mock_send_message_flow_mod_for_lldp_over_ip,
                nw_dst, 0x89ABCDEF );
  will_return( mock_send_message_flow_mod_for_lldp_over_ip, true );

  expect_switch_and_port_status_hook_clear();
  expect_disable_discovery();
  disable_discovery();

  delete_sw_entry( sw );

  teardown_discovery_mgmt_over_ip();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test_setup_teardown( test_init_finalize, setup, teardown ),
      unit_test_setup_teardown( test_init_finalize_with_always_discovery, setup, teardown ),
      unit_test_setup_teardown( test_enable_discovery_twice_prints_message, setup_discovery_mgmt, teardown_discovery_mgmt ),
      unit_test_setup_teardown( test_disable_discovery_twice_prints_message, setup_discovery_mgmt, teardown_discovery_mgmt ),

      unit_test( test_enable_discovery_when_sw_exist_then_flow_mod_add_lldp ),
      unit_test( test_enable_discovery_when_sw_exist_then_flow_mod_add_lldp_over_ip ),

      unit_test( test_disable_discovery_when_sw_exist_then_flow_mod_del_lldp ),
      unit_test( test_disable_discovery_when_sw_exist_then_flow_mod_del_lldp_over_ip ),

      unit_test( test_do_nothing_when_switch_status_event_down ),
      unit_test( test_flow_mod_lldp_when_switch_status_event_up ),
      unit_test( test_flow_mod_lldp_over_ip_when_switch_status_event_up ),
      unit_test_setup_teardown( test_port_status_event, setup_discovery_mgmt, teardown_discovery_mgmt ),
  };

  setup_leak_detector();
  return run_tests( tests );
}

