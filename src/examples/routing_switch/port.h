/*
 * Sample routing switch (switching HUB) application.
 *
 * This application provides a switching HUB function using multiple
 * openflow switches.
 *
 * Author: Shuji Ishii
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


#ifndef PORT_H
#define PORT_H


#include "trema.h"


typedef struct port_info {
  uint64_t dpid;
  uint16_t port_no;
  bool external_link;
  bool switch_to_switch_link;
  bool switch_to_switch_reverse_link;
} port_info;


typedef struct switch_info {
  uint64_t dpid;
  list_element *ports; // list of port_info
} switch_info;


void delete_port( list_element **switches, port_info *delete_port );
void add_port( list_element **switches, uint64_t dpid, uint16_t port_no, uint8_t external );
void update_port( port_info *port, uint8_t external );
void delete_all_ports( list_element **switches );
port_info *lookup_port( list_element *switches, uint64_t dpid, uint16_t port_no );
int foreach_port( const list_element *ports,
                  int ( *function )( port_info *port,
                                     openflow_actions *actions,
                                     uint64_t dpid, uint16_t in_port ),
                  openflow_actions *actions, uint64_t dpid, uint16_t port );
void foreach_switch( const list_element *switches,
                     void ( *function )( switch_info *sw,
                                         buffer *packet,
                                         uint64_t dpid,
                                         uint16_t in_port ),
                     buffer *packet, uint64_t dpid, uint16_t in_port );
list_element *create_ports( list_element **switches );


#endif // PORT_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
