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

#include "messenger.h"
#include "openflow_application_interface.h"

#include "lldp.h"

#include "trema.h"

/********************************************************************************
 * Common function.
 ********************************************************************************/


static bool ( *original_send_message )( const char *service_name, const uint16_t tag, const void *data, size_t len );


static const uint8_t default_lldp_mac_dst[] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e };


static const uint8_t broadcast_mac_dst[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


const char* TEST_TREMA_NAME = "test_lldp";


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;


static bool
mock_send_message_lldp( const char *service_name, uint16_t tag, const void *data, size_t len ) {
  check_expected( service_name );
  check_expected( tag );

  const openflow_service_header_t* of_s_h = data;
  const uint64_t datapath_id = ntohll( of_s_h->datapath_id );
  check_expected( datapath_id );
  const size_t ofs_header_length = sizeof(openflow_service_header_t) + ntohs( of_s_h->service_name_length );

  const struct ofp_header* ofp_header = (const struct ofp_header*) (((const char*)data) + ofs_header_length);
  assert( ofp_header->type == OFPT_PACKET_OUT );

  const struct ofp_packet_out* packet_out = (const struct ofp_packet_out*) (((const char*)data) + ofs_header_length);
  const uint16_t actions_len = ntohs(packet_out->actions_len);
  assert( actions_len > 0 );
  assert( actions_len == sizeof( struct ofp_action_output ) );

  const struct ofp_action_header* act_header = (const struct ofp_action_header*) (((const char*)packet_out) + offsetof( struct ofp_packet_out, actions ));
  const uint16_t action_type = act_header->type;
  assert( action_type == OFPAT_OUTPUT );
  const uint16_t action_length = ntohs(act_header->len);// no byte order conversion?
  assert( action_length == sizeof( struct ofp_action_output ) );

  const struct ofp_action_output* act_out = (const struct ofp_action_output*) act_header;
  assert( ntohs(act_out->len) == sizeof( struct ofp_action_output ) );
  const uint16_t port = ntohs( act_out->port );
  check_expected( port );

  const ether_header_t* ether_frame = (const ether_header_t*) (((const char*)packet_out) + offsetof( struct ofp_packet_out, actions ) + actions_len);

  buffer* ether_buffer = alloc_buffer_with_length( len );
  void* parse_data = append_back_buffer( ether_buffer, len );
  memcpy( parse_data, data, len );
  remove_front_buffer( ether_buffer, (size_t)(((const char*)ether_frame) - ((const char*)data)) );
  assert_true( parse_packet( ether_buffer ) );
  packet_info packet_info = get_packet_info( ether_buffer );

  assert_int_equal( packet_info.eth_type, ETH_ETHTYPE_LLDP );
  assert_memory_equal( packet_info.eth_macda, default_lldp_mac_dst, ETH_ADDRLEN );
  const uint8_t* macsa = packet_info.eth_macsa;
  check_expected( macsa );

  // chassis
  const struct tlv* chassis_id_tlv = (const struct tlv*) (((const char*)ether_frame) + sizeof(ether_header_t));
  assert_int_equal( ((ntohs(chassis_id_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_CHASSIS_ID ); // upper 7bits in uint16_t;
  const uint16_t chassis_id_tlv_len = ntohs(chassis_id_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  const size_t chassis_id_strlen = chassis_id_tlv_len - LLDP_SUBTYPE_LEN;
  assert_in_range( chassis_id_strlen, 1, LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN );
  char chassis_id[LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN] = {};
  memcpy( chassis_id, chassis_id_tlv->val+LLDP_SUBTYPE_LEN, chassis_id_strlen );

  //  struct tlv* port_id_tlv;
  const struct tlv* port_id_tlv = (const struct tlv*) (((const char*)chassis_id_tlv) + LLDP_TLV_HEAD_LEN + chassis_id_tlv_len);
  assert_int_equal( ((ntohs(port_id_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_PORT_ID ); // upper 7bits in uint16_t;
  const uint16_t port_id_tlv_len = ntohs(port_id_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  const size_t port_id_strlen = port_id_tlv_len - LLDP_SUBTYPE_LEN;
  assert_in_range( port_id_strlen, 1, LLDP_TLV_PORT_ID_INFO_MAX_LEN );
  char port_id[LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN] = {};
  memcpy( port_id, port_id_tlv->val+LLDP_SUBTYPE_LEN, port_id_strlen );

  //  struct tlv* ttl_tlv;
  const struct tlv* ttl_tlv = (const struct tlv*) (((const char*)port_id_tlv) + LLDP_TLV_HEAD_LEN + port_id_tlv_len);
  assert_int_equal( ((ntohs(ttl_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_TTL ); // upper 7bits in uint16_t;
  const uint16_t ttl_tlv_len = ntohs(ttl_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  assert_int_equal( ttl_tlv_len, sizeof(uint16_t) );
  assert_int_equal( ntohs( *((const uint16_t*)ttl_tlv->val) ), LLDP_DEFAULT_TTL );

  //  struct tlv* end_tlv;
  const struct tlv* end_tlv = (const struct tlv*) (((const char*)ttl_tlv) + LLDP_TTL_LEN );
  assert_int_equal( ((ntohs(end_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_END ); // upper 7bits in uint16_t;
  const uint16_t end_tlv_len = ntohs(end_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  assert_int_equal( end_tlv_len, 0 );

  // test parse_lldp() against self parsed result.
  uint64_t dpid;
  uint16_t port_no;
  assert_true( parse_lldp( &dpid, &port_no, ether_buffer ) );

  assert_int_equal( dpid, datapath_id );
  assert_int_equal( dpid, strtoull( chassis_id, NULL, 16 ) );
  check_expected( dpid );

  assert_int_equal( port_no, atoi(port_id) );
  check_expected( port_no );

  free_buffer( ether_buffer );

  return ( bool ) mock();
}


static bool
mock_send_message_lldp_over_ip( const char* service_name, uint16_t tag, const void *data, size_t len ) {
  check_expected( service_name );
  check_expected( tag );

  const openflow_service_header_t* of_s_h = data;
  const uint64_t datapath_id = ntohll( of_s_h->datapath_id );
  check_expected( datapath_id );
  const size_t header_length = sizeof(openflow_service_header_t) + ntohs( of_s_h->service_name_length );

  const struct ofp_header* ofp_header = (const struct ofp_header*) (((const char*)data) + header_length);
  assert( ofp_header->type == OFPT_PACKET_OUT );

  const struct ofp_packet_out* packet_out = (const struct ofp_packet_out*) (((const char*)data) + header_length);
  const uint16_t actions_len = ntohs(packet_out->actions_len);
  assert( actions_len > 0 );
  assert( actions_len == sizeof( struct ofp_action_output ) );

  const struct ofp_action_header* act_header = (const struct ofp_action_header*) (((const char*)packet_out) + offsetof( struct ofp_packet_out, actions ));
  const uint16_t action_type = act_header->type;
  assert( action_type == OFPAT_OUTPUT );
  const uint16_t action_length = ntohs(act_header->len);// no byte order conversion?
  assert( action_length == sizeof( struct ofp_action_output ) );

  const struct ofp_action_output* act_out = (const struct ofp_action_output*) act_header;
  assert( ntohs(act_out->len) == sizeof( struct ofp_action_output ) );
  const uint16_t port = ntohs( act_out->port );
  check_expected( port );

  const ether_header_t* ether_frame = (const ether_header_t*) (((const char*)packet_out) + offsetof( struct ofp_packet_out, actions ) + actions_len);

  buffer* ether_buffer = alloc_buffer_with_length( len );
  void* parse_data = append_back_buffer( ether_buffer, len );
  memcpy( parse_data, data, len );
  remove_front_buffer( ether_buffer, (size_t)(((const char*)ether_frame) - ((const char*)data)) );
  assert_true( parse_packet( ether_buffer ) );
  packet_info packet_info = get_packet_info( ether_buffer );

  assert_int_equal( packet_info.eth_type, ETH_ETHTYPE_IPV4 );
  assert_memory_equal( packet_info.eth_macda, broadcast_mac_dst, ETH_ADDRLEN );
  const uint8_t* macsa = packet_info.eth_macsa;
  check_expected( macsa );

//  const ipv4_header_t* ip = (const ipv4_header_t*) (((const char*)ether_frame) + sizeof(ether_header_t));
  const uint32_t src_ip = packet_info.ipv4_saddr;
  check_expected( src_ip );
  const uint32_t dst_ip = packet_info.ipv4_daddr;
  check_expected( dst_ip );

//  const etherip_header* etherip = (const etherip_header*) (((const char*)ip) + sizeof(ipv4_header_t));
  const etherip_header* etherip = (const etherip_header*) packet_info.l3_payload;

  // ip payload ether
  const ether_header_t* etherip_frame = (const ether_header_t*) (((const char*)etherip) + sizeof(etherip_header));
  assert_int_equal( ntohs(etherip_frame->type), ETH_ETHTYPE_LLDP );
  assert_memory_equal( etherip_frame->macda, default_lldp_mac_dst, ETH_ADDRLEN );
  assert_memory_equal( etherip_frame->macsa, ether_frame->macsa, ETH_ADDRLEN );

  // chassis
  const struct tlv* chassis_id_tlv = (const struct tlv*) (((const char*)etherip_frame) + sizeof(ether_header_t));
  assert_int_equal( ((ntohs(chassis_id_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_CHASSIS_ID ); // upper 7bits in uint16_t;
  const uint16_t chassis_id_tlv_len = ntohs(chassis_id_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  const size_t chassis_id_strlen = chassis_id_tlv_len - LLDP_SUBTYPE_LEN;
  assert_in_range( chassis_id_strlen, 1, LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN );
  char chassis_id[LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN] = {};
  memcpy( chassis_id, chassis_id_tlv->val+LLDP_SUBTYPE_LEN, chassis_id_strlen );

  //  struct tlv* port_id_tlv;
  const struct tlv* port_id_tlv = (const struct tlv*) (((const char*)chassis_id_tlv) + LLDP_TLV_HEAD_LEN + chassis_id_tlv_len);
  assert_int_equal( ((ntohs(port_id_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_PORT_ID ); // upper 7bits in uint16_t;
  const uint16_t port_id_tlv_len = ntohs(port_id_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  const size_t port_id_strlen = port_id_tlv_len - LLDP_SUBTYPE_LEN;
  assert_in_range( port_id_strlen, 1, LLDP_TLV_PORT_ID_INFO_MAX_LEN );
  char port_id[LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN] = {};
  memcpy( port_id, port_id_tlv->val+LLDP_SUBTYPE_LEN, port_id_strlen );

  //  struct tlv* ttl_tlv;
  const struct tlv* ttl_tlv = (const struct tlv*) (((const char*)port_id_tlv) + LLDP_TLV_HEAD_LEN + port_id_tlv_len);
  assert_int_equal( ((ntohs(ttl_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_TTL ); // upper 7bits in uint16_t;
  const uint16_t ttl_tlv_len = ntohs(ttl_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  assert_int_equal( ttl_tlv_len, sizeof(uint16_t) );
  assert_int_equal( ntohs( *((const uint16_t*)ttl_tlv->val) ), LLDP_DEFAULT_TTL );

  //  struct tlv* end_tlv;
  const struct tlv* end_tlv = (const struct tlv*) (((const char*)ttl_tlv) + LLDP_TTL_LEN );
  assert_int_equal( ((ntohs(end_tlv->type_len) & 0xFE00)>>9), LLDP_TYPE_END ); // upper 7bits in uint16_t;
  const uint16_t end_tlv_len = ntohs(end_tlv->type_len) & 0x1FF; // lower 9bits in uint16_t;
  assert_int_equal( end_tlv_len, 0 );

  // test parse_lldp() against self parsed result.
  uint64_t dpid;
  uint16_t port_no;
  assert_true( parse_lldp( &dpid, &port_no, ether_buffer ) );

  assert_int_equal( dpid, datapath_id );
  assert_int_equal( dpid, strtoull( chassis_id, NULL, 16 ) );
  check_expected( dpid );

  assert_int_equal( port_no, atoi(port_id) );
  check_expected( port_no );

  free_buffer( ether_buffer );

  return ( bool ) mock();
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/


static void
setup() {
  init_messenger("/tmp");
  init_timer();
  init_stat();
  init_openflow_application_interface( TEST_TREMA_NAME );
}


static void
teardown() {
  finalize_openflow_application_interface();
  finalize_timer();
  finalize_stat();
  finalize_messenger();
}


static void
setup_mock_message_lldp() {
  original_send_message = send_message;
  send_message = mock_send_message_lldp;
}


static void
teardown_mock_message_lldp() {
  send_message = original_send_message;
}


static void
setup_mock_message_lldp_over_ip() {
  original_send_message = send_message;
  send_message = mock_send_message_lldp_over_ip;
}


static void
teardown_mock_message_lldp_over_ip() {
  send_message = original_send_message;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/


//bool send_lldp( probe_timer_entry *port );
static void
test_send_lldp() {
  setup();

  lldp_options options = {
      .lldp_mac_dst = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e },
      .lldp_over_ip = false,
  };

  assert_true( init_lldp( options ) );

  uint64_t datapath_id = 0x1234;
  uint16_t port_no = 42;
  uint8_t mac[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";

  expect_string( mock_send_message_lldp, service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_lldp, tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_lldp, datapath_id, 0x1234 );
  expect_value( mock_send_message_lldp, port, 42 );
  expect_memory( mock_send_message_lldp, macsa, mac, ETH_ADDRLEN );
  expect_value( mock_send_message_lldp, dpid, 0x1234 );
  expect_value( mock_send_message_lldp, port_no, 42 );

  will_return( mock_send_message_lldp, true );

  assert_true( send_lldp( mac, datapath_id, port_no ) );

  assert_true( finalize_lldp() );


  teardown();
}

static void
test_send_lldp_over_ip() {
  setup();

  lldp_options options = {
      .lldp_mac_dst = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e },
      .lldp_over_ip = true,
      .lldp_ip_src = ((127<< 24) + (0<< 16) + (1 <<8) + 1),
      .lldp_ip_dst = ((127<< 24) + (1<< 16) + (2 <<8) + 2),
  };

  assert_true( init_lldp( options ) );

  uint64_t datapath_id = 0x1234;
  uint16_t port_no = 42;
  uint8_t mac[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

  const char* SRC_SW_MSNGER_NAME = "switch.0x1234";

  expect_string( mock_send_message_lldp_over_ip, service_name, SRC_SW_MSNGER_NAME );
  expect_value( mock_send_message_lldp_over_ip, tag, MESSENGER_OPENFLOW_MESSAGE );
  expect_value( mock_send_message_lldp_over_ip, datapath_id, 0x1234 );
  expect_value( mock_send_message_lldp_over_ip, port, 42 );
  expect_value( mock_send_message_lldp_over_ip, src_ip, options.lldp_ip_src );
  expect_value( mock_send_message_lldp_over_ip, dst_ip, options.lldp_ip_dst );
  expect_memory( mock_send_message_lldp_over_ip, macsa, mac, ETH_ADDRLEN );
  expect_value( mock_send_message_lldp_over_ip, dpid, 0x1234 );
  expect_value( mock_send_message_lldp_over_ip, port_no, 42 );
  will_return( mock_send_message_lldp_over_ip, true );

  assert_true( send_lldp( mac, datapath_id, port_no ) );

  assert_true( finalize_lldp() );

  teardown();
}

//bool init_lldp( lldp_options options );
//bool finalize_lldp( void );
static void
test_init_finalize_lldp() {
  lldp_options options = {
      .lldp_mac_dst = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e },
      .lldp_over_ip = false,
  };

  assert_true( init_lldp( options ) );
  assert_true( finalize_lldp() );
}
/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test_setup_teardown( test_send_lldp, setup_mock_message_lldp, teardown_mock_message_lldp ),
      unit_test_setup_teardown( test_send_lldp_over_ip, setup_mock_message_lldp_over_ip, teardown_mock_message_lldp_over_ip ),
      unit_test_setup_teardown( test_init_finalize_lldp, setup, teardown ),
  };

  setup_leak_detector();
  return run_tests( tests );
}

