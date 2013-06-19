/*
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


#ifndef DISCOVERY_MANAGEMENT_H_
#define DISCOVERY_MANAGEMENT_H_

#include "lldp.h"

typedef struct discovery_management_options {
  lldp_options lldp;
  bool always_enabled;
} discovery_management_options;

bool init_discovery_management( discovery_management_options new_options );
void finalize_discovery_management( void );

bool start_discovery_management( void );


/**
 * Enable discovery.
 */
extern void (* enable_discovery )( void );
extern void (* disable_discovery )( void );

// TODO Future work: port masking API etc.


extern bool ( *send_probe )( const uint8_t *mac, uint64_t dpid, uint16_t port_no );


#endif /* DISCOVERY_MANAGEMENT_H_ */
