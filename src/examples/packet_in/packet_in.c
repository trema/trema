/*
 * Dumps packet-in message.
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
#include "trema.h"


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  info( "received a packet_in" );
  info( "datapath_id: %#" PRIx64, datapath_id );
  info( "transaction_id: %#x", message.transaction_id );
  info( "buffer_id: %#x", message.buffer_id );
  info( "total_len: %u", message.total_len );
  info( "in_port: %u", message.in_port );
  info( "reason: %#x", message.reason );
  info( "data:" );
  dump_buffer( message.data, info );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  set_packet_in_handler( handle_packet_in, NULL );
  start_trema();
  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
