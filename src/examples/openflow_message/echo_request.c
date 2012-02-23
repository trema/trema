/*
 * Sends echo_request messages to the specified datapath ID.
 *
 * Author: Shin-ya Zenke, Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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
send_echo_requests( uint64_t datapath_id, void *count ) {
  for ( int i = 0; i < *( ( int * ) count ); i++ ) {
    buffer *echo_request = create_echo_request( get_transaction_id(), NULL );
    bool ret = send_openflow_message( datapath_id, echo_request );
    if ( !ret ) {
      error( "Failed to send an echo request message to the switch with datapath ID = %#" PRIx64 ".", datapath_id );
    }
    free_buffer( echo_request );
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
  set_switch_ready_handler( send_echo_requests, &count );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
