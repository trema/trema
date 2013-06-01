/*
 * Sends a features request message.
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


static void
handle_features_reply(
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
  info( "capabilities:" );
  if ( capabilities & OFPC_FLOW_STATS ) {
    info( "  OFPC_FLOW_STATS" );
  }
  if ( capabilities & OFPC_TABLE_STATS ) {
    info( "  OFPC_TABLE_STATS" );
  }
  if ( capabilities & OFPC_PORT_STATS ) {
    info( "  OFPC_PORT_STATS" );
  }
  if ( capabilities & OFPC_STP ) {
    info( "  OFPC_STP" );
  }
  if ( capabilities & OFPC_RESERVED ) {
    info( "  OFPC_RESERVED" );
  }
  if ( capabilities & OFPC_IP_REASM ) {
    info( "  OFPC_IP_REASM" );
  }
  if ( capabilities & OFPC_QUEUE_STATS ) {
    info( "  OFPC_QUEUE_STATS" );
  }
  if ( capabilities & OFPC_ARP_MATCH_IP ) {
    info( "  OFPC_ARP_MATCH_IP" );
  }
  info( "actions:" );
  if ( actions & ( 1 << OFPAT_OUTPUT ) ) {
    info( "  OFPAT_OUTPUT" );
  }
  if ( actions & ( 1 << OFPAT_SET_VLAN_VID ) ) {
    info( "  OFPAT_SET_VLAN_VID" );
  }
  if ( actions & ( 1 << OFPAT_SET_VLAN_PCP ) ) {
    info( "  OFPAT_SET_VLAN_PCP" );
  }
  if ( actions & ( 1 << OFPAT_STRIP_VLAN ) ) {
    info( "  OFPAT_STRIP_VLAN" );
  }
  if ( actions & ( 1 << OFPAT_SET_DL_SRC ) ) {
    info( "  OFPAT_SET_DL_SRC" );
  }
  if ( actions & ( 1 << OFPAT_SET_DL_DST ) ) {
    info( "  OFPAT_SET_DL_DST" );
  }
  if ( actions & ( 1 << OFPAT_SET_NW_SRC ) ) {
    info( "  OFPAT_SET_NW_SRC" );
  }
  if ( actions & ( 1 << OFPAT_SET_NW_DST ) ) {
    info( "  OFPAT_SET_NW_DST" );
  }
  if ( actions & ( 1 << OFPAT_SET_NW_TOS ) ) {
    info( "  OFPAT_SET_NW_TOS" );
  }
  if ( actions & ( 1 << OFPAT_SET_TP_SRC ) ) {
    info( "  OFPAT_SET_TP_SRC" );
  }
  if ( actions & ( 1 << OFPAT_SET_TP_DST ) ) {
    info( "  OFPAT_SET_TP_DST" );
  }
  if ( actions & ( 1 << OFPAT_ENQUEUE ) ) {
    info( "  OFPAT_ENQUEUE" );
  }
  if ( actions & OFPAT_VENDOR ) {
    info( "  OFPAT_VENDOR" );
  }

  info( "ports:" );
  list_element ports_list;
  memcpy( &ports_list, phy_ports, sizeof( list_element ) );
  for ( list_element *port = &ports_list; port != NULL; port = port->next ) {
    struct ofp_phy_port *phy_port = port->data;
    info( "  port_no: %u", phy_port->port_no );
    info(
      "    hw_addr = %02x:%02x:%02x:%02x:%02x:%02x",
      phy_port->hw_addr[ 0 ],
      phy_port->hw_addr[ 1 ],
      phy_port->hw_addr[ 2 ],
      phy_port->hw_addr[ 3 ],
      phy_port->hw_addr[ 4 ],
      phy_port->hw_addr[ 5 ]
    );
    info( "    name = %s", phy_port->name );
    info( "    config = %#" PRIx32 "", phy_port->config );
    info( "    state = %#" PRIx32 "", phy_port->state );
    info( "    curr = %#" PRIx32 "", phy_port->curr );
    info( "    advertised = %#" PRIx32 "", phy_port->advertised );
    info( "    supported = %#" PRIx32 "", phy_port->supported );
    info( "    peer = %#" PRIx32 "", phy_port->peer );
  }
}


static void
send_features_request( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  buffer *features_request = create_features_request( get_transaction_id() );
  bool ret = send_openflow_message( datapath_id, features_request );
  if ( !ret ) {
    error( "Failed to send a features request message to the switch with datapath ID = %#" PRIx64 ".", datapath_id );
    stop_trema();
  }
  free_buffer( features_request );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  set_switch_ready_handler( send_features_request, NULL );
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
