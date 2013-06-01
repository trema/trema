/*
 * IPv4 header definitions
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


#ifndef IPV4_H
#define IPV4_H


#include "buffer.h"


typedef struct  {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  uint8_t ihl:4;
  uint8_t version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
  uint8_t version:4;
  uint8_t ihl:4;
#endif
  uint8_t tos;
  uint16_t tot_len;
  uint16_t id;
  uint16_t frag_off;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t csum;
  uint32_t saddr;
  uint32_t daddr;
} ipv4_header_t;


#define IPV4_ADDRLEN 4


#define IPV4_IS_CLASSDE( _addr ) ( ( ( _addr ) & 0xe0000000UL ) == 0xe0000000UL )
#define IPV4_IS_LOOPBACK( _addr ) ( ( ( _addr ) & 0xffffff00UL ) == 0x7f000000UL )
#define IPV4_IS_LIMITEDBC( _addr ) ( ( _addr ) == 0xffffffffUL )


#endif // IPV4_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
