/*
 * Sample routing switch (switching HUB) application.
 *
 * This application provides a switching HUB function using multiple
 * openflow switches.
 *
 * Author: Shuji Ishii
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


#include <assert.h>
#include <inttypes.h>
#include <sys/types.h>
#include "trema.h"
#include "fdb.h"


static const time_t FDB_ENTRY_TIMEOUT = 300;
static const time_t FDB_AGING_INTERVAL = 5;
static const time_t HOST_MOVE_GUARD_SEC = 5;


typedef struct mac_db_entry {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint64_t dpid;
  uint16_t port;
  time_t updated_at;
  time_t created_at;
} fdb_entry;


static void
poison( uint64_t dpid, const uint8_t mac[ OFP_ETH_ALEN ] ) {
  struct ofp_match match;  
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = ( OFPFW_ALL & ~OFPFW_DL_DST );
  memcpy( match.dl_dst, mac, OFP_ETH_ALEN );

  const uint16_t idle_timeout = 0;
  const uint16_t hard_timeout = 0;
  const uint16_t priority = 0;
  const uint32_t buffer_id = 0;
  const uint16_t flags = 0;
  buffer *flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(),
                                      OFPFC_DELETE, idle_timeout, hard_timeout,
                                      priority, buffer_id, OFPP_NONE, flags,
                                      NULL );
  send_openflow_message( dpid, flow_mod );
  free_buffer( flow_mod );
  
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = ( OFPFW_ALL & ~OFPFW_DL_SRC );
  memcpy( match.dl_src, mac, OFP_ETH_ALEN );
  flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(),
                              OFPFC_DELETE, idle_timeout, hard_timeout,
                              priority, buffer_id, OFPP_NONE, flags, NULL );
  send_openflow_message( dpid, flow_mod );
  free_buffer( flow_mod );

  debug( "Poisoning all entries whose dl_src or dl_dst matches %02x:%02x:%02x:%02x:%02x:%02x at dpid %#" PRIx64, 
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ], dpid );

}


bool
is_ether_multicast( const uint8_t mac[ OFP_ETH_ALEN ] ) {
  return ( mac[ 0 ] & 1 ) == 1; // check I/G bit
}


hash_table *
create_fdb() {
  return create_hash( compare_mac, hash_mac );
}


void
delete_fdb( hash_table *fdb ) {
  if ( fdb != NULL ) {
    hash_iterator iter;
    hash_entry *e;
    init_hash_iterator( fdb, &iter );
    while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
      xfree( e->value );
    }
    delete_hash( fdb );
  }
}


bool
update_fdb( hash_table *fdb, const uint8_t mac[ OFP_ETH_ALEN ], uint64_t dpid, uint16_t port ) {
  assert( fdb != NULL );
  assert( mac != NULL );
  assert( port != 0 );

  fdb_entry *entry = lookup_hash_entry( fdb, mac );

  debug( "Updating fdb ( mac = %02x:%02x:%02x:%02x:%02x:%02x, dpid = %#" PRIx64 ", port = %u ).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ], dpid, port );

  if ( entry != NULL ) {
    if ( ( entry->dpid == dpid ) && ( entry->port == port ) ) {
      entry->updated_at = time( NULL );

      return true;
    }

    if ( entry->created_at + HOST_MOVE_GUARD_SEC < time( NULL ) ) {
      // Poisoning when the terminal moves
      poison( entry->dpid, mac );

      entry->dpid = dpid;
      entry->port = port;
      entry->created_at = time( NULL );
      entry->updated_at = entry->created_at;

      return true;
    }

    warn( "Failed to update fdb because host move detected in %d sec "
          "( mac = %02x:%02x:%02x:%02x:%02x:%02x, "
          "dpid = %#" PRIx64 " -> %#" PRIx64 ", port = %u -> %u ).",
          HOST_MOVE_GUARD_SEC,
          mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ],
          entry->dpid, dpid, entry->port, port );

    return false;
  }

  entry = xmalloc( sizeof( fdb_entry ) );
  entry->dpid = dpid;
  memcpy( entry->mac, mac, OFP_ETH_ALEN );
  entry->port = port;
  entry->created_at = time( NULL );
  entry->updated_at = entry->created_at;
  insert_hash_entry( fdb, entry->mac, entry );

  return true;
}


bool
lookup_fdb( hash_table *fdb, const uint8_t mac[ OFP_ETH_ALEN ], uint64_t *dpid, uint16_t *port ) {
  assert( fdb != NULL );
  assert( mac != NULL );
  assert( dpid != NULL );
  assert( port != NULL );

  if ( is_ether_multicast( mac ) ) {
    debug( "Skip looking up fdb. Because multicast/broadcast frame is received" );
    return false;
  }

  fdb_entry *entry = lookup_hash_entry( fdb, mac );

  debug( "Lookup mac:%02x:%02x:%02x:%02x:%02x:%02x", 
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

  if ( entry != NULL ) {
    *dpid = entry->dpid;
    *port = entry->port;

    debug( "Found at dpid = %#" PRIx64 ", port = %u", *dpid, *port );
    return true;
  }

  debug( "Not found" );
  return false;
}


void
delete_fdb_entries( hash_table *fdb, uint64_t dpid, uint16_t port ) {
  if ( fdb == NULL ) {
    return;
  }

  debug( "Deleting fdb entries ( dpid = %#" PRIx64 ", port = %u ).", dpid, port );

  fdb_entry *entry = NULL;
  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( fdb, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    if ( e->value == NULL ) {
      continue;
    }
    entry = e->value;
    if ( entry->dpid == dpid && entry->port == port ) {
      delete_hash_entry( fdb, entry->mac );
      xfree( entry );
    }
  }
}


static void
age_fdb_entry( void *key, void *value, void *user_data ) {
  hash_table *fdb = user_data;
  fdb_entry *entry = value;

  if ( entry->updated_at + FDB_ENTRY_TIMEOUT < time( NULL ) ) {
    debug( "Age out" );
    void *deleted = delete_hash_entry( fdb, key );
    xfree( deleted ); // free fdb_entry
  }
}


static void
age_fdb( void *user_data ) {
  hash_table *fdb = user_data;
  foreach_hash( fdb, age_fdb_entry, fdb );
}


void
init_age_fdb( hash_table *fdb ) {
  assert( fdb != NULL );
  add_periodic_event_callback( FDB_AGING_INTERVAL, age_fdb, fdb );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
