/*
 * Author: Shuji Ishii, Kazushi SUGYO
 *
 * Copyright (C) 2008-2011 NEC Corporation
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


bool init_libtopology( const char *service_name );
bool finalize_libtopology( void );

void subscribe_topology( void ( *callback )( void *user_data ), void *user_data );

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
                      void ( *callback )( void *user_data ), void *user_data );


#endif // LIBTOPOLOGY_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

