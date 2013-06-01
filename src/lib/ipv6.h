/*
 * IPv6 header definitions
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


#ifndef IPV6_H
#define IPV6_H


#define IPV6_ADDRLEN 16


typedef struct  {
  uint32_t hdrctl;
  uint16_t plen;
  uint8_t nexthdr;
  uint8_t hoplimit;
  uint8_t saddr[ IPV6_ADDRLEN ];
  uint8_t daddr[ IPV6_ADDRLEN ];
} ipv6_header_t;


#endif // IPV6_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
