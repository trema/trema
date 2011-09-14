/*
 * Author: Naoyoshi Tada
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

/**
 * @file
 *
 * @brief Packet Header Information handling implementation
 *
 * File containing function declarations and type definitions for accessing
 * commonly-used header fields values
 */

#ifndef PACKET_INFO_H
#define PACKET_INFO_H


#include "checks.h"
#include "bool.h"
#include "arp.h"
#include "ether.h"
#include "icmp.h"
#include "ipv4.h"
#include "tcp.h"
#include "udp.h"


/**
 * Packet header information definitions
 */
typedef struct packet_header_info {
  uint16_t ethtype;
  uint8_t nvtags;
  uint8_t ipproto;
  uint32_t nexthop;
  union {
    void *l2;
    ether_header_t *eth;
  } l2_data;
  vlantag_header_t *vtag;
  union {
    void *l3;
    arp_header_t *arp;
    ipv4_header_t *ipv4;
  } l3_data;
  union {
    void *l4;
    icmp_header_t *icmp;
    tcp_header_t *tcp;
    udp_header_t *udp;
  } l4_data;
} packet_header_info;


void free_packet( buffer *buf ) DEPRECATED;
void alloc_packet( buffer *buf );


#define packet_info( buf ) ( ( packet_header_info * ) ( ( buf )->user_data ) )
/*!<Returns pointer to structure of type packet_header_info*/


#endif // PACKET_INFO_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
