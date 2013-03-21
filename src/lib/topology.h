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


#ifndef LIBTOPOLOGY_H
#define LIBTOPOLOGY_H


#include "trema.h"
#include "topology_service_interface.h"


bool init_libtopology( const char *topology_service_name );
bool finalize_libtopology( void );


// following functions returns true on "message enqueue success".


bool subscribe_topology( void ( *callback )( void *user_data, topology_response *res ), void *user_data );
bool unsubscribe_topology( void ( *callback )( void *user_data, topology_response *res ), void *user_data );

bool add_callback_switch_status_updated( void ( *callback )( void *user_data,
                                                             const topology_switch_status *switch_status ),
                                         void *user_data );
bool add_callback_link_status_updated( void ( *callback )( void *user_data,
                                                           const topology_link_status *link_status ),
                                       void *user_data );
bool add_callback_port_status_updated( void ( *callback )( void *user_data,
                                                           const topology_port_status *port_status ),
                                       void *user_data );

bool get_all_link_status( void ( *callback )( void *user_data, size_t number,
                                              const topology_link_status *link_status ),
                          void *user_data );
bool get_all_port_status( void ( *callback )( void *user_data, size_t number,
                                              const topology_port_status *port_status ),
                          void *user_data );
bool get_all_switch_status( void ( *callback )( void *user_data, size_t number,
                                                const topology_switch_status *sw_status ),
                            void *user_data );

bool set_link_status( const topology_update_link_status *link_status,
                      void ( *callback )( void *user_data, const topology_response *res ),
                      void *user_data );


bool enable_topology_discovery( void ( *callback )( void *user_data, const topology_response *res ), void *user_data );
bool disable_topology_discovery( void ( *callback )( void *user_data, const topology_response *res ), void *user_data );
// TODO Future work: implement discovery control (port masking, etc.) methods
//bool add_discovery_ignore_switch( uint64_t dpid );
//bool remove_discovery_ignore_switch( uint64_t dpid );
//bool add_discovery_ignore_port( uint64_t dpid, uint16_t port_no );
//bool remove_discovery_ignore_port( uint64_t dpid, uint16_t port_no );


#endif // LIBTOPOLOGY_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

