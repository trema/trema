/*
 * ARP header definitions
 *
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
 * @file arp.h
 * This header file contains types for defining a ARP packet
 * @see arp.c
 */

#ifndef ARP_H
#define ARP_H


#include <net/if_arp.h>
#include <stdint.h>
#include "buffer.h"
#include "ether.h"


/**
 * ARP Header specification
 * For details, please refer RFC 826 (protocol description)
 * http://tools.ietf.org/html/rfc826
 */
typedef struct arp_header {
  uint16_t ar_hrd;
  uint16_t ar_pro;
  uint8_t ar_hln;
  uint8_t ar_pln;
  uint16_t ar_op;
  uint8_t sha[ ETH_ADDRLEN ];
  uint32_t sip;
  uint8_t tha[ ETH_ADDRLEN ];
  uint32_t tip;
} __attribute__((packed)) arp_header_t;


bool valid_arp_packet( const buffer *buf );


#endif // ARP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
