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

#ifndef _ETH_H_
#define _ETH_H_

#include <stdint.h>

#define ETH_ADDR_LEN 6
#define ETH_TYPE_LEN 2
#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_ARP 0x0806

typedef struct eth {
    uint8_t src[ETH_ADDR_LEN];
    uint8_t dst[ETH_ADDR_LEN];
    uint16_t type;
    uint32_t length; /* payload length not including header fields */
    uint8_t *payload;
} eth;

extern uint8_t eth_mac_addr_bc[ETH_ADDR_LEN];

eth *eth_create(uint8_t src[ETH_ADDR_LEN], uint8_t dst[ETH_ADDR_LEN],
                uint16_t type, uint8_t *payload, uint32_t length);
eth *eth_create_from_raw(uint8_t *frame, uint32_t length);
int eth_destroy(eth *eth);
int eth_set_src(eth *eth, uint8_t *src);
int eth_set_dst(eth *eth, uint8_t *dst);
int eth_set_type(eth *eth, uint16_t type);
int eth_set_payload(eth *eth, uint8_t *payload, uint32_t length);
int eth_set_payload_nocopy(eth *eth, uint8_t *payload, uint32_t length);

int eth_get_src(eth *eth, uint8_t *src);
int eth_get_dst(eth *eth, uint8_t *dst);
int eth_get_type(eth *eth, uint16_t *type);
int eth_get_payload(eth *eth, uint8_t *payload, uint32_t *length);

uint8_t *eth_get_frame(eth *eth, uint8_t *buffer, uint32_t *length);

char *eth_dump(eth *eth, char *dump);

#endif /* _ETH_H_ */
