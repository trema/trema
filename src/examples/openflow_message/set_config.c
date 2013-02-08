/*
 * Sends a set_config message to the specified datapath ID.
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


#include <inttypes.h>
#include <stdio.h>
#include "trema.h"


void
usage() {
  printf( "Usage: %s COUNT\n", get_executable_name() );
}


static void
send_set_config( uint64_t datapath_id, void *count ) {
  for ( int i = 0; i < *( ( int * ) count ); i++ ) {
    buffer *set_config = create_set_config( get_transaction_id(), OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );
    bool ret = send_openflow_message( datapath_id, set_config );
    if ( !ret ) {
      error( "Failed to send set_config message to the switch with datapath ID = %#" PRIx64 ".", datapath_id );
    }
    free_buffer( set_config );
  }
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  if ( argc < 2 ) {
    usage();
    return -1;
  }

  int count = atoi( argv[ 1 ] );
  set_switch_ready_handler( send_set_config, &count );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
