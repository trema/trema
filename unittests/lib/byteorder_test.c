/*
 * Unit tests for byteorder converters.
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
#include <openflow.h>
#include <stdlib.h>
#include <string.h>
#include "byteorder.h"
#include "checks.h"
#include "cmockery_trema.h"
#include "log.h"
#include "utility.h"
#include "wrapper.h"


void
mock_die( const char *format, ... ) {
  UNUSED( format );

  mock_assert( false, "mock_die", __FILE__, __LINE__ );
}


/********************************************************************************
 * Helpers.
 ********************************************************************************/

static const uint8_t MAC_ADDR_X[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 };
static const uint8_t MAC_ADDR_Y[ OFP_ETH_ALEN ] = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d };
static const uint32_t IP_ADDR_X = 0x01020304;
static const uint32_t IP_ADDR_Y = 0x0a090807;
static const uint16_t TP_PORT_X = 1024;
static const uint16_t TP_PORT_Y = 2048;
static const char *PORT_NAME = "port 1";
static const uint32_t PORT_FEATURES = ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD |
                                        OFPPF_100MB_FD | OFPPF_1GB_HD | OFPPF_1GB_FD |
                                        OFPPF_COPPER |  OFPPF_AUTONEG | OFPPF_PAUSE );
static const struct ofp_match MATCH = { OFPFW_ALL, 1,
                                        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 },
                                        { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d },
                                        1, 1, { 0 }, 0x800, 1, 0x6, { 0, 0 },
                                        0x0a090807, 0x0a090807, 1024, 2048 };
static const uint64_t COOKIE = 0x0102030405060708ULL;
static const uint64_t PACKET_COUNT = 10000;
static const uint64_t BYTE_COUNT = 10000000;
static const char *TABLE_NAME = "flow table 1";


static struct ofp_action_output *
create_action_output() {
  struct ofp_action_output *action = xmalloc( sizeof( struct ofp_action_output ) );
  memset( action, 0, sizeof( struct ofp_action_output ) );

  action->type = htons( OFPAT_OUTPUT );
  action->len = htons( 8 );
  action->port = htons( 1 );
  action->max_len = htons( 2048 );

  return action;
}


static struct ofp_action_vlan_vid *
create_action_vlan_vid() {
  struct ofp_action_vlan_vid *action = xmalloc( sizeof( struct ofp_action_vlan_vid ) );
  memset( action, 0, sizeof( struct ofp_action_vlan_vid ) );

  action->type = htons( OFPAT_SET_VLAN_VID );
  action->len = htons( 8 );
  action->vlan_vid = htons( 256 );

  return action;
}


static struct ofp_action_vlan_pcp *
create_action_vlan_pcp() {
  struct ofp_action_vlan_pcp *action = xmalloc( sizeof( struct ofp_action_vlan_pcp ) );
  memset( action, 0, sizeof( struct ofp_action_vlan_pcp ) );

  action->type = htons( OFPAT_SET_VLAN_PCP );
  action->len = htons( 8 );
  action->vlan_pcp = 3;

  return action;
}


static struct ofp_action_header *
create_action_strip_vlan() {
  struct ofp_action_header *action = xmalloc( sizeof( struct ofp_action_header ) );
  memset( action, 0, sizeof( struct ofp_action_header ) );

  action->type = htons( OFPAT_STRIP_VLAN );
  action->len = htons( 8 );

  return action;
}


static struct ofp_action_dl_addr *
create_action_dl_addr() {
  struct ofp_action_dl_addr *action = xmalloc( sizeof( struct ofp_action_dl_addr ) );
  memset( action, 0, sizeof( struct ofp_action_dl_addr ) );

  action->type = htons( OFPAT_SET_DL_SRC );
  action->len = htons( 16 );
  memcpy( action->dl_addr, MAC_ADDR_X, sizeof( action->dl_addr ) );

  return action;
}


static struct ofp_action_nw_addr *
create_action_nw_addr() {
  struct ofp_action_nw_addr *action = xmalloc( sizeof( struct ofp_action_nw_addr ) );
  memset( action, 0, sizeof( struct ofp_action_nw_addr ) );

  action->type = htons( OFPAT_SET_NW_SRC );
  action->len = htons( 8 );
  action->nw_addr = htonl( IP_ADDR_X );

  return action;
}


static struct ofp_action_nw_tos *
create_action_nw_tos() {
  struct ofp_action_nw_tos *action = xmalloc( sizeof( struct ofp_action_nw_tos ) );
  memset( action, 0, sizeof( struct ofp_action_nw_tos ) );

  action->type = htons( OFPAT_SET_NW_TOS );
  action->len = htons( 8 );
  action->nw_tos = 3;

  return action;
}


static struct ofp_action_tp_port *
create_action_tp_port() {
  struct ofp_action_tp_port *action = xmalloc( sizeof( struct ofp_action_tp_port ) );
  memset( action, 0, sizeof( struct ofp_action_tp_port ) );

  action->type = htons( OFPAT_SET_TP_SRC );
  action->len = htons( 8 );
  action->tp_port = htons( TP_PORT_X );

  return action;
}


static struct ofp_action_enqueue *
create_action_enqueue() {
  struct ofp_action_enqueue *action = xmalloc( sizeof( struct ofp_action_enqueue ) );
  memset( action, 0, sizeof( struct ofp_action_enqueue ) );

  action->type = htons( OFPAT_ENQUEUE );
  action->len = htons( 16 );
  action->port = htons( 1 );
  action->queue_id = htonl( 8 );

  return action;
}


static struct ofp_action_vendor_header *
create_action_vendor( size_t body_length ) {
  size_t length = sizeof( struct ofp_action_vendor_header ) + body_length;
  struct ofp_action_vendor_header *action = xmalloc( length );
  memset( action, 0, length );

  action->type = htons( OFPAT_VENDOR );
  action->len = htons( ( uint16_t ) length );
  action->vendor = htonl( 2048 );

  if ( body_length > 0 ) {
    uint8_t *body = ( uint8_t * ) action + sizeof( struct ofp_action_vendor_header );
    for ( size_t i = 0; i < body_length; i++ ) {
      body[ i ] = ( uint8_t ) rand();
    }
  }

  return action;
}


/********************************************************************************
 * ntoh_match() test.
 ********************************************************************************/

void
test_ntoh_match() {
  struct ofp_match dst;
  struct ofp_match src;

  memset( &src, 0, sizeof( struct ofp_match ) );
  memset( &dst, 0, sizeof( struct ofp_match ) );

  src.wildcards = htonl( OFPFW_ALL );
  src.in_port = htons( 1 );
  memcpy( &src.dl_src, MAC_ADDR_X, sizeof( src.dl_src ) );
  memcpy( &src.dl_dst, MAC_ADDR_Y, sizeof( src.dl_dst ) );
  src.dl_vlan = htons( 1 );
  src.dl_vlan_pcp = 1;
  src.dl_type = htons( 0x800 );
  src.nw_tos = 1;
  src.nw_proto = 0x6;
  src.nw_src = htonl( IP_ADDR_X );
  src.nw_dst = htonl( IP_ADDR_Y );
  src.tp_src = htons( TP_PORT_X );
  src.tp_dst = htons( TP_PORT_Y );

  ntoh_match( &dst, &src );

  assert_int_equal( ( int ) htonl( dst.wildcards ), ( int ) src.wildcards );
  assert_int_equal( htons( dst.in_port ), src.in_port );
  assert_memory_equal( dst.dl_src, src.dl_src, sizeof( src.dl_src ) );
  assert_memory_equal( dst.dl_dst, src.dl_dst, sizeof( src.dl_dst ) );
  assert_int_equal( htons( dst.dl_vlan ), src.dl_vlan );
  assert_int_equal( dst.dl_vlan_pcp, src.dl_vlan_pcp );
  assert_int_equal( htons( dst.dl_type ), src.dl_type );
  assert_int_equal( dst.nw_tos, src.nw_tos );
  assert_int_equal( dst.nw_proto, src.nw_proto );
  assert_int_equal( ( int ) htonl( dst.nw_src ), ( int ) src.nw_src );
  assert_int_equal( ( int ) htonl( dst.nw_dst ), ( int ) src.nw_dst );
  assert_int_equal( htons( dst.tp_src ), src.tp_src );
  assert_int_equal( htons( dst.tp_dst ), src.tp_dst );
}


/********************************************************************************
 * ntoh_phy_port() test.
 ********************************************************************************/

void
test_ntoh_phy_port() {
  struct ofp_phy_port dst;
  struct ofp_phy_port src;

  memset( &src, 0, sizeof( struct ofp_phy_port ) );
  memset( &dst, 0, sizeof( struct ofp_phy_port ) );

  src.port_no = htons( 1 );
  memcpy( src.hw_addr, MAC_ADDR_X, sizeof( src.hw_addr ) );
  memset( src.name, '\0', OFP_MAX_PORT_NAME_LEN );
  memcpy( src.name, PORT_NAME, strlen( PORT_NAME ) );
  src.config = htonl( OFPPC_PORT_DOWN );
  src.state = htonl( OFPPS_LINK_DOWN );
  src.curr = htonl( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  src.advertised = htonl( PORT_FEATURES );
  src.supported = htonl( PORT_FEATURES );
  src.peer = htonl( PORT_FEATURES );

  ntoh_phy_port( &dst, &src );

  assert_int_equal( htons( dst.port_no ), src.port_no );
  assert_memory_equal( dst.hw_addr, src.hw_addr, sizeof( src.hw_addr ) );
  assert_memory_equal( dst.name, src.name, OFP_MAX_PORT_NAME_LEN );
  assert_int_equal( ( int ) htonl( dst.config ), ( int ) src.config );
  assert_int_equal( ( int ) htonl( dst.state ), ( int ) src.state );
  assert_int_equal( ( int ) htonl( dst.curr ), ( int ) src.curr );
  assert_int_equal( ( int ) htonl( dst.advertised ), ( int ) src.advertised );
  assert_int_equal( ( int ) htonl( dst.supported ), ( int ) src.supported );
  assert_int_equal( ( int ) htonl( dst.peer ), ( int ) src.peer );
}


/********************************************************************************
 * ntoh_action_output() test.
 ********************************************************************************/

void
test_ntoh_action_output() {
  struct ofp_action_output dst;

  memset( &dst, 0, sizeof( struct ofp_action_output ) );

  struct ofp_action_output *src = create_action_output();

  ntoh_action_output( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( htons( dst.port ), src->port );
  assert_int_equal( htons( dst.max_len ), src->max_len );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_vlan_vid() test.
 ********************************************************************************/

void
test_ntoh_action_vlan_vid() {
  struct ofp_action_vlan_vid dst;

  memset( &dst, 0, sizeof( struct ofp_action_vlan_vid ) );

  struct ofp_action_vlan_vid *src = create_action_vlan_vid();

  ntoh_action_vlan_vid( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( htons( dst.vlan_vid ), src->vlan_vid );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_vlan_pcp() test.
 ********************************************************************************/

void
test_ntoh_action_vlan_pcp() {
  struct ofp_action_vlan_pcp dst;

  memset( &dst, 0, sizeof( struct ofp_action_vlan_pcp ) );

  struct ofp_action_vlan_pcp *src = create_action_vlan_pcp();

  ntoh_action_vlan_pcp( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( dst.vlan_pcp, src->vlan_pcp );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_strip_vlan() test.
 ********************************************************************************/

void
test_ntoh_action_strip_vlan() {
  struct ofp_action_header dst;

  memset( &dst, 0, sizeof( struct ofp_action_header ) );

  struct ofp_action_header *src = create_action_strip_vlan();

  ntoh_action_strip_vlan( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_dl_addr() test.
 ********************************************************************************/

void
test_ntoh_action_dl_addr() {
  struct ofp_action_dl_addr dst;

  memset( &dst, 0, sizeof( struct ofp_action_dl_addr ) );

  struct ofp_action_dl_addr *src = create_action_dl_addr();

  ntoh_action_dl_addr( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_memory_equal( dst.dl_addr, src->dl_addr, sizeof( src->dl_addr ) );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_nw_addr() test.
 ********************************************************************************/

void
test_ntoh_action_nw_addr() {
  struct ofp_action_nw_addr dst;

  memset( &dst, 0, sizeof( struct ofp_action_nw_addr ) );

  struct ofp_action_nw_addr *src = create_action_nw_addr();

  ntoh_action_nw_addr( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( ( int ) htonl( dst.nw_addr ), ( int ) src->nw_addr );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_nw_tos() test.
 ********************************************************************************/

void
test_ntoh_action_nw_tos() {
  struct ofp_action_nw_tos dst;

  memset( &dst, 0, sizeof( struct ofp_action_nw_tos ) );

  struct ofp_action_nw_tos *src = create_action_nw_tos();

  ntoh_action_nw_tos( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( dst.nw_tos, src->nw_tos );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_tp_port() test.
 ********************************************************************************/

void
test_ntoh_action_tp_port() {
  struct ofp_action_tp_port dst;

  memset( &dst, 0, sizeof( struct ofp_action_tp_port ) );

  struct ofp_action_tp_port *src = create_action_tp_port();

  ntoh_action_tp_port( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( htons( dst.tp_port ), src->tp_port );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_enqueue() test.
 ********************************************************************************/

void
test_ntoh_action_enqueue() {
  struct ofp_action_enqueue dst;

  memset( &dst, 0, sizeof( struct ofp_action_enqueue ) );

  struct ofp_action_enqueue *src = create_action_enqueue();

  ntoh_action_enqueue( &dst, src );

  assert_int_equal( htons( dst.type ), src->type );
  assert_int_equal( htons( dst.len ), src->len );
  assert_int_equal( htons( dst.port ), src->port );
  assert_int_equal( ( int ) htonl( dst.queue_id ), ( int ) src->queue_id );

  xfree( src );
}


/********************************************************************************
 * ntoh_action_vendor() test.
 ********************************************************************************/

void
test_ntoh_action_vendor() {
  struct ofp_action_vendor_header *src = create_action_vendor( 0 );

  size_t length = sizeof( struct ofp_action_vendor_header );
  struct ofp_action_vendor_header *dst = xmalloc( length );
  memset( dst, 0, length );

  ntoh_action_vendor( dst, src );

  assert_int_equal( htons( dst->type ), src->type );
  assert_int_equal( htons( dst->len ), src->len );
  assert_int_equal( ( int ) htonl( dst->vendor ), ( int ) src->vendor );

  xfree( src );
  xfree( dst );
}


void
test_ntoh_action_vendor_with_body() {
  const size_t body_length = 128;
  struct ofp_action_vendor_header *src = create_action_vendor( body_length );

  size_t length = sizeof( struct ofp_action_vendor_header ) + body_length;
  struct ofp_action_vendor_header *dst = xmalloc( length );
  memset( dst, 0, length );

  ntoh_action_vendor( dst, src );

  assert_int_equal( htons( dst->type ), src->type );
  assert_int_equal( htons( dst->len ), src->len );
  assert_int_equal( ( int ) htonl( dst->vendor ), ( int ) src->vendor );
  void *src_body = ( char * ) src + sizeof( struct ofp_action_header );
  void *dst_body = ( char * ) dst + sizeof( struct ofp_action_header );
  assert_memory_equal( dst_body, src_body, body_length );

  xfree( src );
  xfree( dst );
}


/********************************************************************************
 * hton_action_vendor() test.
 ********************************************************************************/

void
test_hton_action_vendor() {
  struct ofp_action_vendor_header *src = create_action_vendor( 0 );
  hton_action_vendor( src, src );

  size_t length = sizeof( struct ofp_action_vendor_header );
  struct ofp_action_vendor_header *dst = xmalloc( length );
  memset( dst, 0, length );

  hton_action_vendor( dst, src );

  assert_int_equal( ntohs( dst->type ), src->type );
  assert_int_equal( ntohs( dst->len ), src->len );
  assert_int_equal( ( int ) ntohl( dst->vendor ), ( int ) src->vendor );

  xfree( src );
  xfree( dst );
}


void
test_hton_action_vendor_with_body() {
  const size_t body_length = 128;
  struct ofp_action_vendor_header *src = create_action_vendor( body_length );
  hton_action_vendor( src, src );

  size_t length = sizeof( struct ofp_action_vendor_header ) + body_length;
  struct ofp_action_vendor_header *dst = xmalloc( length );
  memset( dst, 0, length );

  hton_action_vendor( dst, src );

  assert_int_equal( ntohs( dst->type ), src->type );
  assert_int_equal( ntohs( dst->len ), src->len );
  assert_int_equal( ( int ) ntohl( dst->vendor ), ( int ) src->vendor );
  void *src_body = ( char * ) src + sizeof( struct ofp_action_header );
  void *dst_body = ( char * ) dst + sizeof( struct ofp_action_header );
  assert_memory_equal( dst_body, src_body, body_length );

  xfree( src );
  xfree( dst );
}


/********************************************************************************
 * ntoh_action() tests.
 ********************************************************************************/

void
test_ntoh_action() {
  {
    struct ofp_action_output dst;

    memset( &dst, 0, sizeof( struct ofp_action_output ) );

    struct ofp_action_output *src = create_action_output();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.port ), src->port );
    assert_int_equal( htons( dst.max_len ), src->max_len );

    xfree( src );
  }

  {
    struct ofp_action_vlan_vid dst;

    memset( &dst, 0, sizeof( struct ofp_action_vlan_vid ) );

    struct ofp_action_vlan_vid *src = create_action_vlan_vid();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.vlan_vid ), src->vlan_vid );

    xfree( src );
  }

  {
    struct ofp_action_vlan_pcp dst;

    memset( &dst, 0, sizeof( struct ofp_action_vlan_pcp ) );

    struct ofp_action_vlan_pcp *src = create_action_vlan_pcp();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( dst.vlan_pcp, src->vlan_pcp );

    xfree( src );
  }

  {
    struct ofp_action_header dst;

    memset( &dst, 0, sizeof( struct ofp_action_header ) );

    struct ofp_action_header *src = create_action_strip_vlan();

    ntoh_action( &dst, src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );

    xfree( src );
  }

  {
    struct ofp_action_dl_addr dst;

    memset( &dst, 0, sizeof( struct ofp_action_dl_addr ) );

    struct ofp_action_dl_addr *src = create_action_dl_addr();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_memory_equal( dst.dl_addr, src->dl_addr, sizeof( src->dl_addr ) );

    xfree( src );
  }

  {
    struct ofp_action_nw_addr dst;

    memset( &dst, 0, sizeof( struct ofp_action_nw_addr ) );

    struct ofp_action_nw_addr *src = create_action_nw_addr();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( ( int ) htonl( dst.nw_addr ), ( int ) src->nw_addr );

    xfree( src );
  }

  {
    struct ofp_action_nw_tos dst;

    memset( &dst, 0, sizeof( struct ofp_action_nw_tos ) );

    struct ofp_action_nw_tos *src = create_action_nw_tos();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( dst.nw_tos, src->nw_tos );

    xfree( src );
  }

  {
    struct ofp_action_tp_port dst;

    memset( &dst, 0, sizeof( struct ofp_action_tp_port ) );

    struct ofp_action_tp_port *src = create_action_tp_port();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.tp_port ), src->tp_port );

    xfree( src );
  }

  {
    struct ofp_action_enqueue dst;

    memset( &dst, 0, sizeof( struct ofp_action_enqueue ) );

    struct ofp_action_enqueue *src = create_action_enqueue();

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.port ), src->port );
    assert_int_equal( ( int ) htonl( dst.queue_id ), ( int ) src->queue_id );

    xfree( src );
  }

  {
    struct ofp_action_vendor_header dst;

    memset( &dst, 0, sizeof( struct ofp_action_vendor_header ) );

    struct ofp_action_vendor_header *src = create_action_vendor( 0 );

    ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( ( int ) htonl( dst.vendor ), ( int ) src->vendor );

    xfree( src );
  }
}


void
test_ntoh_action_with_undefined_action_type() {
  struct ofp_action_output dst;

  memset( &dst, 0, sizeof( struct ofp_action_output ) );

  struct ofp_action_output *src = create_action_output();

  src->type = htons( OFPAT_ENQUEUE + 1 );

  expect_assert_failure( ntoh_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src ) );

  xfree( src );
}


/********************************************************************************
 * hton_action() tests.
 ********************************************************************************/

void
test_hton_action() {
  {
    struct ofp_action_output dst;

    memset( &dst, 0, sizeof( struct ofp_action_output ) );

    struct ofp_action_output *src = create_action_output();
    ntoh_action_output( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.port ), src->port );
    assert_int_equal( htons( dst.max_len ), src->max_len );

    xfree( src );
  }

  {
    struct ofp_action_vlan_vid dst;

    memset( &dst, 0, sizeof( struct ofp_action_vlan_vid ) );

    struct ofp_action_vlan_vid *src = create_action_vlan_vid();
    ntoh_action_vlan_vid( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.vlan_vid ), src->vlan_vid );

    xfree( src );
  }

  {
    struct ofp_action_vlan_pcp dst;

    memset( &dst, 0, sizeof( struct ofp_action_vlan_pcp ) );

    struct ofp_action_vlan_pcp *src = create_action_vlan_pcp();
    ntoh_action_vlan_pcp( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( dst.vlan_pcp, src->vlan_pcp );

    xfree( src );
  }

  {
    struct ofp_action_header dst;

    memset( &dst, 0, sizeof( struct ofp_action_header ) );

    struct ofp_action_header *src = create_action_strip_vlan();
    ntoh_action_strip_vlan( src, src );

    hton_action( &dst, src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );

    xfree( src );
  }

  {
    struct ofp_action_dl_addr dst;

    memset( &dst, 0, sizeof( struct ofp_action_dl_addr ) );

    struct ofp_action_dl_addr *src = create_action_dl_addr();
    ntoh_action_dl_addr( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_memory_equal( dst.dl_addr, src->dl_addr, sizeof( src->dl_addr ) );

    xfree( src );
  }

  {
    struct ofp_action_nw_addr dst;

    memset( &dst, 0, sizeof( struct ofp_action_nw_addr ) );

    struct ofp_action_nw_addr *src = create_action_nw_addr();
    ntoh_action_nw_addr( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( ( int ) htonl( dst.nw_addr ), ( int ) src->nw_addr );

    xfree( src );
  }

  {
    struct ofp_action_nw_tos dst;

    memset( &dst, 0, sizeof( struct ofp_action_nw_tos ) );

    struct ofp_action_nw_tos *src = create_action_nw_tos();
    ntoh_action_nw_tos( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( dst.nw_tos, src->nw_tos );

    xfree( src );
  }

  {
    struct ofp_action_tp_port dst;

    memset( &dst, 0, sizeof( struct ofp_action_tp_port ) );

    struct ofp_action_tp_port *src = create_action_tp_port();
    ntoh_action_tp_port( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.tp_port ), src->tp_port );

    xfree( src );
  }

  {
    struct ofp_action_enqueue dst;

    memset( &dst, 0, sizeof( struct ofp_action_enqueue ) );

    struct ofp_action_enqueue *src = create_action_enqueue();
    ntoh_action_enqueue( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( htons( dst.port ), src->port );
    assert_int_equal( ( int ) htonl( dst.queue_id ), ( int ) src->queue_id );

    xfree( src );
  }

  {
    struct ofp_action_vendor_header dst;

    memset( &dst, 0, sizeof( struct ofp_action_vendor_header ) );

    struct ofp_action_vendor_header *src = create_action_vendor( 0 );
    ntoh_action_vendor( src, src );

    hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src );

    assert_int_equal( htons( dst.type ), src->type );
    assert_int_equal( htons( dst.len ), src->len );
    assert_int_equal( ( int ) htonl( dst.vendor ), ( int ) src->vendor );

    xfree( src );
  }
}


void
test_hton_action_with_undefined_action_type() {
  struct ofp_action_output dst;

  memset( &dst, 0, sizeof( struct ofp_action_output ) );

  struct ofp_action_output *src = create_action_output();
  ntoh_action_output( src, src );

  src->type = OFPAT_ENQUEUE + 1;

  expect_assert_failure( hton_action( ( struct ofp_action_header * ) &dst, ( struct ofp_action_header * ) src ) );

  xfree( src );
}


/********************************************************************************
 * ntoh_flow_stats() tests.
 ********************************************************************************/

void
test_ntoh_flow_stats_without_action() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_flow_stats, actions ) );

  struct ofp_flow_stats *src = xmalloc( length );
  struct ofp_flow_stats *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->length = htons( length );
  src->table_id = 1;
  hton_match( &src->match, &MATCH );
  src->duration_sec = htonl( 60 );
  src->duration_nsec = htonl( 5000 );
  src->priority = htons( UINT16_MAX );
  src->idle_timeout = htons( 60 );
  src->hard_timeout = htons( 300 );
  src->cookie = htonll( COOKIE );
  src->packet_count = htonll( PACKET_COUNT );
  src->byte_count = htonll( BYTE_COUNT );

  ntoh_flow_stats( dst, src );

  assert_int_equal( htons( dst->length ), src->length );
  assert_int_equal( dst->table_id, src->table_id );
  assert_memory_equal( &dst->match, &MATCH, sizeof( struct ofp_match ) );
  assert_int_equal( ( int ) htonl( dst->duration_sec ), ( int ) src->duration_sec );
  assert_int_equal( ( int ) htonl( dst->duration_nsec ), ( int ) src->duration_nsec );
  assert_int_equal( htons( dst->priority ), src->priority );
  assert_int_equal( htons( dst->idle_timeout ), src->idle_timeout );
  assert_int_equal( htons( dst->hard_timeout ), src->hard_timeout );
  assert_memory_equal( &dst->cookie, &COOKIE, sizeof( uint64_t ) );
  assert_memory_equal( &dst->packet_count, &PACKET_COUNT, sizeof( uint64_t ) );
  assert_memory_equal( &dst->byte_count, &BYTE_COUNT, sizeof( uint64_t ) );

  xfree( src );
  xfree( dst );
}


void
test_ntoh_flow_stats_with_single_output_action() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output ) );

  struct ofp_flow_stats *src = xmalloc( length );
  struct ofp_flow_stats *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->length = htons( length );
  src->table_id = 1;
  hton_match( &src->match, &MATCH );
  src->duration_sec = htonl( 60 );
  src->duration_nsec = htonl( 5000 );
  src->priority = htons( UINT16_MAX );
  src->idle_timeout = htons( 60 );
  src->hard_timeout = htons( 300 );
  src->cookie = htonll( COOKIE );
  src->packet_count = htonll( PACKET_COUNT );
  src->byte_count = htonll( BYTE_COUNT );
  struct ofp_action_output *act_src = ( struct ofp_action_output * ) src->actions;
  act_src->type = htons( OFPAT_OUTPUT );
  act_src->len = htons( 8 );
  act_src->port = htons( 1 );
  act_src->max_len = htons( 2048 );

  ntoh_flow_stats( dst, src );

  struct ofp_action_output *act_dst = ( struct ofp_action_output * ) dst->actions;

  assert_int_equal( htons( dst->length ), src->length );
  assert_int_equal( dst->table_id, src->table_id );
  assert_memory_equal( &dst->match, &MATCH, sizeof( struct ofp_match ) );
  assert_int_equal( ( int ) htonl( dst->duration_sec ), ( int ) src->duration_sec );
  assert_int_equal( ( int ) htonl( dst->duration_nsec ), ( int ) src->duration_nsec );
  assert_int_equal( htons( dst->priority ), src->priority );
  assert_int_equal( htons( dst->idle_timeout ), src->idle_timeout );
  assert_int_equal( htons( dst->hard_timeout ), src->hard_timeout );
  assert_memory_equal( &dst->cookie, &COOKIE, sizeof( uint64_t ) );
  assert_memory_equal( &dst->packet_count, &PACKET_COUNT, sizeof( uint64_t ) );
  assert_memory_equal( &dst->byte_count, &BYTE_COUNT, sizeof( uint64_t ) );
  assert_int_equal( htons( act_dst->type ), act_src->type );
  assert_int_equal( htons( act_dst->len ), act_src->len );
  assert_int_equal( htons( act_dst->port ), act_src->port );
  assert_int_equal( htons( act_dst->max_len ), act_src->max_len );

  xfree( src );
  xfree( dst );
}


void
test_ntoh_flow_stats_with_two_output_actions() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output ) * 2 );

  struct ofp_flow_stats *src = xmalloc( length );
  struct ofp_flow_stats *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->length = htons( length );
  src->table_id = 1;
  hton_match( &src->match, &MATCH );
  src->duration_sec = htonl( 60 );
  src->duration_nsec = htonl( 5000 );
  src->priority = htons( UINT16_MAX );
  src->idle_timeout = htons( 60 );
  src->hard_timeout = htons( 300 );
  src->cookie = htonll( COOKIE );
  src->packet_count = htonll( PACKET_COUNT );
  src->byte_count = htonll( BYTE_COUNT );
  struct ofp_action_output *act_src[ 2 ];
  act_src[ 0 ] = ( struct ofp_action_output * ) src->actions;
  act_src[ 0 ]->type = htons( OFPAT_OUTPUT );
  act_src[ 0 ]->len = htons( 8 );
  act_src[ 0 ]->port = htons( 1 );
  act_src[ 0 ]->max_len = htons( 2048 );
  act_src[ 1 ] = ( struct ofp_action_output * ) ( ( char * ) src->actions + sizeof( struct ofp_action_output ) );
  act_src[ 1 ]->type = htons( OFPAT_OUTPUT );
  act_src[ 1 ]->len = htons( 8 );
  act_src[ 1 ]->port = htons( 2 );
  act_src[ 1 ]->max_len = htons( 2048 );

  ntoh_flow_stats( dst, src );

  struct ofp_action_output *act_dst[ 2 ];
  act_dst[ 0 ] = ( struct ofp_action_output * ) dst->actions;
  act_dst[ 1 ] = ( struct ofp_action_output * ) ( ( char * ) dst->actions + sizeof( struct ofp_action_output ) );

  assert_int_equal( htons( dst->length ), src->length );
  assert_int_equal( dst->table_id, src->table_id );
  assert_memory_equal( &dst->match, &MATCH, sizeof( struct ofp_match ) );
  assert_int_equal( ( int ) htonl( dst->duration_sec ), ( int ) src->duration_sec );
  assert_int_equal( ( int ) htonl( dst->duration_nsec ), ( int ) src->duration_nsec );
  assert_int_equal( htons( dst->priority ), src->priority );
  assert_int_equal( htons( dst->idle_timeout ), src->idle_timeout );
  assert_int_equal( htons( dst->hard_timeout ), src->hard_timeout );
  assert_memory_equal( &dst->cookie, &COOKIE, sizeof( uint64_t ) );
  assert_memory_equal( &dst->packet_count, &PACKET_COUNT, sizeof( uint64_t ) );
  assert_memory_equal( &dst->byte_count, &BYTE_COUNT, sizeof( uint64_t ) );
  assert_int_equal( htons( act_dst[ 0 ]->type ), act_src[ 0 ]->type );
  assert_int_equal( htons( act_dst[ 0 ]->len ), act_src[ 0 ]->len );
  assert_int_equal( htons( act_dst[ 0 ]->port ), act_src[ 0 ]->port );
  assert_int_equal( htons( act_dst[ 0 ]->max_len ), act_src[ 0 ]->max_len );
  assert_int_equal( htons( act_dst[ 1 ]->type ), act_src[ 1 ]->type );
  assert_int_equal( htons( act_dst[ 1 ]->len ), act_src[ 1 ]->len );
  assert_int_equal( htons( act_dst[ 1 ]->port ), act_src[ 1 ]->port );
  assert_int_equal( htons( act_dst[ 1 ]->max_len ), act_src[ 1 ]->max_len );

  xfree( src );
  xfree( dst );
}


/********************************************************************************
 * hton_flow_stats() tests.
 ********************************************************************************/

void
test_hton_flow_stats_without_action() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_flow_stats, actions ) );

  struct ofp_flow_stats *src = xmalloc( length );
  struct ofp_flow_stats *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->length = length;
  src->table_id = 1;
  src->match = MATCH;
  src->duration_sec = 60;
  src->duration_nsec = 5000;
  src->priority = UINT16_MAX;
  src->idle_timeout = 60;
  src->hard_timeout = 300;
  src->cookie = COOKIE;
  src->packet_count = PACKET_COUNT;
  src->byte_count = BYTE_COUNT;

  hton_flow_stats( dst, src );

  assert_int_equal( dst->length, htons( src->length ) );
  assert_int_equal( dst->table_id, src->table_id );
  hton_match( &src->match, &MATCH );
  assert_memory_equal( &dst->match, &src->match, sizeof( struct ofp_match ) );
  assert_int_equal( ( int ) dst->duration_sec, ( int ) htonl( src->duration_sec ) );
  assert_int_equal( ( int ) dst->duration_nsec, ( int ) htonl( src->duration_nsec ) );
  assert_int_equal( dst->priority, htons( src->priority ) );
  assert_int_equal( dst->idle_timeout, htons( src->idle_timeout ) );
  assert_int_equal( dst->hard_timeout, htons( src->hard_timeout ) );
  src->cookie = htonll( COOKIE );
  assert_memory_equal( &dst->cookie, &src->cookie, sizeof( uint64_t ) );
  src->packet_count = htonll( PACKET_COUNT );
  assert_memory_equal( &dst->packet_count, &src->packet_count, sizeof( uint64_t ) );
  src->byte_count = htonll( BYTE_COUNT );
  assert_memory_equal( &dst->byte_count, &src->byte_count, sizeof( uint64_t ) );

  xfree( src );
  xfree( dst );
}


void
test_hton_flow_stats_with_single_output_action() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output ) );

  struct ofp_flow_stats *src = xmalloc( length );
  struct ofp_flow_stats *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->length = length;
  src->table_id = 1;
  src->match = MATCH;
  src->duration_sec = 60;
  src->duration_nsec = 5000;
  src->priority = UINT16_MAX;
  src->idle_timeout = 60;
  src->hard_timeout = 300;
  src->cookie = COOKIE;
  src->packet_count = PACKET_COUNT;
  src->byte_count = BYTE_COUNT;
  struct ofp_action_output *act_src = ( struct ofp_action_output * ) src->actions;
  act_src->type = OFPAT_OUTPUT;
  act_src->len = 8;
  act_src->port = 1;
  act_src->max_len = 2048;

  hton_flow_stats( dst, src );

  struct ofp_action_output *act_dst = ( struct ofp_action_output * ) dst->actions;

  assert_int_equal( dst->length, htons( src->length ) );
  assert_int_equal( dst->table_id, src->table_id );
  hton_match( &src->match, &MATCH );
  assert_memory_equal( &dst->match, &src->match, sizeof( struct ofp_match ) );
  assert_int_equal( ( int ) dst->duration_sec, ( int ) htonl( src->duration_sec ) );
  assert_int_equal( ( int ) dst->duration_nsec, ( int ) htonl( src->duration_nsec ) );
  assert_int_equal( dst->priority, htons( src->priority ) );
  assert_int_equal( dst->idle_timeout, htons( src->idle_timeout ) );
  assert_int_equal( dst->hard_timeout, htons( src->hard_timeout ) );
  src->cookie = htonll( COOKIE );
  assert_memory_equal( &dst->cookie, &src->cookie, sizeof( uint64_t ) );
  src->packet_count = htonll( PACKET_COUNT );
  assert_memory_equal( &dst->packet_count, &src->packet_count, sizeof( uint64_t ) );
  src->byte_count = htonll( BYTE_COUNT );
  assert_memory_equal( &dst->byte_count, &src->byte_count, sizeof( uint64_t ) );
  assert_int_equal( act_dst->type, htons( act_src->type ) );
  assert_int_equal( act_dst->len, htons( act_src->len ) );
  assert_int_equal( act_dst->port, htons( act_src->port ) );
  assert_int_equal( act_dst->max_len, htons( act_src->max_len ) );

  xfree( src );
  xfree( dst );
}


void
test_hton_flow_stats_with_two_output_actions() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_flow_stats, actions ) + sizeof( struct ofp_action_output ) * 2 );

  struct ofp_flow_stats *src = xmalloc( length );
  struct ofp_flow_stats *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->length = length;
  src->table_id = 1;
  src->match = MATCH;
  src->duration_sec = 60;
  src->duration_nsec = 5000;
  src->priority = UINT16_MAX;
  src->idle_timeout = 60;
  src->hard_timeout = 300;
  src->cookie = COOKIE;
  src->packet_count = PACKET_COUNT;
  src->byte_count = BYTE_COUNT;
  struct ofp_action_output *act_src[ 2 ];
  act_src[ 0 ] = ( struct ofp_action_output * ) src->actions;
  act_src[ 0 ]->type = OFPAT_OUTPUT;
  act_src[ 0 ]->len = 8;
  act_src[ 0 ]->port = 1;
  act_src[ 0 ]->max_len = 2048;
  act_src[ 1 ] = ( struct ofp_action_output * ) ( ( char * ) src->actions + sizeof( struct ofp_action_output ) );
  act_src[ 1 ]->type = OFPAT_OUTPUT;
  act_src[ 1 ]->len = 8;
  act_src[ 1 ]->port = 2;
  act_src[ 1 ]->max_len = 2048;

  hton_flow_stats( dst, src );

  struct ofp_action_output *act_dst[ 2 ];
  act_dst[ 0 ] = ( struct ofp_action_output * ) dst->actions;
  act_dst[ 1 ] = ( struct ofp_action_output * ) ( ( char * ) dst->actions + sizeof( struct ofp_action_output ) );

  assert_int_equal( htons( dst->length ), src->length );
  assert_int_equal( dst->table_id, src->table_id );
  hton_match( &src->match, &MATCH );
  assert_memory_equal( &dst->match, &src->match, sizeof( struct ofp_match ) );
  assert_int_equal( ( int ) htonl( dst->duration_sec ), ( int ) src->duration_sec );
  assert_int_equal( ( int ) htonl( dst->duration_nsec ), ( int ) src->duration_nsec );
  assert_int_equal( htons( dst->priority ), src->priority );
  assert_int_equal( htons( dst->idle_timeout ), src->idle_timeout );
  assert_int_equal( htons( dst->hard_timeout ), src->hard_timeout );
  src->cookie = htonll( COOKIE );
  assert_memory_equal( &dst->cookie, &src->cookie, sizeof( uint64_t ) );
  src->packet_count = htonll( PACKET_COUNT );
  assert_memory_equal( &dst->packet_count, &src->packet_count, sizeof( uint64_t ) );
  src->byte_count = htonll( BYTE_COUNT );
  assert_memory_equal( &dst->byte_count, &src->byte_count, sizeof( uint64_t ) );
  assert_int_equal( act_dst[ 0 ]->type, htons( act_src[ 0 ]->type ) );
  assert_int_equal( act_dst[ 0 ]->len, htons( act_src[ 0 ]->len ) );
  assert_int_equal( act_dst[ 0 ]->port, htons( act_src[ 0 ]->port ) );
  assert_int_equal( act_dst[ 0 ]->max_len, htons( act_src[ 0 ]->max_len ) );
  assert_int_equal( act_dst[ 1 ]->type, htons( act_src[ 1 ]->type ) );
  assert_int_equal( act_dst[ 1 ]->len, htons( act_src[ 1 ]->len ) );
  assert_int_equal( act_dst[ 1 ]->port, htons( act_src[ 1 ]->port ) );
  assert_int_equal( act_dst[ 1 ]->max_len, htons( act_src[ 1 ]->max_len ) );

  xfree( src );
  xfree( dst );
}


/********************************************************************************
 * ntoh_aggregate_stats() test.
 ********************************************************************************/

void
test_ntoh_aggregate_stats() {
  struct ofp_aggregate_stats_reply dst;
  struct ofp_aggregate_stats_reply src;

  memset( &src, 0, sizeof( struct ofp_aggregate_stats_reply ) );
  memset( &dst, 0, sizeof( struct ofp_aggregate_stats_reply ) );

  src.packet_count = htonll( PACKET_COUNT );
  src.byte_count = htonll( BYTE_COUNT );
  src.flow_count = htonl( 1000 );

  ntoh_aggregate_stats( &dst, &src );

  assert_memory_equal( &dst.packet_count, &PACKET_COUNT, sizeof( uint64_t ) );
  assert_memory_equal( &dst.byte_count, &BYTE_COUNT, sizeof( uint64_t ) );
  assert_int_equal( ( int ) htonl( dst.flow_count ), ( int ) src.flow_count );
}


/********************************************************************************
 * ntoh_table_stats() test.
 ********************************************************************************/

void
test_ntoh_table_stats() {
  struct ofp_table_stats dst;
  struct ofp_table_stats src;

  memset( &src, 0, sizeof( struct ofp_table_stats ) );
  memset( &dst, 0, sizeof( struct ofp_table_stats ) );

  uint64_t lookup_count = 100000000;
  uint64_t matched_count = 10000000;

  src.table_id = 1;
  memset( &src.name, '\0', OFP_MAX_TABLE_NAME_LEN );
  memcpy( &src.name, TABLE_NAME, strlen( TABLE_NAME ) );
  src.wildcards = htonl( OFPFW_ALL );
  src.max_entries = htonl( 1000000 );
  src.active_count = htonl( 1234 );
  src.lookup_count = htonll( lookup_count );
  src.matched_count = htonll( matched_count );

  ntoh_table_stats( &dst, &src );

  assert_int_equal( dst.table_id, src.table_id );
  assert_memory_equal( &dst.name, &src.name, OFP_MAX_TABLE_NAME_LEN );
  assert_int_equal( ( int ) htonl( dst.wildcards ), ( int ) src.wildcards );
  assert_int_equal( ( int ) htonl( dst.max_entries ), ( int ) src.max_entries );
  assert_int_equal( ( int ) htonl( dst.active_count ), ( int ) src.active_count );
  assert_memory_equal( &dst.lookup_count, &lookup_count, sizeof( uint64_t ) );
  assert_memory_equal( &dst.matched_count, &matched_count, sizeof( uint64_t ) );
}


/********************************************************************************
 * ntoh_port_stats() test.
 ********************************************************************************/

void
test_ntoh_port_stats() {
  struct ofp_port_stats dst;
  struct ofp_port_stats src;

  memset( &src, 0, sizeof( struct ofp_port_stats ) );
  memset( &dst, 0, sizeof( struct ofp_port_stats ) );

  uint64_t rx_packets = 8000;
  uint64_t tx_packets = 7000;
  uint64_t rx_bytes = 6000;
  uint64_t tx_bytes = 5000;
  uint64_t rx_dropped = 4000;
  uint64_t tx_dropped = 3000;
  uint64_t rx_errors = 2000;
  uint64_t tx_errors = 1000;
  uint64_t rx_frame_err = 900;
  uint64_t rx_over_err = 100;
  uint64_t rx_crc_err = 10;
  uint64_t collisions = 1;

  src.port_no = htons( 1 );
  src.rx_packets = htonll( rx_packets );
  src.tx_packets = htonll( tx_packets );
  src.rx_bytes = htonll( rx_bytes );
  src.tx_bytes = htonll( tx_bytes );
  src.rx_dropped = htonll( rx_dropped );
  src.tx_dropped = htonll( tx_dropped );
  src.rx_errors = htonll( rx_errors );
  src.tx_errors = htonll( tx_errors );
  src.rx_frame_err = htonll( rx_frame_err );
  src.rx_over_err = htonll( rx_over_err );
  src.rx_crc_err = htonll( rx_crc_err );
  src.collisions = htonll( collisions );

  ntoh_port_stats( &dst, &src );

  assert_int_equal( htons( dst.port_no ), src.port_no );
  assert_memory_equal( &dst.rx_packets, &rx_packets, sizeof( uint64_t ) );
  assert_memory_equal( &dst.tx_packets, &tx_packets, sizeof( uint64_t ) );
  assert_memory_equal( &dst.rx_bytes, &rx_bytes, sizeof( uint64_t ) );
  assert_memory_equal( &dst.tx_bytes, &tx_bytes, sizeof( uint64_t ) );
  assert_memory_equal( &dst.rx_dropped, &rx_dropped, sizeof( uint64_t ) );
  assert_memory_equal( &dst.tx_dropped, &tx_dropped, sizeof( uint64_t ) );
  assert_memory_equal( &dst.rx_errors, &rx_errors, sizeof( uint64_t ) );
  assert_memory_equal( &dst.tx_errors, &tx_errors, sizeof( uint64_t ) );
  assert_memory_equal( &dst.rx_frame_err, &rx_frame_err, sizeof( uint64_t ) );
  assert_memory_equal( &dst.rx_over_err, &rx_over_err, sizeof( uint64_t ) );
  assert_memory_equal( &dst.rx_crc_err, &rx_crc_err, sizeof( uint64_t ) );
  assert_memory_equal( &dst.collisions, &collisions, sizeof( uint64_t ) );
}


/********************************************************************************
 * ntoh_queue_stats() test.
 ********************************************************************************/

void
test_ntoh_queue_stats() {
  struct ofp_queue_stats dst;
  struct ofp_queue_stats src;

  memset( &src, 0, sizeof( struct ofp_queue_stats ) );
  memset( &dst, 0, sizeof( struct ofp_queue_stats ) );

  uint64_t tx_bytes = 10000000;
  uint64_t tx_packets = 10000;
  uint64_t tx_errors = 1;

  src.port_no = htons( 1 );
  src.queue_id = htonl( 3 );
  src.tx_bytes = htonll( tx_bytes );
  src.tx_packets = htonll( tx_packets );
  src.tx_errors = htonll( tx_errors );

  ntoh_queue_stats( &dst, &src );

  assert_int_equal( htons( dst.port_no ), src.port_no );
  assert_int_equal( ( int ) htonl( dst.queue_id ), ( int ) src.queue_id );
  assert_memory_equal( &dst.tx_bytes, &tx_bytes, sizeof( uint64_t ) );
  assert_memory_equal( &dst.tx_packets, &tx_packets, sizeof( uint64_t ) );
  assert_memory_equal( &dst.tx_errors, &tx_errors, sizeof( uint64_t ) );
}


/********************************************************************************
 * ntoh_queue_property() tests.
 ********************************************************************************/

void
test_ntoh_queue_property_with_OFPQT_NONE() {
  struct ofp_queue_prop_header dst;
  struct ofp_queue_prop_header src;

  memset( &src, 0, sizeof( struct ofp_queue_prop_header ) );
  memset( &dst, 0, sizeof( struct ofp_queue_prop_header ) );

  src.property = htons( OFPQT_NONE );
  src.len = htons( 8 );

  ntoh_queue_property( &dst, &src );

  assert_int_equal( htons( dst.property ), src.property );
  assert_int_equal( htons( dst.len ), src.len );
}


void
test_ntoh_queue_property_with_OFPQT_MIN_RATE() {
  struct ofp_queue_prop_min_rate dst;
  struct ofp_queue_prop_min_rate src;

  memset( &src, 0, sizeof( struct ofp_queue_prop_min_rate ) );
  memset( &dst, 0, sizeof( struct ofp_queue_prop_min_rate ) );

  src.prop_header.property = htons( OFPQT_MIN_RATE );
  src.prop_header.len = htons( 16 );
  src.rate = htons( 500 );

  ntoh_queue_property( ( struct ofp_queue_prop_header * ) &dst, ( struct ofp_queue_prop_header * ) &src );

  assert_int_equal( htons( dst.prop_header.property ), src.prop_header.property );
  assert_int_equal( htons( dst.prop_header.len ), src.prop_header.len );
  assert_int_equal( htons( dst.rate ), src.rate );
}


/********************************************************************************
 * hton_queue_property() tests.
 ********************************************************************************/

void
test_hton_queue_property_with_OFPQT_NONE() {
  struct ofp_queue_prop_header dst;
  struct ofp_queue_prop_header src;

  memset( &src, 0, sizeof( struct ofp_queue_prop_header ) );
  memset( &dst, 0, sizeof( struct ofp_queue_prop_header ) );

  src.property = OFPQT_NONE;
  src.len = 8;

  hton_queue_property( &dst, &src );

  assert_int_equal( dst.property, htons( src.property ) );
  assert_int_equal( dst.len, htons( src.len ) );
}


void
test_hton_queue_property_with_OFPQT_MIN_RATE() {
  struct ofp_queue_prop_min_rate dst;
  struct ofp_queue_prop_min_rate src;

  memset( &src, 0, sizeof( struct ofp_queue_prop_min_rate ) );
  memset( &dst, 0, sizeof( struct ofp_queue_prop_min_rate ) );

  src.prop_header.property = OFPQT_MIN_RATE;
  src.prop_header.len = 16;
  src.rate = 500;

  hton_queue_property( ( struct ofp_queue_prop_header * ) &dst, ( struct ofp_queue_prop_header * ) &src );

  assert_int_equal( dst.prop_header.property, htons( src.prop_header.property ) );
  assert_int_equal( dst.prop_header.len, htons( src.prop_header.len ) );
  assert_int_equal( dst.rate, htons( src.rate ) );
}


/********************************************************************************
 * ntoh_packet_queue() tests.
 ********************************************************************************/

void
test_ntoh_packet_queue_with_single_OFPQT_NONE() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header ) );

  struct ofp_packet_queue *src = xmalloc( length );
  struct ofp_packet_queue *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->queue_id = htonl( 3 );
  src->len = htons( length );
  struct ofp_queue_prop_header *ph_src = src->properties;
  ph_src->property = htons( OFPQT_NONE );
  ph_src->len = htons( 8 );

  ntoh_packet_queue( dst, src );

  struct ofp_queue_prop_header *ph_dst = dst->properties;

  assert_int_equal( ( int ) htonl( dst->queue_id ), ( int ) src->queue_id );
  assert_int_equal( htons( dst->len ), src->len );
  assert_int_equal( htons( ph_dst->property ), ph_src->property );
  assert_int_equal( htons( ph_dst->len ), ph_src->len );

  xfree( src );
  xfree( dst );
}


void
test_ntoh_packet_queue_with_single_OFPQT_MIN_RATE() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_min_rate ) );

  struct ofp_packet_queue *src = xmalloc( length );
  struct ofp_packet_queue *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->queue_id = htonl( 3 );
  src->len = htons( length );
  struct ofp_queue_prop_min_rate *pm_src = ( struct ofp_queue_prop_min_rate * ) src->properties;
  pm_src->prop_header.property = htons( OFPQT_MIN_RATE );
  pm_src->prop_header.len = htons( 16 );
  pm_src->rate = htons( 500 );

  ntoh_packet_queue( dst, src );

  struct ofp_queue_prop_min_rate *pm_dst = ( struct ofp_queue_prop_min_rate * ) dst->properties;

  assert_int_equal( ( int ) htonl( dst->queue_id ), ( int ) src->queue_id );
  assert_int_equal( htons( dst->len ), src->len );
  assert_int_equal( htons( pm_dst->prop_header.property ), pm_src->prop_header.property );
  assert_int_equal( htons( pm_dst->prop_header.len ), pm_src->prop_header.len );
  assert_int_equal( htons( pm_dst->rate ), pm_src->rate );

  xfree( src );
  xfree( dst );
}


void
test_ntoh_packet_queue_with_OFPQT_NONE_and_OFPQT_MIN_RATE() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header ) + sizeof( struct ofp_queue_prop_min_rate ) );

  struct ofp_packet_queue *src = xmalloc( length );
  struct ofp_packet_queue *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->queue_id = htonl( 3 );
  src->len = htons( length );
  struct ofp_queue_prop_header *ph_src = src->properties;
  ph_src->property = htons( OFPQT_NONE );
  ph_src->len = htons( 8 );
  struct ofp_queue_prop_min_rate *pm_src = ( struct ofp_queue_prop_min_rate * ) ( ( char * ) src->properties + sizeof( struct ofp_queue_prop_header ) );
  pm_src->prop_header.property = htons( OFPQT_MIN_RATE );
  pm_src->prop_header.len = htons( 16 );
  pm_src->rate = htons( 500 );

  ntoh_packet_queue( dst, src );

  struct ofp_queue_prop_header *ph_dst = dst->properties;
  struct ofp_queue_prop_min_rate *pm_dst = ( struct ofp_queue_prop_min_rate * ) ( ( char * ) dst->properties + sizeof( struct ofp_queue_prop_header ) );

  assert_int_equal( ( int ) htonl( dst->queue_id ), ( int ) src->queue_id );
  assert_int_equal( htons( dst->len ), src->len );
  assert_int_equal( htons( ph_dst->property ), ph_src->property );
  assert_int_equal( htons( ph_dst->len ), ph_src->len );
  assert_int_equal( htons( pm_dst->prop_header.property ),
                    pm_src->prop_header.property );
  assert_int_equal( htons( pm_dst->prop_header.len ),
                    pm_src->prop_header.len );
  assert_int_equal( htons( pm_dst->rate ), pm_src->rate );

  xfree( src );
  xfree( dst );
}


/********************************************************************************
 * hton_packet_queue() tests.
 ********************************************************************************/

void
test_hton_packet_queue_with_single_OFPQT_NONE() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header ) );

  struct ofp_packet_queue *src = xmalloc( length );
  struct ofp_packet_queue *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->queue_id = 3;
  src->len = length;
  struct ofp_queue_prop_header *ph_src = src->properties;
  ph_src->property = OFPQT_NONE;
  ph_src->len = 8;

  hton_packet_queue( dst, src );

  struct ofp_queue_prop_header *ph_dst = dst->properties;

  assert_int_equal( ( int ) dst->queue_id, ( int ) htonl( src->queue_id ) );
  assert_int_equal( dst->len, htons( src->len ) );
  assert_int_equal( ph_dst->property, htons( ph_src->property ) );
  assert_int_equal( ph_dst->len, htons( ph_src->len ) );

  xfree( src );
  xfree( dst );
}


void
test_hton_packet_queue_with_single_OFPQT_MIN_RATE() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_min_rate ) );

  struct ofp_packet_queue *src = xmalloc( length );
  struct ofp_packet_queue *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->queue_id = 3;
  src->len = length;
  struct ofp_queue_prop_min_rate *pm_src = ( struct ofp_queue_prop_min_rate * ) src->properties;
  pm_src->prop_header.property = OFPQT_MIN_RATE;
  pm_src->prop_header.len = 16;
  pm_src->rate = 500;

  hton_packet_queue( dst, src );

  struct ofp_queue_prop_min_rate *pm_dst = ( struct ofp_queue_prop_min_rate * ) dst->properties;

  assert_int_equal( ( int ) dst->queue_id, ( int ) htonl( src->queue_id ) );
  assert_int_equal( dst->len, htons( src->len ) );
  assert_int_equal( pm_dst->prop_header.property, htons( pm_src->prop_header.property ) );
  assert_int_equal( pm_dst->prop_header.len, htons( pm_src->prop_header.len ) );
  assert_int_equal( pm_dst->rate, htons( pm_src->rate ) );

  xfree( src );
  xfree( dst );
}


void
test_hton_packet_queue_with_OFPQT_NONE_and_OFPQT_MIN_RATE() {
  uint16_t length = ( uint16_t ) ( offsetof( struct ofp_packet_queue, properties ) + sizeof( struct ofp_queue_prop_header ) + sizeof( struct ofp_queue_prop_min_rate ) );

  struct ofp_packet_queue *src = xmalloc( length );
  struct ofp_packet_queue *dst = xmalloc( length );

  memset( src, 0, length );
  memset( dst, 0, length );

  src->queue_id = 3;
  src->len = length;
  struct ofp_queue_prop_header *ph_src = src->properties;
  ph_src->property = OFPQT_NONE;
  ph_src->len = 8;
  struct ofp_queue_prop_min_rate *pm_src = ( struct ofp_queue_prop_min_rate * ) ( ( char * ) src->properties + sizeof( struct ofp_queue_prop_header ) );
  pm_src->prop_header.property = OFPQT_MIN_RATE;
  pm_src->prop_header.len = 16;
  pm_src->rate = 500;

  hton_packet_queue( dst, src );

  struct ofp_queue_prop_header *ph_dst = dst->properties;
  struct ofp_queue_prop_min_rate *pm_dst = ( struct ofp_queue_prop_min_rate * ) ( ( char * ) dst->properties + sizeof( struct ofp_queue_prop_header ) );

  assert_int_equal( ( int ) dst->queue_id, ( int ) htonl( src->queue_id ) );
  assert_int_equal( dst->len, htons( src->len ) );
  assert_int_equal( ph_dst->property, htons( ph_src->property ) );
  assert_int_equal( ph_dst->len, htons( ph_src->len ) );
  assert_int_equal( pm_dst->prop_header.property,
                    htons( pm_src->prop_header.property ) );
  assert_int_equal( pm_dst->prop_header.len,
                    htons( pm_src->prop_header.len ) );
  assert_int_equal( pm_dst->rate, htons( pm_src->rate ) );

  xfree( src );
  xfree( dst );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  // FIXME: mockanize in setup()
  die = mock_die;

  const UnitTest tests[] = {
    unit_test( test_ntoh_match ),
    unit_test( test_ntoh_phy_port ),
    unit_test( test_ntoh_action_output ),
    unit_test( test_ntoh_action_vlan_vid ),
    unit_test( test_ntoh_action_vlan_pcp ),
    unit_test( test_ntoh_action_strip_vlan ),
    unit_test( test_ntoh_action_dl_addr ),
    unit_test( test_ntoh_action_nw_addr ),
    unit_test( test_ntoh_action_nw_tos ),
    unit_test( test_ntoh_action_tp_port ),
    unit_test( test_ntoh_action_enqueue ),
    unit_test( test_ntoh_action_vendor ),
    unit_test( test_ntoh_action_vendor_with_body ),
    unit_test( test_hton_action_vendor ),
    unit_test( test_hton_action_vendor_with_body ),
    unit_test( test_ntoh_action ),
    unit_test( test_ntoh_action_with_undefined_action_type ),
    unit_test( test_hton_action ),
    unit_test( test_hton_action_with_undefined_action_type ),
    unit_test( test_ntoh_flow_stats_without_action ),
    unit_test( test_ntoh_flow_stats_with_single_output_action ),
    unit_test( test_ntoh_flow_stats_with_two_output_actions ),
    unit_test( test_hton_flow_stats_without_action ),
    unit_test( test_hton_flow_stats_with_single_output_action ),
    unit_test( test_hton_flow_stats_with_two_output_actions ),
    unit_test( test_ntoh_aggregate_stats ),
    unit_test( test_ntoh_table_stats ),
    unit_test( test_ntoh_port_stats ),
    unit_test( test_ntoh_queue_stats ),
    unit_test( test_ntoh_queue_property_with_OFPQT_NONE ),
    unit_test( test_ntoh_queue_property_with_OFPQT_MIN_RATE ),
    unit_test( test_hton_queue_property_with_OFPQT_NONE ),
    unit_test( test_hton_queue_property_with_OFPQT_MIN_RATE ),
    unit_test( test_ntoh_packet_queue_with_single_OFPQT_NONE ),
    unit_test( test_ntoh_packet_queue_with_single_OFPQT_MIN_RATE ),
    unit_test( test_ntoh_packet_queue_with_OFPQT_NONE_and_OFPQT_MIN_RATE ),
    unit_test( test_hton_packet_queue_with_single_OFPQT_NONE ),
    unit_test( test_hton_packet_queue_with_single_OFPQT_MIN_RATE ),
    unit_test( test_hton_packet_queue_with_OFPQT_NONE_and_OFPQT_MIN_RATE ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
