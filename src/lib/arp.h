/*
 * ARP header definitions
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


#ifndef ARP_H
#define ARP_H


#include <stdint.h>
#include "buffer.h"
#include "ether.h"


/**
 * ARP Header Specification
 * For protocol details, please refer RFC 826 at http://tools.ietf.org/html/rfc826
 *
 * TODO: Move the following type definition into packet_info.h and
 *       remove this file.
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
} __attribute__( ( packed ) ) arp_header_t;


#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2
#define ARP_OP_RREQUEST 3
#define ARP_OP_RREPLY 4


#endif // ARP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
