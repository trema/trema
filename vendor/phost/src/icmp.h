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

#ifndef _ICMP_H_
#define _ICMP_H_

#include "ipv4.h"

#define ICMP_TYPE_ECHO_REPLY 0
#define ICMP_TYPE_ECHO_REQUEST 8

#define ICMP_HEADER_LEN 8
#define ICMP_TYPE_LEN 1
#define ICMP_CODE_LEN 1
#define ICMP_CHECKSUM_LEN 2
#define ICMP_ID_LEN 2
#define ICMP_SEQNUM_LEN 2

int icmp_handle_message(ipv4 *ip);
int icmp_send_echo_reply(ipv4 *ip);

#endif /* _ICMP_H */
