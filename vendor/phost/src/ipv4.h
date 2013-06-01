/*
  Copyright (C) 2009-2013 NEC Corporation

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _IPV4_H_
#define _IPV4_H_

#include "eth.h"

#define IPV4_VERSION 4
#define IPV4_DEFAULT_TTL 64
#define IPV4_DEFAULT_HLEN 0x05
#define IPV4_DEFAULT_FLAGS 0x40

#define IPV4_VERSION_HLEN_LEN 1
#define IPV4_DSCP_LEN 1
#define IPV4_TOTAL_LEN_LEN 2
#define IPV4_ID_LEN 2
#define IPV4_FRAGMENT_LEN 2 /* flags: 3[bits], offset: 13[bits]*/
#define IPV4_TTL_LEN 1
#define IPV4_PROTOCOL_LEN 1
#define IPV4_CHECKSUM_LEN 2
#define IPV4_ADDR_LEN 4

#define IPV4_PROTOCOL_ICMP 1
#define IPV4_PROTOCOL_TCP 6
#define IPV4_PROTOCOL_UDP 17

typedef struct ipv4 {
    uint32_t src;
    uint32_t dst;
    uint8_t protocol;
    uint8_t hdr_length;
    uint16_t payload_length;
    uint8_t *payload;
} ipv4;


int ipv4_init(uint32_t ip_addr, uint32_t ip_mask);
int ipv4_uninit();
int ipv4_handle_message(eth *eth);
ipv4 *ipv4_create(uint32_t src, uint32_t dst, uint16_t protocol,
                  uint8_t *payload, uint16_t payload_length);
ipv4 *ipv4_create_from_raw(uint8_t *packet, uint32_t length);
int ipv4_set_payload(ipv4 *ip, uint8_t *payload, uint32_t length);
int ipv4_set_payload_nocopy(ipv4 *ip, uint8_t *payload, uint32_t length);
int ipv4_destroy(ipv4 *ip);
uint8_t *ipv4_get_packet(ipv4 *ip, uint8_t *buffer, uint32_t *length);
uint32_t ipv4_get_host_addr();
int ipv4_enable_promiscuous();
int ipv4_disable_promiscuous();

#endif /* _IPV4_H */
