/*
 * Unit tests for routing switch
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


#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"
#include "cmockery.h"


#include "trema.h"
#include "ether.h"
#include "fdb.h"
#include "libtopology.h"
#include "libpathresolver.h"
#include "port.h"

#define VOID_FUNCTION (0)
#define FLOW_TIMER 60
#define FDB_ENTRY_TIMEOUT 300
#define FDB_AGING_INTERVAL 5
#define HOST_MOVE_GUARD_SEC 5

static const char topology_service[] = "topology";


typedef struct routing_switch_options {
  uint16_t idle_timeout;
} routing_switch_options;


typedef struct mac_db_entry {
  uint8_t mac[ OFP_ETH_ALEN ];
  uint64_t dpid;
  uint16_t port;
  time_t updated_at;
} fdb_entry;


typedef struct routing_switch {
  uint16_t idle_timeout;
  list_element *switches;
  hash_table *fdb;
} routing_switch;


typedef struct resolve_path_replied_params {
  routing_switch *routing_switch;
  buffer *original_packet;
} resolve_path_replied_params;


// static functions
void modify_flow_entry( const pathresolver_hop *h, const buffer *original_packet, uint16_t idle_timeout );
void output_packet( buffer *packet, uint64_t dpid, uint16_t port_no );
void output_packet_from_last_switch( const pathresolver_hop *last_hop, buffer *packet );
uint32_t count_hops( const dlist_element *hops );
void resolve_path_replied( void *user_data, dlist_element *hops );
void receive_features_reply( uint64_t datapath_id, uint32_t transaction_id,
                             uint32_t n_buffers, uint8_t n_tables,
                             uint32_t capabilities, uint32_t actions,
                             const list_element *phy_ports, void *user_data );
void switch_ready( uint64_t datapath_id, void *user_data );
void port_status_updated( void *user_data, const topology_port_status *status );
void flood_packet( uint64_t datapath_id, uint16_t in_port, buffer *packet, list_element *outbound_ports );
void handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                       uint32_t buffer_id, uint16_t total_len,
                       uint16_t in_port, uint8_t reason, const buffer *data,
                       void *user_data );
void init_outbound_ports( list_element **switches, size_t n_entries, const topology_port_status *s );
void init_last_stage( void *user_data, size_t n_entries, const topology_port_status *s );
void after_subscribed( void *user_data );
routing_switch *create_routing_switch( const char *topology_service, const routing_switch_options *options );
void delete_routing_switch( routing_switch *routing_switch );
void reset_getopt( void );
void init_routing_switch_options( routing_switch_options *options, int *argc, char ***argv );


/**********************************************************************
 * Helper functions.                                                  
 **********************************************************************/


static const topology_port_status test_port_status[] = {
  {
    .dpid = 0x1234ULL,
    .port_no = 1,
    .mac = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    .external = TD_PORT_EXTERNAL,
    .status = TD_PORT_UP,
  }, {
    .dpid = 0x1234ULL,
    .port_no = 2,
    .mac = { 0x22, 0x44, 0x66, 0x88, 0xaa, 0xcc },
    .external = TD_PORT_EXTERNAL,
    .status = TD_PORT_UP,
  }, {
    .dpid = 0x456789ULL,
    .port_no = 1,
    .mac = { 0xcc, 0xdd, 0xee, 0xff, 0x00, 0xee },
    .external = TD_PORT_EXTERNAL,
    .status = TD_PORT_UP,
  }, {
    .dpid = 0x456789ULL,
    .port_no = 2,
    .mac = { 0xff, 0xdd, 0xee, 0x88, 0x77, 0x11 },
    .external = TD_PORT_INACTIVE,
    .status = TD_PORT_UP,
  }, {
    .dpid = 0x456789ULL,
    .port_no = 3,
    .mac = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc },
    .external = TD_PORT_EXTERNAL,
    .status = TD_PORT_DOWN,
  }, {
    .dpid = 0x456789ULL,
    .port_no = 4,
    .mac = { 0xab, 0xcd, 0xef, 0x1a, 0xbf, 0x78 },
    .external = TD_PORT_INACTIVE,
    .status = TD_PORT_DOWN,
  },
};

static const size_t number_of_test_port_status =
  sizeof( test_port_status ) / sizeof( test_port_status[ 0 ] );


/*
 * add padding data to arp/ipv4 header size forcibly to be able to adapt
 * itself to the minimum size of the ethernet packet (= 46 byte).
 */
static const unsigned int arp_padding_size = 46 - sizeof( arp_header_t );


// switch A, host a, host aa
static uint64_t dpid_switch_A = 0x1111ULL;
static uint16_t port_no_from_A_to_B = 1;
__attribute__((unused)) static uint16_t port_no_from_A_to_C = 2;
static uint16_t port_no_from_A_to_host_a = 3;
static uint16_t port_no_from_A_to_host_aa = 4;
static uint16_t new_port_no_of_switch_A = 5;
static uint16_t unknown_port_of_switch_A = 6;
static uint8_t host_a_mac[ ETH_ADDRLEN ] = { 0x11, 0x22, 0x33, 0xaa, 0xbb, 0xcc };
static uint8_t host_aa_mac[ ETH_ADDRLEN ] = { 0x11, 0x33, 0x55, 0x88, 0x99, 0xff };

// switch B, host b
static uint64_t dpid_switch_B = 0x2222ULL;
static uint16_t port_no_from_B_to_C = 1;
static uint16_t port_no_from_B_to_A = 2;
static uint16_t port_no_from_B_to_host_b = 3;
static uint8_t host_b_mac[ ETH_ADDRLEN ] = { 0xbb, 0xcc, 0xaa, 0x00, 0x88, 0x99 };

// switch C, host c
static uint64_t dpid_switch_C = 0x3333ULL;
__attribute__((unused)) static uint16_t port_no_from_C_to_A = 1;
static uint16_t port_no_from_C_to_B = 2;
static uint16_t port_no_from_C_to_host_c = 3;
static uint8_t host_c_mac[ ETH_ADDRLEN ] = { 0xcc, 0x77, 0x11, 0x55, 0xaa, 0xff };

// switch D (not registered)
static uint64_t dpid_switch_D = 0x4444ULL;
static uint16_t new_port_of_switch_D = 3333;

// paramaters for handle_packet_in()
static uint64_t dpid = 0;
static uint32_t transaction_id = 0x22223333UL;
static uint32_t buffer_id = 0x44443333UL;
static uint16_t total_len = 64;
static uint16_t in_port = 0;
static uint8_t dummy_reason = 0; // don't care
static buffer *test_packet = NULL;
static void *dummy_user_data = &dummy_user_data; // don't care


// parameters for receive_features_reply()
static uint32_t n_buffers = 128;
static uint8_t n_tables = 8;
static uint32_t capabilities = 0x11112222;
static uint32_t actions = 10;
static list_element phy_ports = {
  .next = NULL,
  .data = &phy_ports,
};


static switch_info *
helper_alloc_switch( uint64_t dpid ) {
  switch_info *sw = xmalloc( sizeof( switch_info ) );
  sw->dpid = dpid;
  create_list( &sw->ports );

  return sw;
}


static switch_info *
helper_lookup_switch( list_element *switches, uint64_t dpid ) {
  for ( list_element *e = switches; e != NULL; e = e->next ) {
    switch_info *sw = e->data;
    if ( sw->dpid == dpid ) {
      return sw;
    }
  }

  return NULL;
}


static void
helper_add_outbound_port( list_element **switches, uint64_t dpid, uint16_t port_no ) {
  // lookup switch
  switch_info *sw = helper_lookup_switch( *switches, dpid );
  if ( sw == NULL ) {
    sw = helper_alloc_switch( dpid );
    append_to_tail( switches, sw );
  }

  port_info *new_port = xmalloc( sizeof( port_info ) );
  new_port->dpid = dpid;
  new_port->port_no = port_no;
  append_to_tail( &sw->ports, new_port );
}


static void
setup_outbound_ports_for_test( list_element **switches ) {
  assert_true( switches != NULL );

  create_list( switches );

  helper_add_outbound_port( switches, dpid_switch_A, port_no_from_A_to_host_a );
  helper_add_outbound_port( switches, dpid_switch_A, port_no_from_A_to_host_aa );
  helper_add_outbound_port( switches, dpid_switch_B, port_no_from_B_to_host_b );
  helper_add_outbound_port( switches, dpid_switch_C, port_no_from_C_to_host_c );
}


static void
teardown_outbound_ports_for_test( list_element **switches ) {
  if ( switches != NULL && *switches != NULL ) {
    for ( list_element *s = *switches; s != NULL; s = s->next ) {
      switch_info *sw = s->data;
      for ( list_element *p = sw->ports; p != NULL; p = p->next ) {
        port_info *port = p->data;
        xfree( port );
      }
      delete_list( sw->ports );
      xfree( sw );
    }

    delete_list( *switches );
    *switches = NULL;
  }
}


static void
setup_port_status( topology_port_status *port, uint64_t dpid, uint16_t port_no, uint8_t external, uint8_t status ) {
  port->dpid = dpid;
  port->port_no = port_no;
  uint8_t test_mac[ ETH_ADDRLEN ] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };
  memcpy( port->mac, test_mac, ETH_ADDRLEN );
  port->external = external;
  port->status = status;
}


static buffer *
setup_test_packet( const uint8_t src[ ETH_ADDRLEN ], const uint8_t dst[ ETH_ADDRLEN ] ) {
  buffer *b = alloc_buffer_with_length( 128 );

  alloc_packet( b );
  packet_info( b )->l2_data.l2 = ( uint8_t * ) b->data - ETH_PREPADLEN;
  memcpy( packet_info( b )->l2_data.eth->macsa, src, ETH_ADDRLEN );
  memcpy( packet_info( b )->l2_data.eth->macda, dst, ETH_ADDRLEN );
  packet_info( b )->ethtype = ETH_ETHTYPE_ARP;

  return b;
}


static dlist_element *
setup_test_hops() {
  dlist_element *hops;

  hops = create_dlist();

  pathresolver_hop *h = xmalloc( sizeof( pathresolver_hop ) );
  h->dpid = dpid_switch_A;
  h->in_port_no = port_no_from_A_to_host_a;
  h->out_port_no = port_no_from_A_to_B;
  hops->data = h;

  h = xmalloc( sizeof( pathresolver_hop ) );
  h->dpid = dpid_switch_B;
  h->in_port_no = port_no_from_B_to_A;
  h->out_port_no = port_no_from_B_to_C;
  insert_after_dlist( get_last_element( hops ), h );

  h = xmalloc( sizeof( pathresolver_hop ) );
  h->dpid = dpid_switch_C;
  h->in_port_no = port_no_from_C_to_B;
  h->out_port_no = port_no_from_C_to_host_c;
  insert_after_dlist( get_last_element( hops ), h );

  return hops;
}


static void
reset_getopt_long() {
  optind = 0;
  opterr = 1;
}


/*************************************************************************
 * Mock function.                                          
 *************************************************************************/

void
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


/*************************************************************************
 * Setup and teardown function.                                          
 *************************************************************************/


static hash_table dummy_fdb;
static port_info dummy_port;


static routing_switch *
setup() {
  init_log( "routing_switch_test", false );

  routing_switch *routing_switch = xmalloc( sizeof( struct routing_switch ) );
  
  routing_switch->idle_timeout = FLOW_TIMER;
  setup_outbound_ports_for_test( &routing_switch->switches );
  routing_switch->fdb = &dummy_fdb;

  return routing_switch;
}

static void
teardown( routing_switch *routing_switch ) {
  if ( test_packet != NULL ) {
    free_packet( test_packet );
    test_packet = NULL;
  }

  if ( routing_switch != NULL ) {
    assert( routing_switch->fdb == &dummy_fdb );
    teardown_outbound_ports_for_test( &routing_switch->switches );
    xfree( routing_switch );
  }
}


/*************************************************************************
 * Mock                                                                  
 *************************************************************************/


void
mock_die( char *format, ... ) {
  UNUSED( format );
}


void
mock_free_hop_list( dlist_element *hops ) {
  dlist_element *e = get_first_element( hops );
  while ( e != NULL ) {
    dlist_element *delete_me = e;
    e = e->next;
    xfree( delete_me->data );
    xfree( delete_me );
  }
}


openflow_actions_t *
mock_create_actions() {
  return ( openflow_actions_t * ) mock();
}


bool
mock_append_action_output( openflow_actions_t *actions,
                           /*const*/ uint16_t port16,
                           /*const*/ uint16_t max_len16 ) {
  check_expected( actions );
  uint32_t port = port16;
  check_expected( port );
  uint32_t max_len = max_len16;
  check_expected( max_len );

  return ( bool ) mock();
}


uint32_t
mock_get_transaction_id( void ) {
  return transaction_id;
}


buffer *
mock_create_packet_out( /*const*/ uint32_t transaction_id,
                        /*const*/ uint32_t buffer_id,
                        /*const*/ uint16_t in_port16,
                        /*const*/ openflow_actions_t *actions,
                        /*const*/ buffer *data ) {
  check_expected( transaction_id );
  check_expected( buffer_id );
  uint32_t in_port = in_port16;
  check_expected( in_port );
  check_expected( actions );
  check_expected( data );

  return ( buffer * ) mock();
}


bool
mock_delete_actions( openflow_actions_t *actions ) {
  check_expected( actions );

  return ( bool ) mock();
}


void
mock_set_match_from_packet( struct ofp_match *match,
                            /*const*/ uint16_t in_port16,
                            /*const*/ uint32_t wildcards,
                            /*const*/ buffer *packet ) {
  check_expected( match );
  uint32_t in_port = in_port16;
  check_expected( in_port );
  check_expected( wildcards );
  check_expected( packet );

  ( void ) mock();
}


uint64_t
mock_get_cookie( void ) {
  return 0x123456789abcdef0ULL;
}


buffer *
mock_create_flow_mod( /*const*/ uint32_t transaction_id,
                      /*const*/ struct ofp_match match0,
                      /*const*/ uint64_t cookie64,
                      /*const*/ uint16_t command16,
                      /*const*/ uint16_t idle_timeout16,
                      /*const*/ uint16_t hard_timeout16,
                      /*const*/ uint16_t priority16,
                      /*const*/ uint32_t buffer_id,
                      /*const*/ uint16_t out_port16,
                      /*const*/ uint16_t flags16,
                      /*const*/ openflow_actions_t *actions ) {
  check_expected( transaction_id );
  struct ofp_match *match = &match0;
  check_expected( match );
  uint64_t *cookie = &cookie64;
  check_expected( cookie );
  uint32_t command = command16;
  check_expected( command );
  uint32_t idle_timeout = idle_timeout16;
  check_expected( idle_timeout );
  uint32_t hard_timeout = hard_timeout16;
  check_expected( hard_timeout );
  uint32_t priority = priority16;
  check_expected( priority );
  check_expected( buffer_id );
  uint32_t out_port = out_port16;
  check_expected( out_port );
  uint32_t flags = flags16;
  check_expected( flags );
  check_expected( actions );

  return ( buffer * ) mock();
}


buffer *
mock_create_set_config( /*const*/ uint32_t transaction_id, /*const*/ uint16_t flags16, uint16_t miss_send_len16 ) {
  check_expected( transaction_id );
  uint32_t flags = flags16;
  check_expected( flags );
  uint32_t miss_send_len = miss_send_len16;
  check_expected( miss_send_len );

  return ( buffer * ) mock();
}


buffer *
mock_create_features_request( /*const*/ uint32_t transaction_id ) {
  check_expected( transaction_id );

  return ( buffer * ) mock();
}


bool
mock_set_features_reply_handler( features_reply_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}


bool
mock_set_switch_ready_handler( switch_ready_handler callback, void *user_data ) {
  check_expected( callback );
  check_expected( user_data );

  return ( bool ) mock();
}


static dlist_element *resolved_hops = NULL;


bool
mock_resolve_path( uint64_t in_dpid64, uint16_t in_port16,
                   uint64_t out_dpid64, uint16_t out_port16, void *param,
                   resolve_path_callback callback ) {
  uint64_t *in_dpid = &in_dpid64;
  uint32_t in_port = in_port16;
  uint64_t *out_dpid = &out_dpid64;
  uint32_t out_port = out_port16;

  check_expected( in_dpid );
  check_expected( in_port );
  check_expected( out_dpid );
  check_expected( out_port );
  check_expected( param );
  check_expected( callback );

  bool ret = ( bool ) mock();

  ( *callback )( param, resolved_hops );

  return ret;
}


bool
mock_send_openflow_message( /*const*/ uint64_t dpid64, buffer *m ) {
  uint64_t *dpid = &dpid64;
  check_expected( dpid );
  check_expected( m );

  return (bool)mock();
}


bool
mock_init_libtopology( /*const*/ char *service_name ) {
  check_expected( service_name );
  return (bool)mock();
}


void
mock_subscribe_topology( void ( *callback )( void *param ), void *param ) {
  check_expected( callback );
  check_expected( param );
  ( void ) mock();

  ( *callback )( param );
}


bool
mock_set_packet_in_handler( void ( *callback )( uint64_t datapath_id,
                                                uint32_t transaction_id,
                                                uint32_t buffer_id,
                                                uint16_t total_len,
                                                uint16_t in_port,
                                                uint8_t reason,
                                                const buffer *data,
                                                void *user_data ),
                            void *user_data ) {
  check_expected( callback );
  check_expected( user_data );
  return (bool)mock();
}


bool
mock_add_callback_port_status_updated(
  void ( *callback )( void *,
                      const topology_port_status * ),
  void *param ) {
  check_expected( callback );
  check_expected( param );
  return (bool)mock();
}


bool
mock_finalize_libtopology() {
  return (bool)mock();
}


static bool fail_to_parse_packet = false;


bool
mock_parse_packet( buffer *m ) {
  m->user_data = xmalloc( 1 );
  if ( fail_to_parse_packet ) {
    return false;
  } else {
    return true;
  }
}


bool
mock_get_all_port_status( void ( *callback )( void *param, size_t n_entries, const topology_port_status *s ), void *param ) {
  check_expected( callback );
  check_expected( param );

  bool ret = ( bool ) mock();

  ( *callback )( param, number_of_test_port_status, test_port_status );

  return ret;
}


void
mock_finalize_topology_service_interface_options( void ) {
  ( void ) mock();
}


void
mock_exit( int status ) {
  UNUSED( status );
  ( void ) mock();
}


void
mock_usage() {
}


bool
mock_init_age_fdb( hash_table *fdb ) {
  check_expected( fdb );

  return ( bool ) mock();
}


bool
mock_update_fdb( hash_table *fdb, /*const*/ uint8_t mac[ OFP_ETH_ALEN ], uint64_t dpid64, uint16_t port16 ) {
  uint64_t *dpid = &dpid64;
  uint32_t port = port16;

  check_expected( fdb );
  check_expected( mac );
  check_expected( dpid );
  check_expected( port );

  return ( bool ) mock();
}


static uint64_t lookup_fdb_dpid = 0;
static uint16_t lookup_fdb_port = 0;

bool
mock_lookup_fdb( hash_table *fdb, /*const*/ uint8_t mac[ OFP_ETH_ALEN ], uint64_t *dpid, uint16_t *port ) {
  check_expected( fdb );
  check_expected( mac );
  check_expected( dpid );
  check_expected( port );

  *dpid = lookup_fdb_dpid;
  *port = lookup_fdb_port;

  return ( bool ) mock();
}


hash_table *
mock_create_fdb() {
  return ( hash_table * ) mock();
}


void
mock_delete_fdb( hash_table *fdb ) {
  check_expected( fdb );

  ( void ) mock();
}


void
mock_delete_outbound_port( list_element **switches, port_info *delete_port ) {
  check_expected( switches );
  check_expected( delete_port );

  ( void ) mock();
}


void
mock_add_outbound_port( list_element **switches, uint64_t dpid64, uint16_t port_no16 ) {
  check_expected( switches );
  uint64_t *dpid = &dpid64;
  check_expected( dpid );
  uint32_t port_no = port_no16;
  check_expected( port_no );

  ( void ) mock();
}


void
mock_delete_outbound_all_ports( list_element **switches ) {
  check_expected( switches );

  ( void ) mock();
}


port_info *
mock_lookup_outbound_port( list_element *switches, uint64_t dpid64, uint16_t port_no16 ) {
  check_expected( switches );
  uint64_t *dpid = &dpid64;
  check_expected( dpid );
  uint32_t port_no = port_no16;
  check_expected( port_no );

  return ( port_info * ) mock();
}


list_element *
mock_create_outbound_ports( list_element **switches ) {
  create_list( switches );

  return *switches;
}


int
mock_foreach_port( const list_element *ports,
                   int ( *function )( port_info *port,
                                      openflow_actions_t *actions,
                                      uint64_t dpid, uint16_t in_port ),
                   openflow_actions_t *actions, uint64_t dpid, uint16_t in_port ) {
  // call real foreach_port()
  return foreach_port( ports, function, actions, dpid, in_port );
}


void
mock_foreach_switch( const list_element *switches,
                     void ( *function )( switch_info *sw,
                                         buffer *packet,
                                         uint64_t dpid,
                                         uint16_t in_port ),
                     buffer *packet, uint64_t dpid, uint16_t in_port ) {
  // call real foreach_switch()
  foreach_switch( switches, function, packet, dpid, in_port );
}


/************************************************************************
 * Tests.                                                               
 ************************************************************************/


static void
test_mock_die() {
  char message[] = "die";
  mock_die( message );
}


static void
test_create_routing_switch_successed() {
  // Initialize log
  init_log( "routing_switch_test", false );

  char *test_topology_service = xstrdup( topology_service );

  // create_fdb()
  will_return( mock_create_fdb, &dummy_fdb );

  // create_outbound_ports()
  // no mock

  // init_libtopology()
  expect_string( mock_init_libtopology, service_name, test_topology_service );
  will_return( mock_init_libtopology, true );

  // subscribe_topology()
  expect_value( mock_subscribe_topology, callback, after_subscribed );
  expect_not_value( mock_subscribe_topology, param, NULL );
  will_return( mock_subscribe_topology, VOID_FUNCTION );

  // get_all_port_status()
  expect_value( mock_get_all_port_status, callback, init_last_stage );
  expect_not_value( mock_get_all_port_status, param, NULL );
  will_return( mock_get_all_port_status, true );

  // init_outbound_ports() ->
  // add_outbound_port()
  uint64_t dpid_0x1234 = 0x1234ULL;
  expect_not_value( mock_add_outbound_port, switches, NULL );
  expect_memory( mock_add_outbound_port, dpid, &dpid_0x1234, sizeof( uint64_t ) );
  expect_value( mock_add_outbound_port, port_no, 1 );
  will_return( mock_add_outbound_port, VOID_FUNCTION );

  // set_miss_send_len_maximum() ->
  // create_set_config() for 0x1234ULL
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  buffer dummy_set_config_message;
  will_return( mock_create_set_config, &dummy_set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_0x1234, sizeof( dpid_0x1234 ) );
  expect_value( mock_send_openflow_message, m, &dummy_set_config_message );
  will_return( mock_send_openflow_message, true );

  // add_outbound_port()
  expect_not_value( mock_add_outbound_port, switches, NULL );
  expect_memory( mock_add_outbound_port, dpid, &dpid_0x1234, sizeof( uint64_t ) );
  expect_value( mock_add_outbound_port, port_no, 2 );
  will_return( mock_add_outbound_port, VOID_FUNCTION );

  // create_set_config() for 0x1234ULL
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  will_return( mock_create_set_config, &dummy_set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_0x1234, sizeof( dpid_0x1234 ) );
  expect_value( mock_send_openflow_message, m, &dummy_set_config_message );
  will_return( mock_send_openflow_message, true );

  // add_outbound_port()
  uint64_t dpid_0x456789 = 0x456789ULL;
  expect_not_value( mock_add_outbound_port, switches, NULL );
  expect_memory( mock_add_outbound_port, dpid, &dpid_0x456789, sizeof( uint64_t ) );
  expect_value( mock_add_outbound_port, port_no, 1 );
  will_return( mock_add_outbound_port, VOID_FUNCTION );
  
  // create_set_config() for 0x456789ULL
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  will_return( mock_create_set_config, &dummy_set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_0x456789, sizeof( dpid_0x456789 ) );
  expect_value( mock_send_openflow_message, m, &dummy_set_config_message );
  will_return( mock_send_openflow_message, true );
  
  // init_age_fdb()
  expect_value( mock_init_age_fdb, fdb, &dummy_fdb );
  will_return ( mock_init_age_fdb, true );

  // set_features_reply_handler()
  expect_value( mock_set_features_reply_handler, callback, receive_features_reply );
  expect_not_value( mock_set_features_reply_handler, user_data, NULL );
  will_return( mock_set_features_reply_handler, true );

  // set_switch_ready_handler
  expect_value( mock_set_switch_ready_handler, callback, switch_ready );
  expect_not_value( mock_set_switch_ready_handler, user_data, NULL );
  will_return( mock_set_switch_ready_handler, true );

  // add_callback_port_status_updated()
  expect_value( mock_add_callback_port_status_updated, callback, port_status_updated );
  expect_not_value( mock_add_callback_port_status_updated, param, NULL );
  will_return ( mock_add_callback_port_status_updated, true );

  // set_packet_in_handler()
  expect_value( mock_set_packet_in_handler, callback, handle_packet_in );
  expect_not_value( mock_set_packet_in_handler, user_data, NULL );
  will_return( mock_set_packet_in_handler, true );

  // target
  routing_switch_options options;
  options.idle_timeout = FLOW_TIMER;
  routing_switch *routing_switch = create_routing_switch( topology_service, &options );
  assert_true( routing_switch != NULL );

  // check if correct idle_timer is set
  assert_true( routing_switch->idle_timeout == FLOW_TIMER );

  // free outboud_ports
  delete_list( routing_switch->switches );

  // free routing_switch
  xfree( routing_switch );

  // free test_topology_service
  xfree( test_topology_service );
}


static void
test_delete_routing_switch_successed() {
  routing_switch *routing_switch = setup();

  // finalize_libtopology()
  will_return( mock_finalize_libtopology, VOID_FUNCTION );

  // delete_outbound_all_ports()
  list_element *switches = routing_switch->switches;
  expect_value( mock_delete_outbound_all_ports, switches, &routing_switch->switches );
  will_return( mock_delete_outbound_all_ports, VOID_FUNCTION );

  // delete_fdb()
  expect_value( mock_delete_fdb, fdb, routing_switch->fdb );
  will_return( mock_delete_fdb, VOID_FUNCTION );

  // target
  delete_routing_switch( routing_switch );

  // teardown
  teardown_outbound_ports_for_test( &switches );
}


static void
test_receive_port_status_of_up_and_external() {
  routing_switch *routing_switch = setup();

  topology_port_status port_up_and_external;
  uint64_t dpid = dpid_switch_A;
  uint16_t port_no = new_port_no_of_switch_A;
  setup_port_status( &port_up_and_external, dpid, port_no, TD_PORT_EXTERNAL, TD_PORT_UP );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) new_port_no_of_switch_A );
  will_return( mock_lookup_outbound_port, NULL );

  // add_outbound_port()
  expect_value( mock_add_outbound_port, switches, &routing_switch->switches );
  expect_memory( mock_add_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_add_outbound_port, port_no, ( uint32_t ) new_port_no_of_switch_A );
  will_return( mock_add_outbound_port, VOID_FUNCTION );

  // create_set_config()
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  buffer dummy_set_config_message;
  will_return( mock_create_set_config, &dummy_set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid, sizeof( dpid ) );
  expect_value( mock_send_openflow_message, m, &dummy_set_config_message );
  will_return( mock_send_openflow_message, true );
  
  // target
  port_status_updated( routing_switch, &port_up_and_external );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) new_port_no_of_switch_A );
  will_return( mock_lookup_outbound_port, NULL );

  // add_outbound_port()
  expect_value( mock_add_outbound_port, switches, &routing_switch->switches );
  expect_memory( mock_add_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_add_outbound_port, port_no, ( uint32_t ) new_port_no_of_switch_A );
  will_return( mock_add_outbound_port, VOID_FUNCTION );

  // create_set_config()
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  will_return( mock_create_set_config, &dummy_set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid, sizeof( dpid ) );
  expect_value( mock_send_openflow_message, m, &dummy_set_config_message );
  will_return( mock_send_openflow_message, true );
  
  // duplicated update is also OK
  port_status_updated( routing_switch, &port_up_and_external );

  teardown( routing_switch );
}


static void
test_receive_port_status_of_up_and_external_max_port_no() {
  routing_switch *routing_switch = setup();

  topology_port_status port_up_and_external;
  uint64_t dpid = dpid_switch_A;
  uint16_t port_no = OFPP_MAX;
  setup_port_status( &port_up_and_external, dpid, port_no, TD_PORT_EXTERNAL, TD_PORT_UP );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) OFPP_MAX );
  will_return( mock_lookup_outbound_port, NULL );

  // add_outbound_port()
  expect_value( mock_add_outbound_port, switches, &routing_switch->switches );
  expect_memory( mock_add_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_add_outbound_port, port_no, ( uint32_t ) OFPP_MAX );
  will_return( mock_add_outbound_port, VOID_FUNCTION );

  // create_set_config()
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  buffer dummy_set_config_message;
  will_return( mock_create_set_config, &dummy_set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid, sizeof( dpid ) );
  expect_value( mock_send_openflow_message, m, &dummy_set_config_message );
  will_return( mock_send_openflow_message, true );
  
  // target
  port_status_updated( routing_switch, &port_up_and_external );

  teardown( routing_switch );
}


static void
test_receive_port_status_of_up_and_external_pseudo_port_no() {
  routing_switch *routing_switch = setup();

  const uint16_t pseudo_port_no[] = { OFPP_IN_PORT, OFPP_TABLE, OFPP_NORMAL, OFPP_FLOOD, OFPP_ALL, OFPP_CONTROLLER, OFPP_LOCAL, OFPP_NONE };
  const size_t number_of_pseudo_port_no = sizeof( pseudo_port_no ) / sizeof( pseudo_port_no[ 0 ] );
  for ( size_t i = 0; i < number_of_pseudo_port_no; i++ ) {
    topology_port_status pseudo_port;
    uint64_t dpid = dpid_switch_A;
    uint16_t port_no = pseudo_port_no[ i ];
    setup_port_status( &pseudo_port, dpid, port_no, TD_PORT_EXTERNAL, TD_PORT_UP );

    // target
    port_status_updated( routing_switch, &pseudo_port );
  }
  
  teardown( routing_switch );
}


static void
test_receive_port_status_of_up_and_external_which_is_already_existed() {
  routing_switch *routing_switch = setup();

  topology_port_status port_already_existed;
  uint64_t dpid = dpid_switch_A;
  uint16_t port_no = port_no_from_A_to_B; // already registered
  setup_port_status( &port_already_existed, dpid, port_no, TD_PORT_EXTERNAL, TD_PORT_UP );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_B );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // target
  port_status_updated( routing_switch, &port_already_existed );

  teardown( routing_switch );
}


static void
test_receive_port_status_of_down_and_external() {
  routing_switch *routing_switch = setup();

  topology_port_status port_down_and_external;
  uint64_t dpid = dpid_switch_A;
  uint16_t port_no = port_no_from_A_to_B; // already registered

  setup_port_status( &port_down_and_external, dpid, port_no, TD_PORT_EXTERNAL, TD_PORT_DOWN );
  
  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_B );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // delete_outbound_port()
  expect_value( mock_delete_outbound_port, switches, &routing_switch->switches );
  expect_value( mock_delete_outbound_port, delete_port, &dummy_port );
  will_return( mock_delete_outbound_port, VOID_FUNCTION );

  // target
  port_status_updated( routing_switch, &port_down_and_external );

  teardown( routing_switch );
}


static void
test_receive_port_status_of_up_and_inactive() {
  routing_switch *routing_switch = setup();

  topology_port_status port_up_and_inactive;
  uint64_t dpid = dpid_switch_A;
  uint16_t port_no = port_no_from_A_to_B; // already registered

  setup_port_status( &port_up_and_inactive, dpid, port_no, TD_PORT_INACTIVE, TD_PORT_UP );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_B );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // delete_outbound_port()
  expect_value( mock_delete_outbound_port, switches, &routing_switch->switches );
  expect_value( mock_delete_outbound_port, delete_port, &dummy_port );
  will_return( mock_delete_outbound_port, VOID_FUNCTION );

  // target
  port_status_updated( routing_switch, &port_up_and_inactive );

  teardown( routing_switch );
}


static void
test_receive_port_status_of_down_and_inactive() {
  routing_switch *routing_switch = setup();

  topology_port_status port_down_and_inactive;
  uint64_t dpid = dpid_switch_A;
  uint16_t port_no = port_no_from_A_to_B; // already registered

  setup_port_status( &port_down_and_inactive, dpid, port_no, TD_PORT_INACTIVE, TD_PORT_DOWN );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_B );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // delete_outbound_port()
  expect_value( mock_delete_outbound_port, switches, &routing_switch->switches );
  expect_value( mock_delete_outbound_port, delete_port, &dummy_port );
  will_return( mock_delete_outbound_port, VOID_FUNCTION );

  // target
  port_status_updated( routing_switch, &port_down_and_inactive );
  
  teardown( routing_switch );
}


static void
test_receive_port_status_of_down_and_inactive_which_is_not_existed() {
  routing_switch *routing_switch = setup();

  topology_port_status port_down_and_inactive;
  uint64_t dpid = dpid_switch_D; // new switch
  uint16_t port_no = new_port_of_switch_D; // new port

  setup_port_status( &port_down_and_inactive, dpid, port_no, TD_PORT_INACTIVE, TD_PORT_DOWN );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_D, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) new_port_of_switch_D );
  will_return( mock_lookup_outbound_port, NULL );

  // target
  port_status_updated( routing_switch, &port_down_and_inactive );

  teardown( routing_switch );
}


static void
test_receive_packet_in_from_unknown_port() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_A;
  in_port = unknown_port_of_switch_A;
  test_packet = setup_test_packet( host_a_mac, host_aa_mac ); // src:host_a, dst:host_aa

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) unknown_port_of_switch_A );
  will_return( mock_lookup_outbound_port, NULL );

  // lookup_fdb()
  expect_value( mock_lookup_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_lookup_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_not_value( mock_lookup_fdb, dpid, NULL );
  expect_not_value( mock_lookup_fdb, port, NULL );
  will_return( mock_lookup_fdb, false );

  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  // expects ignore this packet

  teardown( routing_switch );
}


static void
test_receive_packet_in_detects_host_moving() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_A;
  in_port = port_no_from_A_to_host_aa; // host_a moved from a to aa
  test_packet = setup_test_packet( host_a_mac, host_aa_mac );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_host_aa );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // update_fdb()
  expect_value( mock_update_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_update_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_memory( mock_update_fdb, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_update_fdb, port, ( uint32_t ) port_no_from_A_to_host_aa );
  will_return( mock_update_fdb, false );

  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  // expects ignore this packet

  teardown( routing_switch );
}


static void
test_receive_packet_in_in_port_and_out_port_are_same() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_A;
  in_port = port_no_from_A_to_host_a;
  test_packet = setup_test_packet( host_a_mac, host_a_mac );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // update_fdb()
  expect_value( mock_update_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_update_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_memory( mock_update_fdb, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_update_fdb, port, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_update_fdb, true );

  // lookup_fdb()
  lookup_fdb_dpid = dpid_switch_A;
  lookup_fdb_port = port_no_from_A_to_host_a;
  expect_value( mock_lookup_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_lookup_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_not_value( mock_lookup_fdb, dpid, NULL );
  expect_not_value( mock_lookup_fdb, port, NULL );
  will_return( mock_lookup_fdb, true );

  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  // expects ignore this packet

  teardown( routing_switch );
}


static void
test_receive_packet_in_no_available_path_found() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_A;
  in_port = port_no_from_A_to_host_a;
  test_packet = setup_test_packet( host_a_mac, host_b_mac );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // update_fdb()
  expect_value( mock_update_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_update_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_memory( mock_update_fdb, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_update_fdb, port, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_update_fdb, true );

  // lookup_fdb()
  lookup_fdb_dpid = dpid_switch_B;
  lookup_fdb_port = port_no_from_B_to_host_b;
  expect_value( mock_lookup_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_lookup_fdb, mac, host_b_mac, ETH_ADDRLEN );
  expect_not_value( mock_lookup_fdb, dpid, NULL );
  expect_not_value( mock_lookup_fdb, port, NULL );
  will_return( mock_lookup_fdb, true );

  // resolve_path()
  expect_memory( mock_resolve_path, in_dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_resolve_path, in_port, ( uint32_t ) port_no_from_A_to_host_a );
  expect_memory( mock_resolve_path, out_dpid, &dpid_switch_B, sizeof( uint64_t ) );
  expect_value( mock_resolve_path, out_port, ( uint32_t ) port_no_from_B_to_host_b );
  expect_any( mock_resolve_path, param );
  expect_value( mock_resolve_path, callback, resolve_path_replied );
  will_return( mock_resolve_path, true );

  resolved_hops = NULL;

  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  // expects ignore this packet

  teardown( routing_switch );
}


static void
test_receive_packet_in_parse_packet_failed() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_A;
  in_port = port_no_from_A_to_host_a;
  test_packet = setup_test_packet( host_a_mac, host_c_mac );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // update_fdb()
  expect_value( mock_update_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_update_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_memory( mock_update_fdb, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_update_fdb, port, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_update_fdb, true );

  // lookup_fdb()
  lookup_fdb_dpid = dpid_switch_C;
  lookup_fdb_port = port_no_from_C_to_host_c;
  expect_value( mock_lookup_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_lookup_fdb, mac, host_c_mac, ETH_ADDRLEN );
  expect_not_value( mock_lookup_fdb, dpid, NULL );
  expect_not_value( mock_lookup_fdb, port, NULL );
  will_return( mock_lookup_fdb, true );

  // resolve_path()
  expect_memory( mock_resolve_path, in_dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_resolve_path, in_port, ( uint32_t ) port_no_from_A_to_host_a );
  expect_memory( mock_resolve_path, out_dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_resolve_path, out_port, ( uint32_t ) port_no_from_C_to_host_c );
  expect_any( mock_resolve_path, param );
  expect_value( mock_resolve_path, callback, resolve_path_replied );
  will_return( mock_resolve_path, true );

  // A->B->C
  resolved_hops = setup_test_hops();

  fail_to_parse_packet = true;

  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  // expects ignore this packet

  fail_to_parse_packet = false;
  teardown( routing_switch );
}


static void
test_receive_packet_in_destination_host_is_located() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_A;
  in_port = port_no_from_A_to_host_a;
  test_packet = setup_test_packet( host_a_mac, host_c_mac );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // update_fdb()
  expect_value( mock_update_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_update_fdb, mac, host_a_mac, ETH_ADDRLEN );
  expect_memory( mock_update_fdb, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_update_fdb, port, ( uint32_t ) port_no_from_A_to_host_a );
  will_return( mock_update_fdb, true );

  // lookup_fdb()
  lookup_fdb_dpid = dpid_switch_C;
  lookup_fdb_port = port_no_from_C_to_host_c;
  expect_value( mock_lookup_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_lookup_fdb, mac, host_c_mac, ETH_ADDRLEN );
  expect_not_value( mock_lookup_fdb, dpid, NULL );
  expect_not_value( mock_lookup_fdb, port, NULL );
  will_return( mock_lookup_fdb, true );

  // resolve_path()
  expect_memory( mock_resolve_path, in_dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_resolve_path, in_port, ( uint32_t ) port_no_from_A_to_host_a );
  expect_memory( mock_resolve_path, out_dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_resolve_path, out_port, ( uint32_t ) port_no_from_C_to_host_c );
  expect_any( mock_resolve_path, param );
  expect_value( mock_resolve_path, callback, resolve_path_replied );
  will_return( mock_resolve_path, true );

  // A->B->C
  resolved_hops = setup_test_hops();


  // for switch C
  // set_match_from_packet();
  expect_any( mock_set_match_from_packet, match );
  expect_value( mock_set_match_from_packet, in_port, ( uint32_t ) port_no_from_C_to_B );
  expect_value( mock_set_match_from_packet, wildcards, 0 );
  expect_any( mock_set_match_from_packet, packet );
  will_return( mock_set_match_from_packet, VOID_FUNCTION );

  // create_actions()
  openflow_actions_t dummy_actions;
  will_return( mock_create_actions, &dummy_actions );

  // append_aciton_output()
  expect_value( mock_append_action_output, actions, &dummy_actions );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_C_to_host_c );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );

  // create_flow_mod()
  expect_value( mock_create_flow_mod, transaction_id, mock_get_transaction_id() );
  expect_any( mock_create_flow_mod, match );
  uint64_t expected_cookie = mock_get_cookie();
  expect_memory( mock_create_flow_mod, cookie, &expected_cookie, sizeof( uint64_t ) );
  expect_value( mock_create_flow_mod, command, OFPFC_ADD );
  expect_value( mock_create_flow_mod, idle_timeout, ( uint32_t ) ( routing_switch->idle_timeout + 3 ) );
  expect_value( mock_create_flow_mod, hard_timeout, 0 );
  expect_value( mock_create_flow_mod, priority, UINT16_MAX );
  expect_value( mock_create_flow_mod, buffer_id, UINT32_MAX );
  expect_value( mock_create_flow_mod, out_port, ( uint32_t ) port_no_from_C_to_host_c );
  expect_value( mock_create_flow_mod, flags, 0 );
  expect_value( mock_create_flow_mod, actions, &dummy_actions );
  buffer *dummy_buffer = alloc_buffer();
  will_return( mock_create_flow_mod, dummy_buffer );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, m, dummy_buffer );
  will_return( mock_send_openflow_message, true );

  // delete_actions()
  expect_value( mock_delete_actions, actions, &dummy_actions );
  will_return( mock_delete_actions, true );

  // for switch B
  // set_match_from_packet();
  expect_any( mock_set_match_from_packet, match );
  expect_value( mock_set_match_from_packet, in_port, ( uint32_t ) port_no_from_B_to_A );
  expect_value( mock_set_match_from_packet, wildcards, 0 );
  expect_any( mock_set_match_from_packet, packet );
  will_return( mock_set_match_from_packet, VOID_FUNCTION );

  // create_actions()
  will_return( mock_create_actions, &dummy_actions );

  // append_aciton_output()
  expect_value( mock_append_action_output, actions, &dummy_actions );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_B_to_C );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );

  // create_flow_mod()
  expect_value( mock_create_flow_mod, transaction_id, mock_get_transaction_id() );
  expect_any( mock_create_flow_mod, match );
  expect_memory( mock_create_flow_mod, cookie, &expected_cookie, sizeof( uint64_t ) ) ;
  expect_value( mock_create_flow_mod, command, OFPFC_ADD );
  expect_value( mock_create_flow_mod, idle_timeout, ( uint32_t ) ( routing_switch->idle_timeout + 2 ) );
  expect_value( mock_create_flow_mod, hard_timeout, 0 );
  expect_value( mock_create_flow_mod, priority, UINT16_MAX );
  expect_value( mock_create_flow_mod, buffer_id, UINT32_MAX );
  expect_value( mock_create_flow_mod, out_port, ( uint32_t ) port_no_from_B_to_C );
  expect_value( mock_create_flow_mod, flags, 0 );
  expect_value( mock_create_flow_mod, actions, &dummy_actions );
  dummy_buffer = alloc_buffer();
  will_return( mock_create_flow_mod, dummy_buffer );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_B, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, m, dummy_buffer );
  will_return( mock_send_openflow_message, true );

  // delete_actions()
  expect_value( mock_delete_actions, actions, &dummy_actions );
  will_return( mock_delete_actions, true );

  // for switch A
  // set_match_from_packet();
  expect_any( mock_set_match_from_packet, match );
  expect_value( mock_set_match_from_packet, in_port, ( uint32_t ) port_no_from_A_to_host_a );
  expect_value( mock_set_match_from_packet, wildcards, 0 );
  expect_any( mock_set_match_from_packet, packet );
  will_return( mock_set_match_from_packet, VOID_FUNCTION );

  // create_actions()
  will_return( mock_create_actions, &dummy_actions );

  // append_aciton_output()
  expect_value( mock_append_action_output, actions, &dummy_actions );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_A_to_B );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );

  // create_flow_mod()
  expect_value( mock_create_flow_mod, transaction_id, mock_get_transaction_id() );
  expect_any( mock_create_flow_mod, match );
  expect_memory( mock_create_flow_mod, cookie, &expected_cookie, sizeof( uint64_t ) );
  expect_value( mock_create_flow_mod, command, OFPFC_ADD );
  expect_value( mock_create_flow_mod, idle_timeout, ( uint32_t ) ( routing_switch->idle_timeout + 1 ) );
  expect_value( mock_create_flow_mod, hard_timeout, 0 );
  expect_value( mock_create_flow_mod, priority, UINT16_MAX );
  expect_value( mock_create_flow_mod, buffer_id, UINT32_MAX );
  expect_value( mock_create_flow_mod, out_port, ( uint32_t ) port_no_from_A_to_B );
  expect_value( mock_create_flow_mod, flags, 0 );
  expect_value( mock_create_flow_mod, actions, &dummy_actions );
  dummy_buffer = alloc_buffer();
  will_return( mock_create_flow_mod, dummy_buffer );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, m, dummy_buffer );
  will_return( mock_send_openflow_message, true );

  // delete_actions()
  expect_value( mock_delete_actions, actions, &dummy_actions );
  will_return( mock_delete_actions, true );

  // -- packet out

  // create_actions()
  will_return( mock_create_actions, &dummy_actions );

  // append_action_output()
  expect_value( mock_append_action_output, actions, &dummy_actions );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_C_to_host_c );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );

  // create_packet_out()
  expect_value( mock_create_packet_out, transaction_id, mock_get_transaction_id() );
  expect_value( mock_create_packet_out, buffer_id, UINT32_MAX );
  expect_value( mock_create_packet_out, in_port, OFPP_NONE );
  expect_value( mock_create_packet_out, actions, &dummy_actions );
  expect_any( mock_create_packet_out, data );
  dummy_buffer = alloc_buffer();
  will_return( mock_create_packet_out, dummy_buffer );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, m, dummy_buffer );
  will_return( mock_send_openflow_message, true );

  // delete_actions()
  expect_value( mock_delete_actions, actions, &dummy_actions );
  will_return( mock_delete_actions, true );

  
  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  teardown( routing_switch );
}


static void
test_receive_packet_in_destination_host_is_not_located() {
  routing_switch *routing_switch = setup();

  dpid = dpid_switch_B;
  in_port = port_no_from_B_to_host_b;
  test_packet = setup_test_packet( host_b_mac, host_c_mac );

  // lookup_outbound_port()
  expect_value( mock_lookup_outbound_port, switches, routing_switch->switches );
  expect_memory( mock_lookup_outbound_port, dpid, &dpid_switch_B, sizeof( uint64_t ) );
  expect_value( mock_lookup_outbound_port, port_no, ( uint32_t ) port_no_from_B_to_host_b );
  will_return( mock_lookup_outbound_port, &dummy_port );

  // update_fdb()
  expect_value( mock_update_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_update_fdb, mac, host_b_mac, ETH_ADDRLEN );
  expect_memory( mock_update_fdb, dpid, &dpid_switch_B, sizeof( uint64_t ) );
  expect_value( mock_update_fdb, port, ( uint32_t ) port_no_from_B_to_host_b );
  will_return( mock_update_fdb, true );

  // lookup_fdb()
  lookup_fdb_dpid = dpid_switch_C;
  lookup_fdb_port = port_no_from_C_to_host_c;
  expect_value( mock_lookup_fdb, fdb, routing_switch->fdb );
  expect_memory( mock_lookup_fdb, mac, host_c_mac, ETH_ADDRLEN );
  expect_not_value( mock_lookup_fdb, dpid, NULL );
  expect_not_value( mock_lookup_fdb, port, NULL );
  will_return( mock_lookup_fdb, false );

  // switch A
  // create_actions()
  openflow_actions_t actions_for_switch_A;
  will_return( mock_create_actions, &actions_for_switch_A );
  // host_a
  // append_action_output()
  expect_value( mock_append_action_output, actions, &actions_for_switch_A );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_A_to_host_a );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );
  
  // host_aa
  // append_action_output()
  expect_value( mock_append_action_output, actions, &actions_for_switch_A );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_A_to_host_aa );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );

  // create_packet_out()
  expect_value( mock_create_packet_out, transaction_id, transaction_id );
  expect_value( mock_create_packet_out, buffer_id, UINT32_MAX );
  expect_value( mock_create_packet_out, in_port, OFPP_NONE );
  expect_value( mock_create_packet_out, actions, &actions_for_switch_A );
  expect_not_value( mock_create_packet_out, data, NULL );
  buffer *packet_out_for_switch_A = alloc_buffer();
  will_return( mock_create_packet_out, packet_out_for_switch_A );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_A, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, m, packet_out_for_switch_A );
  will_return( mock_send_openflow_message, true );

  // delete_actions()
  expect_value( mock_delete_actions, actions, &actions_for_switch_A );
  will_return( mock_delete_actions, true );

  // switch B is skipped
  // create_actions()
  openflow_actions_t actions_for_switch_B;
  will_return( mock_create_actions, &actions_for_switch_B );
  // delete_actions()
  expect_value( mock_delete_actions, actions, &actions_for_switch_B );
  will_return( mock_delete_actions, true );
  
  // switch C
  // create_actions()
  openflow_actions_t actions_for_switch_C;
  will_return( mock_create_actions, &actions_for_switch_C );
  // host_c
  // append_action_output()
  expect_value( mock_append_action_output, actions, &actions_for_switch_C );
  expect_value( mock_append_action_output, port, ( uint32_t ) port_no_from_C_to_host_c );
  expect_value( mock_append_action_output, max_len, UINT16_MAX );
  will_return( mock_append_action_output, true );

  // create_packet_out()
  expect_value( mock_create_packet_out, transaction_id, transaction_id );
  expect_value( mock_create_packet_out, buffer_id, UINT32_MAX );
  expect_value( mock_create_packet_out, in_port, OFPP_NONE );
  expect_value( mock_create_packet_out, actions, &actions_for_switch_C );
  expect_not_value( mock_create_packet_out, data, NULL );
  buffer *packet_out_for_switch_C = alloc_buffer();
  will_return( mock_create_packet_out, packet_out_for_switch_C );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_C, sizeof( uint64_t ) );
  expect_value( mock_send_openflow_message, m, packet_out_for_switch_C );
  will_return( mock_send_openflow_message, true );

  // delete_actions()
  expect_value( mock_delete_actions, actions, &actions_for_switch_C );
  will_return( mock_delete_actions, true );

  // target
  handle_packet_in( dpid, transaction_id, buffer_id, total_len, in_port, dummy_reason, test_packet, routing_switch );

  teardown( routing_switch );
}


static void
test_new_switch_is_connected() {
  // switch_ready()

  // create_features_request()
  expect_value( mock_create_features_request, transaction_id, transaction_id );
  buffer *features_request_message = alloc_buffer();
  will_return( mock_create_features_request, features_request_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_A, sizeof( dpid_switch_A ) );
  expect_value( mock_send_openflow_message, m, features_request_message );
  will_return( mock_send_openflow_message, true );

  // target(switch_ready())
  switch_ready( dpid_switch_A, dummy_user_data );

  // receive_features_reply()

  // create_set_config()
  expect_value( mock_create_set_config, transaction_id, transaction_id );
  expect_value( mock_create_set_config, flags, OFPC_FRAG_NORMAL );
  expect_value( mock_create_set_config, miss_send_len, UINT16_MAX );
  buffer *set_config_message = alloc_buffer();
  will_return( mock_create_set_config, set_config_message );

  // send_openflow_message()
  expect_memory( mock_send_openflow_message, dpid, &dpid_switch_A, sizeof( dpid_switch_A ) );
  expect_value( mock_send_openflow_message, m, set_config_message );
  will_return( mock_send_openflow_message, true );

  // target(receive_features_reply())
  receive_features_reply( dpid_switch_A, transaction_id, n_buffers, n_tables, capabilities, actions, &phy_ports, dummy_user_data );

  // teardown
  free_buffer( features_request_message );
  free_buffer( set_config_message );
}


static void
test_default_idle_timer_is_set() {
  int argc = 2;
  char argv0[] = "argv0";
  char argv1[] = "argv1";
  char **argv = xcalloc( 3, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  // check
  assert_true( options.idle_timeout == FLOW_TIMER );

  // teardown
  xfree( argv );
}


static void
test_correct_idle_timer_option_is_specified() {
  int argc = 3;
  char argv0[] = "argv0";
  char argv1[] = "-i";
  char argv2_value5[] = "5";
  char **argv = xcalloc( 4, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value5;
  argv[ 3 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  // check
  assert_true( options.idle_timeout == 5 );

  reset_getopt_long();

  options.idle_timeout = 0;

  // target
  char argv2_value3333[] = "3333";
  argc = 3;
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value3333;
  argv[ 3 ] = NULL;

  init_routing_switch_options( &options, &argc, &argv );

  assert_true( options.idle_timeout == 3333 );

  reset_getopt_long();

  // teardown
  xfree( argv );
}


static void
test_correct_idle_timer_long_option_is_specified() {
  int argc = 3;
  char argv0[] = "argv0";
  char argv1[] = "--idle_timeout";
  char argv2_value5[] = "5";
  char **argv = xcalloc( 4, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value5;
  argv[ 3 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  // check
  assert_true( options.idle_timeout == 5 );

  reset_getopt_long();

  options.idle_timeout = 0;
  // target
  char argv2_value3333[] = "3333";
  argc = 3;
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value3333;
  argv[ 3 ] = NULL;

  init_routing_switch_options( &options, &argc, &argv );

  assert_true( options.idle_timeout == 3333 );

  reset_getopt_long();

  // teardown
  xfree( argv );
}


static void
test_correct_idle_timer_long_option_with_equal_sign_is_specified() {
  int argc = 2;
  char argv0[] = "argv0";
  char argv1[] = "--idle_timeout=5";
  char **argv = xcalloc( 3, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  // check
  assert_true( options.idle_timeout == 5 );

  reset_getopt_long();

  options.idle_timeout = 0;
  // target
  char argv1_value3333[] = "--idle_timeout=3333";
  argc = 2;
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1_value3333;
  argv[ 2 ] = NULL;

  init_routing_switch_options( &options, &argc, &argv );

  assert_true( options.idle_timeout == 3333 );

  reset_getopt_long();

  // teardown
  xfree( argv );
}


static void
test_invalid_idle_timer_option_is_specified() {
  int argc = 3;
  char argv0[] = "argv0";
  char argv1[] = "-i";
  char argv2_value0[] = "0";
  char **argv = xcalloc( 4, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value0;
  argv[ 3 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // finalize_topology_service_interface_options()
  will_return( mock_finalize_topology_service_interface_options, VOID_FUNCTION );
  // exit()
  will_return( mock_exit, VOID_FUNCTION );

  // target
  init_routing_switch_options( &options, &argc, &argv );

  reset_getopt_long();

  
  options.idle_timeout = 0;

  // finalize_topology_service_interface_options()
  will_return( mock_finalize_topology_service_interface_options, VOID_FUNCTION );
  // exit()
  will_return( mock_exit, VOID_FUNCTION );

  char argv2_value65536[] = "65536";
  argc = 3;
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value65536;
  argv[ 3 ] = NULL;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  reset_getopt_long();

  // teardown
  xfree( argv );
}


static void
test_invalid_idle_timer_long_option_is_specified() {
  int argc = 3;
  char argv0[] = "argv0";
  char argv1[] = "--idle_timeout";
  char argv2_value0[] = "0";
  char **argv = xcalloc( 4, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value0;
  argv[ 3 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // finalize_topology_service_interface_options()
  will_return( mock_finalize_topology_service_interface_options, VOID_FUNCTION );
  // exit()
  will_return( mock_exit, VOID_FUNCTION );

  // target
  init_routing_switch_options( &options, &argc, &argv );

  reset_getopt_long();

  
  options.idle_timeout = 0;

  // finalize_topology_service_interface_options()
  will_return( mock_finalize_topology_service_interface_options, VOID_FUNCTION );
  // exit()
  will_return( mock_exit, VOID_FUNCTION );

  char argv2_value65536[] = "65536";
  argc = 3;
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = argv2_value65536;
  argv[ 3 ] = NULL;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  reset_getopt_long();

  // teardown
  xfree( argv );
}


static void
test_unknown_option_is_specified() {
  int argc = 2;
  char argv0[] = "argv0";
  char argv1[] = "-Q";
  char **argv = xcalloc( 3, sizeof( char * ) );
  argv[ 0 ] = argv0;
  argv[ 1 ] = argv1;
  argv[ 2 ] = NULL;

  routing_switch_options options;
  options.idle_timeout = 0;

  // target
  init_routing_switch_options( &options, &argc, &argv );

  reset_getopt_long();

  // teardown
  xfree( argv );
}


int
main() {
  const UnitTest tests[] = {
    unit_test( test_mock_die ),

    unit_test( test_create_routing_switch_successed ),
    unit_test( test_delete_routing_switch_successed ),

    unit_test( test_receive_port_status_of_up_and_external ),
    unit_test( test_receive_port_status_of_up_and_external_max_port_no ),
    unit_test( test_receive_port_status_of_up_and_external_pseudo_port_no ),
    unit_test( test_receive_port_status_of_up_and_external_which_is_already_existed ),
    unit_test( test_receive_port_status_of_down_and_external ),
    unit_test( test_receive_port_status_of_up_and_inactive ),
    unit_test( test_receive_port_status_of_down_and_inactive ),
    unit_test( test_receive_port_status_of_down_and_inactive_which_is_not_existed ),

    unit_test( test_receive_packet_in_from_unknown_port ),
    unit_test( test_receive_packet_in_detects_host_moving ),
    unit_test( test_receive_packet_in_in_port_and_out_port_are_same ),
    unit_test( test_receive_packet_in_no_available_path_found ),
    unit_test( test_receive_packet_in_parse_packet_failed ),
    unit_test( test_receive_packet_in_destination_host_is_located ),
    unit_test( test_receive_packet_in_destination_host_is_not_located ),

    unit_test( test_new_switch_is_connected ),

    unit_test( test_default_idle_timer_is_set ),
    unit_test( test_correct_idle_timer_option_is_specified ),
    unit_test( test_correct_idle_timer_long_option_is_specified ),
    unit_test( test_correct_idle_timer_long_option_with_equal_sign_is_specified ),
    unit_test( test_invalid_idle_timer_option_is_specified ),
    unit_test( test_invalid_idle_timer_long_option_is_specified ),
    unit_test( test_unknown_option_is_specified ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
