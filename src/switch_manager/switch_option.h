/*
 * Copyright (C) 2013 NEC Corporation
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

#ifndef SWITCH_OPTION_H_
#define SWITCH_OPTION_H_

#include <getopt.h>

enum switch_long_options_val {
  NO_FLOW_CLEANUP_LONG_OPTION_VALUE = 1,
  NO_COOKIE_TRANSLATION_LONG_OPTION_VALUE = 2,
  NO_PACKET_IN_LONG_OPTION_VALUE = 3,
};


extern struct option switch_long_options[];


extern char switch_short_options[];


#define VENDOR_PREFIX "vendor::"
#define PACKET_IN_PREFIX "packet_in::"
#define PORTSTATUS_PREFIX "port_status::"
#define STATE_PREFIX "state_notify::"

#endif /* SWITCH_OPTION_H_ */
