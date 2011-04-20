/*
 * Sample OpenFlow event dumper.
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


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "trema.h"


#define dump info


void
usage() {
  printf(
          "OpenFlow Event Dumper.\n"
          "Usage: %s [OPTION]...\n"
          "\n"
          "  -n, --name=SERVICE_NAME     service name\n"
          "  -d, --daemonize             run in the background\n"
          "  -l, --logging_level=LEVEL   set logging level\n"
          "  -h, --help                  display this help and exit\n"
          , get_executable_name()
        );
}


static void
dump_phy_port( const struct ofp_phy_port *phy_port ) {
  dump( "port_no: %u", phy_port->port_no );
  dump( "  hw_addr: %02x:%02x:%02x:%02x:%02x:%02x",
        phy_port->hw_addr[ 0 ], phy_port->hw_addr[ 1 ], phy_port->hw_addr[ 2 ],
        phy_port->hw_addr[ 3 ], phy_port->hw_addr[ 4 ], phy_port->hw_addr[ 5 ] );
  dump( "  name: %s", phy_port->name );
  dump( "  config: %#lx", phy_port->config );
  dump( "  state: %#lx", phy_port->state );
  dump( "  curr: %#lx", phy_port->curr );
  dump( "  advertised: %#lx", phy_port->advertised );
  dump( "  supported: %#lx", phy_port->supported );
  dump( "  peer: %#lx", phy_port->peer );
}


static void
dump_packet_queue( const struct ofp_packet_queue *packet_queue ) {
  uint16_t properties_length;
  struct ofp_queue_prop_header *prop_header, *properties_head;
  struct ofp_queue_prop_min_rate *prop_min_rate;

  dump( "queue_id: %#lx", packet_queue->queue_id );
  dump( "  len: %u", packet_queue->len );
  dump( "  properties:" );

  properties_length =
    ( uint16_t ) ( packet_queue->len -
                   offsetof( struct ofp_packet_queue, properties ) );

  properties_head = ( struct ofp_queue_prop_header * ) xmalloc( properties_length );
  memcpy( properties_head, packet_queue->properties, properties_length );
  prop_header = properties_head;

  while ( properties_length > 0 ) {
    dump( "    property: %#x", prop_header->property );
    dump( "    len: %#x", prop_header->len );

    if ( prop_header->property == OFPQT_MIN_RATE ) {
      prop_min_rate = ( struct ofp_queue_prop_min_rate * ) prop_header;
      dump( "    rate: %u", prop_min_rate->rate );
    }

    properties_length = ( uint16_t ) ( properties_length - prop_header->len );
    prop_header =
      ( struct ofp_queue_prop_header * ) ( ( char * ) prop_header +
                                           prop_header->len );
  }

  xfree( properties_head );
}


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  buffer *buffer;

  dump( "[switch_ready]" );
  dump( "datapath_id: %#llx", datapath_id );

  buffer = create_features_request( get_transaction_id() );

  send_openflow_message( datapath_id, buffer );

  free_buffer( buffer );
}


static void
handle_switch_disconnected( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  dump( "[switch_disconnected]" );
  dump( "datapath_id: %#llx", datapath_id );
}


static void
handle_error( uint64_t datapath_id, uint32_t transaction_id,
              uint16_t type, uint16_t code, const buffer *data,
              void *user_data ) {
  UNUSED( user_data );

  dump( "[error]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "type: %#x", type );
  dump( "code: %#x", code );
  dump( "data:" );
  dump_buffer( data, dump );
}


static void
handle_vendor( uint64_t datapath_id, uint32_t transaction_id,
               uint32_t vendor, const buffer *data, void *user_data ) {
  UNUSED( user_data );

  dump( "[vendor]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "vendor: %#lx", vendor );
  dump( "data:" );
  dump_buffer( data, dump );
}


static void
handle_features_reply( uint64_t datapath_id, uint32_t transaction_id,
                       uint32_t n_buffers, uint8_t n_tables, uint32_t capabilities,
                       uint32_t actions, const list_element *phy_ports,
                       void *user_data ) {
  UNUSED( user_data );

  list_element *element, *phy_ports_head;
  struct ofp_phy_port *phy_port;

  dump( "[features_reply]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "n_buffers: %lu", n_buffers );
  dump( "n_tables: %u", n_tables );
  dump( "capabilities: %lu", capabilities );
  dump( "actions: %lu", actions );

  phy_ports_head = ( list_element * ) xmalloc( sizeof( list_element ) );
  memcpy( phy_ports_head, phy_ports, sizeof( list_element ) );

  element = phy_ports_head;
  while ( element != NULL ) {
    phy_port = ( struct ofp_phy_port * ) element->data;

    dump_phy_port( phy_port );

    element = element->next;
  }

  xfree( phy_ports_head );
}


static void
handle_get_config_reply( uint64_t datapath_id, uint32_t transaction_id,
                         uint16_t flags, uint16_t miss_send_len,
                         void *user_data ) {
  UNUSED( user_data );

  dump( "[get_config_reply]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "flags: %#x", flags );
  dump( "miss_send_len: %u", miss_send_len );
}


static void
handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                  uint32_t buffer_id, uint16_t total_len,
                  uint16_t in_port, uint8_t reason, const buffer *data,
                  void *user_data ) {
  UNUSED( user_data );

  dump( "[packet_in]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "buffer_id: %#lx", buffer_id );
  dump( "total_len: %u", total_len );
  dump( "in_port: %u", in_port );
  dump( "reason: %#x", reason );
  dump( "data:" );
  dump_buffer( data, dump );
}


static void
handle_flow_removed( uint64_t datapath_id, uint32_t transaction_id,
                     struct ofp_match match,
                     uint64_t cookie, uint16_t priority, uint8_t reason,
                     uint32_t duration_sec, uint32_t duration_nsec,
                     uint16_t idle_timeout, uint64_t packet_count,
                     uint64_t byte_count, void *user_data ) {
  UNUSED( user_data );

  dump( "[flow_removed]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );

  dump( "match:" );
  dump( "  wildcards: %#lx", match.wildcards );
  dump( "  in_port: %u", match.in_port );
  dump( "  dl_src: %02x:%02x:%02x:%02x:%02x:%02x",
        match.dl_src[ 0 ], match.dl_src[ 1 ], match.dl_src[ 2 ],
        match.dl_src[ 3 ], match.dl_src[ 4 ], match.dl_src[ 5 ] );
  dump( "  dl_dst: %02x:%02x:%02x:%02x:%02x:%02x",
        match.dl_dst[ 0 ], match.dl_dst[ 1 ], match.dl_dst[ 2 ],
        match.dl_dst[ 3 ], match.dl_dst[ 4 ], match.dl_dst[ 5 ] );
  dump( "  dl_vlan: %u", match.dl_vlan );
  dump( "  dl_vlan_pcp: %u", match.dl_vlan_pcp );
  dump( "  dl_type: %#x", match.dl_type );
  dump( "  nw_tos: %u", match.nw_tos );
  dump( "  nw_proto: %#x", match.nw_proto );
  dump( "  nw_src: %#llx", match.nw_src );
  dump( "  nw_dst: %#llx", match.nw_dst );
  dump( "  tp_src: %u", match.tp_src );
  dump( "  tp_dst: %u", match.tp_dst );

  dump( "cookie: %#llx", cookie );
  dump( "priority: %u", priority );
  dump( "reason: %#x", reason );
  dump( "duration_sec: %lu", duration_sec );
  dump( "duration_nsec: %lu", duration_nsec );
  dump( "idle_timeout: %u", idle_timeout );
  dump( "packet_count: %llu", packet_count );
  dump( "byte_count: %llu", byte_count );
}


static void
handle_port_status( uint64_t datapath_id, uint32_t transaction_id,
                    uint8_t reason, struct ofp_phy_port phy_port,
                    void *user_data ) {
  UNUSED( user_data );

  dump( "[port_status]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "reason: %#x", reason );
  dump( "phy_port:" );
  dump_phy_port( &phy_port );
}


static void
handle_stats_reply( uint64_t datapath_id, uint32_t transaction_id,
                    uint16_t type, uint16_t flags, const buffer *data,
                    void *user_data ) {
  UNUSED( user_data );

  dump( "[stats_reply]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "type: %#x", type );
  dump( "flags: %#x", flags );
  dump( "data:" );
  dump_buffer( data, dump );
}


static void
handle_barrier_reply( uint64_t datapath_id, uint32_t transaction_id,
                      void *user_data ) {
  UNUSED( user_data );

  dump( "[barrier_reply]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
}


static void
handle_queue_get_config_reply( uint64_t datapath_id, uint32_t transaction_id,
                               uint16_t port, const list_element *queues,
                               void *user_data ) {
  UNUSED( user_data );

  list_element *queues_head, *element;
  struct ofp_packet_queue *packet_queue;

  dump( "[queue_get_config_reply]" );
  dump( "datapath_id: %#llx", datapath_id );
  dump( "transaction_id: %#lx", transaction_id );
  dump( "port: %u", port );
  dump( "queues:" );

  queues_head = ( list_element * ) xmalloc( sizeof( list_element ) );
  memcpy( queues_head, queues, sizeof( list_element ) );

  element = queues_head;
  while ( element != NULL ) {
    packet_queue = ( struct ofp_packet_queue * ) element->data;

    dump_packet_queue( packet_queue );

    element = element->next;
  }

  xfree( queues_head );
}                 


int
main( int argc, char *argv[] ) {
  // Initialize the Trema world
  init_trema( &argc, &argv );

  // Set event handlers
  set_switch_ready_handler( handle_switch_ready, NULL );
  set_switch_disconnected_handler( handle_switch_disconnected, NULL );
  set_error_handler( handle_error, NULL );
  set_vendor_handler( handle_vendor, NULL );
  set_features_reply_handler( handle_features_reply, NULL );
  set_get_config_reply_handler( handle_get_config_reply, NULL );
  set_packet_in_handler( handle_packet_in, NULL );
  set_flow_removed_handler( handle_flow_removed, NULL );
  set_port_status_handler( handle_port_status, NULL );
  set_stats_reply_handler( handle_stats_reply, NULL );
  set_barrier_reply_handler( handle_barrier_reply, NULL );
  set_queue_get_config_reply_handler( handle_queue_get_config_reply, NULL );

  // Main loop
  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
