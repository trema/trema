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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

uint16_t calc_checksum(const void *data_, uint32_t length);
char *hexdump(uint8_t *data, uint32_t length, char *out);
int strtomac(char *str, uint8_t mac_addr[6]);
int ipaddrtoul(char *str, uint32_t *ip_addr);
char *ultoipaddr(uint32_t ip_addr, char *mac_addr);
uint64_t htonll(uint64_t n);
uint64_t ntohll(uint64_t n);

#endif /* _UTILS_H_ */
