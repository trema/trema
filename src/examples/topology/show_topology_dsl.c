/*
 * Author: Shuji Ishii, Kazushi SUGYO
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
#include <stdio.h>
#include <unistd.h>
#include "trema.h"
#include "libtopology.h"
#include "show_topology.h"
#include "topology_service_interface_option_parser.h"


struct dpid_entry {
  uint64_t dpid;
};


struct link_entry {
  uint64_t dpid0;
  uint64_t dpid1;
};


static bool
compare_link( const void *x, const void *y ) {
  const struct link_entry *lx = x;
  const struct link_entry *ly = y;

  return ( lx->dpid0 == ly->dpid0 && lx->dpid1 == ly->dpid1 );
}


static unsigned int
hash_link( const void *key ) {
  const struct link_entry *lkey = key;
  unsigned int hash = 0;

  hash ^= ( uint ) ( lkey->dpid0 >> 32 ^ lkey->dpid0 );
  hash ^= ( uint ) ( lkey->dpid1 >> 32 ^ lkey->dpid1 );

  return hash;
}


static uint64_t
max_dpid( const uint64_t a, const uint64_t b ) {
  return ( a > b ) ? a : b;
}


static uint64_t
min_dpid( const uint64_t a, const uint64_t b ) {
  return ( a > b ) ? b : a;
}


void
print_with_dsl_format( void *param, size_t entries, const topology_link_status *s ) {
  size_t i;

  UNUSED( param );

  debug( "topology: entries %d", entries );

  hash_table *dpid_hash = create_hash( compare_datapath_id, hash_datapath_id );
  hash_table *link_hash = create_hash( compare_link, hash_link );

  for ( i = 0; i < entries; i++ ) {
    if ( s[ i ].status != TD_LINK_UP ) {
      continue;
    }

    // add dpid
    struct dpid_entry *src = xmalloc( sizeof( struct dpid_entry ) );
    src->dpid = s[ i ].from_dpid;
    struct dpid_entry *dentry = lookup_hash_entry( dpid_hash, src );
    if ( dentry == NULL ) {
      insert_hash_entry( dpid_hash, src, src );
    } else {
      // skip this entry
      xfree( src );
    }

    struct dpid_entry *dst = xmalloc( sizeof( struct dpid_entry ) );
    dst->dpid = s[ i ].to_dpid;
    dentry = lookup_hash_entry( dpid_hash, dst );
    if ( dentry == NULL ) {
      insert_hash_entry( dpid_hash, dst, dst );
    } else {
      // skip this entry
      xfree( dst );
    }

    // add link
    struct link_entry *le = xmalloc( sizeof( struct link_entry ) );
    le->dpid0 = max_dpid( s[ i ].from_dpid, s[ i ].to_dpid );
    le->dpid1 = min_dpid( s[ i ].from_dpid, s[ i ].to_dpid );

    struct link_entry *lentry = lookup_hash_entry( link_hash, le );
    if ( lentry == NULL ) {
      insert_hash_entry( link_hash, le, le );
    } else {
      // skip this entry
      xfree( le );
    }
  } // for()

  // dump switches
  hash_iterator iter;
  init_hash_iterator( dpid_hash, &iter );
  hash_entry *e;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    struct dpid_entry *de = e->value;

    printf( "switch {\n" );
    printf( "  datapath_id \"%#" PRIx64 "\"\n", de->dpid );
    printf( "}\n\n" );
    xfree( de );
  }
  delete_hash( dpid_hash );

  // dump links
  init_hash_iterator( link_hash, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    struct link_entry *le = e->value;
    printf( "link \"%#" PRIx64 "\", \"%#" PRIx64 "\"\n", le->dpid0, le->dpid1 );
    xfree( le );
  }

  delete_hash( link_hash );

  stop_messenger();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
