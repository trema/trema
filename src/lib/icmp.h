/*
 * ICMP header definitions
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


#ifndef ICMP_H
#define ICMP_H


typedef struct icmp_header {
  uint8_t type;
  uint8_t code;
  uint16_t csum;
  union {
    struct {
      uint16_t ident;
      uint16_t seq;
    } echo;
    uint32_t gateway;
    uint32_t pad;
  } icmp_data;
} icmp_header_t;


#define ICMP_TYPE_ECHOREP 0
#define ICMP_TYPE_UNREACH 3
#define ICMP_TYPE_SOURCEQUENCH 4
#define ICMP_TYPE_REDIRECT 5
#define ICMP_TYPE_ECHOREQ 8
#define ICMP_TYPE_ROUTERADV 9
#define ICMP_TYPE_ROUTERSOL 10
#define ICMP_TYPE_TIMEEXCEED 11
#define ICMP_TYPE_PARAMPROBLEM 12

// ICMP codes for Destination Unreachable
#define ICMP_CODE_NETUNREACH 0
#define ICMP_CODE_HOSTUNREACH 1
#define ICMP_CODE_PROTOUNREACH 2
#define ICMP_CODE_PORTUNREACH 3
#define ICMP_CODE_FRAGNEED 4
#define ICMP_CODE_SRCROUTEFAIL 5
#define ICMP_CODE_ADMINPROHIBIT 13

// ICMP codes for Time Exceeded
#define ICMP_CODE_TIMETOLIVE 0
#define ICMP_CODE_FRAGREASM 1


#endif // ICMP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
