/*
 * Unit tests of functions for creating OpenFlow messages.
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


#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/ip.h>
#include <openflow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "byteorder.h"
#include "checks.h"
#include "cmockery_trema.h"
#include "log.h"
#include "openflow_message.h"
#include "wrapper.h"


extern uint16_t get_actions_length( const openflow_actions *actions );
extern buffer * create_stats_request( const uint32_t transaction_id, const uint16_t type,
                                      const uint16_t length, const uint16_t flags );
extern buffer * create_stats_reply( const uint32_t transaction_id, const uint16_t type,
                                    const uint16_t length, const uint16_t flags );

static const uint16_t ARP_OP_MASK = 0x00ff; // 8bits
static const uint32_t BUFFER_ID = 0x12345678;
static const uint16_t NO_FLAGS = 0;
static const uint16_t NO_ERROR = 0;
static const uint16_t PRIORITY = 65535;
static const uint32_t MY_TRANSACTION_ID = 0x04030201;
static const uint16_t ONE_MINUTES_TIMEOUT = 60;
static const uint64_t ISSUED_COOKIE = 0x0102030405060708ULL;
static const uint64_t RECEIVED_PACKETS = 50000;
static const uint64_t RECEIVED_BYTES = 300000;
static const uint64_t TRANSMITTED_PACKETS = 50000;
static const uint64_t TRANSMITTED_BYTES = 300000;
static const uint32_t SUPPORTED_ENTRIES_IN_OFPST_TABLE = 10000;
static const uint32_t ACTIVE_ENTRIES_IN_OFPST_TABLE = 1000;
static const uint64_t PACKETS_LOOK_UP_IN_OFPST_TABLE = 10000;
static const uint64_t PACKETS_HIT_OFPST_TABLE = 100;
static const uint32_t VENDOR_ID = 0x00004cff;
static const uint16_t VENDOR_STATS_FLAG = 0xaabb;
static const uint8_t HW_ADDR[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
static const uint32_t NW_ADDR = 0x12345678;
static const uint8_t NW_TOS = 0xfc;
static const uint16_t MAX_LENGTH_OF_SEND_PACKET = 128;
static const uint16_t SHORT_DATA_LENGTH = 32;
static const uint16_t LONG_DATA_LENGTH = 64;
static const struct ofp_match MATCH = { 0, 1,
                                        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 },
                                        { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d },
                                        1, 1, { 0 }, 0x800, 0xfc, 0x6, { 0, 0 },
                                        0x0a090807, 0x0a090807, 1024, 2048 };
static const uint32_t PORT_FEATURES = ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD |
                                        OFPPF_100MB_FD | OFPPF_1GB_HD | OFPPF_1GB_FD |
                                        OFPPF_COPPER |  OFPPF_AUTONEG | OFPPF_PAUSE );


/********************************************************************************
 * Mock function.
 ********************************************************************************/

#define FAKE_PID 1234;

pid_t
mock_getpid() {
  return FAKE_PID;
}


void
mock_die( char *format, ... ) {
  UNUSED( format );
}


void
mock_debug( char *format, ... ) {
  UNUSED( format );
}


static int
mock_get_logging_level() {
  return LOG_DEBUG;
}


/********************************************************************************
 * Common function.
 ********************************************************************************/

static buffer *
create_dummy_data( uint16_t length ) {
  buffer *data = alloc_buffer_with_length( length );
  void *p = append_back_buffer( data, length );
  memset( p, 0xaf, length );

  return data;
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

static void
init() {
  init_openflow_message();
  get_logging_level = mock_get_logging_level;
}


static void
teardown() {
  init_openflow_message();
}


/********************************************************************************
 * Initialization test.
 ********************************************************************************/

static void
test_init_openflow_message() {
  bool ret;
  uint32_t transaction_id;
  pid_t pid = FAKE_PID;

  transaction_id = ( uint32_t ) ( pid << 16 ) + 1;

  ret = init_openflow_message();

  assert_true( ret );
  assert_int_equal( ( int ) get_transaction_id(), ( int ) transaction_id );
}


/********************************************************************************
 * get_transaction_id() tests.
 ********************************************************************************/

static void
test_get_transaction_id() {
  uint32_t transaction_id;
  pid_t pid = FAKE_PID;

  transaction_id = ( uint32_t ) ( pid << 16 ) + 1;

  assert_int_equal( ( int ) get_transaction_id(), ( int ) transaction_id );
}


static void
test_get_transaction_id_if_id_overflows() {
  int i;
  uint32_t transaction_id;
  pid_t pid = FAKE_PID;

  for ( i = 0; i < 0xffff; i++ ) {
    get_transaction_id();
  }

  transaction_id = ( uint32_t ) ( pid << 16 );

  assert_int_equal( ( int ) get_transaction_id(), ( int ) transaction_id );
}


/********************************************************************************
 * get_get_cookie() tests.
 ********************************************************************************/

static void
test_get_cookie() {
  pid_t pid = FAKE_PID;

  uint64_t expected_cookie = ( ( uint64_t ) pid << 48 ) + 1;
  uint64_t tmp_cookie = get_cookie();
  assert_memory_equal( &tmp_cookie, &expected_cookie, sizeof( uint64_t ) );
}


extern uint64_t cookie;

static void
test_get_cookie_if_cookie_overflows() {
  pid_t pid = FAKE_PID;
  cookie = ( ( uint64_t ) pid << 48 ) | ( UINT64_MAX >> 16 );

  uint64_t expected_cookie = ( ( uint64_t ) pid << 48 );
  uint64_t tmp_cookie = get_cookie();
  assert_memory_equal( &tmp_cookie, &expected_cookie, sizeof( uint64_t ) );
}


/********************************************************************************
 * Tests of functions for OFPT_HELLO.
 ********************************************************************************/

static void
test_create_hello() {
  buffer *buffer = create_hello( MY_TRANSACTION_ID );
  assert_true( buffer != NULL );

  struct ofp_header *hello = buffer->data;
  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_header ) );
  assert_int_equal( hello->version, OFP_VERSION );
  assert_int_equal( hello->type, OFPT_HELLO );
  assert_int_equal( ntohs( hello->length ), sizeof( struct ofp_header ) );
  assert_int_equal( ( int ) ntohl( hello->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


static void
test_validate_hello() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );
  assert_true( validate_hello( hello ) == SUCCESS );
  free_buffer( hello );
}


static void
test_validate_hello_fails_with_NULL() {
  expect_assert_failure( validate_hello( NULL ) );
}


static void
test_validate_hello_fails_with_non_hello_message() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );
  assert_int_equal( ERROR_INVALID_TYPE, validate_hello( echo_request ) );
  free_buffer( echo_request );
}


/********************************************************************************
 * Tests of a function for OFPT_ERROR.
 ********************************************************************************/

static void
test_create_error() {
  uint16_t type = OFPET_HELLO_FAILED;
  uint16_t code = OFPHFC_INCOMPATIBLE;
  buffer *data;
  buffer *buffer;
  struct ofp_error_msg *error_msg;
  uint16_t length;

  data = create_hello( MY_TRANSACTION_ID );

  buffer = create_error( MY_TRANSACTION_ID, type, code, data );
  assert_true( buffer != NULL );

  error_msg = ( struct ofp_error_msg * ) buffer->data;
  length = ( uint16_t ) ( sizeof( struct ofp_error_msg ) + data->length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( error_msg->header.version, OFP_VERSION );
  assert_int_equal( error_msg->header.type, OFPT_ERROR );
  assert_int_equal( ntohs( error_msg->header.length ), length );
  assert_int_equal( ( int ) ntohl( error_msg->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( error_msg->type ), type );
  assert_int_equal( ntohs( error_msg->code ), code );
  assert_memory_equal( error_msg->data, data->data, data->length );

  free_buffer( data );
  free_buffer( buffer );
}


static void
test_create_error_without_data() {
  uint16_t type = OFPET_HELLO_FAILED;
  uint16_t code = OFPHFC_INCOMPATIBLE;
  buffer *data = NULL;
  buffer *buffer;
  struct ofp_error_msg *error_msg;
  uint16_t length;

  buffer = create_error( MY_TRANSACTION_ID, type, code, data );
  assert_true( buffer != NULL );

  error_msg = ( struct ofp_error_msg * ) buffer->data;
  length = ( uint16_t ) ( sizeof( struct ofp_error_msg ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( error_msg->header.version, OFP_VERSION );
  assert_int_equal( error_msg->header.type, OFPT_ERROR );
  assert_int_equal( ntohs( error_msg->header.length ), length );
  assert_int_equal( ( int ) ntohl( error_msg->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( error_msg->type ), type );
  assert_int_equal( ntohs( error_msg->code ), code );

  free_buffer( buffer );
}


/********************************************************************************
 * Tests of functions for OFPT_ECHO_REQUEST.
 ********************************************************************************/

static void
test_create_echo_request() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *buffer = create_echo_request( MY_TRANSACTION_ID, body );
  assert_true( buffer != NULL );

  struct ofp_header *echo_request = buffer->data;
  uint16_t length = ( uint16_t ) ( sizeof( struct ofp_header ) + body->length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( echo_request->version, OFP_VERSION );
  assert_int_equal( echo_request->type, OFPT_ECHO_REQUEST );
  assert_int_equal( ntohs( echo_request->length ), length );
  assert_int_equal( ( int ) ntohl( echo_request->xid ), ( int ) MY_TRANSACTION_ID );

  assert_memory_equal( ( char * ) buffer->data + sizeof( struct ofp_header ), body->data, body->length );

  free_buffer( body );
  free_buffer( buffer );
}


static void
test_create_echo_request_without_data() {
  buffer *buffer = create_echo_request( MY_TRANSACTION_ID, NULL );
  assert_true( buffer != NULL );

  struct ofp_header *echo_request = buffer->data;
  uint16_t length = ( uint16_t ) sizeof( struct ofp_header );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( echo_request->version, OFP_VERSION );
  assert_int_equal( echo_request->type, OFPT_ECHO_REQUEST );
  assert_int_equal( ntohs( echo_request->length ), length );
  assert_int_equal( ( int ) ntohl( echo_request->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


static void
test_validate_echo_request() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, body );

  assert_true( validate_echo_request( echo_request ) == SUCCESS );

  free_buffer( body );
  free_buffer( echo_request );
}


static void
test_validate_echo_request_fails_with_NULL() {
  expect_assert_failure( validate_echo_request( NULL ) );
}


static void
test_validate_echo_request_fails_with_non_echo_request_message() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );

  assert_int_equal( validate_echo_request( hello ), ERROR_INVALID_TYPE );

  free_buffer( hello );
}


/********************************************************************************
 * Tests of functions for OFPT_ECHO_REPLY.
 ********************************************************************************/

static void
test_create_echo_reply() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *buffer = create_echo_reply( MY_TRANSACTION_ID, body );
  assert_true( buffer != NULL );

  struct ofp_header *echo_reply = ( struct ofp_header * ) buffer->data;
  uint16_t length = ( uint16_t ) ( sizeof( struct ofp_header ) + body->length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( echo_reply->version, OFP_VERSION );
  assert_int_equal( echo_reply->type, OFPT_ECHO_REPLY );
  assert_int_equal( ntohs( echo_reply->length ), length );
  assert_int_equal( ( int ) ntohl( echo_reply->xid ), ( int ) MY_TRANSACTION_ID );

  assert_memory_equal( ( char * ) buffer->data + sizeof( struct ofp_header ), body->data, body->length );

  free_buffer( body );
  free_buffer( buffer );
}


static void
test_create_echo_reply_without_data() {
  buffer *buffer = create_echo_reply( MY_TRANSACTION_ID, NULL );
  assert_true( buffer != NULL );

  struct ofp_header *echo_reply = ( struct ofp_header * ) buffer->data;
  uint16_t length = ( uint16_t ) sizeof( struct ofp_header );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( echo_reply->version, OFP_VERSION );
  assert_int_equal( echo_reply->type, OFPT_ECHO_REPLY );
  assert_int_equal( ntohs( echo_reply->length ), length );
  assert_int_equal( ( int ) ntohl( echo_reply->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


static void
test_validate_echo_reply() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *echo_reply = create_echo_reply( MY_TRANSACTION_ID, body );

  assert_true( validate_echo_reply( echo_reply ) == SUCCESS );

  free_buffer( body );
  free_buffer( echo_reply );
}


static void
test_validate_echo_reply_fails_with_NULL() {
  expect_assert_failure( validate_echo_reply( NULL ) );
}


static void
test_validate_echo_reply_fails_with_non_echo_reply_message() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );

  assert_int_equal( validate_echo_reply( hello ), ERROR_INVALID_TYPE );

  free_buffer( hello );
}


/********************************************************************************
 * Tests of a function for OFPT_VENDOR.
 ********************************************************************************/

static void
test_create_vendor() {
  const uint16_t body_length = 128;

  buffer *body = create_dummy_data( body_length );
  buffer *buffer = create_vendor( MY_TRANSACTION_ID, VENDOR_ID, body );
  assert_true( buffer != NULL );

  assert_int_equal( ( int ) buffer->length,
                    ( int ) sizeof( struct ofp_vendor_header ) + body_length );

  struct ofp_vendor_header *vendor = buffer->data;

  assert_int_equal( vendor->header.version, OFP_VERSION );
  assert_int_equal( vendor->header.type, OFPT_VENDOR );
  assert_int_equal( ( int ) ntohs( vendor->header.length ),
                    ( int ) sizeof( struct ofp_vendor_header ) + body_length );
  assert_int_equal( ( int ) ntohl( vendor->header.xid ),
                    ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohl( vendor->vendor ), ( int ) VENDOR_ID );
  assert_memory_equal( ( char * ) vendor + sizeof( struct ofp_vendor_header ),
                       body->data,
                       body_length );

  free_buffer( body );
  free_buffer( buffer );
}


static void
test_create_vendor_without_data() {
  buffer *buffer = create_vendor( MY_TRANSACTION_ID, VENDOR_ID, NULL );
  assert_true( buffer != NULL );

  assert_int_equal( ( int ) buffer->length,
                    sizeof( struct ofp_vendor_header ) );

  struct ofp_vendor_header *vendor = buffer->data;

  assert_int_equal( vendor->header.version, OFP_VERSION );
  assert_int_equal( vendor->header.type, OFPT_VENDOR );
  assert_int_equal( ( int ) ntohs( vendor->header.length ),
                    sizeof( struct ofp_vendor_header ) );
  assert_int_equal( ( int ) ntohl( vendor->header.xid ),
                    ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohl( vendor->vendor ), ( int ) VENDOR_ID );

  free_buffer( buffer );
}


/********************************************************************************
 * Tests of functions for OFPT_FEATURES_REQUEST.
 ********************************************************************************/

static void
test_create_features_request() {
  buffer *buffer = create_features_request( MY_TRANSACTION_ID );
  assert_true( buffer != NULL );

  struct ofp_header *features_request = ( struct ofp_header * ) buffer->data;
  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_header ) );
  assert_int_equal( features_request->version, OFP_VERSION );
  assert_int_equal( features_request->type, OFPT_FEATURES_REQUEST );
  assert_int_equal( ntohs( features_request->length ), sizeof( struct ofp_header ) );
  assert_int_equal( ( int ) ntohl( features_request->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


static void
test_validate_features_request() {
  buffer *features_request = create_features_request( MY_TRANSACTION_ID );
  assert_true( validate_features_request( features_request ) == SUCCESS );
  free_buffer( features_request );
}


static void
test_validate_features_request_fails_with_NULL() {
  expect_assert_failure( validate_features_request( NULL ) );
}


static void
test_validate_features_request_fails_with_non_features_request_message() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );
  assert_int_equal( validate_features_request( hello ), ERROR_INVALID_TYPE );
  free_buffer( hello );
}


/********************************************************************************
 * Test of a function for OFPT_GET_CONFIG_REQUEST.
 ********************************************************************************/

static void
test_create_get_config_request() {
  buffer *buffer;
  struct ofp_header *get_config_request;

  buffer = create_get_config_request( MY_TRANSACTION_ID );
  assert_true( buffer != NULL );

  get_config_request = buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_header ) );
  assert_int_equal( get_config_request->version, OFP_VERSION );
  assert_int_equal( get_config_request->type, OFPT_GET_CONFIG_REQUEST );
  assert_int_equal( ntohs( get_config_request->length ), sizeof( struct ofp_header ) );
  assert_int_equal( ( int ) ntohl( get_config_request->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


/********************************************************************************
 * Test of a function for OFPT_GET_CONFIG_REPLY.
 ********************************************************************************/

static void
test_create_get_config_reply() {
  uint16_t flags = OFPC_FRAG_NORMAL;
  uint16_t miss_send_len = OFP_DEFAULT_MISS_SEND_LEN;
  buffer *buffer;
  struct ofp_switch_config *switch_config;

  buffer = create_get_config_reply( MY_TRANSACTION_ID, flags, miss_send_len );
  assert_true( buffer != NULL );

  switch_config = ( struct ofp_switch_config * ) buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_switch_config ) );
  assert_int_equal( switch_config->header.version, OFP_VERSION );
  assert_int_equal( switch_config->header.type, OFPT_GET_CONFIG_REPLY );
  assert_int_equal( ntohs( switch_config->header.length ), sizeof( struct ofp_switch_config ) );
  assert_int_equal( ( int ) ntohl( switch_config->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( switch_config->flags ), flags );
  assert_int_equal( ntohs( switch_config->miss_send_len ), miss_send_len );

  free_buffer( buffer );
}


/********************************************************************************
 * Tests of functions for OFPT_SET_CONFIG.
 ********************************************************************************/

static void
test_create_set_config() {
  buffer *buffer = create_set_config( MY_TRANSACTION_ID, OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );
  assert_true( buffer != NULL );

  struct ofp_switch_config *switch_config = buffer->data;
  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_switch_config ) );
  assert_int_equal( switch_config->header.version, OFP_VERSION );
  assert_int_equal( switch_config->header.type, OFPT_SET_CONFIG );
  assert_int_equal( ntohs( switch_config->header.length ), sizeof( struct ofp_switch_config ) );
  assert_int_equal( ( int ) ntohl( switch_config->header.xid ), ( int ) MY_TRANSACTION_ID );
  assert_int_equal( ntohs( switch_config->flags ), OFPC_FRAG_NORMAL );
  assert_int_equal( ntohs( switch_config->miss_send_len ), OFP_DEFAULT_MISS_SEND_LEN );

  free_buffer( buffer );
}


static void
test_validate_set_config() {
  buffer *set_config = create_set_config( MY_TRANSACTION_ID, OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );
  assert_true( validate_set_config( set_config ) == SUCCESS );
  free_buffer( set_config );
}


static void
test_validate_set_config_fails_with_NULL() {
  expect_assert_failure( validate_set_config( NULL ) );
}


static void
test_validate_set_config_fails_with_non_set_config_message() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );
  assert_int_equal( validate_set_config( hello ), ERROR_INVALID_TYPE );
  free_buffer( hello );
}


/********************************************************************************
 * Test of a function for OFPT_FLOW_REMOVED.
 ********************************************************************************/

static void
test_create_flow_removed() {
  uint64_t cookie = 0x0102030405060708ULL;
  uint8_t reason = OFPRR_IDLE_TIMEOUT;
  uint32_t duration_sec = 180;
  uint32_t duration_nsec = 10000;
  uint16_t idle_timeout = 60;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 100000;
  buffer *buffer;
  struct ofp_flow_removed *flow_removed;
  uint64_t tmp;

  buffer = create_flow_removed( MY_TRANSACTION_ID, MATCH, cookie, PRIORITY, reason, duration_sec,
                                duration_nsec, idle_timeout, packet_count, byte_count );
  assert_true( buffer != NULL );

  flow_removed = buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_flow_removed ) );
  assert_int_equal( flow_removed->header.version, OFP_VERSION );
  assert_int_equal( flow_removed->header.type, OFPT_FLOW_REMOVED );
  assert_int_equal( ntohs( flow_removed->header.length ), sizeof( struct ofp_flow_removed ) );
  assert_int_equal( ( int ) ntohl( flow_removed->header.xid ), ( int ) MY_TRANSACTION_ID );

  ntoh_match( &flow_removed->match, &flow_removed->match );
  assert_memory_equal( &flow_removed->match, &MATCH, sizeof( MATCH ) );

  tmp = ntohll( flow_removed->cookie );
  assert_memory_equal( &tmp, &cookie, sizeof( cookie ) );
  assert_int_equal( ntohs( flow_removed->priority ), PRIORITY );
  assert_int_equal( flow_removed->reason, reason );
  assert_int_equal( ( int ) ntohl( flow_removed->duration_sec ), ( int ) duration_sec );
  assert_int_equal( ( int ) ntohl( flow_removed->duration_nsec ), ( int ) duration_nsec );
  assert_int_equal( ntohs( flow_removed->idle_timeout ), idle_timeout );
  tmp = ntohll( flow_removed->packet_count );
  assert_memory_equal( &tmp, &packet_count, sizeof( packet_count ) );
  tmp = ntohll( flow_removed->byte_count );
  assert_memory_equal( &tmp, &byte_count, sizeof( byte_count ) );

  {
    void *pad = xmalloc( sizeof( flow_removed->pad ) );
    memset( pad, 0, sizeof( flow_removed->pad ) );
    assert_memory_equal( flow_removed->pad, pad, sizeof( flow_removed->pad ) );
    xfree( pad );

    void *pad2 = xmalloc( sizeof( flow_removed->pad2 ) );
    memset( pad2, 0, sizeof( flow_removed->pad2 ) );
    assert_memory_equal( flow_removed->pad2, pad2, sizeof( flow_removed->pad2 ) );
    xfree( pad2 );
  }

  free_buffer( buffer );
}


/********************************************************************************
 * Test of a function for OFPT_PORT_STATUS.
 ********************************************************************************/

static void
test_create_port_status() {
  uint8_t reason = OFPPR_ADD;
  struct ofp_phy_port desc;
  buffer *buffer;
  struct ofp_port_status *port_status;

  desc.port_no = 1;
  memcpy( desc.hw_addr, HW_ADDR, sizeof( desc.hw_addr ) );
  memset( desc.name, '\0', OFP_MAX_PORT_NAME_LEN );
  strcpy( desc.name, "Navy" );
  desc.config = OFPPC_PORT_DOWN;
  desc.state = OFPPS_LINK_DOWN;
  desc.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  desc.advertised = PORT_FEATURES;
  desc.supported = PORT_FEATURES;
  desc.peer = PORT_FEATURES;

  buffer = create_port_status( MY_TRANSACTION_ID, reason, desc );
  assert_true( buffer != NULL );

  port_status = buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_port_status ) );
  assert_int_equal( port_status->header.version, OFP_VERSION );
  assert_int_equal( port_status->header.type, OFPT_PORT_STATUS );
  assert_int_equal( ntohs( port_status->header.length ), sizeof( struct ofp_port_status ) );
  assert_int_equal( ( int ) ntohl( port_status->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( port_status->reason, reason );
  {
    void *pad = xmalloc( sizeof( port_status->pad ) );
    memset( pad, 0, sizeof( port_status->pad ) );
    assert_memory_equal( port_status->pad, pad, sizeof( port_status->pad ) );
    xfree( pad );
  }
  {
    struct ofp_phy_port d;
    ntoh_phy_port( &d, &port_status->desc );
    assert_memory_equal( &d, &desc, sizeof( struct ofp_phy_port ) );
  }

  free_buffer( buffer );
}


/********************************************************************************
 * Test of a function for OFPT_PORT_MOD.
 ********************************************************************************/

static void
test_create_port_mod() {
  uint16_t port_no = 1;
  uint32_t config = OFPPC_PORT_DOWN;
  uint32_t mask = 0xffffffff;
  uint32_t advertise = 1;
  buffer *buffer;
  struct ofp_port_mod *port_mod;

  buffer = create_port_mod( MY_TRANSACTION_ID, port_no, HW_ADDR, config, mask, advertise );
  assert_true( buffer != NULL );

  port_mod = ( struct ofp_port_mod * ) buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_port_mod ) );
  assert_int_equal( port_mod->header.version, OFP_VERSION );
  assert_int_equal( port_mod->header.type, OFPT_PORT_MOD );
  assert_int_equal( ntohs( port_mod->header.length ), sizeof( struct ofp_port_mod ) );
  assert_int_equal( ( int ) ntohl( port_mod->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( port_mod->port_no ), port_no );
  assert_memory_equal( port_mod->hw_addr, HW_ADDR, sizeof( HW_ADDR ) );
  assert_int_equal( ( int ) ntohl( port_mod->config ), ( int ) config );
  assert_int_equal( ( int ) ntohl( port_mod->mask ), ( int ) mask );
  assert_int_equal( ( int ) ntohl( port_mod->advertise ), ( int ) advertise );

  {
    void *pad = xmalloc( sizeof( port_mod->pad ) );
    memset( pad, 0, sizeof( port_mod->pad ) );
    assert_memory_equal( port_mod->pad, pad, sizeof( port_mod->pad ) );
    xfree( pad );
  }

  free_buffer( buffer );
}


/********************************************************************************
 * Test of functions for creating/deleting actions.
 ********************************************************************************/

static void
test_create_and_delete_actions() {
  openflow_actions *actions = create_actions();
  bool ret;

  assert_true( actions != NULL );
  assert_int_equal( actions->n_actions, 0 );

  ret = delete_actions( actions );
  assert_true( ret );

  actions = NULL;
  expect_assert_failure( delete_actions( NULL ) );
}


/********************************************************************************
 * append_action_output() test.
 ********************************************************************************/

static void
test_append_action_output() {
  openflow_actions *actions = NULL;
  uint16_t port = 1;
  uint16_t max_len = 128;
  struct ofp_action_output *action_output;
  bool ret;

  expect_assert_failure( append_action_output( actions, port, max_len ) );

  actions = create_actions();

  ret = append_action_output( actions, port, max_len );
  assert_true( ret );

  action_output = actions->list->data;

  assert_int_equal( action_output->type, OFPAT_OUTPUT );
  assert_int_equal( action_output->len, sizeof( struct ofp_action_output ) );
  assert_int_equal( action_output->port, port );
  assert_int_equal( action_output->max_len, max_len );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_vlan_vid() test.
 ********************************************************************************/

static void
test_append_action_set_vlan_vid() {
  openflow_actions *actions = NULL;
  uint16_t vlan_vid = 0x0001;
  struct ofp_action_vlan_vid *action_vlan_vid;
  bool ret;

  expect_assert_failure( append_action_set_vlan_vid( actions, vlan_vid ) );

  actions = create_actions();
  vlan_vid = 0xffff;

  expect_assert_failure( append_action_set_vlan_vid( actions, vlan_vid ) );

  vlan_vid = 0x0001;

  ret = append_action_set_vlan_vid( actions, vlan_vid );
  assert_true( ret );

  action_vlan_vid = actions->list->data;

  assert_int_equal( action_vlan_vid->type, OFPAT_SET_VLAN_VID );
  assert_int_equal( action_vlan_vid->len, sizeof( struct ofp_action_vlan_vid ) );
  assert_int_equal( action_vlan_vid->vlan_vid, vlan_vid );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_vlan_pcp() test.
 ********************************************************************************/

static void
test_append_action_set_vlan_pcp() {
  openflow_actions *actions = NULL;
  uint8_t vlan_pcp = 0x1;
  struct ofp_action_vlan_pcp *action_vlan_pcp;
  bool ret;

  expect_assert_failure( append_action_set_vlan_pcp( actions, vlan_pcp ) );

  actions = create_actions();
  vlan_pcp = 0xf;

  expect_assert_failure( append_action_set_vlan_pcp( actions, vlan_pcp ) );

  vlan_pcp = 0x1;

  ret = append_action_set_vlan_pcp( actions, vlan_pcp );
  assert_true( ret );

  action_vlan_pcp = actions->list->data;

  assert_int_equal( action_vlan_pcp->type, OFPAT_SET_VLAN_PCP );
  assert_int_equal( action_vlan_pcp->len, sizeof( struct ofp_action_vlan_pcp ) );
  assert_int_equal( action_vlan_pcp->vlan_pcp, vlan_pcp );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_strip_vlan() test.
 ********************************************************************************/

static void
test_append_action_strip_vlan() {
  openflow_actions *actions = NULL;
  struct ofp_action_header *action_strip_vlan;
  bool ret;

  expect_assert_failure( append_action_strip_vlan( actions ) );

  actions = create_actions();

  ret = append_action_strip_vlan( actions );
  assert_true( ret );

  action_strip_vlan = actions->list->data;

  assert_int_equal( action_strip_vlan->type, OFPAT_STRIP_VLAN );
  assert_int_equal( action_strip_vlan->len, sizeof( struct ofp_action_header ) );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_dl_src() test.
 ********************************************************************************/

static void
test_append_action_set_dl_src() {
  openflow_actions *actions = NULL;
  struct ofp_action_dl_addr *action_dl_addr;
  bool ret;

  expect_assert_failure( append_action_set_dl_src( actions, HW_ADDR ) );

  actions = create_actions();

  ret = append_action_set_dl_src( actions, HW_ADDR );
  assert_true( ret );

  action_dl_addr = actions->list->data;

  assert_int_equal( action_dl_addr->type, OFPAT_SET_DL_SRC );
  assert_int_equal( action_dl_addr->len, sizeof( struct ofp_action_dl_addr ) );
  assert_memory_equal( action_dl_addr->dl_addr, HW_ADDR, OFP_ETH_ALEN );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_dl_dst() test.
 ********************************************************************************/

static void
test_append_action_set_dl_dst() {
  openflow_actions *actions = NULL;
  struct ofp_action_dl_addr *action_dl_addr;
  bool ret;

  expect_assert_failure( append_action_set_dl_dst( actions, HW_ADDR ) );

  actions = create_actions();

  ret = append_action_set_dl_dst( actions, HW_ADDR );
  assert_true( ret );

  action_dl_addr = actions->list->data;

  assert_int_equal( action_dl_addr->type, OFPAT_SET_DL_DST );
  assert_int_equal( action_dl_addr->len, sizeof( struct ofp_action_dl_addr ) );
  assert_memory_equal( action_dl_addr->dl_addr, HW_ADDR, OFP_ETH_ALEN );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_nw_src() test.
 ********************************************************************************/

static void
test_append_action_set_nw_src() {
  openflow_actions *actions = NULL;
  struct ofp_action_nw_addr *action_nw_addr;
  bool ret;

  expect_assert_failure( append_action_set_nw_src( actions, NW_ADDR ) );

  actions = create_actions();

  ret = append_action_set_nw_src( actions, NW_ADDR );
  assert_true( ret );

  action_nw_addr = actions->list->data;

  assert_int_equal( action_nw_addr->type, OFPAT_SET_NW_SRC );
  assert_int_equal( action_nw_addr->len, sizeof( struct ofp_action_nw_addr ) );
  assert_int_equal( ( int ) action_nw_addr->nw_addr, ( int ) NW_ADDR );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_nw_dst() test.
 ********************************************************************************/

static void
test_append_action_set_nw_dst() {
  openflow_actions *actions = NULL;
  struct ofp_action_nw_addr *action_nw_addr;
  bool ret;

  expect_assert_failure( append_action_set_nw_dst( actions, NW_ADDR ) );

  actions = create_actions();

  ret = append_action_set_nw_dst( actions, NW_ADDR );
  assert_true( ret );

  action_nw_addr = actions->list->data;

  assert_int_equal( action_nw_addr->type, OFPAT_SET_NW_DST );
  assert_int_equal( action_nw_addr->len, sizeof( struct ofp_action_nw_addr ) );
  assert_int_equal( ( int ) action_nw_addr->nw_addr, ( int ) NW_ADDR );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_nw_tos() test.
 ********************************************************************************/

static void
test_append_action_set_nw_tos() {
  openflow_actions *actions = NULL;
  uint8_t nw_tos = 0xfc;
  struct ofp_action_nw_tos *action_nw_tos;
  bool ret;

  expect_assert_failure( append_action_set_nw_tos( actions, nw_tos ) );

  actions = create_actions();
  nw_tos = 0xff;

  expect_assert_failure( append_action_set_nw_tos( actions, nw_tos ) );

  nw_tos = 0xfc;

  ret = append_action_set_nw_tos( actions, nw_tos );
  assert_true( ret );

  action_nw_tos = actions->list->data;

  assert_int_equal( action_nw_tos->type, OFPAT_SET_NW_TOS );
  assert_int_equal( action_nw_tos->len, sizeof( struct ofp_action_nw_tos ) );
  assert_int_equal( action_nw_tos->nw_tos, nw_tos );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_tp_src() test.
 ********************************************************************************/

static void
test_append_action_set_tp_src() {
  openflow_actions *actions = NULL;
  uint16_t tp_port = 1;
  struct ofp_action_tp_port *action_tp_port;
  bool ret;

  expect_assert_failure( append_action_set_tp_src( actions, tp_port ) );

  actions = create_actions();

  ret = append_action_set_tp_src( actions, tp_port );
  assert_true( ret );

  action_tp_port = actions->list->data;

  assert_int_equal( action_tp_port->type, OFPAT_SET_TP_SRC );
  assert_int_equal( action_tp_port->len, sizeof( struct ofp_action_tp_port ) );
  assert_int_equal( action_tp_port->tp_port, tp_port );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_set_tp_dst() test.
 ********************************************************************************/

static void
test_append_action_set_tp_dst() {
  openflow_actions *actions = NULL;
  uint16_t tp_port = 1;
  struct ofp_action_tp_port *action_tp_port;
  bool ret;

  expect_assert_failure( append_action_set_tp_dst( actions, tp_port ) );

  actions = create_actions();

  ret = append_action_set_tp_dst( actions, tp_port );
  assert_true( ret );

  action_tp_port = actions->list->data;

  assert_int_equal( action_tp_port->type, OFPAT_SET_TP_DST );
  assert_int_equal( action_tp_port->len, sizeof( struct ofp_action_tp_port ) );
  assert_int_equal( action_tp_port->tp_port, tp_port );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_enqueue() test.
 ********************************************************************************/

static void
test_append_action_enqueue() {
  openflow_actions *actions = NULL;
  uint16_t port = 1;
  uint32_t queue_id = 10;
  struct ofp_action_enqueue *action_enqueue;
  bool ret;

  expect_assert_failure( append_action_enqueue( actions, port, queue_id ) );

  actions = create_actions();

  ret = append_action_enqueue( actions, port, queue_id );
  assert_true( ret );

  action_enqueue = actions->list->data;

  assert_int_equal( action_enqueue->type, OFPAT_ENQUEUE );
  assert_int_equal( action_enqueue->len, sizeof( struct ofp_action_enqueue ) );
  assert_int_equal( action_enqueue->port, port );
  assert_int_equal( ( int ) action_enqueue->queue_id, ( int ) queue_id );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * append_action_vendor() test.
 ********************************************************************************/

static void
test_append_action_vendor() {
  openflow_actions *actions = NULL;
  uint32_t vendor = 1;
  buffer *body = NULL;
  struct ofp_action_vendor_header *action_vendor;
  uint16_t length;
  bool ret;

  body = create_dummy_data( SHORT_DATA_LENGTH );

  length = ( uint16_t ) ( sizeof( struct ofp_action_vendor_header ) + body->length );

  expect_assert_failure( append_action_vendor( actions, vendor, body ) );

  actions = create_actions();

  ret = append_action_vendor( actions, vendor, body );
  assert_true( ret );

  action_vendor = actions->list->data;

  assert_int_equal( action_vendor->type, OFPAT_VENDOR );
  assert_int_equal( action_vendor->len, length );
  assert_int_equal( ( int ) action_vendor->vendor, ( int ) vendor );
  assert_memory_equal( ( char * ) action_vendor + sizeof( struct ofp_action_vendor_header ), body->data, body->length );

  assert_int_equal( actions->n_actions, 1 );

  free_buffer( body );
  delete_actions( actions );
}


/********************************************************************************
 * append_action_vendor_without_data() test.
 ********************************************************************************/

static void
test_append_action_vendor_without_data() {
  openflow_actions *actions = NULL;
  uint32_t vendor = 1;
  buffer *body = NULL;
  struct ofp_action_vendor_header *action_vendor;
  uint16_t length;
  bool ret;

  length = ( uint16_t ) sizeof( struct ofp_action_vendor_header );

  expect_assert_failure( append_action_vendor( actions, vendor, body ) );

  actions = create_actions();

  ret = append_action_vendor( actions, vendor, body );
  assert_true( ret );

  action_vendor = actions->list->data;

  assert_int_equal( action_vendor->type, OFPAT_VENDOR );
  assert_int_equal( action_vendor->len, length );
  assert_int_equal( ( int ) action_vendor->vendor, ( int ) vendor );

  assert_int_equal( actions->n_actions, 1 );

  delete_actions( actions );
}


/********************************************************************************
 * create_packet_out() tests.
 ********************************************************************************/

static void
test_create_packet_out() {
  uint16_t in_port = 2;
  uint16_t port = 1;
  uint16_t max_len = 128;
  openflow_actions *actions;
  buffer *expected_data;
  buffer *buffer;
  struct ofp_packet_out *packet_out;
  uint16_t actions_len;
  uint16_t length;

  actions = create_actions();
  append_action_output( actions, port, max_len );

  expected_data = create_dummy_data( LONG_DATA_LENGTH );

  buffer = create_packet_out( MY_TRANSACTION_ID, BUFFER_ID, in_port, actions, expected_data );
  assert_true( buffer != NULL );

  packet_out = buffer->data;

  actions_len = get_actions_length( actions );
  length = ( uint16_t ) ( sizeof( struct ofp_packet_out ) + actions_len + expected_data->length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( packet_out->header.version, OFP_VERSION );
  assert_int_equal( packet_out->header.type, OFPT_PACKET_OUT );
  assert_int_equal( ntohs( packet_out->header.length ), length );
  assert_int_equal( ( int ) ntohl( packet_out->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohl( packet_out->buffer_id ), ( int ) BUFFER_ID );
  assert_int_equal( ntohs( packet_out->in_port ), in_port );
  assert_int_equal( ntohs( packet_out->actions_len ), actions_len );

  {
    void *a = packet_out->actions;
    list_element *expected_action = actions->list;

    while ( expected_action != NULL ) {
      struct ofp_action_header tmp_a;
      ntoh_action( &tmp_a, ( struct ofp_action_header * ) a );

      struct ofp_action_header *expected_action_header = expected_action->data;

      assert_int_equal( tmp_a.type, expected_action_header->type );
      assert_int_equal( tmp_a.len, expected_action_header->len );
      assert_memory_equal( tmp_a.pad, expected_action_header->pad, sizeof( expected_action_header->pad ) );

      a = ( void * ) ( ( char * ) a + tmp_a.len );
      expected_action = expected_action->next;
    }
  }

  void *d = ( void * ) ( ( char * ) buffer->data + sizeof( struct ofp_packet_out ) + actions_len );
  assert_memory_equal( d, expected_data->data, expected_data->length );

  assert_int_equal( actions->n_actions, 1 );

  free_buffer( expected_data );
  free_buffer( buffer );
  delete_actions( actions );
}


static void
test_create_packet_out_without_actions() {
  uint16_t in_port = 1;
  openflow_actions *actions = NULL;
  buffer *expected_data;
  buffer *buffer;
  struct ofp_packet_out *packet_out;
  uint16_t length;

  expected_data = create_dummy_data( SHORT_DATA_LENGTH );

  buffer = create_packet_out( MY_TRANSACTION_ID, BUFFER_ID, in_port, actions, expected_data );
  assert_true( buffer != NULL );

  packet_out = buffer->data;

  length = ( uint16_t ) ( sizeof( struct ofp_packet_out ) + expected_data->length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( packet_out->header.version, OFP_VERSION );
  assert_int_equal( packet_out->header.type, OFPT_PACKET_OUT );
  assert_int_equal( ntohs( packet_out->header.length ), length );
  assert_int_equal( ( int ) ntohl( packet_out->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohl( packet_out->buffer_id ), ( int ) BUFFER_ID );
  assert_int_equal( ntohs( packet_out->in_port ), in_port );
  assert_int_equal( ntohs( packet_out->actions_len ), 0 );

  void *d = ( void * ) ( ( char * ) buffer->data + sizeof( struct ofp_packet_out ) );
  assert_memory_equal( d, expected_data->data, expected_data->length );

  free_buffer( expected_data );
  free_buffer( buffer );
}


/********************************************************************************
 * create_flow_mod() test.
 ********************************************************************************/

static void
test_create_flow_mod() {
  uint64_t cookie = 10;
  uint16_t command = OFPFC_ADD;
  uint16_t idle_timeout = 5;
  uint16_t hard_timeout = 10;
  uint32_t buffer_id = 10;
  uint16_t out_port = UINT16_MAX;
  uint16_t flags = OFPFF_CHECK_OVERLAP | OFPFF_SEND_FLOW_REM;
  openflow_actions *actions;
  uint16_t port = 1;
  uint16_t max_len = 128;
  buffer *buffer;
  struct ofp_flow_mod *flow_mod;
  uint16_t actions_len;
  uint16_t length;
  uint64_t tmp;

  actions = create_actions();
  append_action_output( actions, port, max_len );

  actions_len = get_actions_length( actions );

  buffer = create_flow_mod( MY_TRANSACTION_ID, MATCH, cookie, command, idle_timeout,
                            hard_timeout, PRIORITY, buffer_id, out_port, flags, actions );
  assert_true( buffer != NULL );

  flow_mod = ( struct ofp_flow_mod * ) buffer->data;

  length = ( uint16_t ) ( sizeof( struct ofp_flow_mod ) + actions_len );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( flow_mod->header.version, OFP_VERSION );
  assert_int_equal( flow_mod->header.type, OFPT_FLOW_MOD );
  assert_int_equal( ntohs( flow_mod->header.length ), length );
  assert_int_equal( ( int ) ntohl( flow_mod->header.xid ), ( int ) MY_TRANSACTION_ID );

  ntoh_match( &flow_mod->match, &flow_mod->match );
  assert_memory_equal( &flow_mod->match, &MATCH, sizeof( MATCH ) );

  tmp = ntohll( flow_mod->cookie );
  assert_memory_equal( &tmp, &cookie, sizeof( cookie ) );
  assert_int_equal( ntohs( flow_mod->idle_timeout ), idle_timeout );
  assert_int_equal( ntohs( flow_mod->hard_timeout ), hard_timeout );
  assert_int_equal( ntohs( flow_mod->priority ), PRIORITY );
  assert_int_equal( ( int ) ntohl( flow_mod->buffer_id ), ( int ) buffer_id );
  assert_int_equal( ntohs( flow_mod->out_port ), out_port );
  assert_int_equal( ntohs( flow_mod->flags ), flags );

  {
    void *a = flow_mod->actions;
    list_element *expected_action = actions->list;

    while ( expected_action != NULL ) {
      struct ofp_action_header tmp_a;
      ntoh_action( &tmp_a, ( struct ofp_action_header * ) a );

      struct ofp_action_header *expected_action_header = expected_action->data;

      assert_int_equal( tmp_a.type, expected_action_header->type );
      assert_int_equal( tmp_a.len, expected_action_header->len );
      assert_memory_equal( tmp_a.pad, expected_action_header->pad, sizeof( expected_action_header->pad ) );

      a = ( void * ) ( ( char * ) a + tmp_a.len );
      expected_action = expected_action->next;
    }
  }

  assert_int_equal( actions->n_actions, 1 );

  free_buffer( buffer );
  delete_actions( actions );
}


/********************************************************************************
 * create_stats_request() test.
 ********************************************************************************/

static void
test_create_stats_request() {
  uint16_t type = OFPST_DESC;
  buffer *buffer;
  struct ofp_stats_request *stats_request;
  uint16_t length;

  length = sizeof( struct ofp_stats_request );

  buffer = create_stats_request( MY_TRANSACTION_ID, type, length, NO_FLAGS );
  assert_true( buffer != NULL );

  stats_request = buffer->data;

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), type );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  free_buffer( buffer );
}


/********************************************************************************
 * create_stats_reply() test.
 ********************************************************************************/

static void
test_create_stats_reply() {
  uint16_t type = OFPST_DESC;
  uint16_t length = sizeof( struct ofp_stats_reply );
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;

  buffer = create_stats_reply( MY_TRANSACTION_ID, type, length, NO_FLAGS );
  assert_true( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;

  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), type );
  assert_int_equal( ntohs( stats_reply->flags ), NO_FLAGS );

  free_buffer( buffer );
}


/********************************************************************************
 * create_desc_stats_request() test.
 ********************************************************************************/

static void
test_create_desc_stats_request() {
  buffer *buffer;
  struct ofp_stats_request *stats_request;

  buffer = create_desc_stats_request( MY_TRANSACTION_ID, NO_FLAGS );

  assert_true( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_stats_request ) );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), sizeof( struct ofp_stats_request ) );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), OFPST_DESC );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  free_buffer( buffer );
}


/********************************************************************************
 * create_desc_stats_reply() test.
 ********************************************************************************/

static void
test_create_desc_stats_reply() {
  const char mfr_desc[ DESC_STR_LEN ] = "NEC Corporation";
  const char hw_desc[ DESC_STR_LEN ] = "OpenFlow Switch Hardware";
  const char sw_desc[ DESC_STR_LEN ] = "OpenFlow Switch Software";
  const char serial_num[ SERIAL_NUM_LEN ] = "1234";
  const char dp_desc[ DESC_STR_LEN ] = "Datapath 0";
  uint16_t length;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;
  struct ofp_desc_stats *desc_stats;

  length = ( uint16_t ) ( sizeof( struct ofp_stats_reply ) + sizeof( struct ofp_desc_stats ) );

  buffer = create_desc_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, mfr_desc, hw_desc, sw_desc, serial_num, dp_desc );
  assert_true( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  desc_stats = ( struct ofp_desc_stats * ) ( ( char * ) buffer->data + sizeof( struct ofp_stats_reply ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_DESC );
  assert_int_equal( ntohs( stats_reply->flags ), NO_FLAGS );

  assert_memory_equal( desc_stats->mfr_desc, mfr_desc, strlen( mfr_desc ) + 1 );
  assert_memory_equal( desc_stats->hw_desc, hw_desc, strlen( mfr_desc ) + 1 );
  assert_memory_equal( desc_stats->sw_desc, sw_desc, strlen( mfr_desc ) + 1 );
  assert_memory_equal( desc_stats->serial_num, serial_num, strlen( mfr_desc ) + 1 );
  assert_memory_equal( desc_stats->dp_desc, dp_desc, strlen( mfr_desc ) + 1 );

  free_buffer( buffer );
}


/********************************************************************************
 * create_flow_stats_request() test.
 ********************************************************************************/

static void
test_create_flow_stats_request() {
  uint8_t table_id = 0xff;
  uint16_t out_port = 1;
  buffer *buffer;
  struct ofp_stats_request *stats_request;
  struct ofp_flow_stats_request *flow_stats_request;
  uint16_t length;

  buffer = create_flow_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, table_id, out_port );
  assert_true( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body ) + sizeof( struct ofp_flow_stats_request ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), OFPST_FLOW );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  flow_stats_request = ( struct ofp_flow_stats_request * ) ( ( char * ) buffer->data
                       + offsetof( struct ofp_stats_request, body ) );

  ntoh_match( &flow_stats_request->match, &flow_stats_request->match );
  assert_memory_equal( &flow_stats_request->match, &MATCH, sizeof( MATCH ) );

  assert_int_equal( flow_stats_request->table_id, table_id );
  assert_int_equal( flow_stats_request->pad, 0 );
  assert_int_equal( ntohs( flow_stats_request->out_port ), out_port );

  free_buffer( buffer );
}


/********************************************************************************
 * create_flow_stats_reply() test.
 ********************************************************************************/

static void
test_create_flow_stats_reply() {
  uint16_t flags = OFPSF_REPLY_MORE;
  list_element *expected_list, *list;
  buffer *buffer;
  uint16_t stats_len = 0;
  struct ofp_stats_reply *stats_reply;
  struct ofp_flow_stats *expected_stats[ 2 ], *flow_stats;
  struct ofp_action_output *action;
  uint16_t length;

  stats_len = offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output );

  expected_stats[ 0 ] = xcalloc( 1, stats_len );
  expected_stats[ 1 ] = xcalloc( 1, stats_len );

  expected_stats[ 0 ]->length = stats_len;
  expected_stats[ 0 ]->table_id = 1;
  expected_stats[ 0 ]->pad = 0;
  expected_stats[ 0 ]->match = MATCH;
  expected_stats[ 0 ]->duration_sec = 60;
  expected_stats[ 0 ]->duration_nsec = 10000;
  expected_stats[ 0 ]->priority = 1024;
  expected_stats[ 0 ]->idle_timeout = 60;
  expected_stats[ 0 ]->hard_timeout = 3600;
  expected_stats[ 0 ]->cookie = 0x0102030405060708ULL;
  expected_stats[ 0 ]->packet_count = 1000;
  expected_stats[ 0 ]->byte_count = 100000;
  action = ( struct ofp_action_output * ) expected_stats[ 0 ]->actions;
  action->type = OFPAT_OUTPUT;
  action->len = 8;
  action->port = 1;
  action->max_len = 2048;

  memcpy( expected_stats[ 1 ], expected_stats[ 0 ], stats_len );
  expected_stats[ 1 ]->cookie = 0x0203040506070809ULL;
  action = ( struct ofp_action_output * ) expected_stats[ 1 ]->actions;
  action->port = 2;

  create_list( &expected_list );
  append_to_tail( &expected_list, expected_stats[ 0 ] );
  append_to_tail( &expected_list, expected_stats[ 1 ] );

  buffer = create_flow_stats_reply( MY_TRANSACTION_ID, flags, expected_list );
  assert_true( buffer != NULL );

  stats_reply = buffer->data;
  flow_stats = ( struct ofp_flow_stats * ) stats_reply->body;
  length = ( uint16_t ) ( stats_len * 2 );
  length = ( uint16_t ) ( length + offsetof( struct ofp_stats_reply, body ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_FLOW );
  assert_int_equal( ntohs( stats_reply->flags ), flags );

  list = expected_list;
  while ( list != NULL ) {
    struct ofp_flow_stats *next = flow_stats;
    struct ofp_flow_stats *expected_flow_stats = list->data;

    next = ( struct ofp_flow_stats * ) ( ( char * ) flow_stats + ntohs( flow_stats->length ) );
    ntoh_flow_stats( flow_stats, flow_stats );

    assert_memory_equal( flow_stats, expected_flow_stats, expected_flow_stats->length );

    flow_stats = next;
    list = list->next;
  }

  xfree( expected_stats[ 0 ] );
  xfree( expected_stats[ 1 ] );
  delete_list( expected_list );
  free_buffer( buffer );
}


/********************************************************************************
 * create_aggregate_stats_request() test.
 ********************************************************************************/

static void
test_create_aggregate_stats_request() {
  uint8_t table_id = 0xff;
  uint16_t out_port = 1;
  buffer *buffer;
  struct ofp_stats_request *stats_request;
  struct ofp_aggregate_stats_request *aggregate_stats_request;
  uint16_t length;

  buffer = create_aggregate_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, table_id, out_port );
  assert_true( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body ) + sizeof( struct ofp_flow_stats_request ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), OFPST_AGGREGATE );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  aggregate_stats_request = ( struct ofp_aggregate_stats_request * ) stats_request->body;

  ntoh_match( &aggregate_stats_request->match, &aggregate_stats_request->match );
  assert_memory_equal( &aggregate_stats_request->match, &MATCH, sizeof( MATCH ) );

  assert_int_equal( aggregate_stats_request->table_id, table_id );
  assert_int_equal( aggregate_stats_request->pad, 0 );
  assert_int_equal( ntohs( aggregate_stats_request->out_port ), out_port );

  free_buffer( buffer );
}


/********************************************************************************
 * create_aggregate_stats_reply() test.
 ********************************************************************************/

static void
test_create_aggregate_stats_reply() {
  uint32_t flow_count = 1000;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 10000;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;
  struct ofp_aggregate_stats_reply *aggregate_stats_reply;
  uint16_t length;
  uint64_t tmp;

  buffer = create_aggregate_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, packet_count, byte_count, flow_count );
  assert_true( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body ) + sizeof( struct ofp_aggregate_stats_reply ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_AGGREGATE );
  assert_int_equal( ntohs( stats_reply->flags ), NO_FLAGS );

  aggregate_stats_reply = ( struct ofp_aggregate_stats_reply * ) stats_reply->body;

  tmp = ntohll( aggregate_stats_reply->packet_count );
  assert_memory_equal( &tmp, &packet_count, sizeof( packet_count ) );
  tmp = ntohll( aggregate_stats_reply->byte_count );
  assert_memory_equal( &tmp, &byte_count, sizeof( byte_count ) );
  assert_int_equal( ( int ) ntohl( aggregate_stats_reply->flow_count ), ( int ) flow_count );

  free_buffer( buffer );
}


/********************************************************************************
 * create_table_stats_request() test.
 ********************************************************************************/

static void
test_create_table_stats_request() {
  buffer *buffer;
  struct ofp_stats_request *stats_request;

  buffer = create_table_stats_request( MY_TRANSACTION_ID, NO_FLAGS );
  assert_true( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_stats_request ) );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), sizeof( struct ofp_stats_request ) );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), OFPST_TABLE );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  free_buffer( buffer );
}


/********************************************************************************
 * create_table_stats_reply() test.
 ********************************************************************************/

static void
test_create_table_stats_reply() {
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  buffer *buffer;
  list_element *expected_list, *list;
  struct ofp_stats_reply *stats_reply;
  struct ofp_table_stats *stats[ 2 ], *table_stats, *next;
  uint16_t length = 0;

  stats_len = sizeof( struct ofp_table_stats );

  stats[ 0 ] = xcalloc( 1, stats_len );
  stats[ 1 ] = xcalloc( 1, stats_len );

  stats[ 0 ]->table_id = 1;
  strcpy( stats[ 0 ]->name, "Table 1" );
  stats[ 0 ]->wildcards =  OFPFW_ALL;
  stats[ 0 ]->max_entries = 10000;
  stats[ 0 ]->active_count = 1000;
  stats[ 0 ]->lookup_count = 100000;
  stats[ 0 ]->matched_count = 10000;

  memcpy( stats[ 1 ], stats[ 0 ], stats_len );
  stats[ 1 ]->table_id = 2;
  strcpy( stats[ 1 ]->name, "Table 2" );

  create_list( &expected_list );
  append_to_tail( &expected_list, stats[ 0 ] );
  append_to_tail( &expected_list, stats[ 1 ] );

  buffer = create_table_stats_reply( MY_TRANSACTION_ID, flags, expected_list );
  assert_true( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body ) + sizeof( struct ofp_table_stats ) * 2 );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_TABLE );
  assert_int_equal( ntohs( stats_reply->flags ), flags );

  table_stats = ( struct ofp_table_stats * ) stats_reply->body;

  list = expected_list;
  while ( list != NULL ) {
    struct ofp_table_stats *expected_table_stats = ( struct ofp_table_stats * ) list->data;

    next = ( struct ofp_table_stats * ) ( ( char * ) table_stats + sizeof( struct ofp_table_stats ) );
    ntoh_table_stats( table_stats, table_stats );

    assert_memory_equal( table_stats, expected_table_stats, sizeof( struct ofp_table_stats ) );

    table_stats = next;
    list = list->next;
  }

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( expected_list );
  free_buffer( buffer );
}


/********************************************************************************
 * create_port_stats_request() test.
 ********************************************************************************/

static void
test_create_port_stats_request() {
  uint16_t port_no = 1;
  buffer *buffer;
  struct ofp_stats_request *stats_request;
  struct ofp_port_stats_request *port_stats_request;
  uint16_t length;

  buffer = create_port_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no );
  assert_true( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body )
           + sizeof( struct ofp_port_stats_request ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), OFPST_PORT );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  port_stats_request = ( struct ofp_port_stats_request * ) ( ( char * ) buffer->data
                       + offsetof( struct ofp_stats_request, body ) );

  assert_int_equal( ntohs( port_stats_request->port_no ), port_no );

  {
    void *pad = xmalloc( sizeof( port_stats_request->pad ) );
    memset( pad, 0, sizeof( port_stats_request->pad ) );
    assert_memory_equal( port_stats_request->pad, pad, sizeof( port_stats_request->pad ) );
    xfree( pad );
  }

  free_buffer( buffer );
}


/********************************************************************************
 * create_port_stats_reply() test.
 ********************************************************************************/

static void
test_create_port_stats_reply() {
  void *expected_data;
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  buffer *buffer;
  list_element *expected_list, *list;
  struct ofp_stats_reply *stats_reply;
  struct ofp_port_stats *stats[ 2 ], *port_stats, *next;
  uint16_t length = 0;

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

  create_list( &expected_list );
  append_to_tail( &expected_list, stats[ 0 ] );
  append_to_tail( &expected_list, stats[ 1 ] );

  expected_data = xcalloc( 1, ( size_t ) ( stats_len * 2 ) );
  memcpy( expected_data, stats[ 0 ], stats_len );
  memcpy( ( char * ) expected_data + stats_len, stats[ 1 ], stats_len );

  buffer = create_port_stats_reply( MY_TRANSACTION_ID, flags, expected_list );
  assert_true( buffer != NULL );

  stats_reply = buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
           + sizeof( struct ofp_port_stats ) * 2 );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_PORT );
  assert_int_equal( ntohs( stats_reply->flags ), flags );

  port_stats = ( struct ofp_port_stats * ) stats_reply->body;

  list = expected_list;
  while ( list != NULL ) {
    struct ofp_port_stats *expected_port_stats = ( struct ofp_port_stats * ) list->data;

    next = ( struct ofp_port_stats * ) ( ( char * ) port_stats + sizeof( struct ofp_port_stats ) );
    ntoh_port_stats( port_stats, port_stats );

    assert_memory_equal( port_stats, expected_port_stats, sizeof( struct ofp_port_stats ) );

    port_stats = next;
    list = list->next;
  }

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  xfree( expected_data );
  delete_list( expected_list );
  free_buffer( buffer );
}


/********************************************************************************
 * create_queue_stats_request() test.
 ********************************************************************************/

static void
test_create_queue_stats_request() {
  uint16_t port_no = 1;
  uint32_t queue_id = 10;
  buffer *buffer;
  struct ofp_stats_request *stats_request;
  struct ofp_queue_stats_request *queue_stats_request;
  uint16_t length;

  buffer = create_queue_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no, queue_id );
  assert_true( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body )
           + sizeof( struct ofp_queue_stats_request ) );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_request->header.version, OFP_VERSION );
  assert_int_equal( stats_request->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ntohs( stats_request->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_request->type ), OFPST_QUEUE );
  assert_int_equal( ntohs( stats_request->flags ), NO_FLAGS );

  queue_stats_request = ( struct ofp_queue_stats_request * ) ( ( char * ) buffer->data
                        + offsetof( struct ofp_stats_request, body ) );

  assert_int_equal( ntohs( queue_stats_request->port_no ), port_no );

  {
    void *pad = xmalloc( sizeof( queue_stats_request->pad ) );
    memset( pad, 0, sizeof( queue_stats_request->pad ) );
    assert_memory_equal( queue_stats_request->pad, pad, sizeof( queue_stats_request->pad ) );
    xfree( pad );
  }

  assert_int_equal( ( int ) ntohl( queue_stats_request->queue_id ), ( int ) queue_id );

  free_buffer( buffer );
}


/********************************************************************************
 * create_queue_stats_reply() test.
 ********************************************************************************/

static void
test_create_queue_stats_reply() {
  uint16_t flags = OFPSF_REPLY_MORE;
  list_element *list, *l;
  uint16_t stats_len;
  struct ofp_stats_reply *stats_reply;
  struct ofp_queue_stats *stats[ 2 ], *queue_stats;
  uint16_t n_queues = 2;
  uint16_t length;
  buffer *buffer;

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

  create_list( &list );
  append_to_tail( &list, stats[ 0 ] );
  append_to_tail( &list, stats[ 1 ] );

  buffer = create_queue_stats_reply( MY_TRANSACTION_ID, flags, list );
  assert_true( buffer != NULL );

  stats_reply = buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
           + sizeof( struct ofp_queue_stats ) * n_queues );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_QUEUE );
  assert_int_equal( ntohs( stats_reply->flags ), flags );

  queue_stats = ( struct ofp_queue_stats * ) stats_reply->body;
  l = list;
  while ( list != NULL ) {
    void *next = ( char * ) queue_stats + stats_len;
    ntoh_queue_stats( queue_stats, queue_stats );

    assert_memory_equal( queue_stats, list->data, stats_len );

    list = list->next;
    queue_stats = next;
  }

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( l );
  free_buffer( buffer );
}


/********************************************************************************
 * create_barrier_request() test.
 ********************************************************************************/

static void
test_create_barrier_request() {
  buffer *buffer;
  struct ofp_header *barrier_request;

  buffer = create_barrier_request( MY_TRANSACTION_ID );

  assert_true( buffer != NULL );

  barrier_request = ( struct ofp_header * ) buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_header ) );
  assert_int_equal( barrier_request->version, OFP_VERSION );
  assert_int_equal( barrier_request->type, OFPT_BARRIER_REQUEST );
  assert_int_equal( ntohs( barrier_request->length ), sizeof( struct ofp_header ) );
  assert_int_equal( ( int ) ntohl( barrier_request->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


/********************************************************************************
 * create_barrier_reply() test.
 ********************************************************************************/

static void
test_create_barrier_reply() {
  buffer *buffer;
  struct ofp_header *barrier_reply;

  buffer = create_barrier_reply( MY_TRANSACTION_ID );
  assert_true( buffer != NULL );

  barrier_reply = buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_header ) );
  assert_int_equal( barrier_reply->version, OFP_VERSION );
  assert_int_equal( barrier_reply->type, OFPT_BARRIER_REPLY );
  assert_int_equal( ntohs( barrier_reply->length ), sizeof( struct ofp_header ) );
  assert_int_equal( ( int ) ntohl( barrier_reply->xid ), ( int ) MY_TRANSACTION_ID );

  free_buffer( buffer );
}


/********************************************************************************
 * create_queue_get_config_request() test.
 ********************************************************************************/

static void
test_create_queue_get_config_request() {
  uint16_t port = 1;
  buffer *buffer;
  struct ofp_queue_get_config_request *queue_get_config_request;

  buffer = create_queue_get_config_request( MY_TRANSACTION_ID, port );

  assert_true( buffer != NULL );

  queue_get_config_request = buffer->data;

  assert_int_equal( ( int ) buffer->length, sizeof( struct ofp_queue_get_config_request ) );
  assert_int_equal( queue_get_config_request->header.version, OFP_VERSION );
  assert_int_equal( queue_get_config_request->header.type, OFPT_QUEUE_GET_CONFIG_REQUEST );
  assert_int_equal( ntohs( queue_get_config_request->header.length ), sizeof( struct ofp_queue_get_config_request ) );
  assert_int_equal( ( int ) ntohl( queue_get_config_request->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohs( queue_get_config_request->port ), ( int ) port );

  {
    void *pad = xmalloc( sizeof( queue_get_config_request->pad ) );
    memset( pad, 0, sizeof( queue_get_config_request->pad ) );
    assert_memory_equal( queue_get_config_request->pad, pad, sizeof( queue_get_config_request->pad ) );
    xfree( pad );
  }

  free_buffer( buffer );
}


/********************************************************************************
 * create_queue_get_config_reply() test.
 ********************************************************************************/

static void
test_create_queue_get_config_reply() {
  size_t queue_len;
  uint16_t port = 1;
  uint8_t expected_pad[ 6 ];
  list_element *list, *l;
  buffer *buffer;
  struct ofp_queue_get_config_reply *queue_get_config_reply;
  struct ofp_packet_queue *queue[ 2 ], *packet_queue;
  struct ofp_queue_prop_header *prop_header;
  uint16_t queues_length;
  uint16_t length;

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

  create_list( &list );
  append_to_tail( &list, queue[ 0 ] );
  append_to_tail( &list, queue[ 1 ] );

  buffer = create_queue_get_config_reply( MY_TRANSACTION_ID, port, list );
  assert_true( buffer != NULL );

  queue_get_config_reply = buffer->data;

  queues_length = ( uint16_t ) ( queue[ 0 ]->len + queue[ 1 ]->len );
  length = ( uint16_t ) ( offsetof( struct ofp_queue_get_config_reply, queues ) + queues_length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( queue_get_config_reply->header.version, OFP_VERSION );
  assert_int_equal( queue_get_config_reply->header.type, OFPT_QUEUE_GET_CONFIG_REPLY );
  assert_int_equal( ntohs( queue_get_config_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( queue_get_config_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  memset( &expected_pad, 0, sizeof( expected_pad ) );
  assert_int_equal( ntohs( queue_get_config_reply->port ), port );
  assert_memory_equal( queue_get_config_reply->pad, expected_pad, sizeof( expected_pad ) );

  l = list;
  packet_queue = queue_get_config_reply->queues;
  while ( list != NULL ) {
    ntoh_packet_queue( packet_queue, packet_queue );
    void *next = ( char * ) packet_queue + packet_queue->len;
    struct ofp_packet_queue *expected_packet_queue = ( struct ofp_packet_queue * ) list->data;
    assert_memory_equal( packet_queue, expected_packet_queue, expected_packet_queue->len );

    list = list->next;
    packet_queue = next;
  }

  xfree( queue[ 0 ] );
  xfree( queue[ 1 ] );
  delete_list( l );
  free_buffer( buffer );
}


/********************************************************************************
 * create_vendor_stats_request() tests.
 ********************************************************************************/

static void
test_create_vendor_stats_request() {
  const uint16_t body_length = 128;

  buffer *body = create_dummy_data( body_length );
  buffer *buffer = create_vendor_stats_request( MY_TRANSACTION_ID, VENDOR_STATS_FLAG, VENDOR_ID, body );
  assert_true( buffer != NULL );

  assert_int_equal( ( int ) buffer->length,
                    ( int ) ( offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t ) + body_length ) );

  struct ofp_stats_request *stats = buffer->data;

  assert_int_equal( stats->header.version, OFP_VERSION );
  assert_int_equal( stats->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ( int ) ntohs( stats->header.length ),
                    ( int ) ( offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t ) + body_length ) );
  assert_int_equal( ( int ) ntohl( stats->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohs( stats->type ), ( int ) OFPST_VENDOR );
  assert_int_equal( ( int ) ntohs( stats->flags ), ( int ) VENDOR_STATS_FLAG );
  uint32_t vendor_id = *( ( uint32_t * ) ( ( char * ) stats + offsetof( struct ofp_stats_request, body ) ) );
  assert_int_equal( ( int ) ntohl( vendor_id ), ( int ) VENDOR_ID );
  assert_memory_equal( ( char * ) stats + offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t ),
                       body->data, body_length );

  free_buffer( body );
  free_buffer( buffer );
}


static void
test_create_vendor_stats_request_without_data() {
  buffer *buffer = create_vendor_stats_request( MY_TRANSACTION_ID, VENDOR_STATS_FLAG, VENDOR_ID, NULL );
  assert_true( buffer != NULL );

  struct ofp_stats_request *stats = buffer->data;

  assert_int_equal( stats->header.version, OFP_VERSION );
  assert_int_equal( stats->header.type, OFPT_STATS_REQUEST );
  assert_int_equal( ( int ) ntohs( stats->header.length ),
                    ( int ) ( offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t ) ) );
  assert_int_equal( ( int ) ntohl( stats->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ( int ) ntohs( stats->type ), ( int ) OFPST_VENDOR );
  assert_int_equal( ( int ) ntohs( stats->flags ), ( int ) VENDOR_STATS_FLAG );
  uint32_t vendor_id = *( ( uint32_t * ) ( ( char * ) stats + offsetof( struct ofp_stats_request, body ) ) );
  assert_int_equal( ( int ) ntohl( vendor_id ), ( int ) VENDOR_ID );

  free_buffer( buffer );
}


/********************************************************************************
 * create_vendor_stats_reply() test.
 ********************************************************************************/

static void
test_create_vendor_stats_reply() {
  void *data;
  uint32_t expected_vendor = VENDOR_ID;
  uint32_t *vendor;
  buffer *buffer, *body;
  struct ofp_stats_reply *stats_reply;
  uint16_t length;

  body = alloc_buffer_with_length( 128 );
  append_back_buffer( body, 128 );
  memset( body->data, 0xa1, body->length );

  buffer = create_vendor_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, expected_vendor, body );
  assert_true( buffer != NULL );

  stats_reply = buffer->data;
  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body ) + sizeof( uint32_t ) + body->length );

  assert_int_equal( ( int ) buffer->length, length );
  assert_int_equal( stats_reply->header.version, OFP_VERSION );
  assert_int_equal( stats_reply->header.type, OFPT_STATS_REPLY );
  assert_int_equal( ntohs( stats_reply->header.length ), length );
  assert_int_equal( ( int ) ntohl( stats_reply->header.xid ), ( int ) MY_TRANSACTION_ID );

  assert_int_equal( ntohs( stats_reply->type ), OFPST_VENDOR );
  assert_int_equal( ntohs( stats_reply->flags ), NO_FLAGS );

  vendor = ( uint32_t * ) stats_reply->body;
  *vendor = ntohl( *vendor );

  assert_int_equal( ( int ) *vendor, ( int ) expected_vendor );

  data = ( char * ) vendor + sizeof( uint32_t );
  assert_memory_equal( data, body->data, body->length );

  free_buffer( body );
  free_buffer( buffer );
}


/********************************************************************************
 * validate_error() test.
 ********************************************************************************/

static void
test_validate_error() {
  uint16_t type = OFPET_HELLO_FAILED;
  uint16_t code = OFPHFC_INCOMPATIBLE;
  buffer *expected_data;
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_error( expected_message ) );

  expected_data = create_dummy_data( SHORT_DATA_LENGTH );

  expected_message = create_error( MY_TRANSACTION_ID, type, code, expected_data );

  ret_val = validate_error( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_data );
  free_buffer( expected_message );

  // error case.
  expected_message = create_echo_request( MY_TRANSACTION_ID, NULL );

  ret_val = validate_error( expected_message );
  assert_int_equal( ret_val, ERROR_INVALID_TYPE );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_validate_vendor() tests.
 ********************************************************************************/

static void
test_validate_vendor() {
  buffer *data = create_dummy_data( 32 );
  buffer *message = create_vendor( MY_TRANSACTION_ID, VENDOR_ID, data );

  assert_int_equal( validate_vendor( message ), 0 );

  free_buffer( data );
  free_buffer( message );
}


static void
test_validate_vendor_without_data() {
  buffer *message = create_vendor( MY_TRANSACTION_ID, VENDOR_ID, NULL );

  assert_int_equal( validate_vendor( message ), 0 );

  free_buffer( message );
}


static void
test_validate_vendor_fails_if_message_is_NULL() {
  expect_assert_failure( validate_vendor( NULL ) );
}


static void
test_validate_vendor_fails_if_message_is_not_vendor_header() {
  buffer *message = create_hello( MY_TRANSACTION_ID );

  assert_int_equal( validate_vendor( message ), ERROR_INVALID_TYPE );

  free_buffer( message );
}


/********************************************************************************
 * validate_features_reply() test.
 ********************************************************************************/

static void
test_validate_features_reply() {
  uint64_t datapath_id = 0x12345600;
  uint32_t n_buffers = 128;
  uint8_t n_tables = 1;
  uint32_t capabilities;
  uint32_t actions;
  struct ofp_phy_port phy_port[ 2 ];
  list_element *ports;
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_features_reply( expected_message ) );

  capabilities = ( OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS | OFPC_QUEUE_STATS | OFPC_ARP_MATCH_IP );
  actions = ( ( 1 << OFPAT_OUTPUT ) | ( 1 << OFPAT_SET_VLAN_VID ) | ( 1 << OFPAT_SET_TP_SRC ) | ( 1 << OFPAT_SET_TP_DST ) );

  phy_port[ 0 ].port_no = 1;
  memcpy( phy_port[ 0 ].hw_addr, HW_ADDR, sizeof( phy_port[ 0 ].hw_addr ) );
  strcpy( phy_port[ 0 ].name, "Brown" );
  phy_port[ 0 ].config = OFPPC_PORT_DOWN;
  phy_port[ 0 ].state = OFPPS_LINK_DOWN;
  phy_port[ 0 ].curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port[ 0 ].advertised = PORT_FEATURES;
  phy_port[ 0 ].supported = PORT_FEATURES;
  phy_port[ 0 ].peer = PORT_FEATURES;

  create_list( &ports );
  append_to_tail( &ports, &phy_port[ 0 ] );

  phy_port[ 1 ].port_no = 2;
  memcpy( phy_port[ 1 ].hw_addr, HW_ADDR, sizeof( phy_port[ 1 ].hw_addr ) );
  strcpy( phy_port[ 1 ].name, "Amber" );
  phy_port[ 1 ].config = OFPPC_PORT_DOWN;
  phy_port[ 1 ].state = OFPPS_LINK_DOWN;
  phy_port[ 1 ].curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port[ 1 ].advertised = PORT_FEATURES;
  phy_port[ 1 ].supported = PORT_FEATURES;
  phy_port[ 1 ].peer = PORT_FEATURES;

  append_to_tail( &ports, &phy_port[ 1 ] );
  expected_message = create_features_reply( MY_TRANSACTION_ID, datapath_id, n_buffers, n_tables, capabilities, actions, ports );

  ret_val = validate_features_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  delete_list( ports );
  free_buffer( expected_message );

  // error case.
  expected_message = create_hello( MY_TRANSACTION_ID );

  ret_val = validate_features_reply( expected_message );
  assert_int_equal( ret_val, ERROR_INVALID_TYPE );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_get_config_request() tests.
 ********************************************************************************/

static void
test_validate_get_config_request() {
  buffer *get_config_request = create_get_config_request( MY_TRANSACTION_ID );

  assert_int_equal( validate_get_config_request( get_config_request ), 0 );

  free_buffer( get_config_request );
}


static void
test_validate_get_config_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_get_config_request( NULL ) );
}


static void
test_validate_get_config_request_fails_if_message_is_not_get_config_request() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );

  assert_int_equal( validate_get_config_request( hello ), ERROR_INVALID_TYPE );

  free_buffer( hello );
}


/********************************************************************************
 * validate_get_config_reply() test.
 ********************************************************************************/

static void
test_validate_get_config_reply() {
  uint16_t flags = OFPC_FRAG_NORMAL;
  uint16_t miss_send_len = OFP_DEFAULT_MISS_SEND_LEN;
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_get_config_reply( expected_message ) );

  expected_message = create_get_config_reply( MY_TRANSACTION_ID, flags, miss_send_len );

  ret_val = validate_get_config_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_message );

  // error case.
  expected_message = create_echo_request( MY_TRANSACTION_ID, NULL );

  ret_val = validate_get_config_reply( expected_message );
  assert_int_equal( ret_val, ERROR_INVALID_TYPE );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_packet_in() test.
 ********************************************************************************/

static void
test_validate_packet_in() {
  uint16_t total_len;
  uint16_t in_port = 1;
  uint8_t reason = OFPR_NO_MATCH;
  buffer *expected_data = NULL;
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_packet_in( expected_message ) );

  expected_data = create_dummy_data( LONG_DATA_LENGTH );
  total_len = ( uint16_t ) expected_data->length;
  expected_message = create_packet_in( MY_TRANSACTION_ID, BUFFER_ID, total_len, in_port, reason, expected_data );

  ret_val = validate_packet_in( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_message );

  // error case.
  expected_message = create_echo_request( MY_TRANSACTION_ID, NULL );

  ret_val = validate_packet_in( expected_message );
  assert_int_equal( ret_val, ERROR_INVALID_TYPE );

  free_buffer( expected_data );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_flow_removed() test.
 ********************************************************************************/

static void
test_validate_flow_removed() {
  uint64_t cookie = 0x0102030405060708ULL;
  uint8_t reason = OFPRR_IDLE_TIMEOUT;
  uint32_t duration_sec = 180;
  uint32_t duration_nsec = 10000;
  uint16_t idle_timeout = 60;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 100000;
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_flow_removed( expected_message ) );

  expected_message = create_flow_removed( MY_TRANSACTION_ID, MATCH, cookie, PRIORITY, reason, duration_sec,
                                          duration_nsec, idle_timeout, packet_count, byte_count );

  ret_val = validate_flow_removed( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_message );

  // error case.
  expected_message = create_echo_request( MY_TRANSACTION_ID, NULL );

  ret_val = validate_flow_removed( expected_message );
  assert_int_equal( ret_val, ERROR_INVALID_TYPE );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_port_status() test.
 ********************************************************************************/

static void
test_validate_port_status() {
  uint8_t reason = OFPPR_ADD;
  struct ofp_phy_port desc;
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_port_status( expected_message ) );

  desc.port_no = 1;
  memcpy( desc.hw_addr, HW_ADDR, sizeof( desc.hw_addr ) );
  memset( desc.name, '\0', OFP_MAX_PORT_NAME_LEN );
  strcpy( desc.name, "Navy" );
  desc.config = OFPPC_PORT_DOWN;
  desc.state = OFPPS_LINK_DOWN;
  desc.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  desc.advertised = PORT_FEATURES;
  desc.supported = PORT_FEATURES;
  desc.peer = PORT_FEATURES;
  expected_message = create_port_status( MY_TRANSACTION_ID, reason, desc );

  ret_val = validate_port_status( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_packet_out() tests.
 ********************************************************************************/

static void
test_validate_packet_out() {
  openflow_actions *actions = create_actions();
  append_action_output( actions, 1, 128 );
  buffer *data = create_dummy_data( LONG_DATA_LENGTH );
  buffer *packet_out = create_packet_out( MY_TRANSACTION_ID, UINT32_MAX, 1, actions, data );

  assert_int_equal( validate_packet_out( packet_out ), 0 );

  free_buffer( data );
  free_buffer( packet_out );
  delete_actions( actions );
}


static void
test_validate_packet_out_without_data() {
  openflow_actions *actions = create_actions();
  append_action_output( actions, 1, 128 );
  buffer *packet_out = create_packet_out( MY_TRANSACTION_ID, BUFFER_ID, 1, actions, NULL );

  assert_int_equal( validate_packet_out( packet_out ), 0 );

  free_buffer( packet_out );
  delete_actions( actions );
}


static void
test_validate_packet_out_fails_if_message_is_NULL() {
  expect_assert_failure( validate_packet_out( NULL ) );
}


static void
test_validate_packet_out_fails_if_message_is_not_packet_out() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_packet_out( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_flow_mod() tests.
 ********************************************************************************/

static void
test_validate_flow_mod() {
  uint64_t cookie = 10;
  uint16_t hard_timeout = 10;
  uint16_t idle_timeout = 5;
  uint16_t out_port = UINT16_MAX;
  openflow_actions *actions = create_actions();
  append_action_output( actions, 1, 128 );
  buffer *flow_mod = create_flow_mod( MY_TRANSACTION_ID, MATCH, cookie, OFPFC_ADD, idle_timeout, hard_timeout, PRIORITY,
                                      BUFFER_ID, out_port, OFPFF_CHECK_OVERLAP | OFPFF_SEND_FLOW_REM, actions );

  assert_int_equal( validate_flow_mod( flow_mod ), 0 );

  free_buffer( flow_mod );
  delete_actions( actions );
}


static void
test_validate_flow_mod_fails_if_message_is_NULL() {
  expect_assert_failure( validate_flow_mod( NULL ) );
}


static void
test_validate_flow_mod_fails_if_message_is_not_flow_mod() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_flow_mod( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_port_mod() tests.
 ********************************************************************************/

static void
test_validate_port_mod() {
  uint16_t port_no = 1;
  uint32_t mask = 0;
  uint32_t advertise = 1;
  buffer *port_mod = create_port_mod( MY_TRANSACTION_ID, port_no, HW_ADDR, OFPPC_PORT_DOWN, mask, advertise );

  assert_int_equal( validate_port_mod( port_mod ), 0 );

  free_buffer( port_mod );
}


static void
test_validate_port_mod_fails_if_message_is_NULL() {
  expect_assert_failure( validate_port_mod( NULL ) );
}


static void
test_validate_port_mod_fails_if_message_is_not_port_mod() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_port_mod( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_desc_stats_request() tests.
 ********************************************************************************/

static void
test_validate_desc_stats_request() {
  buffer *desc_stats_request = create_desc_stats_request( MY_TRANSACTION_ID, 0 );

  assert_int_equal( validate_desc_stats_request( desc_stats_request ), 0 );

  free_buffer( desc_stats_request );
}


static void
test_validate_desc_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_desc_stats_request( NULL ) );
}


static void
test_validate_desc_stats_request_fails_if_message_is_not_desc_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_desc_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_desc_stats_reply() test.
 ********************************************************************************/

static void
test_validate_desc_stats_reply() {
  const char mfr_desc[ DESC_STR_LEN ] = "NEC Corporation";
  const char hw_desc[ DESC_STR_LEN ] = "OpenFlow Switch Hardware";
  const char sw_desc[ DESC_STR_LEN ] = "OpenFlow Switch Software";
  const char serial_num[ SERIAL_NUM_LEN ] = "1234";
  const char dp_desc[ DESC_STR_LEN ] = "Datapath 0";
  buffer *expected_message = NULL;
  int ret_val;

  expect_assert_failure( validate_desc_stats_reply( expected_message ) );

  expected_message = create_desc_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, mfr_desc, hw_desc, sw_desc, serial_num, dp_desc );

  ret_val = validate_desc_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_flow_stats_request() tests.
 ********************************************************************************/

static void
test_validate_flow_stats_request() {
  uint8_t table_id = 0xff;
  uint16_t out_port = 1;
  buffer *flow_stats_request = create_flow_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, table_id, out_port );

  assert_int_equal( validate_flow_stats_request( flow_stats_request ), 0 );

  free_buffer( flow_stats_request );
}


static void
test_validate_flow_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_flow_stats_request( NULL ) );
}


static void
test_validate_flow_stats_request_fails_if_message_is_not_flow_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_flow_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_flow_stats_reply() test.
 ********************************************************************************/

static void
test_validate_flow_stats_reply() {
  uint16_t flags = OFPSF_REPLY_MORE;
  list_element *expected_list;
  uint16_t stats_len = 0;
  struct ofp_flow_stats *expected_stats[ 2 ];
  struct ofp_action_output *action;
  buffer *expected_message = NULL;

  expect_assert_failure( validate_flow_stats_reply( expected_message ) );

  stats_len = offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output );

  expected_stats[ 0 ] = xcalloc( 1, stats_len );
  expected_stats[ 1 ] = xcalloc( 1, stats_len );

  expected_stats[ 0 ]->length = stats_len;
  expected_stats[ 0 ]->table_id = 1;
  expected_stats[ 0 ]->pad = 0;
  expected_stats[ 0 ]->match = MATCH;
  expected_stats[ 0 ]->duration_sec = 60;
  expected_stats[ 0 ]->duration_nsec = 10000;
  expected_stats[ 0 ]->priority = UINT16_MAX;
  expected_stats[ 0 ]->idle_timeout = 60;
  expected_stats[ 0 ]->hard_timeout = 3600;
  expected_stats[ 0 ]->cookie = 0x0102030405060708ULL;
  expected_stats[ 0 ]->packet_count = 1000;
  expected_stats[ 0 ]->byte_count = 100000;
  action = ( struct ofp_action_output * ) expected_stats[ 0 ]->actions;
  action->type = OFPAT_OUTPUT;
  action->len = 8;
  action->port = 1;
  action->max_len = 2048;

  memcpy( expected_stats[ 1 ], expected_stats[ 0 ], stats_len );
  expected_stats[ 1 ]->cookie = 0x0203040506070809ULL;
  action = ( struct ofp_action_output * ) expected_stats[ 1 ]->actions;
  action->port = 2;

  create_list( &expected_list );
  append_to_tail( &expected_list, expected_stats[ 0 ] );
  append_to_tail( &expected_list, expected_stats[ 1 ] );

  expected_message = create_flow_stats_reply( MY_TRANSACTION_ID, flags, expected_list );

  int ret_val = validate_flow_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  xfree( expected_stats[ 0 ] );
  xfree( expected_stats[ 1 ] );
  delete_list( expected_list );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_aggregate_stats_request() tests.
 ********************************************************************************/

static void
test_validate_aggregate_stats_request() {
  uint8_t table_id = 0xff;
  uint16_t out_port = 1;
  buffer *aggregate_stats_request = create_aggregate_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, table_id, out_port );

  assert_int_equal( validate_aggregate_stats_request( aggregate_stats_request ), 0 );

  free_buffer( aggregate_stats_request );
}


static void
test_validate_aggregate_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_aggregate_stats_request( NULL ) );
}


static void
test_validate_aggregate_stats_request_fails_if_message_is_not_aggregate_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_aggregate_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_aggregate_stats_reply() test.
 ********************************************************************************/

static void
test_validate_aggregate_stats_reply() {
  uint32_t flow_count = 1000;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 10000;
  buffer *expected_message = NULL;
  int ret_val;

  expected_message = create_aggregate_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, packet_count, byte_count, flow_count );

  ret_val = validate_aggregate_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( expected_message );
}


/********************************************************************************
 * validate_table_stats_request() tests.
 ********************************************************************************/

static void
test_validate_table_stats_request() {
  buffer *table_stats_request = create_table_stats_request( MY_TRANSACTION_ID, 0 );

  assert_int_equal( validate_table_stats_request( table_stats_request ), 0 );

  free_buffer( table_stats_request );
}


static void
test_validate_table_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_table_stats_request( NULL ) );
}


static void
test_validate_table_stats_request_fails_if_message_is_not_table_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_table_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_table_stats_reply() test.
 ********************************************************************************/

static void
test_validate_table_stats_reply() {
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  list_element *expected_list;
  struct ofp_table_stats *stats[ 2 ];
  buffer *expected_message = NULL;
  int ret_val;

  stats_len = sizeof( struct ofp_table_stats );

  stats[ 0 ] = xcalloc( 1, stats_len );
  stats[ 1 ] = xcalloc( 1, stats_len );

  stats[ 0 ]->table_id = 1;
  strcpy( stats[ 0 ]->name, "Table 1" );
  stats[ 0 ]->wildcards =  OFPFW_ALL;
  stats[ 0 ]->max_entries = 10000;
  stats[ 0 ]->active_count = 1000;
  stats[ 0 ]->lookup_count = 100000;
  stats[ 0 ]->matched_count = 10000;

  memcpy( stats[ 1 ], stats[ 0 ], stats_len );
  stats[ 1 ]->table_id = 2;
  strcpy( stats[ 1 ]->name, "Table 2" );

  create_list( &expected_list );
  append_to_tail( &expected_list, stats[ 0 ] );
  append_to_tail( &expected_list, stats[ 1 ] );

  expected_message = create_table_stats_reply( MY_TRANSACTION_ID, flags, expected_list );

  ret_val = validate_table_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( expected_list );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_port_stats_request() tests.
 ********************************************************************************/

static void
test_validate_port_stats_request() {
  uint16_t port_no = 1;
  buffer *port_stats_request = create_port_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no );

  assert_int_equal( validate_port_stats_request( port_stats_request ), 0 );

  free_buffer( port_stats_request );
}


static void
test_validate_port_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_port_stats_request( NULL ) );
}


static void
test_validate_port_stats_request_fails_if_message_is_not_port_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_port_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_port_stats_reply() test.
 ********************************************************************************/

static void
test_validate_port_stats_reply() {
  void *expected_data;
  uint16_t flags = OFPSF_REPLY_MORE;
  uint16_t stats_len;
  list_element *expected_list;
  struct ofp_port_stats *stats[ 2 ];
  buffer *expected_message = NULL;
  int ret_val;

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

  create_list( &expected_list );
  append_to_tail( &expected_list, stats[ 0 ] );
  append_to_tail( &expected_list, stats[ 1 ] );

  expected_data = xcalloc( 1, ( size_t ) ( stats_len * 2 ) );
  memcpy( expected_data, stats[ 0 ], stats_len );
  memcpy( ( char * ) expected_data + stats_len, stats[ 1 ], stats_len );

  expected_message = create_port_stats_reply( MY_TRANSACTION_ID, flags, expected_list );

  ret_val = validate_port_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  xfree( expected_data );
  delete_list( expected_list );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_stats_request() tests.
 ********************************************************************************/

static void
test_validate_stats_request_sucseed_with_OFPST_DESC_message() {
  buffer *stats_desc_request = create_desc_stats_request( MY_TRANSACTION_ID, 0 );

  assert_int_equal( validate_stats_request( stats_desc_request ), 0 );

  free_buffer( stats_desc_request );
}


static void
test_validate_stats_request_sucseed_with_OFPST_FLOW_message() {
  uint8_t table_id = 0xff;
  uint16_t port_no = 1;
  buffer *stats_flow_request = create_flow_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, table_id, port_no );

  assert_int_equal( validate_stats_request( stats_flow_request ), 0 );

  free_buffer( stats_flow_request );
}


static void
test_validate_stats_request_sucseed_with_OFPST_AGGREGATE_message() {
  uint8_t table_id = 0xff;
  uint16_t port_no = 1;
  buffer *stats_addregate_request = create_aggregate_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, table_id, port_no );

  assert_int_equal( validate_stats_request( stats_addregate_request ), 0 );

  free_buffer( stats_addregate_request );
}


static void
test_validate_stats_request_sucseed_with_OFPST_TABLE_message() {
  buffer *stats_table_request = create_table_stats_request( MY_TRANSACTION_ID, 0 );

  assert_int_equal( validate_stats_request( stats_table_request ), 0 );

  free_buffer( stats_table_request );
}


static void
test_validate_stats_request_sucseed_with_OFPST_PORT_message() {
  uint16_t port_no = 1;
  buffer *stats_port_request = create_port_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no );

  assert_int_equal( validate_stats_request( stats_port_request ), 0 );

  free_buffer( stats_port_request );
}


static void
test_validate_stats_request_sucseed_with_OFPST_QUEUE_message() {
  uint16_t port_no = 1;
  uint32_t queue_id = 10;
  buffer *stats_queue_request = create_queue_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no, queue_id );

  assert_int_equal( validate_stats_request( stats_queue_request ), 0 );

  free_buffer( stats_queue_request );
}


static void
test_validate_stats_request_sucseed_with_OFPST_VENDOR_message() {
  buffer *stats_vendor_request = create_vendor_stats_request( MY_TRANSACTION_ID, NO_FLAGS, VENDOR_ID, NULL );

  assert_int_equal( validate_stats_request( stats_vendor_request ), 0 );

  free_buffer( stats_vendor_request );
}


static void
test_validate_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_stats_request( NULL ) );
}


static void
test_validate_stats_request_fails_with_unsupported_stats_type() {
  buffer *broken_stats_request = create_vendor_stats_request( MY_TRANSACTION_ID, NO_FLAGS, VENDOR_ID, NULL );
  struct ofp_stats_request *stats_request = ( struct ofp_stats_request * ) broken_stats_request->data;
  const uint16_t unsupported_stats_type = ( uint16_t ) -2;
  stats_request->type = htons( unsupported_stats_type );

  assert_int_equal( validate_stats_request( broken_stats_request ), ERROR_UNSUPPORTED_STATS_TYPE );

  free_buffer( broken_stats_request );
}


/********************************************************************************
 * validate_stats_reply() tests.
 ********************************************************************************/

static void
test_validate_stats_reply_with_OFPST_DESC_message() {
  const char mfr_desc[ DESC_STR_LEN ] = "Trema Corporation";
  const char hw_desc[ DESC_STR_LEN ] = "Switching Hub type:B - for Trema";
  const char sw_desc[ DESC_STR_LEN ] = "System - b - OS version 1.0.0";
  const char serial_num[ SERIAL_NUM_LEN ] = "SN101224";
  const char dp_desc[ DESC_STR_LEN ] = "readble datapath 1012-1103";
  buffer *desc_stats_reply = create_desc_stats_reply( MY_TRANSACTION_ID, 0, mfr_desc, hw_desc, sw_desc, serial_num, dp_desc );

  assert_int_equal( validate_stats_reply( desc_stats_reply ), 0 );

  free_buffer( desc_stats_reply );
}


static void
test_validate_stats_reply_with_OFPST_FLOW_message() {
  list_element *flow_stats_list;
  struct ofp_flow_stats *flow_stats[ 2 ];
  struct ofp_action_output *action;

  uint16_t flow_stats_len = offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output );
  flow_stats[ 0 ] = xcalloc( 1, flow_stats_len );
  flow_stats[ 1 ] = xcalloc( 1, flow_stats_len );
  flow_stats[ 0 ]->length = flow_stats_len;
  flow_stats[ 0 ]->table_id = 1;
  flow_stats[ 0 ]->pad = 0;
  flow_stats[ 0 ]->match = MATCH;
  flow_stats[ 0 ]->duration_sec = 60;
  flow_stats[ 0 ]->duration_nsec = 10000;
  flow_stats[ 0 ]->priority = PRIORITY;
  flow_stats[ 0 ]->idle_timeout = ONE_MINUTES_TIMEOUT;
  flow_stats[ 0 ]->hard_timeout = ( uint16_t ) ( ONE_MINUTES_TIMEOUT * 60 );
  flow_stats[ 0 ]->cookie = ISSUED_COOKIE;
  flow_stats[ 0 ]->packet_count = RECEIVED_PACKETS;
  flow_stats[ 0 ]->byte_count = RECEIVED_BYTES;
  action = ( struct ofp_action_output * ) flow_stats[ 0 ]->actions;
  action->type = OFPAT_OUTPUT;
  action->len = 8;
  action->port = 1;
  action->max_len = 2048;
  memcpy( flow_stats[ 1 ], flow_stats[ 0 ], flow_stats_len );
  ( ( struct ofp_action_output * ) flow_stats[ 1 ]->actions )->port = 2;
  create_list( &flow_stats_list );
  append_to_tail( &flow_stats_list, flow_stats[ 0 ] );
  append_to_tail( &flow_stats_list, flow_stats[ 1 ] );
  buffer *flow_stats_reply = create_flow_stats_reply( MY_TRANSACTION_ID, OFPSF_REPLY_MORE, flow_stats_list );

  assert_int_equal( validate_stats_reply( flow_stats_reply ), 0 );

  xfree( flow_stats[ 0 ] );
  xfree( flow_stats[ 1 ] );
  delete_list( flow_stats_list );
  free_buffer( flow_stats_reply );
}


static void
test_validate_stats_reply_with_OFPST_AGGREGATE_message() {
  uint32_t flow_count = 1000;
  uint64_t packet_count = RECEIVED_PACKETS;
  uint64_t byte_count = RECEIVED_BYTES;
  buffer *aggregate_stats_reply = create_aggregate_stats_reply( MY_TRANSACTION_ID, 0, packet_count, byte_count, flow_count );

  assert_int_equal( validate_stats_reply( aggregate_stats_reply ), 0 );

  free_buffer( aggregate_stats_reply );
}


static void
test_validate_stats_reply_with_OFPST_TABLE_message() {
  uint16_t table_stats_len;
  list_element *table_stats_list;
  struct ofp_table_stats *table_stats[ 2 ];

  table_stats_len = sizeof( struct ofp_table_stats );
  table_stats[ 0 ] = xcalloc( 1, table_stats_len );
  table_stats[ 1 ] = xcalloc( 1, table_stats_len );
  table_stats[ 0 ]->table_id = 1;
  strcpy( table_stats[ 0 ]->name, "Name of Flow Table No.1" );
  table_stats[ 0 ]->wildcards =  OFPFW_ALL;
  table_stats[ 0 ]->max_entries = SUPPORTED_ENTRIES_IN_OFPST_TABLE;
  table_stats[ 0 ]->active_count = ACTIVE_ENTRIES_IN_OFPST_TABLE;
  table_stats[ 0 ]->lookup_count = PACKETS_LOOK_UP_IN_OFPST_TABLE;
  table_stats[ 0 ]->matched_count = PACKETS_HIT_OFPST_TABLE;
  memcpy( table_stats[ 1 ], table_stats[ 0 ], table_stats_len );
  table_stats[ 1 ]->table_id = 2;
  strcpy( table_stats[ 1 ]->name, "Name of Flow Table No.2" );
  create_list( &table_stats_list );
  append_to_tail( &table_stats_list, table_stats[ 0 ] );
  append_to_tail( &table_stats_list, table_stats[ 1 ] );
  buffer *table_stats_reply = create_table_stats_reply( MY_TRANSACTION_ID, OFPSF_REPLY_MORE, table_stats_list );

  assert_int_equal( validate_stats_reply( table_stats_reply ), 0 );

  xfree( table_stats[ 0 ] );
  xfree( table_stats[ 1 ] );
  delete_list( table_stats_list );
  free_buffer( table_stats_reply );
}


static void
test_validate_stats_reply_with_OFPST_PORT_message() {
  list_element *port_stats_list;
  struct ofp_port_stats *port_stats[ 2 ];

  uint16_t port_stats_len = sizeof( struct ofp_port_stats );
  port_stats[ 0 ] = xcalloc( 1, port_stats_len );
  port_stats[ 1 ] = xcalloc( 1, port_stats_len );
  port_stats[ 0 ]->port_no = 1;
  port_stats[ 0 ]->rx_packets = RECEIVED_PACKETS;
  port_stats[ 0 ]->tx_packets = TRANSMITTED_PACKETS;
  port_stats[ 0 ]->rx_bytes = RECEIVED_BYTES;
  port_stats[ 0 ]->tx_bytes = TRANSMITTED_BYTES;
  // Not packets dropped.
  port_stats[ 0 ]->rx_dropped = NO_ERROR;
  port_stats[ 0 ]->tx_dropped = NO_ERROR;
  // Not packet errors.
  port_stats[ 0 ]->rx_errors = NO_ERROR;
  port_stats[ 0 ]->tx_errors = NO_ERROR;
  // Not frame alignment errors.
  port_stats[ 0 ]->rx_frame_err = NO_ERROR;
  // Not packets with RX overrun.
  port_stats[ 0 ]->rx_over_err = NO_ERROR;
  // Not CRC errors.
  port_stats[ 0 ]->rx_crc_err = NO_ERROR;
  // Not collisions.
  port_stats[ 0 ]->collisions = NO_ERROR;
  memcpy( port_stats[ 1 ], port_stats[ 0 ], port_stats_len );
  port_stats[ 1 ]->port_no = 2;
  create_list( &port_stats_list );
  append_to_tail( &port_stats_list, port_stats[ 0 ] );
  append_to_tail( &port_stats_list, port_stats[ 1 ] );
  buffer *port_stats_reply = create_port_stats_reply( MY_TRANSACTION_ID, OFPSF_REPLY_MORE, port_stats_list );

  assert_int_equal( validate_stats_reply( port_stats_reply ), 0 );

  xfree( port_stats[ 0 ] );
  xfree( port_stats[ 1 ] );
  delete_list( port_stats_list );
  free_buffer( port_stats_reply );
}


static void
test_validate_stats_reply_with_OFPST_QUEUE_message() {
  list_element *queue_stats_list;
  uint16_t queue_stats_len;
  struct ofp_queue_stats *queue_stats[ 2 ];

  queue_stats_len = sizeof( struct ofp_queue_stats );
  queue_stats[ 0 ] = xcalloc( 1, queue_stats_len );
  queue_stats[ 1 ] = xcalloc( 1, queue_stats_len );
  queue_stats[ 0 ]->port_no = 1;
  queue_stats[ 0 ]->queue_id = 2;
  queue_stats[ 0 ]->tx_bytes = TRANSMITTED_BYTES;
  queue_stats[ 0 ]->tx_packets = TRANSMITTED_PACKETS;
  // Not packets dropped.
  queue_stats[ 0 ]->tx_errors = NO_ERROR;
  memcpy( queue_stats[ 1 ], queue_stats[ 0 ], queue_stats_len );
  queue_stats[ 1 ]->queue_id = 3;
  create_list( &queue_stats_list );
  append_to_tail( &queue_stats_list, queue_stats[ 0 ] );
  append_to_tail( &queue_stats_list, queue_stats[ 1 ] );
  buffer *queue_stats_reply = create_queue_stats_reply( MY_TRANSACTION_ID, OFPSF_REPLY_MORE, queue_stats_list );

  assert_int_equal( validate_stats_reply( queue_stats_reply ), 0 );

  xfree( queue_stats[ 0 ] );
  xfree( queue_stats[ 1 ] );
  delete_list( queue_stats_list );
  free_buffer( queue_stats_reply );
}


static void
test_validate_stats_reply_with_OFPST_VENDOR_message() {
  buffer *vendor_body = alloc_buffer_with_length( 128 );
  append_back_buffer( vendor_body, 128 );
  memset( vendor_body->data, 0xa1, vendor_body->length );
  buffer *vendor_stats_reply = create_vendor_stats_reply( MY_TRANSACTION_ID, 0, VENDOR_ID, vendor_body );

  assert_int_equal( validate_stats_reply( vendor_stats_reply ), 0 );

  free_buffer( vendor_body );
  free_buffer( vendor_stats_reply );
}


static void
test_validate_stats_reply_fails_if_message_is_NULL() {
  expect_assert_failure( validate_stats_reply( NULL ) );
}


static void
test_validate_stats_reply_fails_with_unsupported_stats_type() {
  buffer *broken_stats_reply = create_vendor_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, VENDOR_ID, NULL );
  struct ofp_stats_reply *stats_reply = ( struct ofp_stats_reply * ) broken_stats_reply->data;
  const uint16_t unsupported_stats_type = ( uint16_t ) -2;
  stats_reply->type = htons( unsupported_stats_type );

  assert_int_equal( validate_stats_reply( broken_stats_reply ), ERROR_UNSUPPORTED_STATS_TYPE );

  free_buffer( broken_stats_reply );
}


/********************************************************************************
 * validate_queue_stats_request() tests.
 ********************************************************************************/

static void
test_validate_queue_stats_request() {
  buffer *port_stats_request = create_queue_stats_request( MY_TRANSACTION_ID, NO_FLAGS, OFPP_ALL, OFPQ_ALL );

  assert_int_equal( validate_queue_stats_request( port_stats_request ), 0 );

  free_buffer( port_stats_request );
}


static void
test_validate_queue_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_queue_stats_request( NULL ) );
}


static void
test_validate_queue_stats_request_fails_if_message_is_not_queue_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_queue_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_queue_stats_reply() test.
 ********************************************************************************/

static void
test_validate_queue_stats_reply() {
  uint16_t flags = OFPSF_REPLY_MORE;
  list_element *list;
  uint16_t stats_len;
  struct ofp_queue_stats *stats[ 2 ];
  buffer *expected_message;
  int ret_val;

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

  create_list( &list );
  append_to_tail( &list, stats[ 0 ] );
  append_to_tail( &list, stats[ 1 ] );

  expected_message = create_queue_stats_reply( MY_TRANSACTION_ID, flags, list );

  ret_val = validate_queue_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  xfree( stats[ 0 ] );
  xfree( stats[ 1 ] );
  delete_list( list );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_vendor_stats_request() tests.
 ********************************************************************************/

static void
test_validate_vendor_stats_request() {
  buffer *vendor_stats_request = create_vendor_stats_request( MY_TRANSACTION_ID, NO_FLAGS, VENDOR_ID, NULL );

  assert_int_equal( validate_vendor_stats_request( vendor_stats_request ), 0 );

  free_buffer( vendor_stats_request );
}


static void
test_validate_vendor_stats_request_fails_if_message_is_NULL() {
  expect_assert_failure( validate_vendor_stats_request( NULL ) );
}


static void
test_validate_vendor_stats_request_fails_if_message_is_not_vendor_stats_request() {
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, NULL );

  assert_int_equal( validate_vendor_stats_request( echo_request ), ERROR_INVALID_TYPE );

  free_buffer( echo_request );
}


/********************************************************************************
 * validate_vendor_stats_reply() test.
 ********************************************************************************/

static void
test_validate_vendor_stats_reply() {
  uint32_t expected_vendor = VENDOR_ID;
  buffer *expected_message, *body;
  struct ofp_stats_reply;
  int ret_val;

  body = alloc_buffer_with_length( 128 );
  append_back_buffer( body, 128 );
  memset( body->data, 0xa1, body->length );

  expected_message = create_vendor_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, expected_vendor, body );

  ret_val = validate_vendor_stats_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  free_buffer( body );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_barrier_reply() test.
 ********************************************************************************/

static void
test_validate_barrier_reply() {
  buffer *expected_message;
  int ret_val;
  expected_message = create_barrier_reply( MY_TRANSACTION_ID );

  ret_val = validate_barrier_reply( expected_message );
  assert_int_equal( ret_val, 0 );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_queue_get_config_reply() test.
 ********************************************************************************/

static void
test_validate_queue_get_config_reply() {
  size_t queue_len;
  uint16_t port = 1;
  list_element *list;
  buffer *expected_message;
  struct ofp_packet_queue *queue[ 2 ];
  struct ofp_queue_prop_header *prop_header;
  int ret_val;

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

  create_list( &list );
  append_to_tail( &list, queue[ 0 ] );
  append_to_tail( &list, queue[ 1 ] );

  expected_message = create_queue_get_config_reply( MY_TRANSACTION_ID, port, list );

  ret_val = validate_queue_get_config_reply( expected_message );
  assert_int_equal( ret_val, 0 );

  xfree( queue[ 0 ] );
  xfree( queue[ 1 ] );
  delete_list( list );
  free_buffer( expected_message );
}


/********************************************************************************
 * validate_action_output() tests.
 ********************************************************************************/

static void
test_validate_action_output() {
  uint16_t port = 1;

  openflow_actions *actions = create_actions();
  append_action_output( actions, port, MAX_LENGTH_OF_SEND_PACKET );
  struct ofp_action_output action_output;
  hton_action_output( &action_output, ( struct ofp_action_output * ) ( actions->list->data ) );

  assert_int_equal( validate_action_output( &action_output ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_output_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_src( actions, HW_ADDR );
  struct ofp_action_dl_addr action_dl_addr;
  hton_action_dl_addr( &action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_output( ( struct ofp_action_output * ) &action_dl_addr ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_output_fails_with_too_short_ofp_action_output() {
  uint16_t port = 1;

  openflow_actions *actions = create_actions();
  append_action_output( actions, port, MAX_LENGTH_OF_SEND_PACKET );
  uint16_t too_short_action_length = sizeof( struct ofp_action_output ) - 1;
  ( ( struct ofp_action_output * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_output too_short_action_output;
  hton_action_output( &too_short_action_output, ( struct ofp_action_output * ) ( actions->list->data ) );

  assert_int_equal( validate_action_output( &too_short_action_output ), ERROR_TOO_SHORT_ACTION_OUTPUT );

  delete_actions( actions );
}


static void
test_validate_action_output_fails_with_too_long_ofp_action_output() {
  uint16_t port = 1;

  openflow_actions *actions = create_actions();
  append_action_output( actions, port, MAX_LENGTH_OF_SEND_PACKET );
  uint16_t too_long_action_length = sizeof( struct ofp_action_output ) + 1;
  ( ( struct ofp_action_output * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_output too_long_action_output;
  hton_action_output( &too_long_action_output, ( struct ofp_action_output * ) ( actions->list->data ) );

  assert_int_equal( validate_action_output( &too_long_action_output ), ERROR_TOO_LONG_ACTION_OUTPUT );

  delete_actions( actions );
}


static void
test_validate_action_output_fails_with_invalid_port_no() {
  uint16_t port = 0;

  openflow_actions *actions = create_actions();
  append_action_output( actions, port, MAX_LENGTH_OF_SEND_PACKET );
  struct ofp_action_output action_output;
  hton_action_output( &action_output, ( struct ofp_action_output * ) ( actions->list->data ) );

  assert_int_equal( validate_action_output( &action_output ), ERROR_INVALID_PORT_NO );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_vlan_vid() tests.
 ********************************************************************************/

static void
test_validate_action_set_vlan_vid() {
  openflow_actions *actions = create_actions();
  uint16_t vlan_vid = 0x0001;
  append_action_set_vlan_vid( actions, vlan_vid );
  struct ofp_action_vlan_vid action_vlan_vid;
  hton_action_vlan_vid( &action_vlan_vid, actions->list->data );

  assert_int_equal( validate_action_set_vlan_vid( &action_vlan_vid ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_vid_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_dst( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, actions->list->data );

  assert_int_equal( validate_action_set_vlan_vid( ( struct ofp_action_vlan_vid * ) &action_nw_addr ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_vid_fails_with_invalid_vlan_vid() {
  openflow_actions *actions = create_actions();
  uint16_t vlan_vid = 0x0001;
  append_action_set_vlan_vid( actions, vlan_vid );
  uint16_t invalid_vlan_vid = 0x1000;
  ( ( struct ofp_action_vlan_vid * ) actions->list->data )->vlan_vid = invalid_vlan_vid;
  struct ofp_action_vlan_vid action_vlan_vid;
  hton_action_vlan_vid( &action_vlan_vid, actions->list->data );

  assert_int_equal( validate_action_set_vlan_vid( &action_vlan_vid ), ERROR_INVALID_VLAN_VID );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_vid_fails_with_too_short_ofp_action_vlan_vid() {
  openflow_actions *actions = create_actions();
  uint16_t vlan_vid = 0x0001;
  append_action_set_vlan_vid( actions, vlan_vid );
  struct ofp_action_vlan_vid *too_short_action_length = ( struct ofp_action_vlan_vid * ) actions->list->data;
  too_short_action_length->len = ( uint16_t ) ( too_short_action_length->len - 1 );
  struct ofp_action_vlan_vid too_short_action_vlan_vid;
  hton_action_vlan_vid( &too_short_action_vlan_vid, actions->list->data );

  assert_int_equal( validate_action_set_vlan_vid( &too_short_action_vlan_vid ), ERROR_TOO_SHORT_ACTION_VLAN_VID );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_vid_fails_with_too_long_ofp_action_vlan_vid() {
  openflow_actions *actions = create_actions();
  uint16_t vlan_vid = 0x0001;
  append_action_set_vlan_vid( actions, vlan_vid );
  struct ofp_action_vlan_vid *too_long_action_length = ( struct ofp_action_vlan_vid * ) actions->list->data;
  too_long_action_length->len = ( uint16_t ) ( too_long_action_length->len + 1 );
  struct ofp_action_vlan_vid too_long_action_vlan_vid;
  hton_action_vlan_vid( &too_long_action_vlan_vid, actions->list->data );

  assert_int_equal( validate_action_set_vlan_vid( &too_long_action_vlan_vid ), ERROR_TOO_LONG_ACTION_VLAN_VID );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_vlan_pcp() tests.
 ********************************************************************************/

static void
test_validate_action_set_vlan_pcp() {
  openflow_actions *actions = create_actions();
  uint8_t vlan_pcp = 0x01;
  append_action_set_vlan_pcp( actions, vlan_pcp );
  struct ofp_action_vlan_pcp action_vlan_pcp;
  hton_action_vlan_pcp( &action_vlan_pcp, actions->list->data );

  assert_int_equal( validate_action_set_vlan_pcp( &action_vlan_pcp ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_pcp_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_dst( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, actions->list->data );

  assert_int_equal( validate_action_set_vlan_pcp( ( struct ofp_action_vlan_pcp * ) &action_nw_addr ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_pcp_fails_with_invalid_vlan_pcp() {
  openflow_actions *actions = create_actions();
  uint8_t vlan_pcp = 0x01;
  append_action_set_vlan_pcp( actions, vlan_pcp );
  uint8_t invalid_vlan_pcp = 0x08;
  ( ( struct ofp_action_vlan_pcp * ) actions->list->data )->vlan_pcp = invalid_vlan_pcp;
  struct ofp_action_vlan_pcp action_vlan_pcp;
  hton_action_vlan_pcp( &action_vlan_pcp, actions->list->data );

  assert_int_equal( validate_action_set_vlan_pcp( &action_vlan_pcp ), ERROR_INVALID_VLAN_PCP );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_pcp_fails_with_too_short_ofp_action_vlan_pcp() {
  openflow_actions *actions = create_actions();
  uint8_t vlan_pcp = 0x01;
  append_action_set_vlan_pcp( actions, vlan_pcp );
  struct ofp_action_vlan_pcp *too_short_action_length = ( struct ofp_action_vlan_pcp * ) actions->list->data;
  too_short_action_length->len = ( uint16_t ) ( too_short_action_length->len - 1 );
  struct ofp_action_vlan_pcp too_short_action_vlan_pcp;
  hton_action_vlan_pcp( &too_short_action_vlan_pcp, actions->list->data );

  assert_int_equal( validate_action_set_vlan_pcp( &too_short_action_vlan_pcp ), ERROR_TOO_SHORT_ACTION_VLAN_PCP );

  delete_actions( actions );
}


static void
test_validate_action_set_vlan_pcp_fails_with_too_long_ofp_action_vlan_pcp() {
  openflow_actions *actions = create_actions();
  uint8_t vlan_pcp = 0x01;
  append_action_set_vlan_pcp( actions, vlan_pcp );
  struct ofp_action_vlan_pcp *too_long_action_length = ( struct ofp_action_vlan_pcp * ) actions->list->data;
  too_long_action_length->len = ( uint16_t ) ( too_long_action_length->len + 1 );
  struct ofp_action_vlan_pcp too_long_action_vlan_pcp;
  hton_action_vlan_pcp( &too_long_action_vlan_pcp, actions->list->data );

  assert_int_equal( validate_action_set_vlan_pcp( &too_long_action_vlan_pcp ), ERROR_TOO_LONG_ACTION_VLAN_PCP );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_strip_vlan() tests.
 ********************************************************************************/

static void
test_validate_action_strip_vlan() {
  openflow_actions *actions = create_actions();
  append_action_strip_vlan( actions );
  struct ofp_action_header action_strip_vlan;
  hton_action_strip_vlan( &action_strip_vlan, actions->list->data );

  assert_int_equal( validate_action_strip_vlan( &action_strip_vlan ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_strip_vlan_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_dst( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, actions->list->data );

  assert_int_equal( validate_action_strip_vlan( ( struct ofp_action_header * ) &action_nw_addr ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_strip_vlan_fails_with_too_short_ofp_action_header() {
  openflow_actions *actions = create_actions();
  append_action_strip_vlan( actions );
  struct ofp_action_header *too_short_action_length = ( struct ofp_action_header * ) actions->list->data;
  too_short_action_length->len = ( uint16_t ) ( too_short_action_length->len - 1 );
  struct ofp_action_header too_short_action_header;
  hton_action_strip_vlan( &too_short_action_header, actions->list->data );

  assert_int_equal( validate_action_strip_vlan( &too_short_action_header ), ERROR_TOO_SHORT_ACTION_STRIP_VLAN );

  delete_actions( actions );
}


static void
test_validate_action_strip_vlan_fails_with_too_long_ofp_action_header() {
  openflow_actions *actions = create_actions();
  append_action_strip_vlan( actions );
  struct ofp_action_header *too_long_action_length = ( struct ofp_action_header * ) actions->list->data;
  too_long_action_length->len = ( uint16_t ) ( too_long_action_length->len + 1 );
  struct ofp_action_header too_long_action_header;
  hton_action_strip_vlan( &too_long_action_header, actions->list->data );

  assert_int_equal( validate_action_strip_vlan( &too_long_action_header ), ERROR_TOO_LONG_ACTION_STRIP_VLAN );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_dl_src() tests.
 ********************************************************************************/

static void
test_validate_action_set_dl_src() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_src( actions, HW_ADDR );
  struct ofp_action_dl_addr action_dl_addr;
  hton_action_dl_addr( &action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_src( &action_dl_addr ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_dl_src_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_src( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_src( ( struct ofp_action_dl_addr * ) &action_nw_addr ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_dl_src_fails_with_too_short_ofp_action_dl_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_src( actions, HW_ADDR );
  uint16_t too_short_action_length = sizeof( struct ofp_action_dl_addr ) - 1;
  ( ( struct ofp_action_dl_addr * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_dl_addr too_short_action_dl_addr;
  hton_action_dl_addr( &too_short_action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_src( &too_short_action_dl_addr ), ERROR_TOO_SHORT_ACTION_DL_SRC );

  delete_actions( actions );
}


static void
test_validate_action_set_dl_src_fails_with_too_long_ofp_action_dl_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_src( actions, HW_ADDR );
  uint16_t too_long_action_length = sizeof( struct ofp_action_dl_addr ) + 1;
  ( ( struct ofp_action_dl_addr * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_dl_addr too_long_action_dl_addr;
  hton_action_dl_addr( &too_long_action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_src( &too_long_action_dl_addr ), ERROR_TOO_LONG_ACTION_DL_SRC );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_dl_dst() tests.
 ********************************************************************************/

static void
test_validate_action_set_dl_dst() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_dst( actions, HW_ADDR );
  struct ofp_action_dl_addr action_dl_addr;
  hton_action_dl_addr( &action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_dst( &action_dl_addr ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_dl_dst_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_src( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_dst( ( struct ofp_action_dl_addr * ) &action_nw_addr ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_dl_dst_fails_with_too_short_ofp_action_dl_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_dst( actions, HW_ADDR );
  uint16_t too_short_action_length = sizeof( struct ofp_action_dl_addr ) - 1;
  ( ( struct ofp_action_dl_addr * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_dl_addr too_short_action_dl_addr;
  hton_action_dl_addr( &too_short_action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_dst( &too_short_action_dl_addr ), ERROR_TOO_SHORT_ACTION_DL_DST );

  delete_actions( actions );
}


static void
test_validate_action_set_dl_dst_fails_with_too_long_ofp_action_dl_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_dl_dst( actions, HW_ADDR );
  uint16_t too_long_action_length = sizeof( struct ofp_action_dl_addr ) + 1;
  ( ( struct ofp_action_dl_addr * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_dl_addr too_long_action_dl_addr;
  hton_action_dl_addr( &too_long_action_dl_addr, ( struct ofp_action_dl_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_dl_dst( &too_long_action_dl_addr ), ERROR_TOO_LONG_ACTION_DL_DST );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_nw_src() tests.
 ********************************************************************************/

static void
test_validate_action_set_nw_src() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_src( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_src( &action_nw_addr ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_src_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_tos( actions, NW_TOS );
  struct ofp_action_nw_tos action_nw_tos;
  hton_action_nw_tos( &action_nw_tos, ( struct ofp_action_nw_tos * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_src( ( struct ofp_action_nw_addr * ) &action_nw_tos ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_src_fails_with_too_short_ofp_action_nw_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_src( actions, NW_ADDR );
  uint16_t too_short_action_length = sizeof( struct ofp_action_nw_addr ) - 1;
  ( ( struct ofp_action_nw_addr * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_nw_addr too_short_action_nw_addr;
  hton_action_nw_addr( &too_short_action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_src( &too_short_action_nw_addr ), ERROR_TOO_SHORT_ACTION_NW_SRC );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_src_fails_with_too_long_ofp_action_nw_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_src( actions, NW_ADDR );
  uint16_t too_long_action_length = sizeof( struct ofp_action_nw_addr ) + 1;
  ( ( struct ofp_action_nw_addr * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_nw_addr too_long_action_nw_addr;
  hton_action_nw_addr( &too_long_action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_src( &too_long_action_nw_addr ), ERROR_TOO_LONG_ACTION_NW_SRC );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_nw_dst() tests.
 ********************************************************************************/

static void
test_validate_action_set_nw_dst() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_dst( actions, NW_ADDR );
  struct ofp_action_nw_addr action_nw_addr;
  hton_action_nw_addr( &action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_dst( &action_nw_addr ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_dst_fails_with_invalid_action_type() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_tos( actions, NW_TOS );
  struct ofp_action_nw_tos action_nw_tos;
  hton_action_nw_tos( &action_nw_tos, ( struct ofp_action_nw_tos * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_dst( ( struct ofp_action_nw_addr * ) &action_nw_tos ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_dst_fails_with_too_short_ofp_action_nw_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_dst( actions, NW_ADDR );
  uint16_t too_short_action_length = sizeof( struct ofp_action_nw_addr ) - 1;
  ( ( struct ofp_action_nw_addr * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_nw_addr too_short_action_nw_addr;
  hton_action_nw_addr( &too_short_action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_dst( &too_short_action_nw_addr ), ERROR_TOO_SHORT_ACTION_NW_DST );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_dst_fails_with_too_long_ofp_action_nw_addr() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_dst( actions, NW_ADDR );
  uint16_t too_long_action_length = sizeof( struct ofp_action_nw_addr ) + 1;
  ( ( struct ofp_action_nw_addr * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_nw_addr too_long_action_nw_addr;
  hton_action_nw_addr( &too_long_action_nw_addr, ( struct ofp_action_nw_addr * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_dst( &too_long_action_nw_addr ), ERROR_TOO_LONG_ACTION_NW_DST );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_nw_tos() tests.
 ********************************************************************************/

static void
test_validate_action_set_nw_tos() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_tos( actions, NW_TOS );
  struct ofp_action_nw_tos action_nw_tos;
  hton_action_nw_tos( &action_nw_tos, ( struct ofp_action_nw_tos * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_tos( &action_nw_tos ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_tos_fails_with_invalid_action_type() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_src( actions, tp_port );
  struct ofp_action_tp_port action_tp_port;
  hton_action_tp_port( &action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_tos( ( struct ofp_action_nw_tos * ) &action_tp_port ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_tos_fails_with_too_short_ofp_action_nw_tos() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_tos( actions, NW_TOS );
  uint16_t too_short_action_length = sizeof( struct ofp_action_nw_tos ) - 1;
  ( ( struct ofp_action_nw_tos * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_nw_tos too_short_action_nw_tos;
  hton_action_nw_tos( &too_short_action_nw_tos, ( struct ofp_action_nw_tos * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_tos( &too_short_action_nw_tos ), ERROR_TOO_SHORT_ACTION_NW_TOS );

  delete_actions( actions );
}


static void
test_validate_action_set_nw_tos_fails_with_too_long_ofp_action_nw_tos() {
  openflow_actions *actions = create_actions();
  append_action_set_nw_tos( actions, NW_TOS );
  uint16_t too_long_action_length = sizeof( struct ofp_action_nw_tos ) + 1;
  ( ( struct ofp_action_nw_tos * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_nw_tos too_long_action_nw_tos;
  hton_action_nw_tos( &too_long_action_nw_tos, ( struct ofp_action_nw_tos * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_nw_tos( &too_long_action_nw_tos ), ERROR_TOO_LONG_ACTION_NW_TOS );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_tp_src() tests.
 ********************************************************************************/

static void
test_validate_action_set_tp_src() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_src( actions, tp_port );
  struct ofp_action_tp_port action_tp_port;
  hton_action_tp_port( &action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_src( &action_tp_port ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_tp_src_fails_with_invalid_action_type() {
  uint16_t port = 1;
  uint32_t queue_id = 10;

  openflow_actions *actions = create_actions();
  append_action_enqueue( actions, port, queue_id );
  struct ofp_action_enqueue action_enqueue;
  hton_action_enqueue( &action_enqueue, ( struct ofp_action_enqueue * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_src( ( struct ofp_action_tp_port * ) &action_enqueue ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_tp_src_fails_with_too_short_ofp_action_tp_port() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_src( actions, tp_port );
  uint16_t too_short_action_length = sizeof( struct ofp_action_tp_port ) - 1;
  ( ( struct ofp_action_tp_port * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_tp_port too_short_action_tp_port;
  hton_action_tp_port( &too_short_action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_src( &too_short_action_tp_port ), ERROR_TOO_SHORT_ACTION_TP_SRC );

  delete_actions( actions );
}


static void
test_validate_action_set_tp_src_fails_with_too_long_ofp_action_tp_port() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_src( actions, tp_port );
  uint16_t too_long_action_length = sizeof( struct ofp_action_tp_port ) + 1;
  ( ( struct ofp_action_tp_port * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_tp_port too_long_action_tp_port;
  hton_action_tp_port( &too_long_action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_src( &too_long_action_tp_port ), ERROR_TOO_LONG_ACTION_TP_SRC );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_set_tp_dst() tests.
 ********************************************************************************/

static void
test_validate_action_set_tp_dst() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_dst( actions, tp_port );
  struct ofp_action_tp_port action_tp_port;
  hton_action_tp_port( &action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_dst( &action_tp_port ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_set_tp_dst_fails_with_invalid_action_type() {
  uint16_t port = 1;
  uint32_t queue_id = 10;

  openflow_actions *actions = create_actions();
  append_action_enqueue( actions, port, queue_id );
  struct ofp_action_enqueue action_enqueue;
  hton_action_enqueue( &action_enqueue, ( struct ofp_action_enqueue * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_dst( ( struct ofp_action_tp_port * ) &action_enqueue ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_set_tp_dst_fails_with_too_short_ofp_action_tp_port() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_dst( actions, tp_port );
  uint16_t too_short_action_length = sizeof( struct ofp_action_tp_port ) - 1;
  ( ( struct ofp_action_tp_port * ) ( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_tp_port too_short_action_tp_port;
  hton_action_tp_port( &too_short_action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_dst( &too_short_action_tp_port ), ERROR_TOO_SHORT_ACTION_TP_DST );

  delete_actions( actions );
}


static void
test_validate_action_set_tp_dst_fails_with_too_long_ofp_action_tp_port() {
  uint16_t tp_port = 1;

  openflow_actions *actions = create_actions();
  append_action_set_tp_dst( actions, tp_port );
  uint16_t too_long_action_length = sizeof( struct ofp_action_tp_port ) + 1;
  ( ( struct ofp_action_tp_port * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_tp_port too_long_action_tp_port;
  hton_action_tp_port( &too_long_action_tp_port, ( struct ofp_action_tp_port * ) ( actions->list->data ) );

  assert_int_equal( validate_action_set_tp_dst( &too_long_action_tp_port ), ERROR_TOO_LONG_ACTION_TP_DST );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_enqueue() tests.
 ********************************************************************************/

static void
test_validate_action_enqueue() {
  uint16_t port = 1;
  uint32_t queue_id = 10;

  openflow_actions *actions = create_actions();
  append_action_enqueue( actions, port, queue_id );
  struct ofp_action_enqueue action_enqueue;
  hton_action_enqueue( &action_enqueue, ( struct ofp_action_enqueue * ) ( actions->list->data ) );

  assert_int_equal( validate_action_enqueue( &action_enqueue ), 0 );

  delete_actions( actions );
}


static void
test_validate_action_enqueue_fails_with_invalid_action_type() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  openflow_actions *actions = create_actions();
  append_action_vendor( actions, VENDOR_ID, body );
  struct ofp_action_vendor_header *action_vendor = xmalloc( sizeof( struct ofp_action_vendor_header ) + body->length );
  hton_action_vendor( action_vendor, ( struct ofp_action_vendor_header * ) ( actions->list->data ) );

  assert_int_equal( validate_action_enqueue( ( struct ofp_action_enqueue * ) action_vendor ), ERROR_INVALID_ACTION_TYPE );

  free_buffer( body );
  delete_actions( actions );
  xfree( action_vendor );
}


static void
test_validate_action_enqueue_fails_with_too_short_ofp_action_enqueue() {
  uint16_t port = 1;
  uint32_t queue_id = 10;

  openflow_actions *actions = create_actions();
  append_action_enqueue( actions, port, queue_id );
  uint16_t too_short_action_length = sizeof( struct ofp_action_enqueue ) - 1;
  ( ( struct ofp_action_enqueue * )( actions->list->data ) )->len = too_short_action_length;
  struct ofp_action_enqueue too_short_action_enqueue;
  hton_action_enqueue( &too_short_action_enqueue, ( struct ofp_action_enqueue * ) ( actions->list->data ) );

  assert_int_equal( validate_action_enqueue( &too_short_action_enqueue ), ERROR_TOO_SHORT_ACTION_ENQUEUE );

  delete_actions( actions );
}


static void
test_validate_action_enqueue_fails_with_too_long_ofp_action_enqueue() {
  uint16_t port = 1;
  uint32_t queue_id = 10;

  openflow_actions *actions = create_actions();
  append_action_enqueue( actions, port, queue_id );
  uint16_t too_long_action_length = sizeof( struct ofp_action_enqueue ) + 1;
  ( ( struct ofp_action_enqueue * ) ( actions->list->data ) )->len = too_long_action_length;
  struct ofp_action_enqueue too_long_action_enqueue;
  hton_action_enqueue( &too_long_action_enqueue, ( struct ofp_action_enqueue * ) ( actions->list->data ) );

  assert_int_equal( validate_action_enqueue( &too_long_action_enqueue ), ERROR_TOO_LONG_ACTION_ENQUEUE );

  delete_actions( actions );
}


/********************************************************************************
 * validate_action_vendor() tests.
 ********************************************************************************/

static void
test_validate_action_vendor() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  openflow_actions *actions = create_actions();
  append_action_vendor( actions, VENDOR_ID, body );
  struct ofp_action_vendor_header *action_vendor_header = xmalloc( sizeof( struct ofp_action_vendor_header ) + body->length );
  hton_action_vendor( action_vendor_header, ( struct ofp_action_vendor_header * ) ( actions->list->data ) );

  assert_int_equal( validate_action_vendor( action_vendor_header ), 0 );

  free_buffer( body );
  delete_actions( actions );
  xfree( action_vendor_header );
}


static void
test_validate_action_vendor_fails_with_invalid_action_type() {
  uint16_t port = 1;
  uint32_t queue_id = 10;

  openflow_actions *actions = create_actions();
  append_action_enqueue( actions, port, queue_id );
  struct ofp_action_enqueue action_enqueue;
  hton_action_enqueue( &action_enqueue, ( struct ofp_action_enqueue * ) ( actions->list->data ) );

  assert_int_equal( validate_action_vendor( ( struct ofp_action_vendor_header * ) &action_enqueue ), ERROR_INVALID_ACTION_TYPE );

  delete_actions( actions );
}


static void
test_validate_action_vendor_fails_with_too_short_ofp_action_vendor_header() {
  buffer *body = create_dummy_data( SHORT_DATA_LENGTH );
  openflow_actions *actions = create_actions();
  append_action_vendor( actions, VENDOR_ID, body );
  struct ofp_action_vendor_header *too_short_action_vendor_header = xmalloc( sizeof( struct ofp_action_vendor_header ) + body->length );
  hton_action_vendor( too_short_action_vendor_header, ( struct ofp_action_vendor_header * ) ( actions->list->data ) );
  uint16_t too_short_action_length = sizeof( struct ofp_action_vendor_header ) - 1;
  too_short_action_vendor_header->len = htons( too_short_action_length );

  assert_int_equal( validate_action_vendor( too_short_action_vendor_header ), ERROR_TOO_SHORT_ACTION_VENDOR );

  free_buffer( body );
  delete_actions( actions );
  xfree( too_short_action_vendor_header );
}


static void
test_validate_action_vendor_fails_with_invalid_length_ofp_action_vendor_header() {
  buffer *body = create_dummy_data( LONG_DATA_LENGTH );
  openflow_actions *actions = create_actions();
  append_action_vendor( actions, VENDOR_ID, body );
  struct ofp_action_vendor_header *invalid_length_action_vendor_header = xmalloc( sizeof( struct ofp_action_vendor_header ) + body->length );
  hton_action_vendor( invalid_length_action_vendor_header, ( struct ofp_action_vendor_header * ) ( actions->list->data ) );
  uint16_t invalid_length_action_vendor = ( uint16_t ) ( LONG_DATA_LENGTH - 1 );
  invalid_length_action_vendor_header->len = htons( invalid_length_action_vendor );

  assert_int_equal( validate_action_vendor( invalid_length_action_vendor_header ), ERROR_INVALID_LENGTH_ACTION_VENDOR );

  free_buffer( body );
  delete_actions( actions );
  xfree( invalid_length_action_vendor_header );
}


/********************************************************************************
 * validate_openflow_message() tests.
 ********************************************************************************/

static void
test_validate_openflow_message_succeeds_with_valid_OFPT_HELLO_message() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );

  assert_int_equal( validate_openflow_message( hello ), 0 );

  free_buffer( hello );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_ERROR_message() {
  buffer *dummy_data = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *error = create_error( MY_TRANSACTION_ID, OFPET_HELLO_FAILED, OFPHFC_INCOMPATIBLE, dummy_data );

  assert_int_equal( validate_openflow_message( error ), 0 );

  free_buffer( dummy_data );
  free_buffer( error );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_ECHO_REQUEST_message() {
  buffer *dummy_data = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *echo_request = create_echo_request( MY_TRANSACTION_ID, dummy_data );

  assert_int_equal( validate_openflow_message( echo_request ), 0 );

  free_buffer( dummy_data );
  free_buffer( echo_request );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_ECHO_REPLY_message() {
  buffer *dummy_data = create_dummy_data( SHORT_DATA_LENGTH );
  buffer *echo_reply = create_echo_reply( MY_TRANSACTION_ID, dummy_data );

  assert_int_equal( validate_openflow_message( echo_reply ), 0 );

  free_buffer( dummy_data );
  free_buffer( echo_reply );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_VENDOR_message() {
  buffer *dummy_data = create_dummy_data( 32 );
  buffer *vendor = create_vendor( MY_TRANSACTION_ID, VENDOR_ID, dummy_data );

  assert_int_equal( validate_openflow_message( vendor ), 0 );

  free_buffer( dummy_data );
  free_buffer( vendor );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_FEATURES_REQUEST_message() {
  buffer *features_request = create_features_request( MY_TRANSACTION_ID );

  assert_int_equal( validate_openflow_message( features_request ), 0 );

  free_buffer( features_request );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_FEATURES_REPLY_message() {
  uint64_t datapath_id = 0x12345600;
  uint32_t n_buffers = 128;
  uint8_t n_tables = 1;
  struct ofp_phy_port phy_port[ 2 ];
  list_element *ports;

  uint32_t capabilities = ( OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS | OFPC_QUEUE_STATS | OFPC_ARP_MATCH_IP );
  uint32_t actions = ( ( 1 << OFPAT_OUTPUT ) | ( 1 << OFPAT_SET_VLAN_VID ) | ( 1 << OFPAT_SET_TP_SRC ) | ( 1 << OFPAT_SET_TP_DST ) );
  phy_port[ 0 ].port_no = 1;
  memcpy( phy_port[ 0 ].hw_addr, HW_ADDR, sizeof( phy_port[ 0 ].hw_addr ) );
  strcpy( phy_port[ 0 ].name, "Brown" );
  phy_port[ 0 ].config = OFPPC_PORT_DOWN;
  phy_port[ 0 ].state = OFPPS_LINK_DOWN;
  phy_port[ 0 ].curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port[ 0 ].advertised = PORT_FEATURES;
  phy_port[ 0 ].supported = PORT_FEATURES;
  phy_port[ 0 ].peer = PORT_FEATURES;
  create_list( &ports );
  append_to_tail( &ports, &phy_port[ 0 ] );
  phy_port[ 1 ].port_no = 2;
  memcpy( phy_port[ 1 ].hw_addr, HW_ADDR, sizeof( phy_port[ 1 ].hw_addr ) );
  strcpy( phy_port[ 1 ].name, "Amber" );
  phy_port[ 1 ].config = OFPPC_PORT_DOWN;
  phy_port[ 1 ].state = OFPPS_LINK_DOWN;
  phy_port[ 1 ].curr = OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE;
  phy_port[ 1 ].advertised = PORT_FEATURES;
  phy_port[ 1 ].supported = PORT_FEATURES;
  phy_port[ 1 ].peer = PORT_FEATURES;
  append_to_tail( &ports, &phy_port[ 1 ] );
  buffer *features_reply = create_features_reply( MY_TRANSACTION_ID, datapath_id, n_buffers, n_tables, capabilities, actions, ports );

  assert_int_equal( validate_openflow_message( features_reply ), 0 );

  delete_list( ports );
  free_buffer( features_reply );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_GET_CONFIG_REQUEST_message() {
  buffer *get_config_request = create_get_config_request( MY_TRANSACTION_ID );

  assert_int_equal( validate_openflow_message( get_config_request ), 0 );

  free_buffer( get_config_request );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_GET_CONFIG_REPLY_message() {
  buffer *get_config_reply = create_get_config_reply( MY_TRANSACTION_ID, OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );

  assert_int_equal( validate_openflow_message( get_config_reply ), 0 );

  free_buffer( get_config_reply );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_SET_CONFIG_message() {
  buffer *set_config = create_set_config( MY_TRANSACTION_ID, OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );

  assert_int_equal( validate_openflow_message( set_config ), 0 );

  free_buffer( set_config );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_PACKET_IN_message() {
  uint32_t buffer_id = 0x01020304;
  uint16_t in_port = 1;

  buffer *dummy_data = create_dummy_data( LONG_DATA_LENGTH );
  uint16_t total_len = ( uint16_t ) dummy_data->length;
  buffer *packet_in = create_packet_in( MY_TRANSACTION_ID, buffer_id, total_len, in_port, OFPR_NO_MATCH, dummy_data );

  assert_int_equal( validate_openflow_message( packet_in ), 0 );

  free_buffer( dummy_data );
  free_buffer( packet_in );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_FLOW_REMOVED_message() {
  uint64_t cookie = 0x0102030405060708ULL;
  uint16_t priority = 65535;
  uint32_t duration_sec = 180;
  uint32_t duration_nsec = 10000;
  uint16_t idle_timeout = 60;
  uint64_t packet_count = 1000;
  uint64_t byte_count = 100000;

  buffer *flow_removed = create_flow_removed( MY_TRANSACTION_ID, MATCH, cookie, priority, OFPRR_IDLE_TIMEOUT, duration_sec,
                                              duration_nsec, idle_timeout, packet_count, byte_count );

  assert_int_equal( validate_openflow_message( flow_removed ), 0 );

  free_buffer( flow_removed );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_PORT_STATUS_message() {
  struct ofp_phy_port desc;

  desc.port_no = 1;
  memcpy( desc.hw_addr, HW_ADDR, sizeof( desc.hw_addr ) );
  memset( desc.name, '\0', OFP_MAX_PORT_NAME_LEN );
  strcpy( desc.name, "Navy" );
  desc.config = OFPPC_PORT_DOWN;
  desc.state = OFPPS_LINK_DOWN;
  desc.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  desc.advertised = PORT_FEATURES;
  desc.supported = PORT_FEATURES;
  desc.peer = PORT_FEATURES;
  buffer *port_status = create_port_status( MY_TRANSACTION_ID, OFPPR_ADD, desc );

  assert_int_equal( validate_openflow_message( port_status ), 0 );

  free_buffer( port_status );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_PACKET_OUT_message() {
  openflow_actions *actions = create_actions();
  append_action_output( actions, 1, 128 );
  buffer *dummy_data = create_dummy_data( LONG_DATA_LENGTH );
  buffer *packet_out = create_packet_out( MY_TRANSACTION_ID, BUFFER_ID, 1, actions, dummy_data );

  assert_int_equal( validate_openflow_message( packet_out ), 0 );

  free_buffer( dummy_data );
  free_buffer( packet_out );
  delete_actions( actions );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_FLOW_MOD_message() {
  uint64_t cookie = 10;
  uint16_t hard_timeout = 10;
  uint16_t idle_timeout = 5;
  uint16_t out_port = UINT16_MAX;

  openflow_actions *actions = create_actions();
  append_action_output( actions, 1, 128 );
  buffer *flow_mod = create_flow_mod( MY_TRANSACTION_ID, MATCH, cookie, OFPFC_ADD, idle_timeout, hard_timeout, PRIORITY,
                                      BUFFER_ID, out_port, OFPFF_CHECK_OVERLAP | OFPFF_SEND_FLOW_REM, actions );

  assert_int_equal( validate_openflow_message( flow_mod ), 0 );

  free_buffer( flow_mod );
  delete_actions( actions );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_PORT_MOD_message() {
  uint16_t port_no = 1;
  uint32_t mask = 0;
  uint32_t advertise = 1;

  buffer *port_mod = create_port_mod( MY_TRANSACTION_ID, port_no, HW_ADDR, OFPPC_PORT_DOWN, mask, advertise );

  assert_int_equal( validate_openflow_message( port_mod ), 0 );

  free_buffer( port_mod );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_STATS_REQUEST_message() {
  buffer *stats_desc_request = create_desc_stats_request( MY_TRANSACTION_ID, 0 );

  assert_int_equal( validate_openflow_message( stats_desc_request ), 0 );

  free_buffer( stats_desc_request );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_STATS_REPLY_message() {
  const char mfr_desc[ DESC_STR_LEN ] = "NEC Corporation";
  const char hw_desc[ DESC_STR_LEN ] = "OpenFlow Switch Hardware";
  const char sw_desc[ DESC_STR_LEN ] = "OpenFlow Switch Software";
  const char serial_num[ SERIAL_NUM_LEN ] = "1234";
  const char dp_desc[ DESC_STR_LEN ] = "Datapath 0";

  buffer *stats_desc_reply = create_desc_stats_reply( MY_TRANSACTION_ID, NO_FLAGS, mfr_desc, hw_desc, sw_desc, serial_num, dp_desc );

  assert_int_equal( validate_openflow_message( stats_desc_reply ), 0 );

  free_buffer( stats_desc_reply );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_BARRIER_REQUEST_message() {
  buffer *barrier_request = create_barrier_request( MY_TRANSACTION_ID );

  assert_int_equal( validate_openflow_message( barrier_request ), 0 );

  free_buffer( barrier_request );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_BARRIER_REPLY_message() {
  buffer *barrier_reply = create_barrier_reply( MY_TRANSACTION_ID );

  assert_int_equal( validate_openflow_message( barrier_reply ), 0 );

  free_buffer( barrier_reply );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_QUEUE_GET_CONFIG_REQUEST_message() {
  uint16_t port = 1;

  buffer *queue_get_config_request = create_queue_get_config_request( MY_TRANSACTION_ID, port );

  assert_int_equal( validate_openflow_message( queue_get_config_request ), 0 );

  free_buffer( queue_get_config_request );
}


static void
test_validate_openflow_message_succeeds_with_valid_OFPT_QUEUE_GET_CONFIG_REPLY_message() {
  size_t queue_len;
  uint16_t port = 1;
  list_element *list;
  struct ofp_packet_queue *queue[ 2 ];

  queue_len = offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header );
  queue[ 0 ] = xcalloc( 1, queue_len );
  queue[ 1 ] = xcalloc( 1, queue_len );
  queue[ 0 ]->queue_id = 1;
  queue[ 0 ]->len = 16;
  struct ofp_queue_prop_header *prop_header = queue[ 0 ]->properties;
  prop_header->property = OFPQT_NONE;
  prop_header->len = 8;
  queue[ 1 ]->queue_id = 2;
  queue[ 1 ]->len = 16;
  prop_header = queue[ 1 ]->properties;
  prop_header->property = OFPQT_NONE;
  prop_header->len = 8;
  create_list( &list );
  append_to_tail( &list, queue[ 0 ] );
  append_to_tail( &list, queue[ 1 ] );
  buffer *queue_get_config_reply = create_queue_get_config_reply( MY_TRANSACTION_ID, port, list );

  assert_int_equal( validate_openflow_message( queue_get_config_reply ), 0 );

  xfree( queue[ 0 ] );
  xfree( queue[ 1 ] );
  delete_list( list );
  free_buffer( queue_get_config_reply );
}


static void
test_validate_openflow_message_fails_with_undefined_type_message() {
  uint8_t dummy_type = UINT8_MAX;

  buffer *undefined_type = create_dummy_data( sizeof( struct ofp_stats_request ) );
  struct ofp_stats_request *stats_request = ( struct ofp_stats_request * ) undefined_type->data;
  stats_request->header.type = dummy_type;

  assert_int_equal( validate_openflow_message( undefined_type ), ERROR_UNDEFINED_TYPE );

  free_buffer( undefined_type );
}


static void
test_validate_openflow_message_fails_if_message_is_NULL() {
  expect_assert_failure( validate_openflow_message( NULL ) );
}


static void
test_validate_openflow_message_fails_if_data_is_NULL() {
  buffer *buf = alloc_buffer( );

  expect_assert_failure( validate_openflow_message( buf ) );

  free_buffer( buf );
}


/********************************************************************************
 * valid_openflow_message() tests.
 ********************************************************************************/

static void
test_valid_openflow_message() {
  buffer *hello = create_hello( MY_TRANSACTION_ID );

  assert_int_equal( valid_openflow_message( hello ), true );

  free_buffer( hello );
}


static void
test_valid_openflow_message_fails_with_undefined_type_message() {
  uint8_t dummy_type = UINT8_MAX;

  buffer *undefined_type = create_dummy_data( sizeof( struct ofp_stats_request ) );
  struct ofp_stats_request *stats_request = ( struct ofp_stats_request * ) undefined_type->data;
  stats_request->header.type = dummy_type;

  assert_int_equal( valid_openflow_message( undefined_type ), false );

  free_buffer( undefined_type );
}


/********************************************************************************
 * get_error_type_and_code() tests.
 ********************************************************************************/

static void
test_get_error_type_and_code_succeeds_with_OFPT_ECHO_REQUEST_and_ERROR_UNSUPPORTED_VERSION() {
  uint16_t error_type = 0;
  uint16_t error_code = 0;

  assert_int_equal( get_error_type_and_code( OFPT_ECHO_REQUEST, ERROR_UNSUPPORTED_VERSION, &error_type, &error_code ), true );

  assert_int_equal( error_type, OFPET_BAD_REQUEST );
  assert_int_equal( error_code, OFPBRC_BAD_VERSION );
}


static void
test_get_error_type_and_code_succeeds_with_invalid_type_and_ERROR_UNSUPPORTED_VERSION() {
  uint16_t error_type = 0;
  uint16_t error_code = 0;
  uint8_t dummy_type = 56;

  assert_int_equal( get_error_type_and_code( dummy_type, ERROR_UNSUPPORTED_VERSION, &error_type, &error_code ), true );

  assert_int_equal( error_type, OFPET_BAD_REQUEST );
  assert_int_equal( error_code, OFPBRC_BAD_TYPE );
}


static void
test_get_error_type_and_code_fails_with_OFPT_ECHO_REQUEST_and_ERROR_TOO_SHORT_ACTION_NW_SRC() {
  uint16_t error_type = 0;
  uint16_t error_code = 0;

  assert_int_equal( get_error_type_and_code( OFPT_ECHO_REQUEST, ERROR_TOO_SHORT_ACTION_NW_SRC, &error_type, &error_code ), false );
}


/********************************************************************************
 * set_match_from_packet() tests.
 ********************************************************************************/

const char macda[] = {
    ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, ( char ) 0xff
};
const char macsa[] = {
    ( char ) 0x00, ( char ) 0xd0, ( char ) 0x09, ( char ) 0x20, ( char ) 0x09, ( char ) 0xF7
};
const char snap_data[] = {
    ( char ) 0xaa, ( char ) 0xaa, ( char ) 0x03, ( char ) 0x00, ( char ) 0x00, ( char ) 0x00, ( char ) 0x08, ( char ) 0x00
};

const uint16_t src_port = 1024;
const uint16_t dst_port = 2048;
const size_t ipv4_length = sizeof( ether_header_t ) + sizeof( ipv4_header_t );


static buffer *
setup_ether_packet( size_t length, uint16_t type ) {
  size_t l2_length = sizeof( ether_header_t );
  if ( type == ETH_ETHTYPE_TPID ) {
    length += sizeof( vlantag_header_t );
    l2_length += sizeof( vlantag_header_t );
  }

  /* Create the packet for test. */
  buffer *buf = alloc_buffer_with_length( length );
  append_back_buffer( buf, length );

  ether_header_t *ether = buf->data;
  ether->type = htons( type );
  memcpy( ( char * ) ether->macda, macda, ETH_ADDRLEN );
  memcpy( ( char * ) ether->macsa, macsa, ETH_ADDRLEN );

  vlantag_header_t *vtag = ( vlantag_header_t * ) ( ether + 1 );
  if ( type == ETH_ETHTYPE_TPID ) {
    vtag->tci = htons( 20483 ); // prio(3bit):010,cfi(1bit):1,vid(12bit):000000000011
  }

  /* Create the pakcet_info data for verification. */
  if ( buf->user_data == NULL ) {
    calloc_packet_info( buf );
  }
  packet_info *packet_info0 = buf->user_data;
  memcpy( packet_info0->eth_macda, macda, ETH_ADDRLEN );
  memcpy( packet_info0->eth_macsa, macsa, ETH_ADDRLEN );
  packet_info0->eth_type = type;
  packet_info0->vlan_tci = 20483;
  packet_info0->vlan_tpid = type;
  packet_info0->vlan_prio = 2;
  packet_info0->vlan_cfi = 1;
  packet_info0->vlan_vid = 3;
  packet_info0->format |= ETH_DIX;

  packet_info0->l2_header = buf->data;
  if ( type == ETH_ETHTYPE_TPID ) {
    packet_info0->l3_header = ( void * ) ( vtag + 1 );
    packet_info0->format |= ETH_8021Q;
  }
  else {
    packet_info0->l3_header = ( void * ) ( ether + 1 );
  }

  return buf;
}


static buffer *
setup_ipv4_packet( size_t length, uint16_t type ) {
  buffer *buf = setup_ether_packet( length, type );
  packet_info *packet_info0 = buf->user_data;

  /* Fill arp values into the test packet. */
  if ( type == ETH_ETHTYPE_TPID ) {
    ether_header_t *ether = packet_info0->l2_header;
    vlantag_header_t *vtag = ( vlantag_header_t * ) ( ether + 1 );
    vtag->type = htons( ETH_ETHTYPE_IPV4 );
  }

  ipv4_header_t *ipv4 = packet_info0->l3_header;
  ipv4->version = IPVERSION;
  ipv4->ihl = sizeof( ipv4_header_t ) / 4;
  ipv4->tos = 0;
  ipv4->tot_len = htons( sizeof( ipv4_header_t ) );
  ipv4->ttl = 0;
  ipv4->csum = 0;
  ipv4->saddr = htonl( 0xC0A80067 );
  ipv4->daddr = htonl( 0xC0A80036 );
  ipv4->frag_off = htons( 0 );
  packet_info0->l4_header = ( void * ) ( ipv4 + 1 );

  switch ( type ) {
  case IPPROTO_ICMP:
    ipv4->protocol = ( uint8_t ) IPPROTO_ICMP;
    icmp_header_t *icmpv4 = packet_info0->l4_header;
    icmpv4->type = ICMP_TYPE_UNREACH;
    icmpv4->code = ICMP_CODE_PORTUNREACH;
    break;

  case IPPROTO_UDP:
    ipv4->protocol = ( uint8_t ) IPPROTO_UDP;
    udp_header_t *udp = packet_info0->l4_header;
    udp->src_port = ntohs( src_port );
    udp->dst_port = ntohs( dst_port );
    break;

  case IPPROTO_TCP:
    ipv4->protocol = ( uint8_t ) IPPROTO_TCP;
    tcp_header_t *tcp = packet_info0->l4_header;
    tcp->src_port = ntohs( src_port );
    tcp->dst_port = ntohs( dst_port );
    break;

  default:
    break;
  }

  /* Fill ipv4/icmp/udp values into the packet_info data for verification. */
  packet_info0->eth_type = ETH_ETHTYPE_IPV4;
  packet_info0->ipv4_version = IPVERSION;
  packet_info0->ipv4_ihl = sizeof( ipv4_header_t ) / 4;
  packet_info0->ipv4_tos = 0;
  packet_info0->ipv4_tot_len = sizeof( ipv4_header_t );
  packet_info0->ipv4_ttl = 0;
  packet_info0->ipv4_checksum = 0;
  packet_info0->ipv4_saddr = 0xC0A80067;
  packet_info0->ipv4_daddr = 0xC0A80036;
  packet_info0->ipv4_frag_off = 0;
  packet_info0->ipv4_protocol = ( uint8_t ) type;
  packet_info0->format |= NW_IPV4;

  switch ( type ) {
  case IPPROTO_ICMP:
    packet_info0->icmpv4_type = ICMP_TYPE_UNREACH;
    packet_info0->icmpv4_code = ICMP_CODE_PORTUNREACH;
    packet_info0->format |= NW_ICMPV4;
    break;

  case IPPROTO_UDP:
    packet_info0->udp_src_port = src_port;
    packet_info0->udp_dst_port = dst_port;
    packet_info0->format |= TP_UDP;
    break;

  case IPPROTO_TCP:
    packet_info0->tcp_src_port = src_port;
    packet_info0->tcp_dst_port = dst_port;
    packet_info0->format |= TP_TCP;
    break;

  default:
    break;
  }

  return buf;
}


static buffer *
setup_arp_packet( uint16_t type ) {
  buffer *buf = setup_ether_packet( sizeof( ether_header_t ) + sizeof( arp_header_t ), type );
  packet_info *packet_info0 = buf->user_data;

  /* Fill arp values into the test packet. */
  if ( type == ETH_ETHTYPE_TPID ) {
    ether_header_t *ether = packet_info0->l2_header;
    vlantag_header_t *vtag = ( vlantag_header_t * ) ( ether + 1 );
    vtag->type = htons( ETH_ETHTYPE_ARP );
  }
  arp_header_t *arp = packet_info0->l3_header;
  arp->ar_hrd = htons( ARPHRD_ETHER );
  arp->ar_pro = htons( ETH_ETHTYPE_IPV4 );
  arp->ar_hln = ETH_ADDRLEN;
  arp->ar_pln = IPV4_ADDRLEN;
  arp->ar_op = htons( ARPOP_REPLY );
  arp->sha[ 0 ] = 0x00;
  arp->sha[ 1 ] = 0x01;
  arp->sha[ 2 ] = 0x02;
  arp->sha[ 3 ] = 0x03;
  arp->sha[ 4 ] = 0x04;
  arp->sha[ 5 ] = 0x05;
  arp->sip = htonl( 0x01020304 );
  arp->tha[ 0 ] = 0x06;
  arp->tha[ 1 ] = 0x07;
  arp->tha[ 2 ] = 0x08;
  arp->tha[ 3 ] = 0x09;
  arp->tha[ 4 ] = 0x0a;
  arp->tha[ 5 ] = 0x0b;
  arp->tip = htonl( 0x05060708 );

  /* Fill arp values into the packet_info data for verification. */
  packet_info0->eth_type = ETH_ETHTYPE_ARP;
  packet_info0->arp_ar_hrd = ARPHRD_ETHER;
  packet_info0->arp_ar_pro = ETH_ETHTYPE_IPV4;
  packet_info0->arp_ar_hln = ETH_ADDRLEN;
  packet_info0->arp_ar_pln = IPV4_ADDRLEN;
  packet_info0->arp_ar_op = ARPOP_REPLY;
  packet_info0->arp_sha[ 0 ] = 0x00;
  packet_info0->arp_sha[ 1 ] = 0x01;
  packet_info0->arp_sha[ 2 ] = 0x02;
  packet_info0->arp_sha[ 3 ] = 0x03;
  packet_info0->arp_sha[ 4 ] = 0x04;
  packet_info0->arp_sha[ 5 ] = 0x05;
  packet_info0->arp_spa = 0x01020304;
  packet_info0->arp_tha[ 0 ] = 0x06;
  packet_info0->arp_tha[ 1 ] = 0x07;
  packet_info0->arp_tha[ 2 ] = 0x08;
  packet_info0->arp_tha[ 3 ] = 0x09;
  packet_info0->arp_tha[ 4 ] = 0x0a;
  packet_info0->arp_tha[ 5 ] = 0x0b;
  packet_info0->arp_tpa = 0x05060708;
  packet_info0->format |= NW_ARP;

  return buf;
}


static buffer *
setup_snap_packet( uint16_t type ) {
  buffer *buf = setup_ether_packet( sizeof( ether_header_t ) + sizeof( snap_header_t ), type );
  packet_info *packet_info0 = buf->user_data;

  size_t length = sizeof( ether_header_t ) + sizeof( snap_header_t );
  ether_header_t *ether = packet_info0->l2_header;
  if ( type == ETH_ETHTYPE_TPID ) {
    length += sizeof( vlantag_header_t );
    vlantag_header_t *vtag = ( void * ) ( ether + 1 );
    vtag->type = htons( ( uint16_t ) length );
  }
  else {
    ether->type = htons( ( uint16_t ) length );
  }

  snap_header_t *snap = ( snap_header_t * ) packet_info0->l3_header;
  memcpy( ( char * ) snap, snap_data, sizeof( snap_header_t ) );

  return buf;
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_tag_and_wildcards_is_zero() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_TPID );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_zero() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_IN_PORT() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_IN_PORT, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_IN_PORT );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_VLAN() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_DL_VLAN );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_SRC() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_DL_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_DST() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_DL_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_TYPE() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_TYPE, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_DL_TYPE );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_PROTO() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_PROTO, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_NW_PROTO );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_TP_SRC() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_TP_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_TP_DST() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_TP_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_SRC_ALL() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_SRC_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_NW_SRC_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  uint32_t ip_source_address_flag = ntohl( arp->sip ) & ( OFPFW_NW_SRC_ALL >> OFPFW_NW_SRC_SHIFT );
  assert_int_equal( ( int ) match.nw_src, ( int ) ip_source_address_flag );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_DST_ALL() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_DST_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_NW_DST_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  uint32_t ip_destination_address_flag = ntohl( arp->tip ) & ( OFPFW_NW_DST_ALL >> OFPFW_NW_DST_SHIFT );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ip_destination_address_flag );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_VLAN_PCP() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN_PCP, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_DL_VLAN_PCP );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_TOS() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_TOS, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  arp_header_t *arp = packet_info0->l3_header;
  assert_int_equal( ( int ) match.wildcards, OFPFW_NW_TOS );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, ntohs( arp->ar_op ) & ARP_OP_MASK );
  assert_int_equal( ( int ) match.nw_src, ( int ) ntohl( arp->sip ) );
  assert_int_equal( ( int ) match.nw_dst, ( int ) ntohl( arp->tip ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_ALL() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_ARP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_ALL, buf );

  assert_int_equal( ( int ) match.wildcards, OFPFW_ALL );
  assert_int_equal( match.in_port, 0 );
  assert_int_equal( match.dl_vlan, 0 );
  assert_int_equal( match.dl_type, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_int_equal( ( int ) match.nw_src, 0 );
  assert_int_equal( ( int ) match.nw_dst, 0 );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_tag_and_wildcards_is_zero() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), ETH_ETHTYPE_TPID );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  assert_int_equal( match.in_port, expected_in_port );
  assert_int_equal( ( int ) match.wildcards, 0 );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_zero() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_IN_PORT() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_IN_PORT, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_IN_PORT );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_VLAN() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_VLAN );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_SRC() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_DST() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_TYPE() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_TYPE, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, OFPFW_DL_TYPE );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_PROTO() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_PROTO, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  assert_int_equal( match.wildcards, OFPFW_NW_PROTO );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_TP_SRC() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_TP_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_TP_DST() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_TP_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_SRC_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_SRC_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_SRC_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, 0 );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_DST_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_DST_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_DST_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, 0 );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_VLAN_PCP() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN_PCP, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_VLAN_PCP );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_TOS() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_TOS, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  udp_header_t *udp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_TOS );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( udp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( udp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( udp_header_t ), IPPROTO_UDP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_ALL, buf );

  assert_int_equal( match.wildcards, OFPFW_ALL );
  assert_int_equal( match.in_port, 0 );
  assert_int_equal( match.dl_type, 0 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_int_equal( match.nw_src, 0 );
  assert_int_equal( match.nw_dst, 0 );
  assert_int_equal( match.tp_src, 0 );
  assert_int_equal( match.tp_dst, 0 );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_tag_and_wildcards_is_zero() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), ETH_ETHTYPE_TPID );
  packet_info *packet_info0 = buf->user_data;

  /* Add a tcp data into the test packet */
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  ipv4->protocol = IPPROTO_TCP;
  tcp_header_t *tcp = packet_info0->l4_header;
  tcp->src_port = ntohs( src_port );
  tcp->dst_port = ntohs( dst_port );

  /* Add the tcp data into the packet_info data for verification. */
  packet_info0->ipv4_protocol = IPPROTO_TCP;
  packet_info0->tcp_src_port = src_port;
  packet_info0->tcp_dst_port = dst_port;
  packet_info0->format |= TP_TCP;

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_zero() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_IN_PORT() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_IN_PORT, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_IN_PORT );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_VLAN() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_VLAN );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_SRC() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_DST() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_TYPE() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_TYPE, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, OFPFW_DL_TYPE );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_PROTO() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_PROTO, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  assert_int_equal( match.wildcards, OFPFW_NW_PROTO );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_TP_SRC() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_TP_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_TP_DST() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_TP_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_SRC_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_SRC_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_SRC_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, 0 );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_DST_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_DST_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_DST_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, 0 );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_VLAN_PCP() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN_PCP, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_VLAN_PCP );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_TOS() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_TOS, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  tcp_header_t *tcp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_TOS );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.tp_src, ntohs( tcp->src_port ) );
  assert_int_equal( match.tp_dst, ntohs( tcp->dst_port ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( tcp_header_t ), IPPROTO_TCP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_ALL, buf );

  assert_int_equal( match.wildcards, OFPFW_ALL );
  assert_int_equal( match.in_port, 0 );
  assert_int_equal( match.dl_type, 0 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_int_equal( match.nw_src, 0 );
  assert_int_equal( match.nw_dst, 0 );
  assert_int_equal( match.tp_src, 0 );
  assert_int_equal( match.tp_dst, 0 );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_tag_and_wildcards_is_zero() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), ETH_ETHTYPE_TPID );
  packet_info *packet_info0 = buf->user_data;

  /* Add a icmp data into the test packet. */
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  ipv4->protocol = IPPROTO_ICMP;
  icmp_header_t *icmpv4 = packet_info0->l4_header;
  icmpv4->type = ICMP_TYPE_UNREACH;
  icmpv4->code = ICMP_CODE_PORTUNREACH;
  /* Add the icmp data into the packet_info data for verification. */
  packet_info0->ipv4_protocol = IPPROTO_ICMP;
  packet_info0->icmpv4_type = ICMP_TYPE_UNREACH;
  packet_info0->icmpv4_code = ICMP_CODE_PORTUNREACH;
  packet_info0->format |= NW_ICMPV4;

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_zero() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_IN_PORT() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_IN_PORT, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_IN_PORT );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_VLAN() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_VLAN );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_SRC() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_DST() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_TYPE() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_TYPE, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, OFPFW_DL_TYPE );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_PROTO() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_PROTO, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  assert_int_equal( match.wildcards, OFPFW_NW_PROTO );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_TP_SRC() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_SRC, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_TP_SRC );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_TP_DST() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_TP_DST, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_TP_DST );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_SRC_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_SRC_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_SRC_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, 0 );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_DST_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_DST_ALL, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_DST_ALL );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, 0 );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_VLAN_PCP() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_DL_VLAN_PCP, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_DL_VLAN_PCP );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_tos, ipv4->tos );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_TOS() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_NW_TOS, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  ipv4_header_t *ipv4 = packet_info0->l3_header;
  icmp_header_t *icmp = packet_info0->l4_header;
  assert_int_equal( match.wildcards, OFPFW_NW_TOS );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );
  assert_int_equal( match.nw_proto, packet_info0->ipv4_protocol );
  assert_int_equal( match.nw_src, ntohl( ipv4->saddr ) );
  assert_int_equal( match.nw_dst, ntohl( ipv4->daddr ) );
  assert_int_equal( match.icmp_type, icmp->type );
  assert_int_equal( match.icmp_code, icmp->code );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_ALL() {
  buffer *buf = setup_ipv4_packet( ipv4_length + sizeof( icmp_header_t ), IPPROTO_ICMP );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, OFPFW_ALL, buf );

  assert_int_equal( match.wildcards, OFPFW_ALL );
  assert_int_equal( match.in_port, 0 );
  assert_int_equal( match.dl_type, 0 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_int_equal( match.nw_src, 0 );
  assert_int_equal( match.nw_dst, 0 );
  assert_int_equal( match.tp_src, 0 );
  assert_int_equal( match.tp_dst, 0 );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_snap_tag_and_wildcards_is_zero() {
  buffer *buf = setup_snap_packet( ETH_ETHTYPE_TPID );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_netbios_tag_and_wildcards_is_zero() {
  buffer *buf = setup_snap_packet( ETH_ETHTYPE_TPID );
  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  vlantag_header_t *vtag = ( vlantag_header_t * ) ( ether + 1 );

  snap_header_t *snap = ( snap_header_t * ) ( vtag + 1 );
  snap->llc[ 0 ] = 0xF0;
  snap->llc[ 1 ] = 0xF0;

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_not_llc_tag_and_wildcards_is_zero() {
  buffer *buf = setup_snap_packet( ETH_ETHTYPE_TPID );
  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  vlantag_header_t *vtag = ( vlantag_header_t * ) ( ether + 1 );

  snap_header_t *snap = ( snap_header_t * ) ( vtag + 1 );
  snap->llc[ 0 ] = 0xFF;
  snap->llc[ 1 ] = 0xFF;

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, packet_info0->vlan_vid );
  assert_int_equal( match.dl_vlan_pcp, packet_info0->vlan_prio );
  assert_int_equal( match.dl_type, packet_info0->eth_type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_snap_and_wildcards_is_zero() {
  buffer *buf = setup_snap_packet( 0 );

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;
  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_netbios_and_wildcards_is_zero() {
  buffer *buf = setup_snap_packet( 0 );
  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;

  snap_header_t *snap = ( snap_header_t * ) ( ether + 1 );
  snap->llc[ 0 ] = 0xF0;
  snap->llc[ 1 ] = 0xF0;

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_not_llc_and_wildcards_is_zero() {
  buffer *buf = setup_snap_packet( 1 );
  packet_info *packet_info0 = buf->user_data;
  ether_header_t *ether = packet_info0->l2_header;

  snap_header_t *snap = ( snap_header_t * ) ( ether + 1 );
  snap->llc[ 0 ] = 0x00;
  snap->llc[ 1 ] = 0x00;

  uint16_t expected_in_port = 1;
  struct ofp_match match;
  set_match_from_packet( &match, expected_in_port, 0, buf );

  assert_int_equal( match.wildcards, 0 );
  assert_int_equal( match.in_port, expected_in_port );
  assert_memory_equal( match.dl_src, ether->macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, ether->macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_type, packet_info0->eth_type );

  free_buffer( buf );
}


static void
test_set_match_from_packet_fails_if_match_is_NULL() {
  buffer *buf = setup_arp_packet( ETH_ETHTYPE_TPID );

  expect_assert_failure( set_match_from_packet( NULL, 1, 0, buf ) );

  free_buffer( buf );
}


static void
test_set_match_from_packet_fails_if_packet_data_is_NULL() {
  struct ofp_match match;
  expect_assert_failure( set_match_from_packet( &match, 1, 0, NULL ) );
}


static void
test_set_match_from_packet_fails_if_packet_is_not_parsed_yet() {
  struct ofp_match match;
  buffer *buf = alloc_buffer( );

  expect_assert_failure( set_match_from_packet( &match, 1, 0, buf ) );

  free_buffer( buf );
}


/********************************************************************************
 * normalize_match() tests.
 ********************************************************************************/

#define NORMALIZED_OFPFW_ALL \
  ( OFPFW_IN_PORT | OFPFW_DL_VLAN | OFPFW_DL_SRC | OFPFW_DL_DST | OFPFW_DL_TYPE | \
    OFPFW_NW_PROTO | OFPFW_TP_SRC | OFPFW_TP_DST | OFPFW_NW_SRC_ALL | OFPFW_NW_DST_ALL | \
    OFPFW_DL_VLAN_PCP | OFPFW_NW_TOS )


const char all_zero_mac_addr[] = { 0, 0, 0, 0, 0, 0 };
const char all_one_mac_addr[] = { ( char ) 0xff, ( char ) 0xff, ( char ) 0xff,
                                  ( char ) 0xff, ( char ) 0xff, ( char ) 0xff };
const char all_zero_pad1[] = { 0 };
const char all_zero_pad2[] = { 0, 0 };


static void
setup_arp_match( struct ofp_match *match ) {
  memset( match, 0xa8, sizeof( match ) );
  match->wildcards = 0;
  match->in_port = 1;
  memcpy( match->dl_src, macsa, ETH_ADDRLEN );
  memcpy( match->dl_dst, macda, ETH_ADDRLEN );
  match->dl_vlan = UINT16_MAX;
  match->dl_type = ETH_ETHTYPE_ARP;
  match->nw_proto = ARP_OP_REQUEST;
  match->nw_src = 0x01020304;
  match->nw_dst = 0x05060708;
}


static void
setup_icmp_match( struct ofp_match *match ) {
  memset( match, 0xa8, sizeof( match ) );
  match->wildcards = 0;
  match->in_port = 1;
  memcpy( match->dl_src, macsa, ETH_ADDRLEN );
  memcpy( match->dl_dst, macda, ETH_ADDRLEN );
  match->dl_vlan = UINT16_MAX;
  match->dl_type = ETH_ETHTYPE_IPV4;
  match->nw_tos = 0;
  match->nw_proto = IPPROTO_ICMP;
  match->nw_src = 0x01020304;
  match->nw_dst = 0x05060708;
  match->tp_src = ICMP_TYPE_UNREACH;
  match->tp_dst = ICMP_CODE_PORTUNREACH;
}


static void
setup_tcp_match( struct ofp_match *match ) {
  memset( match, 0xa8, sizeof( match ) );
  match->wildcards = 0;
  match->in_port = 1;
  memcpy( match->dl_src, macsa, ETH_ADDRLEN );
  memcpy( match->dl_dst, macda, ETH_ADDRLEN );
  match->dl_vlan = 1;
  match->dl_vlan_pcp = 0xff;
  match->dl_type = ETH_ETHTYPE_IPV4;
  match->nw_tos = 0xff;
  match->nw_proto = IPPROTO_TCP;
  match->nw_src = 0x01020304;
  match->nw_dst = 0x05060708;
  match->tp_src = 0xa;
  match->tp_dst = 0xb;
}


static void
setup_udp_match( struct ofp_match *match ) {
  memset( match, 0xa8, sizeof( match ) );
  match->wildcards = 0;
  match->in_port = 1;
  memcpy( match->dl_src, macsa, ETH_ADDRLEN );
  memcpy( match->dl_dst, macda, ETH_ADDRLEN );
  match->dl_vlan = UINT16_MAX;
  match->dl_vlan_pcp = 0xff;
  match->dl_type = ETH_ETHTYPE_IPV4;
  match->nw_tos = 0xff;
  match->nw_proto = IPPROTO_UDP;
  match->nw_src = 0x01020304;
  match->nw_dst = 0x05060708;
  match->tp_src = 0xa;
  match->tp_dst = 0xb;
}


static void
setup_invalid_match( struct ofp_match *match ) {
  memset( match, 0xa8, sizeof( match ) );
  match->wildcards = 0;
  match->in_port = 1;
  memcpy( match->dl_src, macsa, ETH_ADDRLEN );
  memcpy( match->dl_dst, macda, ETH_ADDRLEN );
  match->dl_vlan = UINT16_MAX;
  match->dl_vlan_pcp = 0xff;
  match->dl_type = UINT16_MAX;
}


static void
test_normalize_match_succeeds_if_datatype_is_arp_and_wildcards_is_zero() {
  struct ofp_match match;
  setup_arp_match( &match );

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_VLAN_PCP | OFPFW_NW_TOS | OFPFW_TP_SRC | OFPFW_TP_DST;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_vlan_pcp, 0 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_ARP );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, ARP_OP_REQUEST );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0 );
  assert_int_equal( ( int ) match.tp_dst, 0 );
}


static void
test_normalize_match_succeeds_if_protocol_is_icmp_and_wildcards_is_zero() {
  struct ofp_match match;
  setup_icmp_match( &match );

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_VLAN_PCP;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_vlan_pcp, 0 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, IPPROTO_ICMP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, ICMP_TYPE_UNREACH );
  assert_int_equal( ( int ) match.tp_dst, ICMP_CODE_PORTUNREACH );
}


static void
test_normalize_match_succeeds_if_protocol_is_tcp_and_wildcards_is_zero() {
  struct ofp_match match;
  setup_tcp_match( &match );

  normalize_match( &match );

  uint32_t wildcards = 0;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_protocol_is_udp_and_wildcards_is_zero() {
  struct ofp_match match;
  setup_udp_match( &match );

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_VLAN_PCP;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_vlan_pcp, 0 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_UDP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_packet_is_invalid_and_wildcards_is_zero() {
  struct ofp_match match;
  setup_invalid_match( &match );

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_VLAN_PCP | OFPFW_NW_PROTO | OFPFW_NW_TOS | OFPFW_NW_SRC_ALL | OFPFW_NW_DST_ALL | OFPFW_TP_SRC | OFPFW_TP_DST;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, UINT16_MAX );
  assert_int_equal( match.dl_vlan_pcp, 0 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, UINT16_MAX );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0 );
  assert_int_equal( ( int ) match.nw_dst, 0 );
  assert_int_equal( ( int ) match.tp_src, 0 );
  assert_int_equal( ( int ) match.tp_dst, 0 );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_ALL() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_ALL;

  normalize_match( &match );

  assert_int_equal( ( int ) match.wildcards, NORMALIZED_OFPFW_ALL );
  assert_int_equal( match.in_port, 0 );
  assert_memory_equal( match.dl_src, all_zero_mac_addr, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, all_zero_mac_addr, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 0 );
  assert_int_equal( match.dl_vlan_pcp, 0 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, 0 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0 );
  assert_int_equal( ( int ) match.nw_dst, 0 );
  assert_int_equal( ( int ) match.tp_src, 0 );
  assert_int_equal( ( int ) match.tp_dst, 0 );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_IN_PORT() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_IN_PORT;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_IN_PORT;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 0 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_VLAN() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_DL_VLAN;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_VLAN;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 0 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_SRC() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_DL_SRC;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_SRC;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, all_zero_mac_addr, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_DST() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_DL_DST;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_DST;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, all_zero_mac_addr, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_TYPE() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_DL_TYPE;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_TYPE | OFPFW_NW_PROTO | OFPFW_NW_TOS | OFPFW_NW_SRC_ALL | OFPFW_NW_DST_ALL | OFPFW_TP_SRC | OFPFW_TP_DST;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, 0 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, 0 );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0 );
  assert_int_equal( ( int ) match.nw_dst, 0 );
  assert_int_equal( ( int ) match.tp_src, 0 );
  assert_int_equal( ( int ) match.tp_dst, 0 );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_PROTO() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_NW_PROTO;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_NW_PROTO;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, 0 );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_TP_SRC() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_TP_SRC;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_TP_SRC;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0 );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_TP_DST() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_TP_DST;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_TP_DST;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0 );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_SRC_ALL() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_NW_SRC_ALL;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_NW_SRC_ALL;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_DST_ALL() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_NW_DST_ALL;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_NW_DST_ALL;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_src_mask_len_is_24() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = 24 << OFPFW_NW_SRC_SHIFT;

  normalize_match( &match );

  uint32_t wildcards = 24 << OFPFW_NW_SRC_SHIFT;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01000000 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_dst_mask_len_is_24() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = 24 << OFPFW_NW_DST_SHIFT;

  normalize_match( &match );

  uint32_t wildcards = 24 << OFPFW_NW_DST_SHIFT;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05000000 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_VLAN_PCP() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_DL_VLAN_PCP;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_DL_VLAN_PCP;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0xfc );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_TOS() {
  struct ofp_match match;
  setup_tcp_match( &match );
  match.wildcards = OFPFW_NW_TOS;

  normalize_match( &match );

  uint32_t wildcards = OFPFW_NW_TOS;

  assert_int_equal( ( int ) match.wildcards, wildcards );
  assert_int_equal( match.in_port, 1 );
  assert_memory_equal( match.dl_src, macsa, ETH_ADDRLEN );
  assert_memory_equal( match.dl_dst, macda, ETH_ADDRLEN );
  assert_int_equal( match.dl_vlan, 1 );
  assert_int_equal( match.dl_vlan_pcp, 0x7 );
  assert_memory_equal( match.pad1, all_zero_pad1, sizeof( match.pad1 ) );
  assert_int_equal( match.dl_type, ETH_ETHTYPE_IPV4 );
  assert_int_equal( match.nw_tos, 0 );
  assert_int_equal( match.nw_proto, IPPROTO_TCP );
  assert_memory_equal( match.pad2, all_zero_pad2, sizeof( match.pad2 ) );
  assert_int_equal( ( int ) match.nw_src, 0x01020304 );
  assert_int_equal( ( int ) match.nw_dst, 0x05060708 );
  assert_int_equal( ( int ) match.tp_src, 0xa );
  assert_int_equal( ( int ) match.tp_dst, 0xb );
}


static void
test_normalize_match_fails_if_match_is_NULL() {
  expect_assert_failure( normalize_match( NULL ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_init_openflow_message ),

    unit_test_setup_teardown( test_get_transaction_id, init, teardown ),
    unit_test_setup_teardown( test_get_transaction_id_if_id_overflows, init, teardown ),
    unit_test_setup_teardown( test_get_cookie, init, teardown ),
    unit_test_setup_teardown( test_get_cookie_if_cookie_overflows, init, teardown ),

    unit_test_setup_teardown( test_create_hello, init, teardown ),
    unit_test_setup_teardown( test_validate_hello, init, teardown ),
    unit_test_setup_teardown( test_validate_hello_fails_with_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_hello_fails_with_non_hello_message, init, teardown ),

    unit_test_setup_teardown( test_create_error, init, teardown ),
    unit_test_setup_teardown( test_create_error_without_data, init, teardown ),

    unit_test_setup_teardown( test_create_echo_request, init, teardown ),
    unit_test_setup_teardown( test_create_echo_request_without_data, init, teardown ),
    unit_test_setup_teardown( test_validate_echo_request, init, teardown ),
    unit_test_setup_teardown( test_validate_echo_request_fails_with_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_echo_request_fails_with_non_echo_request_message, init, teardown ),

    unit_test_setup_teardown( test_create_echo_reply, init, teardown ),
    unit_test_setup_teardown( test_create_echo_reply_without_data, init, teardown ),
    unit_test_setup_teardown( test_validate_echo_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_echo_reply_fails_with_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_echo_reply_fails_with_non_echo_reply_message, init, teardown ),

    unit_test_setup_teardown( test_create_vendor, init, teardown ),
    unit_test_setup_teardown( test_create_vendor_without_data, init, teardown ),
    unit_test_setup_teardown( test_create_features_request, init, teardown ),
    unit_test_setup_teardown( test_create_get_config_request, init, teardown ),
    unit_test_setup_teardown( test_create_get_config_reply, init, teardown ),
    unit_test_setup_teardown( test_create_set_config, init, teardown ),
    unit_test_setup_teardown( test_create_flow_removed, init, teardown ),
    unit_test_setup_teardown( test_create_port_status, init, teardown ),
    unit_test_setup_teardown( test_create_port_mod, init, teardown ),
    unit_test_setup_teardown( test_create_and_delete_actions, init, teardown ),

    unit_test_setup_teardown( test_append_action_output, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_vlan_vid, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_vlan_pcp, init, teardown ),
    unit_test_setup_teardown( test_append_action_strip_vlan, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_dl_src, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_dl_dst, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_nw_src, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_nw_dst, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_nw_tos, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_tp_src, init, teardown ),
    unit_test_setup_teardown( test_append_action_set_tp_dst, init, teardown ),
    unit_test_setup_teardown( test_append_action_enqueue, init, teardown ),
    unit_test_setup_teardown( test_append_action_vendor, init, teardown ),
    unit_test_setup_teardown( test_append_action_vendor_without_data, init, teardown ),

    unit_test_setup_teardown( test_create_packet_out, init, teardown ),
    unit_test_setup_teardown( test_create_packet_out_without_actions, init, teardown ),
    unit_test_setup_teardown( test_create_flow_mod, init, teardown ),
    unit_test_setup_teardown( test_create_flow_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_flow_stats_reply, init, teardown ),

    unit_test_setup_teardown( test_validate_aggregate_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_aggregate_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_aggregate_stats_request_fails_if_message_is_not_aggregate_stats_request, init, teardown ),

    unit_test_setup_teardown( test_create_aggregate_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_aggregate_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_create_port_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_port_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_create_queue_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_queue_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_create_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_create_desc_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_desc_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_create_table_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_table_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_create_barrier_request, init, teardown ),
    unit_test_setup_teardown( test_create_barrier_reply, init, teardown ),
    unit_test_setup_teardown( test_create_queue_get_config_request, init, teardown ),
    unit_test_setup_teardown( test_create_queue_get_config_reply, init, teardown ),
    unit_test_setup_teardown( test_create_vendor_stats_request, init, teardown ),
    unit_test_setup_teardown( test_create_vendor_stats_request_without_data, init, teardown ),
    unit_test_setup_teardown( test_create_vendor_stats_reply, init, teardown ),

    unit_test_setup_teardown( test_validate_error, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_without_data, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_fails_if_message_is_not_vendor_header, init, teardown ),

    unit_test_setup_teardown( test_validate_features_request, init, teardown ),
    unit_test_setup_teardown( test_validate_features_request_fails_with_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_features_request_fails_with_non_features_request_message, init, teardown ),
    unit_test_setup_teardown( test_validate_features_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_get_config_request, init, teardown ),
    unit_test_setup_teardown( test_validate_get_config_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_get_config_request_fails_if_message_is_not_get_config_request, init, teardown ),
    unit_test_setup_teardown( test_validate_get_config_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_set_config, init, teardown ),
    unit_test_setup_teardown( test_validate_set_config_fails_with_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_set_config_fails_with_non_set_config_message, init, teardown ),
    unit_test_setup_teardown( test_validate_packet_in, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_removed, init, teardown ),
    unit_test_setup_teardown( test_validate_port_status, init, teardown ),
    unit_test_setup_teardown( test_validate_packet_out, init, teardown ),
    unit_test_setup_teardown( test_validate_packet_out_without_data, init, teardown ),
    unit_test_setup_teardown( test_validate_packet_out_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_packet_out_fails_if_message_is_not_packet_out, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_mod, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_mod_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_mod_fails_if_message_is_not_flow_mod, init, teardown ),
    unit_test_setup_teardown( test_validate_port_mod, init, teardown ),
    unit_test_setup_teardown( test_validate_port_mod_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_port_mod_fails_if_message_is_not_port_mod, init, teardown ),
    unit_test_setup_teardown( test_validate_desc_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_desc_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_desc_stats_request_fails_if_message_is_not_desc_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_desc_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_stats_request_fails_if_message_is_not_flow_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_flow_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_aggregate_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_table_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_table_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_table_stats_request_fails_if_message_is_not_table_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_table_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_port_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_port_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_port_stats_request_fails_if_message_is_not_port_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_port_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_queue_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_queue_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_queue_stats_request_fails_if_message_is_not_queue_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_queue_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_stats_request_fails_if_message_is_not_vendor_stats_request, init, teardown ),
    unit_test_setup_teardown( test_validate_vendor_stats_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_DESC_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_FLOW_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_AGGREGATE_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_TABLE_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_PORT_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_QUEUE_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_sucseed_with_OFPST_VENDOR_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_request_fails_with_unsupported_stats_type, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_DESC_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_FLOW_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_AGGREGATE_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_TABLE_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_PORT_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_QUEUE_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_with_OFPST_VENDOR_message, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_stats_reply_fails_with_unsupported_stats_type, init, teardown ),
    unit_test_setup_teardown( test_validate_barrier_reply, init, teardown ),
    unit_test_setup_teardown( test_validate_queue_get_config_reply, init, teardown ),

    unit_test_setup_teardown( test_validate_action_output, init, teardown ),
    unit_test_setup_teardown( test_validate_action_output_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_output_fails_with_too_short_ofp_action_output, init, teardown ),
    unit_test_setup_teardown( test_validate_action_output_fails_with_too_long_ofp_action_output, init, teardown ),
    unit_test_setup_teardown( test_validate_action_output_fails_with_invalid_port_no, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_vid, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_vid_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_vid_fails_with_invalid_vlan_vid, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_vid_fails_with_too_short_ofp_action_vlan_vid, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_vid_fails_with_too_long_ofp_action_vlan_vid, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_pcp, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_pcp_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_pcp_fails_with_invalid_vlan_pcp, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_pcp_fails_with_too_short_ofp_action_vlan_pcp, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_vlan_pcp_fails_with_too_long_ofp_action_vlan_pcp, init, teardown ),
    unit_test_setup_teardown( test_validate_action_strip_vlan, init, teardown ),
    unit_test_setup_teardown( test_validate_action_strip_vlan_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_strip_vlan_fails_with_too_short_ofp_action_header, init, teardown ),
    unit_test_setup_teardown( test_validate_action_strip_vlan_fails_with_too_long_ofp_action_header, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_src, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_src_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_src_fails_with_too_short_ofp_action_dl_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_src_fails_with_too_long_ofp_action_dl_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_dst, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_dst_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_dst_fails_with_too_short_ofp_action_dl_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_dl_dst_fails_with_too_long_ofp_action_dl_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_src, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_src_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_src_fails_with_too_short_ofp_action_nw_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_src_fails_with_too_long_ofp_action_nw_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_dst, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_dst_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_dst_fails_with_too_short_ofp_action_nw_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_dst_fails_with_too_long_ofp_action_nw_addr, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_tos, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_tos_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_tos_fails_with_too_short_ofp_action_nw_tos, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_nw_tos_fails_with_too_long_ofp_action_nw_tos, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_src, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_src_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_src_fails_with_too_short_ofp_action_tp_port, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_src_fails_with_too_long_ofp_action_tp_port, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_dst, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_dst_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_dst_fails_with_too_short_ofp_action_tp_port, init, teardown ),
    unit_test_setup_teardown( test_validate_action_set_tp_dst_fails_with_too_long_ofp_action_tp_port, init, teardown ),
    unit_test_setup_teardown( test_validate_action_enqueue, init, teardown ),
    unit_test_setup_teardown( test_validate_action_enqueue_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_enqueue_fails_with_too_short_ofp_action_enqueue, init, teardown ),
    unit_test_setup_teardown( test_validate_action_enqueue_fails_with_too_long_ofp_action_enqueue, init, teardown ),
    unit_test_setup_teardown( test_validate_action_vendor, init, teardown ),
    unit_test_setup_teardown( test_validate_action_vendor_fails_with_invalid_action_type, init, teardown ),
    unit_test_setup_teardown( test_validate_action_vendor_fails_with_too_short_ofp_action_vendor_header, init, teardown ),
    unit_test_setup_teardown( test_validate_action_vendor_fails_with_invalid_length_ofp_action_vendor_header, init, teardown ),

    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_HELLO_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_ERROR_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_ECHO_REQUEST_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_ECHO_REPLY_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_VENDOR_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_FEATURES_REQUEST_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_FEATURES_REPLY_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_GET_CONFIG_REQUEST_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_GET_CONFIG_REPLY_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_SET_CONFIG_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_PACKET_IN_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_FLOW_REMOVED_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_PORT_STATUS_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_PACKET_OUT_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_FLOW_MOD_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_PORT_MOD_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_STATS_REQUEST_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_STATS_REPLY_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_BARRIER_REQUEST_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_BARRIER_REPLY_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_QUEUE_GET_CONFIG_REQUEST_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_succeeds_with_valid_OFPT_QUEUE_GET_CONFIG_REPLY_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_fails_with_undefined_type_message, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_fails_if_message_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_validate_openflow_message_fails_if_data_is_NULL, init, teardown ),

    unit_test_setup_teardown( test_valid_openflow_message, init, teardown ),
    unit_test_setup_teardown( test_valid_openflow_message_fails_with_undefined_type_message, init, teardown ),

    unit_test_setup_teardown( test_get_error_type_and_code_succeeds_with_OFPT_ECHO_REQUEST_and_ERROR_UNSUPPORTED_VERSION, init, teardown ),
    unit_test_setup_teardown( test_get_error_type_and_code_succeeds_with_invalid_type_and_ERROR_UNSUPPORTED_VERSION, init, teardown ),
    unit_test_setup_teardown( test_get_error_type_and_code_fails_with_OFPT_ECHO_REQUEST_and_ERROR_TOO_SHORT_ACTION_NW_SRC, init, teardown ),

    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_IN_PORT, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_VLAN, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_TYPE, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_PROTO, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_TP_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_TP_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_SRC_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_DST_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_DL_VLAN_PCP, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_NW_TOS, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_arp_and_wildcards_is_OFPFW_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_IN_PORT, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_VLAN, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_TYPE, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_PROTO, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_TP_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_TP_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_SRC_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_DST_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_DL_VLAN_PCP, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_NW_TOS, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_udp_and_wildcards_is_OFPFW_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_IN_PORT, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_VLAN, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_TYPE, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_PROTO, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_TP_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_TP_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_SRC_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_DST_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_DL_VLAN_PCP, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_NW_TOS, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_tcp_and_wildcards_is_OFPFW_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_IN_PORT, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_VLAN, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_TYPE, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_PROTO, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_TP_SRC, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_TP_DST, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_SRC_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_DST_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_DL_VLAN_PCP, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_NW_TOS, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ipv4_icmp_and_wildcards_is_OFPFW_ALL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_snap_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_snap_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_netbios_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_netbios_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_not_llc_tag_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_succeeds_if_datatype_is_ieee8023_not_llc_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_fails_if_match_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_fails_if_packet_data_is_NULL, init, teardown ),
    unit_test_setup_teardown( test_set_match_from_packet_fails_if_packet_is_not_parsed_yet, init, teardown ),

    unit_test_setup_teardown( test_normalize_match_succeeds_if_datatype_is_arp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_protocol_is_icmp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_protocol_is_tcp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_protocol_is_udp_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_packet_is_invalid_and_wildcards_is_zero, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_ALL, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_IN_PORT, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_VLAN, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_SRC, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_DST, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_TYPE, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_PROTO, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_TP_SRC, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_TP_DST, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_SRC_ALL, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_DST_ALL, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_src_mask_len_is_24, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_dst_mask_len_is_24, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_DL_VLAN_PCP, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_succeeds_if_wildcards_is_OFPFW_NW_TOS, init, teardown ),
    unit_test_setup_teardown( test_normalize_match_fails_if_match_is_NULL, init, teardown ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
