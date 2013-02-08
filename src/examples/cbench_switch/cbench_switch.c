/*
 * A simple OpenFlow controller for "cbench" benchmark.
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


#include "trema.h"


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, ( uint16_t ) ( message.in_port + 1 ), UINT16_MAX );

  struct ofp_match match;
  set_match_from_packet( &match, message.in_port, 0, message.data );

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    match,
    get_cookie(),
    OFPFC_ADD,
    0,
    0,
    UINT16_MAX,
    message.buffer_id,
    OFPP_NONE,
    0,
    actions
  );
  send_openflow_message( datapath_id, flow_mod );

  free_buffer( flow_mod );
  delete_actions( actions );
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
