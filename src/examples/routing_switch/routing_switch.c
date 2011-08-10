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
    free_buffer( original_packet );
    free_hop_list( hops );
    xfree( param );
    return;
  }

  if ( hops == NULL ) {
    warn( "No available path found ( %#" PRIx64 ":%u -> %#" PRIx64 ":%u ).",
          in_datapath_id, in_port, out_datapath_id, out_port );
    discard_packet_in( in_datapath_id, in_port, original_packet );
    free_buffer( original_packet );
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
  free_buffer( original_packet );
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

  port_info *p = lookup_port( routing_switch->switches, status->dpid, status->port_no );

  delete_fdb_entries( routing_switch->fdb, status->dpid, status->port_no );

  if ( status->status == TD_PORT_UP ) {
    if ( p != NULL ) {
      update_port( p, status->external );
      return;
    }
    add_port( &routing_switch->switches, status->dpid, status->port_no, status->external );
  }
  else {
    if ( p == NULL ) {
      debug( "Ignore this update (not found nor already deleted)" );
      return;
    }
    delete_port( &routing_switch->switches, p );
  }
}


static int
build_packet_out_actions( port_info *port, openflow_actions *actions, uint64_t dpid, uint16_t in_port ) {
  const uint16_t max_len = UINT16_MAX;
  if ( !port->external_link || port->switch_to_switch_reverse_link ) {
    // don't send to not external port
    return 0;
  }
  if ( port->dpid == dpid && port->port_no == in_port ) {
    // don't send to input port
    return 0;
  }

  append_action_output( actions, port->port_no, max_len );
  return 1;
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

  const port_info *port = lookup_port( routing_switch->switches, datapath_id, in_port );
  if ( port == NULL ) {
    debug( "Ignoring Packet-In from unknown port." );
    return;
  }

  const uint8_t *src = packet_info( data )->l2_data.eth->macsa;
  const uint8_t *dst = packet_info( data )->l2_data.eth->macda;

  if ( in_port > OFPP_MAX && in_port != OFPP_LOCAL ) {
    error( "Packet-In from invalid port ( in_port = %u ).", in_port );
    return;
  }
  if ( !port->external_link || port->switch_to_switch_reverse_link ) {
    if ( !port->external_link
         && port->switch_to_switch_link
         && port->switch_to_switch_reverse_link
         && !is_ether_multicast( dst )
         && lookup_fdb( routing_switch->fdb, src, &datapath_id, &in_port ) ) {
      debug( "Found a Packet-In from switch-to-switch link." );
    }
    else {
      debug( "Ignoring Packet-In from not external link." );
      return;
    }
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
    resolve_path_replied_params *param = xmalloc( sizeof( resolve_path_replied_params ) );
    param->routing_switch = routing_switch;
    param->in_datapath_id = datapath_id;
    param->in_port = in_port;
    param->out_datapath_id = out_datapath_id;
    param->out_port = out_port;
    param->original_packet = original_packet;

    resolve_path( datapath_id, in_port, out_datapath_id, out_port,
                  param, resolve_path_replied );
  }
  else {
    // Host's location is unknown, so flood packet
    flood_packet( datapath_id, in_port, original_packet, routing_switch->switches );
  }
}


static void
init_ports( list_element **switches, size_t n_entries, const topology_port_status *s ) {
  for ( size_t i = 0; i < n_entries; i++ ) {
    if ( s[ i ].status == TD_PORT_UP ) {
      add_port( switches, s[ i ].dpid, s[ i ].port_no, s[ i ].external );
    }
  }
}


static void
update_port_status_by_link( list_element *switches, const topology_link_status *s ) {
  port_info *port = lookup_port( switches, s->from_dpid, s->from_portno );
  if ( port != NULL ) {
    debug( "Link status updated: dpid:%#" PRIx64 ", port:%u, %s",
           s->from_dpid, s->from_portno,
           ( s->status == TD_LINK_UP ? "up" : "down" ) );
    port->switch_to_switch_link = ( s->status == TD_LINK_UP );
  }
  if ( s->to_portno == 0 ) {
    return;
  }
  port_info *peer = lookup_port( switches, s->to_dpid, s->to_portno );
  if ( peer != NULL ) {
    debug( "Reverse link status updated: dpid:%#" PRIx64 ", port:%u, %s",
           s->to_dpid, s->to_portno,
           ( s->status == TD_LINK_UP ? "up" : "down" ) );
    peer->switch_to_switch_reverse_link = ( s->status == TD_LINK_UP );
  }
}


static void
link_status_updated( void *user_data, const topology_link_status *status ) {
  assert( user_data != NULL );
  assert( status != NULL );

  routing_switch *routing_switch = user_data;
  update_port_status_by_link( routing_switch->switches, status );
}


static void
update_ports_status( list_element *switches, size_t n_entries,
                              const topology_link_status *status ) {
  for ( size_t i = 0; i < n_entries; i++ ) {
    update_port_status_by_link( switches, &status[ i ] );
  }
}


static void
init_last_stage( void *user_data, size_t n_entries, const topology_link_status *status ) {
  assert( user_data != NULL );

  routing_switch *routing_switch = user_data;

  update_ports_status( routing_switch->switches, n_entries, status );
  add_callback_link_status_updated( link_status_updated, routing_switch );
}


static void
init_second_stage( void *user_data, size_t n_entries, const topology_port_status *s ) {
  assert( user_data != NULL );

  routing_switch *routing_switch = user_data;

  // Initialize ports
  init_ports( &routing_switch->switches, n_entries, s );

  // Initialize aging FDB
  init_age_fdb( routing_switch->fdb );

  // Set asynchronous event handlers
  // (0) Set features_request_reply handler
  set_features_reply_handler( receive_features_reply, routing_switch );

  // (1) Set switch_ready handler
  set_switch_ready_handler( handle_switch_ready, routing_switch );

  // (2) Set port status update callback
  add_callback_port_status_updated( port_status_updated, routing_switch );

  // (3) Set packet-in handler
  set_packet_in_handler( handle_packet_in, routing_switch );

  // (4) Get all link status
  get_all_link_status( init_last_stage, routing_switch );
}


static void
after_subscribed( void *user_data ) {
  assert( user_data != NULL );

  // Get all ports' status
  // init_last_stage() will be called
  get_all_port_status( init_second_stage, user_data );
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
  routing_switch->switches = create_ports( &routing_switch->switches );

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

  // Delete ports
  delete_all_ports( &routing_switch->switches );

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


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
