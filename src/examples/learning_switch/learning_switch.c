/*
 * Simple learning switch application.
 *
 * Author: Yasunobu Chiba
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


// Datapath ID that is controlled by this application
static uint64_t managed_datapath_id = 0;

// Forwarding database
static const int FDB_ENTRY_TIMEOUT = 300;
static const int FDB_AGING_INTERVAL = 5;

// Commandline options
static struct option long_options[] = {
  { "datapath_id", required_argument, NULL, 'i' },
  { NULL, 0, NULL, 0  },
};

static char short_options[] = "i:";


struct fdb_entry {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint16_t port_no;
  time_t updated_at;
};


void
usage() {
  printf(
    "The Implementation of Learning Switch.\n"
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -n, --name=SERVICE_NAME     service name\n"
    "  -i, --datapath_id=ID        datapath ID\n"
    "  -d, --daemonize             run in the background\n"
    "  -l, --logging_level=LEVEL   set logging level\n"
    "  -h, --help                  display this help and exit\n"
    , get_executable_name()
  );
}


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
handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                  uint32_t buffer_id, uint16_t total_len,
                  uint16_t in_port, uint8_t reason, const buffer *data,
                  void *user_data ) {
  uint8_t *mac;
  buffer *buffer;
  struct fdb_entry *entry;
  struct ofp_match match;
  openflow_actions *actions = NULL;
  hash_table *fdb = user_data;

  debug( "packet_in received ( datapath_id = %#" PRIx64 ", transaction_id = %#lx, "
         "buffer_id = %#lx, total_len = %u, in_port = %u, reason = %#x, "
         "length = %u ).", datapath_id, transaction_id, buffer_id,
         total_len, in_port, reason, data->length );

  if( datapath_id != managed_datapath_id ) {
    error( "packet_in message from unmanaged switch ( dpid = %#" PRIx64 " ).",
           datapath_id );
    return;
  }

  // Learn an address
  mac = packet_info( data )->l2_data.eth->macsa;
  update_fdb_entry( fdb, in_port, mac );

  // Lookup a destination port
  mac = packet_info( data )->l2_data.eth->macda;
  entry = lookup_fdb_entry( fdb, mac );

  // Create an empty actions list for flow_mod or packet_out
  actions = create_actions();

  if ( entry != NULL ) {
    // Send a packet to a single port

    append_action_output( actions, entry->port_no, UINT16_MAX );
    set_match_from_packet( &match, in_port, 0, data );

    buffer = create_flow_mod( get_transaction_id(), match, get_cookie(),
                              OFPFC_ADD, 60, 0, UINT16_MAX, buffer_id,
                              OFPP_NONE, OFPFF_SEND_FLOW_REM, actions );

    if ( buffer_id == UINT32_MAX ) {
      send_openflow_message( datapath_id, buffer );
      free_buffer( buffer );

      buffer = create_packet_out( get_transaction_id(),
                                  buffer_id, in_port, actions, data );
    }
  }
  else {
    // Flooding

    append_action_output( actions, OFPP_FLOOD, UINT16_MAX );

    if ( buffer_id != UINT32_MAX ) {
      // Send a packet with a valid buffer_id
      buffer = create_packet_out( get_transaction_id(),
                                  buffer_id, in_port, actions, NULL );
    }
    else {
      // Send an entire packet
      buffer = create_packet_out( get_transaction_id(),
                                  buffer_id, in_port, actions, data );
    }
  }

  send_openflow_message( datapath_id, buffer );

  free_buffer( buffer );
  delete_actions( actions );
}


static bool
set_managed_switch( const char *datapath_id ) {
  if ( datapath_id == NULL ) {
    return false;
  }

  return string_to_datapath_id( datapath_id, &managed_datapath_id );
}


int
main( int argc, char *argv[] ) {
  hash_table *forwarding_database;
  char *datapath_id = NULL;
  int c;

  // Initialize the Trema world
  init_trema( &argc, &argv );

  while ( ( c = getopt_long( argc, argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'i':
        datapath_id = optarg;
        break;
      default:
        usage();
        exit( EXIT_FAILURE );
    }
  }
  if ( datapath_id == NULL ) {
    printf( "--datapath_id option is mandatory.\n" );
    usage();
    exit( EXIT_FAILURE );
  }

  // Set managed switch that is controlled by this application
  set_managed_switch( datapath_id );

  // Create a forwarding database
  forwarding_database = create_hash( compare_mac, hash_mac );

  // Set a timer event to update fdb
  add_periodic_event_callback( FDB_AGING_INTERVAL, update_fdb, forwarding_database );

  // Set an event handler for packet_in event
  set_packet_in_handler( handle_packet_in, forwarding_database );

  // Main loop
  start_trema();

  // Cleanup
  delete_fdb( forwarding_database );

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
