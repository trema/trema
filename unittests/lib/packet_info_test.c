/*
 * Unit tests for packet_info functions and macros.
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


#include <assert.h>
#include <netinet/ip.h>
#include <stdio.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "utility.h"
#include "packet_info.h"


/******************************************************************************
 * Mocks.
 ******************************************************************************/

static void ( *original_die )( const char *format, ... );

static void
mock_die( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );

  mock_assert( false, "mock_die", __FILE__, __LINE__ );
}


/******************************************************************************
 * Setup and teardown.
 ******************************************************************************/

static void
setup() {
  original_die = die;
  die = mock_die;
}


static void
teardown() {
  die = original_die;
}


/******************************************************************************
 * Tests.
 ******************************************************************************/

static void
test_calloc_packet_info_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );

  calloc_packet_info( buf );
  assert_true( buf->user_data != NULL );

  free_buffer( buf );
}


static void
test_calloc_packet_info_fails_if_buffer_is_NULL() {
  expect_string( mock_die, output,
                 "Argument of calloc_packet_info must not be NULL." );
  expect_assert_failure( calloc_packet_info( NULL ) );
}


static void
test_free_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );
  assert_true( buf->user_data_free_function != NULL );
  ( *buf->user_data_free_function )( buf );
  assert_true( buf->user_data == NULL );
  assert_true( buf->user_data_free_function == NULL );

  free_buffer( buf );
}


static void
test_packet_type_eth_dix_fails() {
  expect_string( mock_die, output,
                 "Argument of packet_type_eth_dix must not be NULL." );
  expect_assert_failure( packet_type_eth_dix( NULL ) );
}


static void
test_packet_type_eth_dix() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_eth_dix( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= ETH_DIX;
  assert_true( packet_type_eth_dix( buf ) );

  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_eth_dix( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_eth_vtag() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_eth_vtag( buf ) );
  packet_info *packet_info = buf->user_data;
  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_eth_vtag( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_DIX;
  assert_false( packet_type_eth_vtag( buf ) );
  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_8023_RAW;
  assert_false( packet_type_eth_vtag( buf ) );
  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_8023_LLC;
  assert_false( packet_type_eth_vtag( buf ) );
  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_8023_SNAP;
  assert_false( packet_type_eth_vtag( buf ) );
  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_eth_vtag( buf ) );
  packet_info->format = 0;

  free_buffer( buf );
}


static void
test_packet_type_eth_raw() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_eth_raw( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= ETH_8023_RAW;
  assert_true( packet_type_eth_raw( buf ) );

  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_eth_raw( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_eth_llc() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_eth_llc( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= ETH_8023_LLC;
  assert_true( packet_type_eth_llc( buf ) );

  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_eth_llc( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_eth_snap() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_eth_snap( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= ETH_8023_SNAP;
  assert_true( packet_type_eth_snap( buf ) );

  packet_info->format |= ETH_8021Q;
  assert_true( packet_type_eth_snap( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_ether() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_ether( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= ETH_DIX;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_8023_RAW;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_8023_LLC;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  packet_info->format |= ETH_8023_SNAP;
  assert_true( packet_type_ether( buf ) );
  packet_info->format = 0;

  free_buffer( buf );
}


static void
test_packet_type_arp() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_arp( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= NW_ARP;
  assert_true( packet_type_arp( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_rarp() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_rarp( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= NW_RARP;
  assert_true( packet_type_rarp( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_ipv4() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_ipv4( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= NW_IPV4;
  assert_true( packet_type_ipv4( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_icmpv4() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_icmpv4( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= NW_ICMPV4;
  assert_true( packet_type_icmpv4( buf ) );

  packet_info->format |= NW_IPV4;
  assert_true( packet_type_icmpv4( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_ipv4_udp() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_ipv4_udp( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= TP_UDP;
  assert_false( packet_type_ipv4_udp( buf ) );

  packet_info->format |= NW_IPV4;
  assert_true( packet_type_ipv4_udp( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_ipv4_tcp() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_ipv4_tcp( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= TP_TCP;
  assert_false( packet_type_ipv4_tcp( buf ) );

  packet_info->format |= NW_IPV4;
  assert_true( packet_type_ipv4_tcp( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_lldp() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_lldp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_LLDP;
  assert_true( packet_type_lldp( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_igmp() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_igmp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_IGMP;
  assert_true( packet_type_igmp( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_ipv4_etherip() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_ipv4_etherip( buf ) );

  packet_info *packet_info = buf->user_data;
  packet_info->format |= TP_ETHERIP;
  assert_false( packet_type_ipv4_etherip( buf ) );

  packet_info->format |= NW_IPV4;
  assert_true( packet_type_ipv4_etherip( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_igmp_membership_query() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_igmp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_IGMP;
  packet_info->igmp_type = IGMP_TYPE_MEMBERSHIP_QUERY;

  assert_true( packet_type_igmp_membership_query( buf ) );
  assert_false( packet_type_igmp_v1_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_leave_group( buf ) );
  assert_false( packet_type_igmp_v3_membership_report( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_igmp_v1_membership_report() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_igmp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_IGMP;
  packet_info->igmp_type = IGMP_TYPE_V1_MEMBERSHIP_REPORT;

  assert_false( packet_type_igmp_membership_query( buf ) );
  assert_true( packet_type_igmp_v1_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_leave_group( buf ) );
  assert_false( packet_type_igmp_v3_membership_report( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_igmp_v2_membership_report() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_igmp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_IGMP;
  packet_info->igmp_type = IGMP_TYPE_V2_MEMBERSHIP_REPORT;

  assert_false( packet_type_igmp_membership_query( buf ) );
  assert_false( packet_type_igmp_v1_membership_report( buf ) );
  assert_true( packet_type_igmp_v2_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_leave_group( buf ) );
  assert_false( packet_type_igmp_v3_membership_report( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_igmp_v2_leave_group() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_igmp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_IGMP;
  packet_info->igmp_type = IGMP_TYPE_V2_LEAVE_GROUP;

  assert_false( packet_type_igmp_membership_query( buf ) );
  assert_false( packet_type_igmp_v1_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_membership_report( buf ) );
  assert_true( packet_type_igmp_v2_leave_group( buf ) );
  assert_false( packet_type_igmp_v3_membership_report( buf ) );

  free_buffer( buf );
}


static void
test_packet_type_igmp_v3_membership_report() {
  buffer *buf = alloc_buffer_with_length( sizeof( struct iphdr ) );
  calloc_packet_info( buf );

  assert_false( packet_type_igmp( buf ) );

  packet_info *packet_info = buf->user_data;

  packet_info->format |= NW_IGMP;
  packet_info->igmp_type = IGMP_TYPE_V3_MEMBERSHIP_REPORT;

  assert_false( packet_type_igmp_membership_query( buf ) );
  assert_false( packet_type_igmp_v1_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_membership_report( buf ) );
  assert_false( packet_type_igmp_v2_leave_group( buf ) );
  assert_true( packet_type_igmp_v3_membership_report( buf ) );

  free_buffer( buf );
}


/******************************************************************************
 * Run tests.
 ******************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_calloc_packet_info_succeeds ),
    unit_test_setup_teardown( test_calloc_packet_info_fails_if_buffer_is_NULL,
                              setup, teardown ),

    unit_test( test_free_buffer_succeeds ),

    unit_test_setup_teardown( test_packet_type_eth_dix_fails,
                              setup, teardown ),
    unit_test( test_packet_type_eth_dix ),
    unit_test( test_packet_type_eth_vtag ),
    unit_test( test_packet_type_eth_raw ),
    unit_test( test_packet_type_eth_llc ),
    unit_test( test_packet_type_eth_snap ),
    unit_test( test_packet_type_ether ),

    unit_test( test_packet_type_arp ),
    unit_test( test_packet_type_rarp ),
    unit_test( test_packet_type_ipv4 ),
    unit_test( test_packet_type_icmpv4 ),

    unit_test( test_packet_type_ipv4_udp ),
    unit_test( test_packet_type_ipv4_tcp ),

    unit_test( test_packet_type_lldp ),
    unit_test( test_packet_type_igmp ),
    unit_test( test_packet_type_ipv4_etherip ),

    unit_test( test_packet_type_igmp_membership_query ),
    unit_test( test_packet_type_igmp_v1_membership_report ),
    unit_test( test_packet_type_igmp_v2_membership_report ),
    unit_test( test_packet_type_igmp_v2_leave_group ),
    unit_test( test_packet_type_igmp_v3_membership_report ),

  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
