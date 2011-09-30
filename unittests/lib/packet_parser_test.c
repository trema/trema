/*
 * Unit tests for packet_parser functions and macros.
 *
 * Author: Kazuya Suzuki
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



#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pcap.h>
#include <netinet/ip.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "packet_info.h"
#include "wrapper.h"


/******************************************************************************
 * Helper functions.
 ******************************************************************************/

static buffer *
store_packet_to_buffer( const char *filename ) {
  assert( filename != NULL );

  FILE *fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    // "Can't open a file of test data."
    return NULL;
  }
  
  // Skip 
  if ( fseek( fp, sizeof( struct pcap_file_header ) + sizeof( uint32_t ) * 2,
              SEEK_CUR ) != 0 ) {
    fclose( fp );
    return NULL;
  }  
  
  uint32_t length[ 2 ];
  size_t size = fread( &length, 1, sizeof( length ), fp );
  if ( size < sizeof( length ) ) {
    fclose( fp );
    return NULL;
  }  

  buffer *buffer = alloc_buffer();
  if ( buffer == NULL ) {
    fclose( fp );
    return NULL;
  }
  buffer->length = length[ 0 ];
  buffer->data = xcalloc( 1, buffer->length );
  size = fread( buffer->data, 1, buffer->length, fp );
  if ( size < buffer->length ) {
    free_buffer( buffer );
    fclose( fp );
    return NULL;
  }  

  fclose( fp );
  return buffer;
}


/******************************************************************************
 * Test functions.
 ******************************************************************************/

static void
test_parse_packet_snap_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/ipx.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_8023_SNAP );
  
  u_char macda[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  u_char macsa[] = { 0x00, 0x19, 0xdb, 0x17, 0xb9, 0x6f };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_true( packet_info->eth_type < ETH_MTU );

  u_char llc[] = { 0xe0, 0xe0, 0x03 };
  u_char oui[] = { 0xff, 0xff, 0x00 };
  assert_memory_equal( packet_info->snap_llc, llc, SNAP_LLC_LENGTH );
  assert_memory_equal( packet_info->snap_oui, oui, SNAP_OUI_LENGTH );
  assert_int_equal( packet_info->snap_type, 0xb700 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l2_payload );
  assert_int_equal( sample, 0x0400 );

  free_buffer( buffer );
}


static void
test_parse_packet_arp_request_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/arp_req.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );
  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_ARP );

  u_char macda[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  u_char macsa[] = { 0x8c, 0x89, 0xa5, 0x16, 0x22, 0x09 };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_ARP );

  assert_int_equal( packet_info->arp_ar_hrd, 0x0001 );
  assert_int_equal( packet_info->arp_ar_pro, 0x0800 );
  assert_int_equal( packet_info->arp_ar_hln, 6 );
  assert_int_equal( packet_info->arp_ar_pln, 4 );
  assert_int_equal( packet_info->arp_ar_op, 1 );
  assert_int_equal( packet_info->arp_spa, 0xc0a8642c );
  assert_memory_equal( packet_info->arp_sha, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->arp_tpa, 0xc0a8642b );
  u_char maczero[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  assert_memory_equal( packet_info->arp_tha, maczero, ETH_ADDRLEN );  

  free_buffer( buffer );
}


static void
test_parse_packet_udp_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/udp.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_IPV4_UDP );

  u_char macda[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  u_char macsa[] = { 0x00, 0x21, 0x85, 0x91, 0x92, 0xdb };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );
  
  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x4c );
  assert_int_equal( packet_info->ipv4_id, 0x48d8 );
  assert_int_equal( packet_info->ipv4_frag_off, 0 );
  assert_int_equal( packet_info->ipv4_ttl, 0x80 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_UDP );
  assert_int_equal( packet_info->ipv4_checksum, 0x6fab );
  assert_int_equal( packet_info->ipv4_saddr, 0x0a3835af );
  assert_int_equal( packet_info->ipv4_daddr, 0x0a3837ff );

  assert_int_equal( packet_info->udp_src_port, 61616 );
  assert_int_equal( packet_info->udp_dst_port, 23499 );
  assert_int_equal( packet_info->udp_len, 0x38 );
  assert_int_equal( packet_info->udp_checksum, 0x04a1 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l4_payload );
  assert_int_equal( sample, 0xf937 );

  free_buffer( buffer );
}


static void
test_parse_packet_udp_fragmented_head_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/udp_frag_head.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_IPV4_UDP );

  u_char macda[] = { 0x8c, 0x89, 0xa5, 0x15, 0x84, 0xcb };
  u_char macsa[] = { 0x8c, 0x89, 0xa5, 0x16, 0x22, 0x09 };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );
  
  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x05dc );
  assert_int_equal( packet_info->ipv4_id, 0x2b33 );
  assert_int_equal( packet_info->ipv4_frag_off, 0x2000 );
  assert_int_equal( packet_info->ipv4_ttl, 0x40 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_UDP );
  assert_int_equal( packet_info->ipv4_checksum, 0xe035 );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a8642c );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a8642b );

  assert_int_equal( packet_info->udp_src_port, 0xa2c7 );
  assert_int_equal( packet_info->udp_dst_port, 0x1f90 );
  assert_int_equal( packet_info->udp_len, 0x2330 );
  assert_int_equal( packet_info->udp_checksum, 0x8749 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l4_payload );
  assert_int_equal( sample, 0x8020 );

  free_buffer( buffer );
}


static void
test_parse_packet_udp_fragmented_next_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/udp_frag_next.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_IPV4 );

  u_char macda[] = { 0x8c, 0x89, 0xa5, 0x15, 0x84, 0xcb };
  u_char macsa[] = { 0x8c, 0x89, 0xa5, 0x16, 0x22, 0x09 };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );
  
  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x05dc );
  assert_int_equal( packet_info->ipv4_id, 0x2b33 );
  assert_int_equal( packet_info->ipv4_frag_off, 0x20b9 );
  assert_int_equal( packet_info->ipv4_ttl, 0x40 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_UDP );
  assert_int_equal( packet_info->ipv4_checksum, 0xdf7c );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a8642c );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a8642b );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l3_payload );
  assert_int_equal( sample, 0x9102 );

  // L4 parsing phase is skipped for fragmented packets.
  assert_int_equal( packet_info->udp_src_port, 0 );
  assert_int_equal( packet_info->udp_dst_port, 0 );
  assert_int_equal( packet_info->udp_len, 0 );
  assert_int_equal( packet_info->udp_checksum, 0 );

  assert_int_equal( packet_info->l4_payload, NULL );

  free_buffer( buffer );
}


static void
test_parse_packet_tcp_syn_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/tcp_syn.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_IPV4_TCP );

  u_char macda[] = { 0x00, 0x16, 0x17, 0x00, 0x43, 0xf3 };
  u_char macsa[] = { 0x8c, 0x89, 0xa5, 0x15, 0x84, 0xcb };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );
  
  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0x10 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x003c );
  assert_int_equal( packet_info->ipv4_id, 0x5551 );
  assert_int_equal( packet_info->ipv4_frag_off, 0x4000 );
  assert_int_equal( packet_info->ipv4_ttl, 0x40 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_TCP );
  assert_int_equal( packet_info->ipv4_checksum, 0x9afd );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a8642b );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a864e1 );

  assert_int_equal( packet_info->tcp_src_port, 0xad49 );
  assert_int_equal( packet_info->tcp_dst_port, 0x0050 );
  assert_int_equal( packet_info->tcp_seq_no, 0x51de9851 );
  assert_int_equal( packet_info->tcp_ack_no, 0 );
  assert_int_equal( packet_info->tcp_offset, 0xa );
  assert_int_equal( packet_info->tcp_flags, 0x02 );
  assert_int_equal( packet_info->tcp_window, 0x16d0 );
  assert_int_equal( packet_info->tcp_checksum, 0x76bb );
  assert_int_equal( packet_info->tcp_urgent, 0 );

  assert_int_equal( packet_info->l4_payload, NULL );

  free_buffer( buffer );
}


static void
test_parse_packet_tcp_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/tcp.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_IPV4_TCP );

  u_char macda[] = { 0x8c, 0x89, 0xa5, 0x15, 0x84, 0xcb };
  u_char macsa[] = { 0x00, 0x16, 0x17, 0x00, 0x43, 0xf3 };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );
  
  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x01dd );
  assert_int_equal( packet_info->ipv4_id, 0x0399 );
  assert_int_equal( packet_info->ipv4_frag_off, 0x4000 );
  assert_int_equal( packet_info->ipv4_ttl, 0x40 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_TCP );
  assert_int_equal( packet_info->ipv4_checksum, 0xeb24 );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a864e1 );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a8642b );

  assert_int_equal( packet_info->tcp_src_port, 0x0050 );
  assert_int_equal( packet_info->tcp_dst_port, 0xad49 );
  assert_int_equal( packet_info->tcp_seq_no, 0x20656b68 );
  assert_int_equal( packet_info->tcp_ack_no, 0x51de986e );
  assert_int_equal( packet_info->tcp_offset, 0x8 );
  assert_int_equal( packet_info->tcp_flags, 0x18 );
  assert_int_equal( packet_info->tcp_window, 0x2086 );
  assert_int_equal( packet_info->tcp_checksum, 0x4c2d );
  assert_int_equal( packet_info->tcp_urgent, 0 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l4_payload );
  assert_int_equal( sample, 0x4854 );

  free_buffer( buffer );
}


static void
test_parse_packet_icmpv4_echo_request_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/icmp_echo_req.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_IPV4_ICMPV4 );

  u_char macda[] = { 0x8c, 0x89, 0xa5, 0x15, 0x84, 0xcb };
  u_char macsa[] = { 0x8c, 0x89, 0xa5, 0x16, 0x22, 0x09 };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );
  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );

  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x54 );
  assert_int_equal( packet_info->ipv4_id, 0 );
  assert_int_equal( packet_info->ipv4_frag_off, 0x4000 );
  assert_int_equal( packet_info->ipv4_ttl, 0x40 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_ICMP );
  assert_int_equal( packet_info->ipv4_checksum, 0xf100 );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a8642c );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a8642b );

  assert_int_equal( packet_info->icmpv4_type, ICMP_TYPE_ECHOREQ );
  assert_int_equal( packet_info->icmpv4_code, 0 );
  assert_int_equal( packet_info->icmpv4_id, 1076 );
  assert_int_equal( packet_info->icmpv4_seq, 1 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l4_payload );
  assert_int_equal( sample, 0xa0a9 );

  free_buffer( buffer );
}


static void
test_parse_packet_vtag_icmpv4_echo_request_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/vtag_icmp_echo_req.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_VTAG_IPV4_ICMPV4 );

  u_char macda[] = { 0x00, 0x13, 0xd3, 0x40, 0x2e, 0x22 };
  u_char macsa[] = { 0x00, 0x1f, 0x3c, 0x48, 0xad, 0x3a };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );

  assert_int_equal( packet_info->vlan_tci, 0x0f9f );
  assert_int_equal( packet_info->vlan_tpid, ETH_ETHTYPE_TPID );
  assert_int_equal( packet_info->vlan_prio, 0 );
  assert_int_equal( packet_info->vlan_cfi, 0 );
  assert_int_equal( packet_info->vlan_vid, 0x0f9f );

  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );

  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x3c );
  assert_int_equal( packet_info->ipv4_id, 0x8c1b );
  assert_int_equal( packet_info->ipv4_frag_off, 0 );
  assert_int_equal( packet_info->ipv4_ttl, 0x80 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_ICMP );
  assert_int_equal( packet_info->ipv4_checksum, 0xed09 );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a8204a );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a82001 );

  assert_int_equal( packet_info->icmpv4_type, ICMP_TYPE_ECHOREQ );
  assert_int_equal( packet_info->icmpv4_code, 0 );
  assert_int_equal( packet_info->icmpv4_id, 1024 );
  assert_int_equal( packet_info->icmpv4_seq, 24576 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l4_payload );
  assert_int_equal( sample, 0x6162 );

  free_buffer( buffer );
}


static void
test_parse_packet_vtag_icmpv4_echo_reply_succeeds() {
  const char filename[] = "./unittests/lib/test_packets/vtag_icmp_echo_rep.cap";
  buffer *buffer = store_packet_to_buffer( filename );

  assert_true( parse_packet( buffer ) );

  packet_info *packet_info = buffer->user_data;

  assert_int_equal( packet_info->format, ETH_VTAG_IPV4_ICMPV4 );

  u_char macda[] = { 0x00, 0x1f, 0x3c, 0x48, 0xad, 0x3a };
  u_char macsa[] = { 0x00, 0x13, 0xd3, 0x40, 0x2e, 0x22 };
  assert_memory_equal( packet_info->eth_macda, macda, ETH_ADDRLEN );
  assert_memory_equal( packet_info->eth_macsa, macsa, ETH_ADDRLEN );

  assert_int_equal( packet_info->vlan_tci, 0x6f9f );
  assert_int_equal( packet_info->vlan_tpid, ETH_ETHTYPE_TPID );
  assert_int_equal( packet_info->vlan_prio, 3 );
  assert_int_equal( packet_info->vlan_cfi, 0 );
  assert_int_equal( packet_info->vlan_vid, 0x0f9f );

  assert_int_equal( packet_info->eth_type, ETH_ETHTYPE_IPV4 );

  assert_int_equal( packet_info->ipv4_version, 4 );
  assert_int_equal( packet_info->ipv4_ihl, 5 );
  assert_int_equal( packet_info->ipv4_tos, 0 );
  assert_int_equal( packet_info->ipv4_tot_len, 0x3c );
  assert_int_equal( packet_info->ipv4_id, 0x1652 );
  assert_int_equal( packet_info->ipv4_frag_off, 0 );
  assert_int_equal( packet_info->ipv4_ttl, 0x40 );
  assert_int_equal( packet_info->ipv4_protocol, IPPROTO_ICMP );
  assert_int_equal( packet_info->ipv4_checksum, 0xa2d3 );
  assert_int_equal( packet_info->ipv4_saddr, 0xc0a82001 );
  assert_int_equal( packet_info->ipv4_daddr, 0xc0a8204a );

  assert_int_equal( packet_info->icmpv4_type, ICMP_TYPE_ECHOREP );
  assert_int_equal( packet_info->icmpv4_code, 0 );
  assert_int_equal( packet_info->icmpv4_id, 1024 );
  assert_int_equal( packet_info->icmpv4_seq, 24576 );

  uint16_t sample = ntohs( * ( uint16_t * ) packet_info->l4_payload );
  assert_int_equal( sample, 0x6162 );

  free_buffer( buffer );
}


/******************************************************************************
 * Run tests.
 ******************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_parse_packet_snap_succeeds ),

    unit_test( test_parse_packet_arp_request_succeeds ),

    unit_test( test_parse_packet_udp_succeeds ),
    unit_test( test_parse_packet_udp_fragmented_head_succeeds ),
    unit_test( test_parse_packet_udp_fragmented_next_succeeds ),

    unit_test( test_parse_packet_tcp_syn_succeeds ),
    unit_test( test_parse_packet_tcp_succeeds ),

    unit_test( test_parse_packet_icmpv4_echo_request_succeeds ),

    unit_test( test_parse_packet_vtag_icmpv4_echo_request_succeeds ),
    unit_test( test_parse_packet_vtag_icmpv4_echo_reply_succeeds ),
  };
  stub_logger();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
