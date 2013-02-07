/*
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


#include <stdio.h>
#include <string.h>
#include "trema.h"
#include "utils.h"


static void
add_filter( void ) {
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_DL_SRC | OFPFW_DL_DST;
  match.in_port = 1;
  match.dl_type = 0x0800;
  match.dl_vlan = 0xffff;
  match.dl_vlan_pcp = 0;
  match.nw_src = 0x0a000001;
  match.nw_dst = 0x0a000002;
  match.nw_tos = 0;
  match.nw_proto = 0x0a;
  match.tp_src = 1024;
  match.tp_dst = 2048;

  static handler_data data;
  data.match = match;
  snprintf( data.service_name, sizeof( data.service_name ), "dumper" );
  data.service_name[ sizeof( data.service_name ) - 1 ] = '\0';

  bool ret = add_packetin_filter( data.match, UINT16_MAX, data.service_name, add_filter_completed, &data );
  if ( ret == false ) {
    error( "Failed to add a packetin filter ( ret = %d ).", ret );
  }
  else {
    add_periodic_event_callback( 5, timeout, &data );
  }
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  add_filter();

  start_trema();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
