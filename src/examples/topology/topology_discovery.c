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


#include <stdio.h>
#include <getopt.h>
#include "trema.h"
#include "libtopology.h"
#include "topology_service_interface_option_parser.h"
#include "lldp.h"
#include "probe_timer_table.h"


#ifdef UNIT_TESTING

#define static
#define main topology_discovery_main

#endif // UNIT_TESTING


static bool response_all_port_status_down = false;

void
usage( void ) {
  topology_service_interface_usage( get_executable_name(), "topology discovery", NULL );
}


static void
update_port_status( const topology_port_status *s ) {
  probe_timer_entry *entry = delete_probe_timer_entry( &( s->dpid ), s->port_no );
  if ( s->status == TD_PORT_DOWN ) {
    if ( entry != NULL ) {
      probe_request( entry, PROBE_TIMER_EVENT_DOWN, 0, 0 );
      free_probe_timer_entry( entry );
    }
    return;
  }
  if ( entry == NULL ) {
    entry = allocate_probe_timer_entry( &( s->dpid ), s->port_no, s->mac );
  }
  probe_request( entry, PROBE_TIMER_EVENT_UP, 0, 0 );
}


static void
response_all_port_status( void *param, size_t n_entries, const topology_port_status *s ) {
  UNUSED( param );

  size_t i;
  for ( i = 0; i < n_entries; i++ ) {
    update_port_status( &s[ i ] );
  }
  response_all_port_status_down = true;
}


static void
port_status_updated( void *param, const topology_port_status *status ) {
  UNUSED( param );

  if ( response_all_port_status_down ) {
    update_port_status( status );
  }
}


static void
subscribe_topology_response( void *user_data ) {
  UNUSED( user_data );
  add_callback_port_status_updated( port_status_updated, NULL );
  get_all_port_status( response_all_port_status, NULL );
  response_all_port_status_down = false;
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  init_topology_service_interface_options( &argc, &argv );
  init_probe_timer_table();
  init_libtopology( get_topology_service_interface_name() );
  init_lldp();
  subscribe_topology( subscribe_topology_response, NULL );

  start_trema();

  finalize_lldp();
  finalize_libtopology();
  finalize_probe_timer_table();
  finalize_topology_service_interface_options();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
