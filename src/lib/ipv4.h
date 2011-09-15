/*
 * IPv4 header definitions
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
 * @file
 *
 * @brief Function declarations and type definitions of IPv4 type packets
 */

#ifndef IPV4_H
#define IPV4_H


#include <netinet/ip.h>
#include "buffer.h"


/**
 * IP header definitions
 * @see RFC 791 for detailed protocol description
 * @see http://www.ietf.org/rfc/rfc791.txt
 */
typedef struct iphdr ipv4_header_t;


#define IPV4_ADDRLEN 4 /*!<IPv4 address length*/


#define IPV4_IS_CLASSDE( _addr )     ( ( ( _addr ) & 0xe0000000UL ) == 0xe0000000UL )
/*!<Checks if address corresponds to class D or class E of IPv4 address classes*/
#define IPV4_IS_LOOPBACK( _addr )    ( ( ( _addr ) & 0xffffff00UL ) == 0x7f000000UL )
/*!<Checks if address corresponds to loopback address of IPv4 address classes*/
#define IPV4_IS_LIMITEDBC( _addr )   ( ( _addr ) == 0xffffffffUL )
/*!<Checks if address corresponds to limited broadcast address of IPv4 address classes*/

bool parse_ipv4( buffer *buf );


#endif // IPV4_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
