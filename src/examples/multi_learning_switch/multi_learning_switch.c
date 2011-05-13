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


#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trema.h"


// Forwarding database
static const int FDB_ENTRY_TIMEOUT = 300;
static const int FDB_AGING_INTERVAL = 5;

struct fdb_entry {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint16_t port_no;
  time_t updated_at;
};


static void
delete_fdb_entry( void *key, void *value, void *user_data ) {
  UNUSED( key );
  UNUSED( user_data );

  struct fdb_entry *entry = value;

  debug( "deleting a forwardning database entry ( mac = "
         "%02x:%02x:%02x:%02x:%02x:%02x, port_no = %u, "
         "updated_at = %lu ).",
         entry->mac[ 0 ], entry->mac[ 1 ], entry->mac[ 2 ],
         entry->mac[ 3 ], entry->mac[ 4 ], entry->mac[ 5 ],
         entry->port_no, entry->updated_at );

  xfree( value );
}


static void
delete_fdb( hash_table *fdb ) {
  debug( "deleting a forwarding database." );

  foreach_hash( fdb, delete_fdb_entry, NULL );
  delete_hash( fdb );
}


static void
age_fdb_entry( void *key, void *value, void *user_data ) {
  hash_table *fdb = user_data;
  struct fdb_entry *entry = value;

  if ( entry->updated_at + FDB_ENTRY_TIMEOUT < time( NULL ) ) {
    debug( "age out." );
    delete_hash_entry( fdb, key );
    delete_fdb_entry( key, value, NULL );
  }
}


static void
update_fdb( void *user_data ) {
  hash_table *fdb = user_data;

  debug( "start aging." );

  foreach_hash( fdb, age_fdb_entry, fdb );
}


static void
set_fdb_entry( struct fdb_entry *entry, const uint16_t port_no,
               const uint8_t *mac, const time_t updated_at ) {
  debug( "setting a forwardning database entry ( mac = "
         "%02x:%02x:%02x:%02x:%02x:%02x, port_no = %u, "
         "updated_at = %lu ).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ],
         port_no, updated_at );

  memcpy( entry->mac, mac, sizeof( entry->mac ) );
  entry->port_no = port_no;
  entry->updated_at = updated_at;
}


static void
update_fdb_entry( hash_table *fdb, uint16_t port_no, uint8_t *mac ) {
  time_t now;
  struct fdb_entry *entry;

  now = time( NULL );

  entry = lookup_hash_entry( fdb, mac );

  if ( entry != NULL ) {
    set_fdb_entry( entry, port_no, mac, now );
  }
  else {
    entry = xmalloc( sizeof( struct fdb_entry ) );
    set_fdb_entry( entry, port_no, mac, now );
    insert_hash_entry( fdb, entry->mac, entry );
  }
}


static struct fdb_entry *
lookup_fdb_entry( hash_table *fdb, uint8_t *mac ) {
  struct fdb_entry *entry;

  debug( "looking up forwardning database ( mac = "
         "%02x:%02x:%02x:%02x:%02x:%02x ).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

  entry = lookup_hash_entry( fdb, mac );

  if ( entry != NULL ) {
    debug( "entry found ( port_no = %u, updated_at = %lu ).",
           entry->port_no, entry->updated_at );
  }
  else {
    debug( "entry not found." );
  }

  return entry;
}


static void
handle_packet_in( packet_in packet_in ) {
  uint8_t *mac;
  buffer *buffer;
  struct fdb_entry *entry;
  struct ofp_match match;
  openflow_actions *actions = NULL;
  hash_table *fdb = packet_in.user_data;

  debug( "packet_in received ( datapath_id = %#" PRIx64 ", transaction_id = %#lx, "
         "buffer_id = %#lx, total_len = %u, in_port = %u, reason = %#x, "
         "length = %u ).", packet_in.datapath_id, packet_in.transaction_id, packet_in.buffer_id,
         packet_in.total_len, packet_in.in_port, packet_in.reason, packet_in.data->length );

  // Learn an address
  mac = packet_info( packet_in.data )->l2_data.eth->macsa;
  update_fdb_entry( fdb, packet_in.in_port, mac );

  // Lookup a destination port
  mac = packet_info( packet_in.data )->l2_data.eth->macda;
  entry = lookup_fdb_entry( fdb, mac );

  // Create an empty actions list for flow_mod or packet_out
  actions = create_actions();

  if ( entry != NULL ) {
    // Send a packet to a single port

    append_action_output( actions, entry->port_no, UINT16_MAX );
    set_match_from_packet( &match, packet_in.in_port, 0, packet_in.data );

    buffer = create_flow_mod( get_transaction_id(), match, get_cookie(),
                              OFPFC_ADD, 60, 0, UINT16_MAX, packet_in.buffer_id,
                              OFPP_NONE, OFPFF_SEND_FLOW_REM, actions );

    if ( packet_in.buffer_id == UINT32_MAX ) {
      send_openflow_message( packet_in.datapath_id, buffer );
      free_buffer( buffer );

      buffer = create_packet_out( get_transaction_id(),
                                  packet_in.buffer_id, packet_in.in_port, actions, packet_in.data );
    }
  }
  else {
    // Flooding

    append_action_output( actions, OFPP_FLOOD, UINT16_MAX );

    if ( packet_in.buffer_id != UINT32_MAX ) {
      // Send a packet with a valid buffer_id
      buffer = create_packet_out( get_transaction_id(),
                                  packet_in.buffer_id, packet_in.in_port, actions, NULL );
    }
    else {
      // Send an entire packet
      buffer = create_packet_out( get_transaction_id(),
                                  packet_in.buffer_id, packet_in.in_port, actions, packet_in.data );
    }
  }

  send_openflow_message( packet_in.datapath_id, buffer );

  free_buffer( buffer );
  delete_actions( actions );
}



static bool
compare_dpid( const void *x, const void *y ) {
  return ( ( memcmp( x, y, sizeof ( uint64_t ) ) == 0 ) ? true : false );
}


static unsigned int
hash_dpid( const void *key ) {
  const uint32_t *dpid = ( const uint32_t *) key;
  return ( unsigned int ) dpid[ 0 ] ^ dpid[ 1 ];
}


struct switch_entry {
  uint64_t dpid;
  hash_table *fdb;
};


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  hash_table *switch_db = user_data;
  struct switch_entry *entry;

  entry = lookup_hash_entry( switch_db, &datapath_id );
  if ( entry == NULL ) {
    // Create new switch entry
    entry = xmalloc( sizeof( struct switch_entry ) );
    entry->dpid = datapath_id;
    entry->fdb = create_hash( compare_mac, hash_mac );
    insert_hash_entry( switch_db, &entry->dpid, entry );
    debug ( "Switch connected, DPID = %#" PRIx64 ", initialize FDB", datapath_id );
  }
  else {
    // Refresh switch entry
    delete_fdb( entry->fdb );
    entry->fdb = create_hash( compare_mac, hash_mac );
    warn ( "Switch reconnected, DPID = %#" PRIx64 ", refresh FDB", datapath_id );
  }

  // Set a timer event to update fdb
  add_periodic_event_callback( FDB_AGING_INTERVAL, update_fdb, entry->fdb );
}


static void
dispatch_packet_in_handler( packet_in packet_in ) {
  struct switch_entry *entry = lookup_hash_entry( ( hash_table * ) packet_in.user_data, &packet_in.datapath_id );
  if ( entry == NULL ) {
    warn( "Packet_in from unregistered switch (datapath ID = %#" PRIx64 ")", packet_in.datapath_id );
  }
  else {
    packet_in.user_data = entry->fdb;
    handle_packet_in( packet_in );
  }
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  hash_table *switch_database = create_hash( compare_dpid, hash_dpid );
  set_switch_ready_handler( handle_switch_ready, switch_database );
  set_packet_in_handler( dispatch_packet_in_handler, switch_database );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
