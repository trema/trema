/*
 * Unit tests for libpathresolver
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


#include <sys/types.h>
#include <arpa/inet.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "checks.h"
#include "messenger.h"
#include "libpathresolver.h"
#include "byteorder.h"
#include "cmockery.h"
#include "wrapper.h"


#define VOID_FUNCTION (0)

struct edge;

typedef struct node {
  uint64_t dpid;                /* key */
  hash_table *edges;            /* node * -> edge */
  uint32_t distance;            /* distance from root node */
  bool visited;
  struct {
    struct node *node;
    struct edge *edge;
  } from;
} node;


typedef struct edge {
  uint64_t peer_dpid;          /* key */
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


// static functions
bool comp_node( const void *x0, const void *y0 );
unsigned int hash_node( const void *key0 );
bool comp_edge( const void *x0, const void *y0 );
unsigned int hash_edge( const void *key0 );
edge *lookup_edge( const node *from, const node *to );
node *lookup_node( hash_table *node_table, const uint64_t dpid );
node *allocate_node( hash_table *node_table, const uint64_t dpid );
void free_edge( node *n, edge *e );
void add_edge( hash_table *node_table, const uint64_t from_dpid,
               const uint16_t from_port_no, const uint64_t to_dpid,
               const uint16_t to_port_no, const uint32_t cost );
uint32_t calculate_link_cost( const topology_link_status *l );
node *pickup_next_candidate( hash_table *node_table );
void update_distance( hash_table *node_table, node *candidate );
dlist_element *build_hop_list( node *src_node, uint16_t src_port_no,
                               node *dst_node, uint16_t dst_port_no );
void build_topology_table( hash_table *node_table, const topology_link_status links[], size_t nelems );
dlist_element *dijkstra( hash_table *node_table, uint64_t in_dpid,
                         uint16_t in_port_no, uint64_t out_dpid,
                         uint16_t out_port_no );
void free_node( hash_table *node_table, node *n );
void flush_topology_table( hash_table *node_table );
hash_table *create_node_table( void );
void delete_node_table( hash_table *node_table );
void resolve_path_reply_handler( void *param0, size_t entries, const topology_link_status *s );


/******************************************************************************
 * Helper functions.                                                          
 ******************************************************************************/


#define NODE_A (1ULL)
#define NODE_B (2ULL)
#define NODE_C (3ULL)
#define NODE_D (4ULL)
#define NODE_E (5ULL)
#define NODE_F (6ULL)
#define NODE_ISOLATED_AA (7ULL)
#define NODE_ISOLATED_BB (8ULL)
#define NO_SUCH_NODE (0xffffULL)

static topology_link_status test_link_status[] = {
  {
    .from_dpid = NODE_A,
    .to_dpid =  NODE_B,
    .from_portno = 1,
    .to_portno = 1,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_A,
    .to_dpid = NODE_C,
    .from_portno = 2,
    .to_portno = 1,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_B,
    .to_dpid = NODE_D,
    .from_portno = 3,
    .to_portno = 2,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_E,
    .to_dpid = NODE_B,
    .from_portno = 2,
    .to_portno = 2,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_C,
    .to_dpid = NODE_E,
    .from_portno = 2,
    .to_portno = 1,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_C,
    .to_dpid = NODE_F,
    .from_portno = 3,
    .to_portno = 1,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_E,
    .to_dpid = NODE_D,
    .from_portno = 3,
    .to_portno = 1,
    .status = TD_LINK_UP,
  }, {
    .from_dpid = NODE_ISOLATED_AA,
    .to_dpid = NODE_ISOLATED_BB,
    .from_portno = 1,
    .to_portno = 1,
    .status = TD_LINK_UP,
  },
};


static pathresolver_hop *expected_hops = NULL;


static void
setup() {
  init_log( "libpathresolver_test", false );
}


static void
teardown() {
  if ( expected_hops != NULL ) {
    xfree( expected_hops );
    expected_hops = NULL;
  }
}


static pathresolver_hop *
make_hop( const uint64_t dpid, const uint16_t in_port, const uint16_t out_port ) {
  pathresolver_hop *hop = xmalloc( sizeof( pathresolver_hop ) );

  hop->dpid = dpid;
  hop->in_port_no = in_port;
  hop->out_port_no = out_port;

  return hop;
}


static pathresolver_hop *
setup_test_hops_from_A_to_D() {
  pathresolver_hop *hops = xcalloc( 3, sizeof( pathresolver_hop ) );

  hops[ 0 ].dpid = NODE_A;
  hops[ 0 ].in_port_no = 3;
  hops[ 0 ].out_port_no = 1;

  hops[ 1 ].dpid = NODE_B;
  hops[ 1 ].in_port_no = 1;
  hops[ 1 ].out_port_no = 3;

  hops[ 2 ].dpid = NODE_D;
  hops[ 2 ].in_port_no = 2;
  hops[ 2 ].out_port_no = 3;

  return hops;
}


/******************************************************************************
 * Mock                                                                       
 ******************************************************************************/


static void
mock_callback( void *param, dlist_element *hops ) {
  UNUSED( param );

  if ( hops == NULL ) {
    return;
  }

  int i = 0;
  assert_true( expected_hops != NULL );

  dlist_element *e = hops;
  while ( e != NULL ) {
    pathresolver_hop *h = ( pathresolver_hop * )e->data;
    assert_true( h != NULL );
    assert_true( expected_hops[ i ].dpid == h->dpid );
    assert_true( expected_hops[ i ].in_port_no == h->in_port_no );
    assert_true( expected_hops[ i ].out_port_no == h->out_port_no );
    i++;
    xfree( h );
    e = e->next;
  }

  delete_dlist( hops );
}




void
mock_get_all_link_status( void ( *callback0 )( void *, size_t, topology_link_status * ), void *param0 ) {
  struct resolve_path_param *param = param0;
  uint64_t *in_dpid = &param->in_dpid;
  uint32_t in_port = param->in_port_no;
  uint64_t *out_dpid = &param->out_dpid;
  uint32_t out_port = param->out_port_no;
  void *user_data = param->user_data;
  resolve_path_callback callback = param->callback;

  check_expected( callback0 );
  check_expected( in_dpid );
  check_expected( in_port );
  check_expected( out_dpid );
  check_expected( out_port );
  check_expected( user_data );
  check_expected( callback );
  ( void )mock();

  resolve_path_reply_handler( param,
                              sizeof( test_link_status ) / sizeof( test_link_status[ 0 ] ),
                              test_link_status );
}


void
mock_die( char *format, ... ) {
  UNUSED( format );

  // Do nothing.
}


/******************************************************************************
 * Tests.                                                                     
 ******************************************************************************/


static void
test_mock_die() {
  char message[] = "message";
  mock_die( message );
}


static void
test_initialize_and_finalize() {
  hash_table *node_table = NULL;

  // (1) initialize

  // target
  node_table = create_node_table();

  assert_true( node_table != NULL );

  // (2) finalize

  // target
  delete_node_table( node_table );

}


static void
test_allocate_and_free_node() {
  const uint64_t test_dpid = 0x1010ULL;

  setup();
  hash_table *node_table = create_node_table();

  struct node *n = allocate_node( node_table, test_dpid );
  assert_true( n != NULL );
  assert_true( n == lookup_node( node_table, test_dpid ) );
  free_node( node_table, n );

  n = NULL;
  free_node( node_table, n );

  delete_node_table( node_table );
  teardown();
}


static void
test_add_and_free_edge() {
  const uint64_t src_dpid = 0x1010ULL;
  const uint16_t src_port = 1;
  const uint64_t dst_dpid = 0x1111ULL;
  const uint16_t dst_port = 33;
  const uint32_t cost = 1;
  setup();
  hash_table *node_table = create_node_table();

  // (1) normal case
  add_edge( node_table, src_dpid, src_port, dst_dpid, dst_port, cost );

  struct node *from = lookup_node( node_table, src_dpid );
  assert_true( from != NULL );

  struct node *to = lookup_node( node_table, dst_dpid );
  assert_true( to != NULL );

  struct edge *e = lookup_edge( from, to );
  assert_true( e != NULL );

  free_edge( from, e );

  delete_node_table( node_table );
  teardown();
}


static void
test_add_twice_and_free_edge() {
  const uint64_t src_dpid = 0x1010ULL;
  const uint16_t src_port = 1;
  const uint64_t dst_dpid = 0x1111ULL;
  const uint16_t dst_port = 33;
  const uint32_t cost = 1;
  setup();
  hash_table *node_table = create_node_table();

  // (2) duplicated case
  add_edge( node_table, src_dpid, src_port, dst_dpid, dst_port, cost );
  add_edge( node_table, src_dpid, src_port, dst_dpid, dst_port, cost );

  struct node *from = lookup_node( node_table, src_dpid );
  assert_true( from != NULL );

  struct node *to = lookup_node( node_table, dst_dpid );
  assert_true( to != NULL );

  struct edge *e = lookup_edge( from, to );
  assert_true( e != NULL );

  free_edge( from, e );

  delete_node_table( node_table );
  teardown();
}


static void
test_resolve_path_multi_hops() {
  struct resolve_path_param expected_param;
  void *test_user_data = &test_user_data;
  hash_table dummy_node_table;

  setup();

  memset( &expected_param, 0, sizeof( expected_param ) );
  expected_param.node_table = &dummy_node_table;
  expected_param.in_dpid = NODE_A;
  expected_param.in_port_no = 3;
  expected_param.out_dpid = NODE_D;
  expected_param.out_port_no = 3;
  expected_param.user_data = test_user_data;
  expected_param.callback = mock_callback;

  // get_all_link_status();
  expect_value( mock_get_all_link_status, callback0, resolve_path_reply_handler );
  expect_memory( mock_get_all_link_status, in_dpid, &expected_param.in_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, in_port, 3 );
  expect_memory( mock_get_all_link_status, out_dpid, &expected_param.out_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, out_port, 3 );
  expect_value( mock_get_all_link_status, user_data, test_user_data );
  expect_value( mock_get_all_link_status, callback, mock_callback );
  will_return( mock_get_all_link_status, VOID_FUNCTION );

  // from NODE A to NODE D
  expected_hops = setup_test_hops_from_A_to_D();

  // target
  assert_true( resolve_path( NODE_A, 3, NODE_D, 3, test_user_data, mock_callback ) );

  teardown();
}


static void
test_resolve_path_single_hop() {
  setup();

  struct resolve_path_param expected_param;
  void *test_user_data = &test_user_data;
  hash_table dummy_node_table;

  memset( &expected_param, 0, sizeof( expected_param ) );
  expected_param.node_table = &dummy_node_table;
  expected_param.in_dpid = NODE_A;
  expected_param.in_port_no = 3;
  expected_param.out_dpid = NODE_A;
  expected_param.out_port_no = 4;
  expected_param.user_data = test_user_data;
  expected_param.callback = mock_callback;

  // get_all_link_status();
  expect_value( mock_get_all_link_status, callback0, resolve_path_reply_handler );
  expect_memory( mock_get_all_link_status, in_dpid, &expected_param.in_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, in_port, 3 );
  expect_memory( mock_get_all_link_status, out_dpid, &expected_param.out_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, out_port, 4 );
  expect_value( mock_get_all_link_status, user_data, test_user_data );
  expect_value( mock_get_all_link_status, callback, mock_callback );
  will_return( mock_get_all_link_status, VOID_FUNCTION );

  // from NODE A to NODE A
  expected_hops = xcalloc( 1, sizeof( pathresolver_hop ) );
  expected_hops[ 0 ].dpid = NODE_A;
  expected_hops[ 0 ].in_port_no = 3;
  expected_hops[ 0 ].out_port_no = 4;

  // target
  assert_true( resolve_path( NODE_A, 3, NODE_A, 4, test_user_data, mock_callback ) );

  teardown();
}


static void
test_resolve_path_src_not_exists() {
  setup();

  struct resolve_path_param expected_param;
  void *test_user_data = &test_user_data;
  hash_table dummy_node_table;

  memset( &expected_param, 0, sizeof( expected_param ) );
  expected_param.node_table = &dummy_node_table;
  expected_param.in_dpid = NO_SUCH_NODE;
  expected_param.in_port_no = 3;
  expected_param.out_dpid = NODE_A;
  expected_param.out_port_no = 4;
  expected_param.user_data = test_user_data;
  expected_param.callback = mock_callback;

  // get_all_link_status();
  expect_value( mock_get_all_link_status, callback0, resolve_path_reply_handler );
  expect_memory( mock_get_all_link_status, in_dpid, &expected_param.in_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, in_port, 3 );
  expect_memory( mock_get_all_link_status, out_dpid, &expected_param.out_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, out_port, 4 );
  expect_value( mock_get_all_link_status, user_data, test_user_data );
  expect_value( mock_get_all_link_status, callback, mock_callback );
  will_return( mock_get_all_link_status, VOID_FUNCTION );

  expected_hops = NULL;

  // target
  assert_true( resolve_path( NO_SUCH_NODE, 3, NODE_A, 4, test_user_data, mock_callback ) );

  teardown();
}


static void
test_resolve_path_dst_not_exists() {
  setup();

  struct resolve_path_param expected_param;
  void *test_user_data = &test_user_data;
  hash_table dummy_node_table;

  memset( &expected_param, 0, sizeof( expected_param ) );
  expected_param.node_table = &dummy_node_table;
  expected_param.in_dpid = NODE_A;
  expected_param.in_port_no = 3;
  expected_param.out_dpid = NO_SUCH_NODE;
  expected_param.out_port_no = 4;
  expected_param.user_data = test_user_data;
  expected_param.callback = mock_callback;

  // get_all_link_status();
  expect_value( mock_get_all_link_status, callback0, resolve_path_reply_handler );
  expect_memory( mock_get_all_link_status, in_dpid, &expected_param.in_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, in_port, 3 );
  expect_memory( mock_get_all_link_status, out_dpid, &expected_param.out_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, out_port, 4 );
  expect_value( mock_get_all_link_status, user_data, test_user_data );
  expect_value( mock_get_all_link_status, callback, mock_callback );
  will_return( mock_get_all_link_status, VOID_FUNCTION );

  expected_hops = NULL;

  // target
  assert_true( resolve_path( NODE_A, 3, NO_SUCH_NODE, 4, test_user_data, mock_callback ) );

  teardown();
}


static void
test_resolve_path_src_is_isolated() {
  setup();

  struct resolve_path_param expected_param;
  void *test_user_data = &test_user_data;
  hash_table dummy_node_table;

  memset( &expected_param, 0, sizeof( expected_param ) );
  expected_param.node_table = &dummy_node_table;
  expected_param.in_dpid = NODE_ISOLATED_AA;
  expected_param.in_port_no = 1;
  expected_param.out_dpid = NODE_D;
  expected_param.out_port_no = 3;
  expected_param.user_data = test_user_data;
  expected_param.callback = mock_callback;

  // get_all_link_status();
  expect_value( mock_get_all_link_status, callback0, resolve_path_reply_handler );
  expect_memory( mock_get_all_link_status, in_dpid, &expected_param.in_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, in_port, 1 );
  expect_memory( mock_get_all_link_status, out_dpid, &expected_param.out_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, out_port, 3 );
  expect_value( mock_get_all_link_status, user_data, test_user_data );
  expect_value( mock_get_all_link_status, callback, mock_callback );
  will_return( mock_get_all_link_status, VOID_FUNCTION );

  // from NODE A to NODE D
  expected_hops = setup_test_hops_from_A_to_D();

  // target
  assert_true( resolve_path( NODE_ISOLATED_AA, 1, NODE_D, 3, test_user_data, mock_callback ) );

  teardown();
}


static void
test_resolve_path_dst_is_isolated() {
  setup();

  struct resolve_path_param expected_param;
  void *test_user_data = &test_user_data;
  hash_table dummy_node_table;

  memset( &expected_param, 0, sizeof( expected_param ) );
  expected_param.node_table = &dummy_node_table;
  expected_param.in_dpid = NODE_A;
  expected_param.in_port_no = 1;
  expected_param.out_dpid = NODE_ISOLATED_AA;
  expected_param.out_port_no = 3;
  expected_param.user_data = test_user_data;
  expected_param.callback = mock_callback;

  // get_all_link_status();
  expect_value( mock_get_all_link_status, callback0, resolve_path_reply_handler );
  expect_memory( mock_get_all_link_status, in_dpid, &expected_param.in_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, in_port, 1 );
  expect_memory( mock_get_all_link_status, out_dpid, &expected_param.out_dpid,
                 sizeof( uint64_t ) );
  expect_value( mock_get_all_link_status, out_port, 3 );
  expect_value( mock_get_all_link_status, user_data, test_user_data );
  expect_value( mock_get_all_link_status, callback, mock_callback );
  will_return( mock_get_all_link_status, VOID_FUNCTION );

  // from NODE A to NODE D
  expected_hops = setup_test_hops_from_A_to_D();

  // target
  assert_true( resolve_path( NODE_A, 1, NODE_ISOLATED_AA, 3, test_user_data, mock_callback ) );

  teardown();
}


static void
test_free_hop_list_successed() {
  dlist_element *hops = create_dlist();

  hops = insert_before_dlist( hops, make_hop( 0x1111, 1, 3 ) );
  hops = insert_before_dlist( hops, make_hop( 0x2222, 4, 5 ) );
  hops = insert_before_dlist( hops, make_hop( 0x3333, 10, 11 ) );
  // trim last element
  ( void )delete_dlist_element( get_last_element( hops ) );

  // target
  free_hop_list( hops );
}


/******************************************************************************
 * Run tests.                                                                 
 ******************************************************************************/


int
main() {
  const UnitTest tests[] = {
    unit_test( test_mock_die ),

    unit_test( test_initialize_and_finalize ),
    unit_test( test_allocate_and_free_node ),
    unit_test( test_add_and_free_edge ),
    unit_test( test_add_twice_and_free_edge ),
    unit_test( test_resolve_path_multi_hops ),
    unit_test( test_resolve_path_single_hop ),
    unit_test( test_resolve_path_src_not_exists ),
    unit_test( test_resolve_path_dst_not_exists ),
    unit_test( test_resolve_path_src_is_isolated ),
    unit_test( test_resolve_path_dst_is_isolated ),

    unit_test( test_free_hop_list_successed ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
