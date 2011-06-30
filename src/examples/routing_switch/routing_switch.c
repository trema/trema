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


#include <arpa/inet.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trema.h"
#include "fdb.h"
#include "libpathresolver.h"
#include "libtopology.h"
#include "port.h"
#include "topology_service_interface_option_parser.h"


static const uint16_t FLOW_TIMER = 60;
static const uint16_t PACKET_IN_DISCARD_DURATION = 1;


#ifdef UNIT_TESTING

#ifdef free_hop_list
#undef free_hop_list
#endif
#define free_hop_list mock_free_hop_list
void mock_free_hop_list( dlist_element *hops );

#ifdef create_actions
#undef create_actions
#endif
#define create_actions mock_create_actions
openflow_actions *mock_create_actions( void );

#ifdef append_action_output
#undef append_action_output
#endif
#define append_action_output mock_append_action_output
bool mock_append_action_output( openflow_actions *actions, const uint16_t port, const uint16_t max_len );

#ifdef get_transaction_id
#undef get_transaction_id
#endif
#define get_transaction_id mock_get_transaction_id
uint32_t mock_get_transaction_id( void );

#ifdef create_packet_out
#undef create_packet_out
#endif
#define create_packet_out mock_create_packet_out
buffer *mock_create_packet_out( const uint32_t transaction_id,
                                const uint32_t buffer_id,
                                const uint16_t in_port,
                                const openflow_actions *actions,
                                const buffer *data );

#ifdef delete_actions
#undef delete_actions
#endif
#define delete_actions mock_delete_actions
bool mock_delete_actions( openflow_actions *actions );

#ifdef set_match_from_packet
#undef set_match_from_packet
#endif
#define set_match_from_packet mock_set_match_from_packet
void mock_set_match_from_packet( struct ofp_match *match,
                                 const uint16_t in_port,
                                 const uint32_t wildcards,
                                 const buffer *packet );

#ifdef get_cookie
#undef get_cookie
#endif
#define get_cookie mock_get_cookie
uint64_t mock_get_cookie( void );

#ifdef create_flow_mod
#undef create_flow_mod
#endif
#define create_flow_mod mock_create_flow_mod
buffer *mock_create_flow_mod( const uint32_t transaction_id,
                              const struct ofp_match match,
                              const uint64_t cookie,
                              const uint16_t command,
                              const uint16_t idle_timeout,
                              const uint16_t hard_timeout,
                              const uint16_t priority,
                              const uint32_t buffer_id,
                              const uint16_t out_port,
                              const uint16_t flags,
                              const openflow_actions *actions );

#ifdef time
#undef time
#endif
#define time mock_time
time_t mock_time( time_t *t );

#ifdef resolve_path
#undef resolve_path
#endif
#define resolve_path mock_resolve_path
bool mock_resolve_path( uint64_t in_dpid, uint16_t in_port,
                        uint64_t out_dpid, uint16_t out_port,
                        void *user_data,
                        resolve_path_callback callback );

#ifdef send_openflow_message
#undef send_openflow_message
#endif
#define send_openflow_message mock_send_openflow_message
bool mock_send_openflow_message( const uint64_t datapath_id, buffer *message );

#ifdef init_libtopology
#undef init_libtopology
#endif
#define init_libtopology mock_init_libtopology
int mock_init_libtopology( const char *service_name );

#ifdef subscribe_topology
#undef subscribe_topology
#endif
#define subscribe_topology mock_subscribe_topology
void mock_subscribe_topology( void ( *callback )(), void *user_data );

#ifdef set_packet_in_handler
#undef set_packet_in_handler
#endif
#define set_packet_in_handler mock_set_packet_in_handler
bool mock_set_packet_in_handler( packet_in_handler callback, void *user_data );

#ifdef add_callback_port_status_updated
#undef add_callback_port_status_updated
#endif
#define add_callback_port_status_updated mock_add_callback_port_status_updated
bool mock_add_callback_port_status_updated( void ( *callback )( void *, const topology_port_status * ), void *param );

#ifdef finalize_libtopology
#undef finalize_libtopology
#endif
#define finalize_libtopology mock_finalize_libtopology
bool mock_finalize_libtopology( void );

#ifdef parse_packet
#undef parse_packet
#endif
#define parse_packet mock_parse_packet
bool mock_parse_packet( buffer *buf );

#ifdef get_all_port_status
#undef get_all_port_status
#endif
#define get_all_port_status mock_get_all_port_status
bool mock_get_all_port_status( void ( *callback )(), void *param );

#ifdef create_set_config
#undef create_set_config
#endif
#define create_set_config mock_create_set_config
buffer *mock_create_set_config( const uint32_t transaction_id, const uint16_t flags, uint16_t miss_send_len );

#ifdef create_features_request
#undef create_features_request
#endif
#define create_features_request mock_create_features_request
buffer *mock_create_features_request( const uint32_t transaction_id );

#ifdef add_periodic_event_callback
#undef add_periodic_event_callback
#endif
#define add_periodic_event_callback mock_add_periodic_event_callback
bool mock_add_periodic_event_callback( const time_t seconds, void ( *callback )( void *user_data ), void *user_data );

#ifdef set_features_reply_handler
#undef set_features_reply_handler
#endif
#define set_features_reply_handler mock_set_features_reply_handler
bool mock_set_features_reply_handler( features_reply_handler callback, void *user_data );

#ifdef set_switch_ready_handler
#undef set_switch_ready_handler
#endif
#define set_switch_ready_handler mock_set_switch_ready_handler
bool mock_set_switch_ready_handler( switch_ready_handler callback, void *user_data );

#ifdef finalize_topology_service_interface_options
#undef finalize_topology_service_interface_options
#endif
#define finalize_topology_service_interface_options mock_finalize_topology_service_interface_options
void mock_finalize_topology_service_interface_options( void );

#ifdef exit
#undef exit
#endif
#define exit mock_exit
void mock_exit( int status );

#ifdef usage
#undef usage
#endif
#define usage mock_usage
void mock_usage( void );

#ifdef init_age_fdb
#undef init_age_fdb
#endif
#define init_age_fdb mock_init_age_fdb
void mock_init_age_fdb( hash_table *fdb );

#ifdef update_fdb
#undef update_fdb
#endif
#define update_fdb mock_update_fdb
bool mock_update_fdb( hash_table *fdb, const uint8_t mac[ OFP_ETH_ALEN ], uint64_t dpid, uint16_t port );

#ifdef lookup_fdb
#undef lookup_fdb
#endif
#define lookup_fdb mock_lookup_fdb
bool mock_lookup_fdb( hash_table *fdb, const uint8_t mac[ OFP_ETH_ALEN ], uint64_t *dpid, uint16_t *port );

#ifdef create_fdb
#undef create_fdb
#endif
#define create_fdb mock_create_fdb
hash_table * mock_create_fdb( void );

#ifdef delete_fdb
#undef delete_fdb
#endif
#define delete_fdb mock_delete_fdb
void mock_delete_fdb( hash_table *fdb );

#ifdef delete_outbound_port
#undef delete_outbound_port
#endif
#define delete_outbound_port mock_delete_outbound_port
void mock_delete_outbound_port( list_element **switches, port_info *delete_port );

#ifdef add_outbound_port
#undef add_outbound_port
#endif
#define add_outbound_port mock_add_outbound_port
void mock_add_outbound_port( list_element **switches, uint64_t dpid, uint16_t port_no );

#ifdef delete_outbound_ports
#undef delete_outbound_ports
#endif
#define delete_outbound_all_ports mock_delete_outbound_all_ports
void mock_delete_outbound_all_ports( list_element **switches );

#ifdef lookup_outbound_port
#undef lookup_outbound_port
#endif
#define lookup_outbound_port mock_lookup_outbound_port
port_info *mock_lookup_outbound_port( list_element *switches, uint64_t dpid, uint16_t port_no );

#ifdef create_outbound_ports
#undef create_outbound_ports
#endif
#define create_outbound_ports mock_create_outbound_ports
list_element *mock_create_outbound_ports( list_element **switches );

#ifdef foreach_port
#undef foreach_port
#endif
#define foreach_port mock_foreach_port
int mock_foreach_port( const list_element *ports,
                       int ( *function )( port_info *port,
                                          openflow_actions *actions,
                                          uint64_t dpid, uint16_t in_port ),
                       openflow_actions *actions, uint64_t dpid, uint16_t port );

#ifdef foreach_switch
#undef foreach_switch
#endif
#define foreach_switch mock_foreach_switch
void mock_foreach_switch( const list_element *switches,
                          void ( *function )( switch_info *sw,
                                              buffer *packet,
                                              uint64_t dpid,
                                              uint16_t in_port ),
                          buffer *packet, uint64_t dpid, uint16_t in_port );

#define static

#endif  // UNIT_TESTING


typedef struct routing_switch_options {
  uint16_t idle_timeout;
} routing_switch_options;


typedef struct routing_switch {
  uint16_t idle_timeout;
  list_element *switches;
  hash_table *fdb;
} routing_switch;


typedef struct resolve_path_replied_params {
  routing_switch *routing_switch;
  uint64_t in_datapath_id;
  uint16_t in_port;
  uint64_t out_datapath_id;
  uint16_t out_port;
  buffer *original_packet;
} resolve_path_replied_params;


static void
modify_flow_entry( const pathresolver_hop *h, const buffer *original_packet, uint16_t idle_timeout ) {
  const uint32_t wildcards = 0;
  struct ofp_match match;
  set_match_from_packet( &match, h->in_port_no, wildcards, original_packet );

  uint32_t transaction_id = get_transaction_id();
  openflow_actions *actions = create_actions();
  const uint16_t max_len = UINT16_MAX;
  append_action_output( actions, h->out_port_no, max_len );

  const uint16_t hard_timeout = 0;
  const uint16_t priority = UINT16_MAX;
  const uint32_t buffer_id = UINT32_MAX;
  const uint16_t flags = 0;
  buffer *flow_mod = create_flow_mod( transaction_id, match, get_cookie(),
                                      OFPFC_ADD, idle_timeout, hard_timeout,
                                      priority, buffer_id, 
                                      h->out_port_no, flags, actions );

  send_openflow_message( h->dpid, flow_mod );
  delete_actions( actions );
  free_buffer( flow_mod );
}


static void
output_packet( buffer *packet, uint64_t dpid, uint16_t port_no ) {
  openflow_actions *actions = create_actions();
  const uint16_t max_len = UINT16_MAX;
  append_action_output( actions, port_no, max_len );

  const uint32_t transaction_id = get_transaction_id();
  const uint32_t buffer_id = UINT32_MAX;
  const uint16_t in_port = OFPP_NONE;

  fill_ether_padding( packet );
  buffer *packet_out = create_packet_out( transaction_id, buffer_id, in_port,
                                          actions, packet );

  send_openflow_message( dpid, packet_out );

  free_buffer( packet_out );
  delete_actions( actions );
}


static void
output_packet_from_last_switch( const pathresolver_hop *last_hop, buffer *packet ) {
  output_packet( packet, last_hop->dpid, last_hop->out_port_no );
}


static uint32_t
count_hops( const dlist_element *hops ) {
  uint32_t i = 0;
  for ( const dlist_element *e = hops; e != NULL; e = e->next ) {
    i++;
  }
  return i;
}


static void
discard_packet_in( uint64_t datapath_id, uint16_t in_port, const buffer *packet ) {
  const uint32_t wildcards = 0;
  struct ofp_match match;
  set_match_from_packet( &match, in_port, wildcards, packet );
  char match_str[ 1024 ];
  match_to_string( &match, match_str, sizeof( match_str ) );

  const uint16_t idle_timeout = 0;
  const uint16_t hard_timeout = PACKET_IN_DISCARD_DURATION;
  const uint16_t priority = UINT16_MAX;
  const uint32_t buffer_id = UINT32_MAX;
  const uint16_t flags = 0;

  info( "Discarding packets for a certain period ( datapath_id = %#" PRIx64
        ", match = [%s], duration = %u [sec] ).", datapath_id, match_str, hard_timeout );

  buffer *flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(),
                                      OFPFC_ADD, idle_timeout, hard_timeout,
                                      priority, buffer_id,
                                      OFPP_NONE, flags, NULL );

  send_openflow_message( datapath_id, flow_mod );
  free_buffer( flow_mod );
}


static void
resolve_path_replied( void *user_data, dlist_element *hops ) {
  assert( user_data != NULL );

  resolve_path_replied_params *param = user_data;
  routing_switch *routing_switch = param->routing_switch;
  uint64_t in_datapath_id = param->in_datapath_id;
  uint16_t in_port = param->in_port;
  uint64_t out_datapath_id = param->out_datapath_id;
  uint16_t out_port = param->out_port;
  buffer *original_packet = param->original_packet;

  original_packet->user_data = NULL;
  if ( !parse_packet( original_packet ) ) {
    warn( "Received unsupported packet." );
    free_packet( original_packet );
    free_hop_list( hops );
    xfree( param );
    return;
  }

  if ( hops == NULL ) {
    warn( "No available path found ( %#" PRIx64 ":%u -> %#" PRIx64 ":%u ).",
          in_datapath_id, in_port, out_datapath_id, out_port );
    discard_packet_in( in_datapath_id, in_port, original_packet );
    free_packet( original_packet );
    xfree( param );
    return;
  }

  // count elements
  uint32_t hop_count = count_hops( hops );

  // send flow entry from tail switch
  for ( dlist_element *e  = get_last_element( hops ); e != NULL; e = e->prev, hop_count-- ) {
    uint16_t idle_timer = ( uint16_t ) ( routing_switch->idle_timeout + hop_count );
    modify_flow_entry( e->data, original_packet, idle_timer );
  } // for(;;)

  // send packet out for tail switch
  dlist_element *e = get_last_element( hops );
  pathresolver_hop *last_hop = e->data;
  output_packet_from_last_switch( last_hop, original_packet );

  // free them
  free_hop_list( hops );
  free_packet( original_packet );
  xfree( param );
}


static void
set_miss_send_len_maximum( uint64_t datapath_id ) {
  uint32_t id = get_transaction_id();
  const uint16_t config_flags = OFPC_FRAG_NORMAL;
  const uint16_t miss_send_len = UINT16_MAX;
  buffer *buf = create_set_config( id, config_flags, miss_send_len );
  send_openflow_message( datapath_id, buf );

  free_buffer( buf );
}


static void
receive_features_reply( uint64_t datapath_id, uint32_t transaction_id,
                        uint32_t n_buffers, uint8_t n_tables,
                        uint32_t capabilities, uint32_t actions,
                        const list_element *phy_ports, void *user_data ) {
  UNUSED( transaction_id );
  UNUSED( n_buffers );
  UNUSED( n_tables );
  UNUSED( capabilities );
  UNUSED( actions );
  UNUSED( phy_ports );
  UNUSED( user_data );

  set_miss_send_len_maximum( datapath_id );
}


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  uint32_t id = get_transaction_id();
  buffer *buf = create_features_request( id );
  send_openflow_message( datapath_id, buf );
  free_buffer( buf );
}


static void
port_status_updated( void *user_data, const topology_port_status *status ) {
  assert( user_data != NULL );
  assert( status != NULL );

  routing_switch *routing_switch = user_data;

  debug( "Port status updated: dpid:%#" PRIx64 ", port:%u, %s, %s",
         status->dpid, status->port_no,
         ( status->status == TD_PORT_UP ? "up" : "down" ),
         ( status->external == TD_PORT_EXTERNAL ? "external" : "internal or inactive" ) );

  if ( status->port_no > OFPP_MAX && status->port_no != OFPP_LOCAL ) {
    warn( "Ignore this update ( port = %u )", status->port_no );
    return;
  }

  port_info *p = lookup_outbound_port( routing_switch->switches, status->dpid, status->port_no );

  delete_fdb_entries( routing_switch->fdb, status->dpid, status->port_no );

  if ( status->status == TD_PORT_UP
       && status->external == TD_PORT_EXTERNAL ) {
    if ( p != NULL ) {
      debug( "Ignore this update (already exists)" );
      return;
    }
    add_outbound_port( &routing_switch->switches, status->dpid, status->port_no );
    set_miss_send_len_maximum( status->dpid );
  } else {
    if ( p == NULL ) {
      debug( "Ignore this update (not found nor already deleted)" );
      return;
    }
    delete_outbound_port( &routing_switch->switches, p );
  }
}


static int
build_packet_out_actions( port_info *port, openflow_actions *actions, uint64_t dpid, uint16_t in_port ) {
  const uint16_t max_len = UINT16_MAX;
  if ( port->dpid == dpid && port->port_no == in_port ) {
    // don't send to input port
    return 0;
  } else {
    append_action_output( actions, port->port_no, max_len );
    return 1;
  }
}


static void
send_packet_out_for_each_switch( switch_info *sw, buffer *packet, uint64_t dpid, uint16_t in_port ) {
  openflow_actions *actions = create_actions();
  int number_of_actions = foreach_port( sw->ports, build_packet_out_actions, actions, dpid, in_port );

  // check if no action is build
  if ( number_of_actions > 0 ) {
    const uint32_t transaction_id = get_transaction_id();
    const uint32_t buffer_id = UINT32_MAX;
    const uint16_t in_port = OFPP_NONE;

    fill_ether_padding( packet );
    buffer *packet_out = create_packet_out( transaction_id, buffer_id, in_port,
                                            actions, packet );

    send_openflow_message( sw->dpid, packet_out );

    free_buffer( packet_out );
  }

  delete_actions( actions );
}


static void
flood_packet( uint64_t datapath_id, uint16_t in_port, buffer *packet, list_element *switches ) {
  foreach_switch( switches, send_packet_out_for_each_switch, packet, datapath_id, in_port );
  free_buffer( packet );
}


static void
handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                  uint32_t buffer_id, uint16_t total_len,
                  uint16_t in_port, uint8_t reason, const buffer *data,
                  void *user_data ) {
  assert( in_port != 0 );
  assert( data != NULL );
  assert( user_data != NULL );

  routing_switch *routing_switch = user_data;

  debug( "Packet-In received ( datapath_id = %#" PRIx64 ", transaction_id = %#lx, "
         "buffer_id = %#lx, total_len = %u, in_port = %u, reason = %#x, "
         "data_len = %u ).", datapath_id, transaction_id, buffer_id,
         total_len, in_port, reason, data->length );

  const port_info *port = lookup_outbound_port( routing_switch->switches, datapath_id, in_port );

  const uint8_t *src = packet_info( data )->l2_data.eth->macsa;
  const uint8_t *dst = packet_info( data )->l2_data.eth->macda;

  if ( in_port <= OFPP_MAX || in_port == OFPP_LOCAL ) {
    if ( port == NULL && !lookup_fdb( routing_switch->fdb, src, &datapath_id, &in_port ) ) {
      debug( "Ignoring Packet-In from switch-to-switch link." );
      return;
    }
  }
  else {
    error( "Packet-In from invalid port ( in_port = %u ).", in_port );
    return;
  }

  if ( !update_fdb( routing_switch->fdb, src, datapath_id, in_port ) ) {
    return;
  }

  buffer *original_packet = duplicate_buffer( data );
  uint16_t out_port;
  uint64_t out_datapath_id;

  if ( lookup_fdb( routing_switch->fdb, dst, &out_datapath_id, &out_port ) ) {
    // Host is located, so resolve path and send flowmod
    if ( ( datapath_id == out_datapath_id ) && ( in_port == out_port ) ) {
      // in and out are same
      free_buffer( original_packet );
      return;
    }

    // Ask path resolver service to lookup a path
    // resolve_path_replied() will be called later
    resolve_path_replied_params *param = xmalloc( sizeof( *param ) );
    param->routing_switch = routing_switch;
    param->in_datapath_id = datapath_id;
    param->in_port = in_port;
    param->out_datapath_id = out_datapath_id;
    param->out_port = out_port;
    param->original_packet = original_packet;

    resolve_path( datapath_id, in_port, out_datapath_id, out_port,
                  param, resolve_path_replied );
  } else {
    // Host's location is unknown, so flood packet
    flood_packet( datapath_id, in_port, original_packet, routing_switch->switches );
  }
}


static void
init_outbound_ports( list_element **switches, size_t n_entries, const topology_port_status *s ) {
  for ( size_t i = 0; i < n_entries; i++ ) {
    if ( s[ i ].status == TD_PORT_UP && s[ i ].external == TD_PORT_EXTERNAL ) {
      add_outbound_port( switches, s[ i ].dpid, s[ i ].port_no );
      set_miss_send_len_maximum( s[ i ].dpid );
    }
  }
}


static void
init_last_stage( void *user_data, size_t n_entries, const topology_port_status *s ) {
  assert( user_data != NULL );

  routing_switch *routing_switch = user_data;

  // Initialize outbound ports
  init_outbound_ports( &routing_switch->switches, n_entries, s );

  // Initialize aging FDB
  init_age_fdb( routing_switch->fdb );

  // Finally, set asynchronous event handlers
  // (0) Set features_request_reply handler
  set_features_reply_handler( receive_features_reply, routing_switch );

  // (1) Set switch_ready handler
  set_switch_ready_handler( handle_switch_ready, routing_switch );

  // (2) Set port status update callback
  add_callback_port_status_updated( port_status_updated, routing_switch );

  // (3) Set packet-in handler
  set_packet_in_handler( handle_packet_in, routing_switch );
}


static void
after_subscribed( void *user_data ) {
  assert( user_data != NULL );

  // Get all ports' status
  // init_last_stage() will be called
  get_all_port_status( init_last_stage, user_data );
}


static routing_switch *
create_routing_switch( const char *topology_service, const routing_switch_options *options ) {
  assert( topology_service != NULL );
  assert( options != NULL );

  // Allocate routing_switch object
  routing_switch *routing_switch = xmalloc( sizeof( struct routing_switch ) );
  routing_switch->idle_timeout = options->idle_timeout;
  routing_switch->switches = NULL;
  routing_switch->fdb = NULL;

  info( "idle_timeout is set to %u [sec].", routing_switch->idle_timeout );

  // Create forwarding database
  routing_switch->fdb = create_fdb();

  // Initialize port database
  routing_switch->switches = create_outbound_ports( &routing_switch->switches );

  // Initialize libraries
  init_libtopology( topology_service );

  // Ask topology manager to notify any topology change events.
  // after_subscribed() will be called
  subscribe_topology( after_subscribed, routing_switch );

  return routing_switch;
}


static void
delete_routing_switch( routing_switch *routing_switch ) {
  assert( routing_switch != NULL );

  // Finalize libraries
  finalize_libtopology();

  // Delete outbound ports
  delete_outbound_all_ports( &routing_switch->switches );

  // Delete forwarding database
  delete_fdb( routing_switch->fdb );

  // Delete routing_switch object
  xfree( routing_switch );
}


static char option_description[] = "  -i, --idle_timeout=TIMEOUT  Idle timeout value of flow entry\n";
static char short_options[] = "i:";
static struct option long_options[] = {
  { "idle_timeout", 1, NULL, 'i' },
  { NULL, 0, NULL, 0  },
};

static void
reset_getopt() {
  optind = 0;
  opterr = 1;
}


void usage( void );


static void
init_routing_switch_options( routing_switch_options *options, int *argc, char **argv[] ) {
  assert( options != NULL );
  assert( argc != NULL );
  assert( *argc >= 0 );
  assert( argv != NULL );

  // set default values
  options->idle_timeout = FLOW_TIMER;

  int argc_tmp = *argc;
  char *new_argv[ *argc ];

  for ( int i = 0; i <= *argc; ++i ) {
    new_argv[ i ] = ( *argv )[ i ];
  }

  int c;
  uint32_t idle_timeout;
  while ( ( c = getopt_long( *argc, *argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'i':
        idle_timeout = ( uint32_t ) atoi( optarg );
        if ( idle_timeout == 0 || idle_timeout > UINT16_MAX ) {
          printf( "Invalid idle_timeout value.\n" );
          usage();
          finalize_topology_service_interface_options();
          exit( EXIT_SUCCESS );
          return;
        }
        options->idle_timeout = ( uint16_t ) idle_timeout;
        break;

      default:
        continue;
    }

    if ( optarg == 0 || strchr( new_argv[ optind - 1 ], '=' ) != NULL ) {
      argc_tmp -= 1;
      new_argv[ optind - 1 ] = NULL;
    }
    else {
      argc_tmp -= 2;
      new_argv[ optind - 1 ] = NULL;
      new_argv[ optind - 2 ] = NULL;
    }
  }

  for ( int i = 0, j = 0; i < *argc; ++i ) {
    if ( new_argv[ i ] != NULL ) {
      ( *argv )[ j ] = new_argv[ i ];
      j++;
    }
  }

  ( *argv )[ *argc ] = NULL;
  *argc = argc_tmp;

  reset_getopt();
}


#ifndef UNIT_TESTING


void
usage() {
  topology_service_interface_usage( get_executable_name(), "Switching HUB.", option_description );
}


int
main( int argc, char *argv[] ) {
  // Initialize Trema world
  init_trema( &argc, &argv );
  routing_switch_options options;
  init_routing_switch_options( &options, &argc, &argv );
  init_topology_service_interface_options( &argc, &argv );

  // Initialize routing_switch
  routing_switch *routing_switch = create_routing_switch( get_topology_service_interface_name(), &options );

  // Main loop
  start_trema();

  // Finalize routing_switch
  delete_routing_switch( routing_switch );

  return 0;
}
#endif  // UNIT_TESTING


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
