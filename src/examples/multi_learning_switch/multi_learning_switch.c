/*
 * Learning switch application that supports multiple switches.
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


#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include "trema.h"


typedef struct {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint16_t port_no;
  time_t last_update;
} forwarding_entry;


typedef struct {
  uint64_t datapath_id;
  hash_table *forwarding_db;
} known_switch;


time_t
now() {
  return time( NULL );
}


/********************************************************************************
 * switch_ready event handler
 ********************************************************************************/

static known_switch *
new_switch( uint64_t datapath_id ) {
  known_switch *sw = xmalloc( sizeof( known_switch ) );
  sw->datapath_id = datapath_id;
  sw->forwarding_db = create_hash( compare_mac, hash_mac );
  return sw;
}


static void
delete_forwarding_entry( void *mac, void *entry, void *user_data ) {
  UNUSED( mac );
  UNUSED( user_data );

  xfree( entry );
}


static void
refresh( known_switch *sw ) {
  foreach_hash( sw->forwarding_db, delete_forwarding_entry, NULL );
  delete_hash( sw->forwarding_db );
  sw->forwarding_db = create_hash( compare_mac, hash_mac );
}


static const int MAX_AGE = 300;

static bool
aged_out( forwarding_entry *entry ) {
  if ( entry->last_update + MAX_AGE < now() ) {
    return true;
  }
  else {
    return false;
  };
}


static void
age_forwarding_db( void *mac, void *entry, void *forwarding_db ) {
  if ( aged_out( entry ) ) {
    delete_hash_entry( forwarding_db, mac );
    xfree( entry );
  }
}


static void
update_all_entries( void *datapath_id, void *sw, void *user_data ) {
  UNUSED( datapath_id );
  UNUSED( user_data );

  hash_table *forwarding_db = ( ( known_switch * ) sw )->forwarding_db;
  foreach_hash( forwarding_db, age_forwarding_db, forwarding_db );
}


static void
update_all_switches( void *switch_db ) {
  foreach_hash( switch_db, update_all_entries, NULL );
}


static const int AGING_INTERVAL = 5;

static void
handle_switch_ready( uint64_t datapath_id, void *switch_db ) {
  known_switch *sw = lookup_hash_entry( switch_db, &datapath_id );
  if ( sw == NULL ) {
    sw = new_switch( datapath_id );
    insert_hash_entry( switch_db, &sw->datapath_id, sw );
  }
  else {
    refresh( sw );
  }
}


/********************************************************************************
 * switch_disconnected event handler
 ********************************************************************************/

static void
delete_switch( known_switch *sw ) {
  foreach_hash( sw->forwarding_db, delete_forwarding_entry, NULL );
  delete_hash( sw->forwarding_db );
  xfree( sw );
}


static void
handle_switch_disconnected( uint64_t datapath_id, void *switch_db ) {
  known_switch *sw = delete_hash_entry( switch_db, &datapath_id );
  if ( sw != NULL ) {
    delete_switch( sw );
  }
}


/********************************************************************************
 * packet_in event handler
 ********************************************************************************/

static void
learn( hash_table *forwarding_db, uint16_t port_no, uint8_t *mac ) {
  forwarding_entry *entry = lookup_hash_entry( forwarding_db, mac );

  if ( entry == NULL ) {
    entry = xmalloc( sizeof( forwarding_entry ) );
    memcpy( entry->mac, mac, sizeof( entry->mac ) );
    insert_hash_entry( forwarding_db, entry->mac, entry );
  }
  entry->port_no = port_no;
  entry->last_update = now();
}


static void
do_flooding( packet_in packet_in ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_FLOOD, UINT16_MAX );

  buffer *packet_out;
  if ( packet_in.buffer_id == UINT32_MAX ) {
    buffer *frame = duplicate_buffer( packet_in.data );
    fill_ether_padding( frame );
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      packet_in.in_port,
      actions,
      frame
    );
    free_buffer( frame );
  }
  else {
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      packet_in.in_port,
      actions,
      NULL
    );
  }
  send_openflow_message( packet_in.datapath_id, packet_out );
  free_buffer( packet_out );
  delete_actions( actions );
}


static void
send_packet( uint16_t destination_port, packet_in packet_in ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, destination_port, UINT16_MAX );

  struct ofp_match match;
  set_match_from_packet( &match, packet_in.in_port, 0, packet_in.data );

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    match,
    get_cookie(),
    OFPFC_ADD,
    60,
    0,
    UINT16_MAX,
    packet_in.buffer_id,
    OFPP_NONE,
    OFPFF_SEND_FLOW_REM,
    actions
  );
  send_openflow_message( packet_in.datapath_id, flow_mod );
  free_buffer( flow_mod );

  if ( packet_in.buffer_id == UINT32_MAX ) {
    buffer *frame = duplicate_buffer( packet_in.data );
    fill_ether_padding( frame );
    buffer *packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      packet_in.in_port,
      actions,
      frame
    );
    send_openflow_message( packet_in.datapath_id, packet_out );
    free_buffer( packet_out );
    free_buffer( frame );
  }

  delete_actions( actions );
}


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  known_switch *sw = lookup_hash_entry(
    message.user_data,
    &datapath_id
  );
  if ( sw == NULL ) {
    warn( "Unknown switch (datapath ID = %#" PRIx64 ")", datapath_id );
    return;
  }

  packet_info packet_info = get_packet_info( message.data );
  learn( sw->forwarding_db, message.in_port, packet_info.eth_macsa );
  forwarding_entry *destination = lookup_hash_entry( sw->forwarding_db,
                                                     packet_info.eth_macda );

  if ( destination == NULL ) {
    do_flooding( message );
  }
  else {
    send_packet( destination->port_no, message );
  }
}


/********************************************************************************
 * Start multi_learning_switch controller.
 ********************************************************************************/

int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  hash_table *switch_db = create_hash( compare_datapath_id, hash_datapath_id );
  add_periodic_event_callback( AGING_INTERVAL, update_all_switches, switch_db );
  set_switch_ready_handler( handle_switch_ready, switch_db );
  set_switch_disconnected_handler( handle_switch_disconnected, switch_db );
  set_packet_in_handler( handle_packet_in, switch_db );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
