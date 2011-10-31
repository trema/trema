/*
 * Functions for accessing commonly-used header fields values.
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


#ifndef PACKET_INFO_H
#define PACKET_INFO_H


#include "arp.h"
#include "checks.h"
#include "bool.h"
#include "ether.h"
#include "icmp.h"
#include "ipv4.h"
#include "tcp.h"
#include "udp.h"


enum {
  ETH_DIX = 0x00000001,
  ETH_8023_RAW = 0x00000002,
  ETH_8023_LLC = 0x00000004,
  ETH_8023_SNAP = 0x00000008,
  ETH_8021Q = 0x00000010,
  NW_IPV4 = 0x00000100,
  NW_ICMPV4 = 0x00000200,
  NW_IPV6 = 0x00000400,
  NW_ICMPV6 = 0x00000800,
  NW_ARP = 0x00001000,
  TP_TCP = 0x00010000,
  TP_UDP = 0x00020000,

  ETH_VTAG_DIX = ETH_8021Q | ETH_DIX,
  ETH_VTAG_RAW = ETH_8021Q | ETH_8023_RAW,
  ETH_VTAG_LLC = ETH_8021Q | ETH_8023_LLC,
  ETH_VTAG_SNAP = ETH_8021Q | ETH_8023_SNAP,
  ETH_ARP = ETH_DIX | NW_ARP,
  ETH_IPV4 = ETH_DIX | NW_IPV4,
  ETH_IPV4_ICMPV4 = ETH_IPV4 | NW_ICMPV4,
  ETH_IPV4_TCP = ETH_IPV4 | TP_TCP,
  ETH_IPV4_UDP = ETH_IPV4 | TP_UDP,
  ETH_VTAG_ARP = ETH_VTAG_DIX | NW_ARP,
  ETH_VTAG_IPV4 = ETH_VTAG_DIX | NW_IPV4,
  ETH_VTAG_IPV4_ICMPV4 = ETH_VTAG_IPV4 | NW_ICMPV4,
  ETH_VTAG_IPV4_TCP = ETH_VTAG_IPV4 | TP_TCP,
  ETH_VTAG_IPV4_UDP = ETH_VTAG_IPV4 | TP_UDP,
  ETH_SNAP_ARP = ETH_8023_SNAP | NW_ARP,
  ETH_SNAP_IPV4 = ETH_8023_SNAP | NW_IPV4,
  ETH_SNAP_IPV4_ICMPV4 = ETH_SNAP_IPV4 | NW_ICMPV4,
  ETH_SNAP_IPV4_TCP = ETH_SNAP_IPV4 | TP_TCP,
  ETH_SNAP_IPV4_UDP = ETH_SNAP_IPV4 | TP_UDP,
  ETH_VTAG_SNAP_ARP = ETH_VTAG_SNAP | NW_ARP,
  ETH_VTAG_SNAP_IPV4 = ETH_VTAG_SNAP | NW_IPV4,
  ETH_VTAG_SNAP_IPV4_ICMPV4 = ETH_VTAG_SNAP_IPV4 | NW_ICMPV4,
  ETH_VTAG_SNAP_IPV4_TCP = ETH_VTAG_SNAP_IPV4 | TP_TCP,
  ETH_VTAG_SNAP_IPV4_UDP = ETH_VTAG_SNAP_IPV4 | TP_UDP,

  NW_MASK = NW_IPV4 | NW_ICMPV4 | NW_IPV6 | NW_ICMPV6 | NW_ARP,
};


enum {
  SNAP_LLC_LENGTH = 3,
  SNAP_OUI_LENGTH = 3,
};


typedef struct {
  uint32_t format;
  
  uint8_t eth_macda[ ETH_ADDRLEN ];
  uint8_t eth_macsa[ ETH_ADDRLEN ];
  uint16_t eth_type;

  uint16_t vlan_tci;
  uint16_t vlan_tpid;
  uint8_t vlan_prio;
  uint8_t vlan_cfi;
  uint16_t vlan_vid;

  uint8_t snap_llc[ SNAP_LLC_LENGTH ];
  uint8_t snap_oui[ SNAP_OUI_LENGTH ];
  uint16_t snap_type;

  uint16_t arp_ar_hrd;
  uint16_t arp_ar_pro;
  uint8_t arp_ar_hln;
  uint8_t arp_ar_pln;
  uint16_t arp_ar_op;
  uint8_t arp_sha[ ETH_ADDRLEN ];
  uint32_t arp_spa;
  uint8_t arp_tha[ ETH_ADDRLEN ];
  uint32_t arp_tpa;

  uint8_t ipv4_version;
  uint8_t ipv4_ihl;
  uint8_t ipv4_tos;
  uint16_t ipv4_tot_len;
  uint16_t ipv4_id;
  uint16_t ipv4_frag_off;
  uint8_t ipv4_ttl;
  uint8_t ipv4_protocol;
  uint16_t ipv4_checksum;
  uint32_t ipv4_saddr;
  uint32_t ipv4_daddr;

  uint8_t icmpv4_type;
  uint8_t icmpv4_code;
  uint16_t icmpv4_checksum;
  uint16_t icmpv4_id;
  uint16_t icmpv4_seq;
  uint32_t icmpv4_gateway;

  uint16_t tcp_src_port;
  uint16_t tcp_dst_port;
  uint32_t tcp_seq_no;
  uint32_t tcp_ack_no;
  uint8_t tcp_offset;
  uint8_t tcp_flags;
  uint16_t tcp_window;
  uint16_t tcp_checksum;
  uint16_t tcp_urgent;

  uint16_t udp_src_port;
  uint16_t udp_dst_port;
  uint16_t udp_len;
  uint16_t udp_checksum;

  void *l2_header;
  void *l2_payload;
  void *l3_header;
  void *l3_payload;
  void *l4_header;
  void *l4_payload;
} packet_info;


bool parse_packet( buffer *buf );
int write_packet( packet_info *info, char *buffer, int buffer_length );

void calloc_packet_info( buffer *frame );
void free_packet_info( buffer *frame );
packet_info get_packet_info( const buffer *frame );

bool packet_type_eth_dix( const buffer *frame );
bool packet_type_eth_vtag( const buffer *frame );
bool packet_type_eth_raw( const buffer *frame );
bool packet_type_eth_llc( const buffer *frame );
bool packet_type_eth_snap( const buffer *frame );
bool packet_type_ether( const buffer *frame );
bool packet_type_arp( const buffer *frame );
bool packet_type_ipv4( const buffer *frame );
bool packet_type_icmpv4( const buffer *frame );
bool packet_type_ipv4_tcp( const buffer *frame );
bool packet_type_ipv4_udp( const buffer *frame );


#endif // PACKET_INFO_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
