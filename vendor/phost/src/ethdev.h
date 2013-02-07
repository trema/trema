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

#ifndef _ETHDEV_H_
#define _ETHDEV_H_

#include <stdint.h>

#define ETHDEV_SELECT_TIMEOUT 1000 /* in usec */

#define ETHDEV_DEV_TXQ_LEN 100000

int ethdev_init(const char *name);
int ethdev_close();
int ethdev_read(uint8_t *data, uint32_t *length);
int ethdev_send(const uint8_t *data, uint32_t length);
int ethdev_enable_promiscuous();
int ethdev_disable_promiscuous();

#endif /* _ETHDEV_H_ */
