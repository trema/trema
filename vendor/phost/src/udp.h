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

#ifndef _UDP_H_
#define _UDP_H_

#include "ipv4.h"

#define UDP_SRC_PORT_LEN 2
#define UDP_DST_PORT_LEN 2
#define UDP_LEN_LEN 2
#define UDP_CHECKSUM_LEN 2

#define UDP_HDR_LEN 8

typedef struct udp {
    uint16_t src;
    uint16_t dst;
    uint32_t payload_length;
    uint8_t *payload;
} udp;

int udp_init(void *recv_callback);
int udp_set_recv_callback(void *callback);
int udp_handle_message(ipv4 *ip);
udp *udp_create(uint16_t src, uint16_t dst, uint8_t *payload,
                uint32_t payload_length);
udp *udp_create_from_raw(uint8_t *packet, uint32_t length);
int udp_destroy(udp *udp);
uint8_t *udp_get_packet(udp *udp, uint8_t *buffer, uint32_t *length);

#endif /* _UDP_H_ */
