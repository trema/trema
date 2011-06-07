/*
 * Dumps packet-in message.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


#include <inttypes.h>
#include "trema.h"


static void
handle_packet_in( packet_in event ) {
  info( "received a packet_in" );
  info( "datapath_id: %#" PRIx64, event.datapath_id );
  info( "transaction_id: %#x", event.transaction_id );
  info( "buffer_id: %#x", event.buffer_id );
  info( "total_len: %u", event.total_len );
  info( "in_port: %u", event.in_port );
  info( "reason: %#x", event.reason );
  info( "data:" );
  dump_buffer( event.data, info );
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
