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
#include <inttypes.h>
#include <unistd.h>
#include <netinet/ether.h>
#include "trema.h"
#include "libtopology.h"
#include "topology_service_interface_option_parser.h"


void
usage() {
  topology_service_interface_usage( get_executable_name(), "list port", NULL );
}


static void
print_port_status( const topology_port_status *s ) {
  printf( "0x%-14" PRIx64 " %-5u %-12s %-17s %-4s %s\n",
          s->dpid, s->port_no, s->name,
          ether_ntoa( ( const struct ether_addr * ) s->mac ),
          ( s->status == TD_PORT_UP ? "up" : "down" ),
          ( s->status == TD_PORT_UP ? 
            ( s->external == TD_PORT_EXTERNAL ? "external" : "inactive-or-internal" )
            : " -" ) );
}


static void
print_all_port_status( void *param, size_t entries, const topology_port_status *s ) {
  size_t i;

  UNUSED( param );

  printf( "datapath-id      no    name         ethernet-address  state\n" );
  for ( i = 0; i < entries; i++ ) {
    print_port_status( &s[ i ] );
  }

  stop_messenger();
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  init_topology_service_interface_options( &argc, &argv );
  init_libtopology( get_topology_service_interface_name() );

  get_all_port_status( print_all_port_status, NULL);
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
