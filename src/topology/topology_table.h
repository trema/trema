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


#ifndef TOPOLOGY_TABLE_H
#define TOPOLOGY_TABLE_H


#include "trema.h"


typedef struct link_to {
  uint64_t datapath_id;
  uint16_t port_no;
  bool up;
} link_to;


typedef struct port_entry {
  struct sw_entry *sw;
  uint16_t port_no;
  char name[ OFP_MAX_PORT_NAME_LEN ];
  uint8_t mac[ ETH_ADDRLEN ];
  bool up;
  bool external;
  link_to *link_to;
  uint32_t id;
} port_entry;


typedef struct sw_entry {
  uint64_t datapath_id;
  uint32_t id;
  list_element *port_table;
} sw_entry;


void init_topology_table( void );
void finalize_topology_table( void );

sw_entry *update_sw_entry( uint64_t *datapath_id );
void delete_sw_entry( sw_entry *sw );
sw_entry *lookup_sw_entry( uint64_t *datapath_id );
void foreach_sw_entry( void function( sw_entry *entry, void *user_data ), void *user_data );

port_entry *update_port_entry( sw_entry *sw, uint16_t port_no, const char *name );
void delete_port_entry( sw_entry *sw, port_entry *port );
port_entry *lookup_port_entry( sw_entry *sw, uint16_t port_no, const char *name );
void foreach_port_entry( void function( port_entry *entry, void *user_data ), void *user_data );

link_to *update_link_to( port_entry *port, uint64_t *datapath_id, uint16_t port_no, bool up );
void delete_link_to( port_entry *port );


#endif // TOPOLOGY_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
