/*
 * Getting switch info with a features request message.
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
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  buffer *message = create_features_request( get_transaction_id() );
  send_openflow_message( datapath_id, message );
  free_buffer( message );
}


static void
handle_features_reply (
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t n_buffers,
  uint8_t n_tables,
  uint32_t capabilities,
  uint32_t actions,
  const list_element *phy_ports,
  void *user_data
) {
  UNUSED( user_data );

  info( "datapath_id: %#" PRIx64, datapath_id );
  info( "transaction_id: %#" PRIx32 "", transaction_id );
  info( "n_buffers: %" PRIu32 "", n_buffers );
  info( "n_tables: %u", n_tables );
  info( "capabilities: %" PRIu32 "", capabilities );
  info( "actions: %" PRIu32 "", actions );
  info( "#ports: %d", list_length_of( phy_ports ) );

  stop_trema();
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  set_switch_ready_handler( handle_switch_ready, NULL );
  set_features_reply_handler( handle_features_reply, NULL );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
