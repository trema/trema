/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#ifndef SERVICE_MANAGEMENT_H
#define SERVICE_MANAGEMENT_H


#include "trema.h"
#include "topology_table.h"
#include "topology_service_interface.h"

typedef struct service_management_options {
  time_t ping_interval_sec;
  int ping_ageout_cycles;
} service_management_options;


typedef void ( *switch_status_updated_hook )( void *user_data, const sw_entry *sw );
typedef void ( *port_status_updated_hook )( void *user_data, const port_entry *port );
typedef void ( *link_status_updated_hook )( void *user_data, const port_entry *port );

bool init_service_management( service_management_options new_options );
void finalize_service_management();

bool start_service_management( void );

extern void ( *notify_switch_status_for_all_user )( sw_entry *sw );
extern void ( *notify_port_status_for_all_user )( port_entry *port );
extern void ( *notify_link_status_for_all_user )( port_entry *port );

// for topology local use

extern bool ( *set_link_status_updated_hook )( link_status_updated_hook, void *user_data );
extern bool ( *set_port_status_updated_hook )( port_status_updated_hook, void *user_data );
extern bool ( *set_switch_status_updated_hook )( switch_status_updated_hook, void *user_data );

extern uint8_t ( *set_discovered_link_status )( topology_update_link_status* link_status );


const char*
get_topology_messenger_name( void );


#endif // SERVICE_MANAGEMENT_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
