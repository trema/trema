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


static void
result_response( void *user_data, const topology_response *res ) {
  UNUSED( user_data );
  if ( res->status != TD_RESPONSE_OK ) {
    error( "Enable topology discovery failed" );
  } else {
    info( "Enabled topology discovery" );
  }
  stop_trema();
}

int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  init_topology_service_interface_options( &argc, &argv );
  init_libtopology( get_topology_service_interface_name() );

  enable_topology_discovery( result_response, NULL );

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
