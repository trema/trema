/*
 * EtherIP definitions
 *
 * Author: Kazushi SUGYO
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
 * @file etherip.h
 * This source file contain functions for handling EtherIP type packets.
 * @see rfc3378
 */

#ifndef ETHERIP_H
#define ETHERIP_H


/**
 * Protocol NUmbers
 * @see http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xml
 */
#define IPPROTO_ETHERIP 97 /*!<Ethernet-within-IP Encapsulation*/


/**
 * This is the type that specifies EtherIP header definitions
 * @see rfc3378
 */
typedef struct etherip_headr {
  uint16_t version;
} etherip_header;


#define ETHERIP_VERSION ( 0x3 << 12 ) /*!<Protocol version (included reserved field)*/

#endif // ETHERIP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
