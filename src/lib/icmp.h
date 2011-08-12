/*
 * ICMP header definitions
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
 * @brief Internet Control Message Protocol(ICMP) header
 *
 * This header file contain type definitions, type codes, error codes of
 * Internet Control Message Protocol(ICMP)
 */

#ifndef ICMP_H
#define ICMP_H


/**
 * This is the type that specifies ICMP header information
 */
typedef struct icmp_header {
  uint8_t type; /*!<ICMP error type*/
  uint8_t code; /*!<ICMP error code*/
  uint16_t csum; /*!<Checksum*/
  union {
    struct {
      uint16_t ident; /*!<Identifier*/
      uint16_t seq; /*!<Sequence Number*/
    } echo;
    uint32_t gateway; /*!<Gateway internet address*/
    uint32_t pad; /*!<Padding for destination Unreachable or Time Exceeded*/
  } icmp_data;
} icmp_header_t;


// ICMP types
#define ICMP_TYPE_ECHOREP 0 /*!<Echo Reply*/
#define ICMP_TYPE_UNREACH 3 /*!<Destination Unreachable*/
#define ICMP_TYPE_SOURCEQUENCH 4 /*!<Source Quench*/
#define ICMP_TYPE_REDIRECT 5 /*!<Redirect (change route)*/
#define ICMP_TYPE_ECHOREQ 8 /*!<Echo Request*/
#define ICMP_TYPE_ROUTERADV 9 /*!<Router Advertisement*/
#define ICMP_TYPE_ROUTERSOL 10 /*!<Router Solicitation*/
#define ICMP_TYPE_TIMEEXCEED 11 /*!<Time Exceeded*/
#define ICMP_TYPE_PARAMPROBLEM 12 /*!<Parameter Problem*/

// ICMP codes for Destination Unreachable
#define ICMP_CODE_NETUNREACH 0 /*!<Network Unreachable*/
#define ICMP_CODE_HOSTUNREACH 1 /*!<Host Unreachable*/
#define ICMP_CODE_PROTOUNREACH 2 /*!<Protocol Unreachable*/
#define ICMP_CODE_PORTUNREACH 3 /*!<Port Unreachable*/
#define ICMP_CODE_FRAGNEED 4 /*!<Fragmentation Needed set*/
#define ICMP_CODE_SRCROUTEFAIL 5 /*!<Source Route failed*/
#define ICMP_CODE_ADMINPROHIBIT 13 /*!<Administrator Prohibited Filter*/

// ICMP codes for Time Exceeded
#define ICMP_CODE_TIMETOLIVE 0 /*!<Time to live*/
#define ICMP_CODE_FRAGREASM 1 /*!<Fragment reassembly timeout*/


#endif // ICMP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
