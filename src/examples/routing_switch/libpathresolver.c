/*
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


#include "libpathresolver.h"
#include "hash_table.h"
#include "doubly_linked_list.h"


struct edge;

typedef struct node {
  uint64_t dpid;                // key
  hash_table *edges;            // node * -> edge
  uint32_t distance;            // distance from root node
  bool visited;
  struct {
    struct node *node;
    struct edge *edge;
  } from;
} node;


typedef struct edge {
  uint64_t peer_dpid;           // key
  uint16_t port_no;
  uint16_t peer_port_no;
  uint32_t cost;
} edge;


struct resolve_path_param {
  hash_table *node_table;
  uint64_t in_dpid;
  uint16_t in_port_no;
  uint64_t out_dpid;
  uint16_t out_port_no;
  void *user_data;
  resolve_path_callback callback;
};


static bool
comp_node( const void *x0, const void *y0 ) {
  const node *x = x0;
  const node *y = y0;

  return ( x->dpid == y->dpid );
}


static unsigned int
hash_node( const void *key0 ) {
  const node *key = key0;

  return ( unsigned int )(( key->dpid >> 32 ) ^ ( key->dpid & 0xffffffffUL ));
}


static bool
comp_edge( const void *x0, const void *y0 ) {
  const uint64_t *x = x0;
  const uint64_t *y = y0;

  return ( *x == *y );
}


static unsigned int
hash_edge( const void *key0 ) {
  const uint64_t *key = key0;

  return ( unsigned int ) ( ( *key >> 32 ) ^ ( *key & 0xffffffffUL ) );
}


static edge *
lookup_edge( const node *from, const node *to ) {
  return ( edge * ) lookup_hash_entry( from->edges, &to->dpid );
}


static node *
lookup_node( hash_table *node_table, const uint64_t dpid ) {
  node key;

  key.dpid = dpid;

  return ( node * )lookup_hash_entry( node_table, &key );
}


static node *
allocate_node( hash_table *node_table, const uint64_t dpid ) {
  node *n = lookup_node( node_table, dpid );
  if ( n == NULL ) {
    n = xmalloc( sizeof( node ) );

    n->dpid = dpid;
    n->edges = create_hash( comp_edge, hash_edge );
    n->distance = UINT32_MAX;
    n->visited = false;
    n->from.node = NULL;
    n->from.edge = NULL;

    insert_hash_entry( node_table, n, n );
  }

  return n;
}


static void
free_edge( node *n, edge *e ) {
  assert( n != NULL );
  assert( e != NULL );
  edge *delete_me = delete_hash_entry( n->edges, &e->peer_dpid );
  if ( delete_me != NULL ) {
    xfree( delete_me );
  }
}


static void
add_edge( hash_table *node_table, const uint64_t from_dpid,
          const uint16_t from_port_no, const uint64_t to_dpid,
          const uint16_t to_port_no, const uint32_t cost ) {
  node *from = allocate_node( node_table, from_dpid );
  node *to = allocate_node( node_table, to_dpid );

  edge *e = lookup_edge( from, to );
  if ( e != NULL ) {
    free_edge( from, e );
  }
  e = xmalloc( sizeof( edge ) );
  e->port_no = from_port_no;
  e->peer_dpid = to_dpid;
  e->peer_port_no = to_port_no;
  e->cost = cost;

  insert_hash_entry( from->edges, &e->peer_dpid, e );
}


static uint32_t
calculate_link_cost( const topology_link_status *l ) {
  UNUSED( l );
  return 1;
}


static node *
pickup_next_candidate( hash_table *node_table ) {
  node *candidate = NULL;
  uint32_t min_cost = UINT32_MAX;

  hash_iterator iter;
  hash_entry *entry = NULL;
  node *n = NULL;

  // pickup candidate node whose distance is minimum from visited nodes
  init_hash_iterator( node_table, &iter );
  while ( ( entry = iterate_hash_next( &iter ) ) != NULL ) {
    n = ( node * )entry->value;
    if ( !n->visited && n->distance < min_cost ) {
      min_cost = n->distance;
      candidate = n;
    }
  }

  return candidate;
}


static void
update_distance( hash_table *node_table, node *candidate ) {
  hash_iterator iter;
  hash_entry *entry = NULL;

  // update distance
  init_hash_iterator( node_table, &iter );
  while ( ( entry = iterate_hash_next( &iter ) ) != NULL ) {
    node *n = ( node * )entry->value;
    edge *e;
    if ( n->visited || ( e = lookup_edge( candidate, n ) ) == NULL ) {
      continue;               /* skip */
    }
    if ( candidate->distance + e->cost < n->distance ) {
      // short path via edge 'e'
      n->distance = candidate->distance + e->cost;
      n->from.node = candidate; // (candidate)->(n)
      n->from.edge = e;
    }
  } // while()
}


static dlist_element *
build_hop_list( node *src_node, uint16_t src_port_no,
                node *dst_node, uint16_t dst_port_no ) {
  node *n;
  uint16_t prev_out_port = UINT16_MAX;
  dlist_element *h = create_dlist();

  n = dst_node;
  while ( n != NULL ) {
    pathresolver_hop *hop = xmalloc( sizeof( pathresolver_hop ) );
    hop->dpid = n->dpid;

    if ( prev_out_port != 0xffffU ) {
      hop->out_port_no = prev_out_port;
    }
    else {
      hop->out_port_no = dst_port_no;
    }

    edge *e = n->from.edge;
    if ( e != NULL ) {
      prev_out_port = e->port_no;
      hop->in_port_no = e->peer_port_no;
    }

    h = insert_before_dlist( h, hop );
    n = n->from.node;
  }

  // adjust first hop
  pathresolver_hop *hh = ( pathresolver_hop * )h->data; // points source node
  if (  hh->dpid != src_node->dpid ) {
    free_hop_list( h );
    return NULL;
  }
  
  hh->in_port_no = src_port_no;

  // trim last element
  ( void )delete_dlist_element( get_last_element( h ) );

  return h;
}


static void
build_topology_table( hash_table *node_table, const topology_link_status links[], size_t nelems ) {
  for ( size_t i = 0; i < nelems; i++ ) {
    topology_link_status const *l = &links[ i ];
    add_edge( node_table, l->from_dpid, l->from_portno, l->to_dpid,
              l->to_portno, calculate_link_cost( l ) );
  }
}


static dlist_element *
dijkstra( hash_table *node_table, uint64_t in_dpid, uint16_t in_port_no,
          uint64_t out_dpid, uint16_t out_port_no ) {
  node *src_node = lookup_node( node_table, in_dpid );
  if ( src_node == NULL ) {
    return NULL;
  }

  src_node->distance = 0;
  src_node->from.node = NULL;
  src_node->from.edge = NULL;

  node *candidate;
  while ( ( candidate = pickup_next_candidate( node_table ) ) != NULL ) {
    candidate->visited = true;
    update_distance( node_table, candidate );
  }

  // build path hop list
  node *dst_node = lookup_node( node_table, out_dpid );
  if ( dst_node == NULL ) {
    return NULL; // not found
  }

  return build_hop_list( src_node, in_port_no, dst_node, out_port_no );
}


static void
free_node( hash_table *node_table, node *n ) {
  if ( n == NULL ) {
    return;
  }

  hash_iterator iter;
  hash_entry *entry;

  init_hash_iterator( n->edges, &iter );
  while ( ( entry = iterate_hash_next( &iter ) ) != NULL ) {
    edge *e = ( edge * )entry->value;
    free_edge( n, e );
  }

  delete_hash( n->edges );
  delete_hash_entry( node_table, n );
  xfree( n );
}


static void
flush_topology_table( hash_table *node_table ) {
  hash_iterator iter;
  hash_entry *e;

  init_hash_iterator( node_table, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    node *n = ( node * )e->value;
    free_node( node_table, n );
  }
}


static hash_table *
create_node_table() {
  return create_hash( comp_node, hash_node );
}


static void
delete_node_table( hash_table *node_table ) {
  flush_topology_table( node_table );
  delete_hash( node_table );
}


static void
resolve_path_reply_handler( void *param0, size_t entries, const topology_link_status *s ) {
  struct resolve_path_param *param = param0;
  dlist_element *hops;

  build_topology_table( param->node_table, s, entries );

  hops = dijkstra( param->node_table, param->in_dpid, param->in_port_no,
                   param->out_dpid, param->out_port_no );

  // call user's callback
  param->callback( param->user_data, hops );

  // finalize
  delete_node_table( param->node_table );

  xfree( param0 );
}


bool
resolve_path( uint64_t in_dpid, uint16_t in_port,
              uint64_t out_dpid, uint16_t out_port,
              void *user_data,
              resolve_path_callback callback ) {
  struct resolve_path_param *param = xmalloc( sizeof( struct resolve_path_param ) );

  param->node_table = create_node_table();
  param->in_dpid = in_dpid;
  param->in_port_no = in_port;
  param->out_dpid = out_dpid;
  param->out_port_no = out_port;
  param->user_data = user_data;
  param->callback = callback;

  get_all_link_status( resolve_path_reply_handler, param ); // resolve_path_reply_handler() will be called later

  return true;
}


void
free_hop_list( dlist_element *hops ) {
  dlist_element *e = get_first_element( hops );
  while ( e != NULL ) {
    dlist_element *delete_me = e;
    e = e->next;
    if ( delete_me->data != NULL ) {
      xfree( delete_me->data );
    }
    xfree( delete_me );
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
