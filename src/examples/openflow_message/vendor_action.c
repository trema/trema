/*
 * "Vendor Action" sample application
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

#define NX_VENDOR_ID 0x00002320
#define NXAST_NOTE 8

struct nx_action_note {
    uint16_t type;
    uint16_t len;
    uint32_t vendor;
    uint16_t subtype;
    uint8_t note[6];
};

void
usage() {
  printf( "Usage: %s COUNT\n", get_executable_name() );
}


static void
send_flow_mod( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );
  uint8_t note[] = { 0x54, 0x72, 0x65, 0x6d, 0x61, 0x00 };

  buffer *body = alloc_buffer();
  struct nx_action_note *action = append_back_buffer( body, sizeof( struct nx_action_note ) );
  action->subtype = htons( NXAST_NOTE );
  memcpy( action->note, note, sizeof( action->note ) );
  remove_front_buffer( body, sizeof( struct ofp_action_vendor_header ) );

  openflow_actions *actions = create_actions();
  append_action_vendor( actions, NX_VENDOR_ID, body );

  struct ofp_match match;
  memset( &match, 0, sizeof( match ) );
  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    match,
    get_cookie(),
    OFPFC_MODIFY_STRICT,
    0,
    60,
    UINT16_MAX,
    UINT32_MAX,
    OFPP_NONE,
    OFPFF_SEND_FLOW_REM,
    actions
  );

  bool ret = send_openflow_message( datapath_id, flow_mod );
  if ( !ret ) {
    error( "Failed to send an flow_mod message to the switch with datapath ID = %#" PRIx64 ".", datapath_id );
    stop_trema();
  }

  free_buffer( flow_mod );

  delete_actions( actions );

  free_buffer( body );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  set_switch_ready_handler( send_flow_mod, NULL );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
