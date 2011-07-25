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


#include <assert.h>
#include <inttypes.h>
#include <sys/types.h>
#include "trema.h"
#include "fdb.h"
#include "port.h"
#include "topology_service_interface.h"


static switch_info *
lookup_switch( list_element *switches, uint64_t dpid ) {
  for ( list_element *e = switches; e != NULL; e = e->next ) {
    switch_info *sw = e->data;
    if ( sw->dpid == dpid ) {
      return sw;
    }
  }

  return NULL;
}


static switch_info *
allocate_switch( uint64_t dpid ) {
  switch_info *sw = xmalloc( sizeof( switch_info ) );
  sw->dpid = dpid;
  create_list( &sw->ports );

  return sw;
}


static port_info *
allocate_port( uint64_t dpid, uint16_t port_no ) {
  port_info *port = xmalloc( sizeof( port_info ) );
  port->dpid = dpid;
  port->port_no = port_no;
  port->external_link = false;
  port->switch_to_switch_link = false;
  port->switch_to_switch_reverse_link = false;

  return port;
}


static void
delete_switch( list_element **switches, switch_info *delete_switch ) {
  list_element *ports = delete_switch->ports;

  // delete ports
  for ( list_element *p = ports; p != NULL; p = p->next ) {
    xfree( p->data );
  }
  delete_list( ports );

  // delete switch
  delete_element( switches, delete_switch );
  xfree( delete_switch );
}


void
delete_port( list_element **switches, port_info *delete_port ) {
  assert( switches != NULL );
  assert( delete_port != NULL );

  info( "Deleting a port: dpid = %#" PRIx64 ", port = %u",
        delete_port->dpid, delete_port->port_no );

  // lookup switch
  switch_info *sw = lookup_switch( *switches, delete_port->dpid );
  if( sw == NULL ) {
    debug( "No such port: dpid = %#" PRIx64 ", port = %u", delete_port->dpid, delete_port->port_no );
    return;
  }

  delete_element( &sw->ports, delete_port );
  xfree( delete_port );

  if ( sw->ports == NULL ) {
    delete_switch( switches, sw );
  }
}


void
add_port( list_element **switches, uint64_t dpid, uint16_t port_no, uint8_t external ) {
  assert( port_no != 0 );

  info( "Adding a port: dpid = %#" PRIx64 ", port = %u", dpid, port_no );

  // lookup switch
  switch_info *sw = lookup_switch( *switches, dpid );
  if ( sw == NULL ) {
    sw = allocate_switch( dpid );
    append_to_tail( switches, sw );
  }

  port_info *new_port = allocate_port( dpid, port_no );
  update_port( new_port, external );
  append_to_tail( &sw->ports, new_port );
}


void
update_port( port_info *port, uint8_t external ) {
  assert( port != 0 );

  port->external_link = ( external == TD_PORT_EXTERNAL );
}


void
delete_all_ports( list_element **switches ) {
  if ( switches != NULL ) {
    for ( list_element *s = *switches ; s != NULL ; s = s->next ) {
      switch_info *sw = s->data;
      for ( list_element *p = sw->ports; p != NULL; p = p->next ) {
        xfree( p->data );
      }
      delete_list( sw->ports );

      xfree( sw );
    }

    delete_list( *switches );

    *switches = NULL;
  }
}


port_info *
lookup_port( list_element *switches, uint64_t dpid, uint16_t port_no ) {
  assert( port_no != 0 );

  switch_info *sw = lookup_switch( switches, dpid );
  if ( sw == NULL ) {
    return NULL;
  }

  for ( list_element *e = sw->ports; e != NULL; e = e->next ) {
    port_info *p = e->data;
    if ( dpid == p->dpid && port_no == p->port_no ) {
      return p;
    }
  }

  return NULL;
}


int
foreach_port( const list_element *ports,
              int ( *function )( port_info *port,
                                 openflow_actions *actions,
                                 uint64_t dpid, uint16_t in_port ),
              openflow_actions *actions, uint64_t dpid, uint16_t port ) {
  int r = 0;
  for ( const list_element *p = ports; p != NULL; p = p->next ) {
    r += ( *function )( p->data, actions, dpid, port );
  }

  return r;
}


void
foreach_switch( const list_element *switches,
                void ( *function )( switch_info *sw,
                                    buffer *packet,
                                    uint64_t dpid,
                                    uint16_t in_port ),
                buffer *packet, uint64_t dpid, uint16_t in_port ) {
  for ( const list_element *s = switches; s != NULL; s = s->next ) {
    ( *function )( s->data, packet, dpid, in_port );
  }
}


list_element *
create_ports( list_element **ports ) {
  assert( ports != NULL );

  create_list( ports );
  return *ports;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
