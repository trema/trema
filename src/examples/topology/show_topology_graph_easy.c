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


static bool
compare_link( const void *x, const void *y ) {
  const topology_link_status *lx = x, *ly = y;
  if ( lx->from_dpid != ly->from_dpid )
    return false;
  if ( lx->from_portno != ly->from_portno )
    return false;
  if ( lx->to_dpid != ly->to_dpid )
    return false;
  if ( lx->to_portno != ly->to_portno )
    return false;

  return true;
}


static unsigned int
hash_link( const void *key ) {
  const topology_link_status *s = key;
  unsigned int hash = 0;
  hash ^= ( unsigned int ) ( s->from_dpid >> 32 ^ s->from_dpid );
  hash ^= s->from_portno;
  hash ^= ( unsigned int ) ( s->to_dpid >> 32 ^ s->to_dpid );
  hash ^= s->to_portno;

  return hash;
}


static void
print_link_status( const topology_link_status *s, bool doubly_linked ) {
  const char *l = "--";
  const char *r = "-->";
  if ( doubly_linked ) {
    l = "<==";
    r = "==>";
  }
  printf( "[ 0x%" PRIx64 " ] %s sp:%u\\ndp:%u %s [ 0x%" PRIx64 " ]\n",
          s->from_dpid, l, s->from_portno, s->to_portno, r, s->to_dpid );
}


void
print_with_graph_easy_format( void *param, size_t entries, const topology_link_status *s ) {
  size_t i;

  UNUSED( param );

  debug( "topology: entries %d", entries );

  hash_table *link_hash = create_hash( compare_link, hash_link );
  // show_topology graph-easy | graph-easy
  // Graph-Easy:
  //   http://search.cpan.org/~shlomif/Graph-Easy/bin/graph-easy
  printf("#! /usr/bin/env graph-easy\n" );

  for ( i = 0; i < entries; i++ ) {
    if ( s[ i ].status != TD_LINK_UP ) {
      continue;
    }
    topology_link_status r;
    r.from_dpid = s[ i ].to_dpid;
    r.from_portno = s[ i ].to_portno;
    r.to_dpid = s[ i ].from_dpid;
    r.to_portno = s[ i ].from_portno;
    
    topology_link_status *e = lookup_hash_entry( link_hash, &r );
    if ( e != NULL ) {
      delete_hash_entry( link_hash, e );
      print_link_status( e, true );
    } else {
      insert_hash_entry( link_hash, ( void * ) ( intptr_t ) &s[ i ], ( void *) ( intptr_t ) &s[ i ] );
    }
  }
  hash_iterator iter;
  init_hash_iterator( link_hash, &iter );
  hash_entry *e;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    topology_link_status *le = e->value;
    print_link_status( le, false );
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
