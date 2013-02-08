/*
 * Traffic counter.
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
#include "fdb.h"
#include "counter.h"


typedef struct {
  hash_table *counter;
  hash_table *fdb;
} traffic;


static void
send_flow_mod( uint64_t datapath_id, uint8_t *macsa, uint8_t *macda, uint16_t out_port ) {
  struct ofp_match match;
  memset( &match, 0, sizeof( match ) );
  match.wildcards = ( OFPFW_ALL & ~( OFPFW_DL_SRC | OFPFW_DL_DST ) );
  memcpy( match.dl_src, macsa, OFP_ETH_ALEN );
  memcpy( match.dl_dst, macda, OFP_ETH_ALEN );
  uint16_t idle_timeout = 0;
  uint16_t hard_timeout = 10;
  uint16_t priority = UINT16_MAX;
  uint32_t buffer_id = UINT32_MAX;
  uint16_t out_port_for_delete_command = OFPP_NONE;
  uint16_t flags = OFPFF_SEND_FLOW_REM;
  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );
  buffer *flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(), OFPFC_ADD,
                                      idle_timeout, hard_timeout, priority, buffer_id,
                                      out_port_for_delete_command, flags, actions );
  delete_actions( actions );

  send_openflow_message( datapath_id, flow_mod );
  free_buffer( flow_mod );
}


static void
send_packet_out( uint64_t datapath_id, packet_in *message, uint16_t out_port ) {
  uint32_t buffer_id = message->buffer_id;
  uint16_t in_port = message->in_port;
  if ( datapath_id != message->datapath_id ) {
    buffer_id = UINT32_MAX;
    in_port = OFPP_NONE;
  }
  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );
  const buffer *data = NULL;
  if ( buffer_id == UINT32_MAX ) {
    data = message->data;
  }
  buffer *packet_out = create_packet_out( get_transaction_id(), buffer_id, in_port, actions, data );
  delete_actions( actions );

  send_openflow_message( datapath_id, packet_out );
  free_buffer( packet_out );
}


static void
do_flooding( uint64_t datapath_id, packet_in *message ) {
  send_packet_out( datapath_id, message, OFPP_FLOOD );
}


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  UNUSED( datapath_id );
  packet_info packet_info = get_packet_info( message.data );
  traffic *db = message.user_data;

  uint8_t *macsa = packet_info.eth_macsa;
  uint8_t *macda = packet_info.eth_macda;

  learn_fdb( db->fdb, macsa, message.in_port );
  add_counter( db->counter, macsa, 1, message.data->length );
  uint16_t out_port = lookup_fdb( db->fdb, macda );
  if ( out_port != ENTRY_NOT_FOUND_IN_FDB ) {
     send_packet_out( datapath_id, &message, out_port );
     send_flow_mod( datapath_id, macsa, macda, out_port );
  }
  else {
     do_flooding( datapath_id, &message );
  }
}


static void
handle_flow_removed( uint64_t datapath_id, flow_removed message ) {
  UNUSED( datapath_id );
  traffic *db = message.user_data;
  add_counter( db->counter, message.match.dl_src, message.packet_count, message.byte_count );
}


static void
show_each_counter( uint8_t *mac, counter *counter, void *user_data ) {
  UNUSED( user_data );
  printf( "%02x:%02x:%02x:%02x:%02x:%02x %" PRIu64 " packets (%" PRIu64 " bytes)\n",
          mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ],
          counter->packet_count, counter->byte_count );
}


static void
show_counter( void *user_data ) {
  traffic *db = user_data;
  time_t now = time( NULL );
  printf( "%s", ctime( &now ) );
  foreach_counter( db->counter, show_each_counter, NULL );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  traffic db;
  db.counter = create_counter();
  db.fdb = create_fdb();

  add_periodic_event_callback( 10, show_counter, &db );

  set_packet_in_handler( handle_packet_in, &db );
  set_flow_removed_handler( handle_flow_removed, &db );

  start_trema();

  delete_fdb( db.fdb );
  delete_counter( db.counter );

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
