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

#include "trema.h"
#include "switch_option.h"

struct option switch_long_options[] = {
  { "socket", 1, NULL, 's' },
  { "no-flow-cleanup", 0, NULL, NO_FLOW_CLEANUP_LONG_OPTION_VALUE },
  { "no-cookie-translation", 0, NULL, NO_COOKIE_TRANSLATION_LONG_OPTION_VALUE },
  { "no-packet_in", 0, NULL, NO_PACKET_IN_LONG_OPTION_VALUE },
  { NULL, 0, NULL, 0  },
};

char switch_short_options[] = "s:";

