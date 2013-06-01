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

#ifndef _TAP_H_
#define _TAP_H_

#include <stdint.h>

#define TAP_DEV "/dev/net/tun"
#define TAP_SELECT_TIMEOUT 1000 /* in usec */

#define TAP_DEV_TXQ_LEN 100000

int tap_init(const char *name);
int tap_close();
int tap_read(uint8_t *data, uint32_t *length);
int tap_send(const uint8_t *data, uint32_t length);
int tap_enable_promiscuous();
int tap_disable_promiscuous();

#endif /* _TAP_H_ */
