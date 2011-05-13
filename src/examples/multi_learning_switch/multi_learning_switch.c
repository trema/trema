/*
 * Learning switch application that supports multiple switches.
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


typedef struct {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint16_t port_no;
  time_t last_update;
} host;


typedef struct {
  uint64_t datapath_id;
  hash_table *host_db;
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
  sw->host_db = create_hash( compare_mac, hash_mac );
  return sw;
}


static void
delete_host( void *mac, void *host, void *user_data ) {
  UNUSED( mac );
  UNUSED( user_data );

  xfree( host );
}


static void
refresh( known_switch *sw ) {
  foreach_hash( sw->host_db, delete_host, NULL );
  delete_hash( sw->host_db );
  sw->host_db = create_hash( compare_mac, hash_mac );
}


static const int MAX_AGE = 300;

static bool
aged_out( host *host ) {
  if ( host->last_update + MAX_AGE < now() ) {
    return true;
  }
  else {
    return false;
  };
}


static void
age_host( void *mac, void *host, void *host_db ) {
  if ( aged_out( host ) ) {
    delete_hash_entry( host_db, mac );
    xfree( host );
  }
}


static void
update_all_hosts( void *host_db ) {
  foreach_hash( host_db, age_host, host_db );
}


static const int AGING_INTERVAL = 5;

static void
handle_switch_ready( uint64_t datapath_id, void *switch_db ) {
  known_switch *sw = lookup_hash_entry( switch_db, &datapath_id );
  if ( sw == NULL ) {
    sw = new_switch( datapath_id );
    insert_hash_entry( switch_db, &datapath_id, sw );
  }
  else {
    refresh( sw );
  }

  add_periodic_event_callback( AGING_INTERVAL, update_all_hosts, sw->host_db );
}


/********************************************************************************
 * packet_in event handler
 ********************************************************************************/

static void
learn( hash_table *host_db, uint16_t port_no, uint8_t *mac ) {
  host *entry = lookup_hash_entry( host_db, mac );

  if ( entry == NULL ) {
    entry = xmalloc( sizeof( host ) );
    memcpy( entry->mac, mac, sizeof( entry->mac ) );
    insert_hash_entry( host_db, entry->mac, entry );
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
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      packet_in.in_port,
      actions,
      packet_in.data
    );
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
    buffer *packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      packet_in.in_port,
      actions,
      packet_in.data
    );
    send_openflow_message( packet_in.datapath_id, packet_out );
    free_buffer( packet_out );
  }

  delete_actions( actions );
}


static void
handle_packet_in( packet_in packet_in ) {
  known_switch *sw = lookup_hash_entry(
    packet_in.user_data,
    &packet_in.datapath_id
  );
  if ( sw == NULL ) {
    warn( "Unknown switch (datapath ID = %#" PRIx64 ")", packet_in.datapath_id );
    return;
  }

  uint8_t *macsa = packet_info( packet_in.data )->l2_data.eth->macsa;
  learn( sw->host_db, packet_in.in_port, macsa );

  uint8_t *macda = packet_info( packet_in.data )->l2_data.eth->macda;
  host *destination = lookup_hash_entry( sw->host_db, macda );

  if ( destination == NULL ) {
    do_flooding( packet_in );
  }
  else {
    send_packet( destination->port_no, packet_in );
  }
}


/********************************************************************************
 * Start multi_learning_switch controller.
 ********************************************************************************/

int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  hash_table *switch_db = create_hash( compare_datapath_id, hash_datapath_id );
  set_switch_ready_handler( handle_switch_ready, switch_db );
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
