/*
 * Copyright (C) 2012 NEC Corporation
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
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include "trema.h"
#include "libtopology.h"
#include "show_topology.h"
#include "topology_service_interface_option_parser.h"


void
usage() {
  topology_service_interface_usage( get_executable_name(), "Show switch status", NULL );
}


static void
timed_out( void *user_data ) {
  UNUSED( user_data );

  error( "timed out." );

  stop_trema();
}


void
print_port_status( void *user_data, size_t number, const topology_port_status *ports ) {
  UNUSED( user_data );
  const topology_port_status *port = ports;
  
  printf( "Port status\n" );
  for ( size_t i = 0; i < number; i++, port++ ) {
    const char *status = "unknown";
    switch ( port->status ) {
      case TD_PORT_DOWN:
        status = "down";
        break;
      case TD_PORT_UP:
        status = "up";
        break;
    }
    printf( "  dpid : 0x%" PRIx64 ", port : 0x%d(%s), status : %s\n", 
            port->dpid, port->port_no, port->name, status );
  }
  
  stop_trema();
}


void
print_switch_status( void *user_data, size_t number, const topology_switch_status *switches ) {
  UNUSED( user_data );
  const topology_switch_status *sw = switches;

  printf( "Switch status\n" );
  for ( size_t i = 0; i < number; i++, sw++ ) {
    const char *status = "unknown";
    switch ( sw->status ) {
      case TD_SWITCH_DOWN:
        status = "down";
        break;
      case TD_SWITCH_UP:
        status = "up";
        break;
    }
    printf( "  dpid : 0x%" PRIx64 ", status : %s\n", sw->dpid, status );
  }
  
  get_all_port_status( print_port_status, NULL );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  init_topology_service_interface_options( &argc, &argv );
  init_libtopology( get_topology_service_interface_name() );

  get_all_switch_status( print_switch_status, NULL );

  add_periodic_event_callback( 15, timed_out, NULL );

  start_trema();

  finalize_libtopology();
  finalize_topology_service_interface_options();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
