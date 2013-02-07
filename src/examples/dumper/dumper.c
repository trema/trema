/*
 * Sample OpenFlow event dumper.
 *
 * Copyright (C) 2008-2013 NEC Corporation
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
          "  -n, --name=SERVICE_NAME         service name\n"
          "  -d, --daemonize                 run in the background\n"
          "  -l, --logging_level=LEVEL       set logging level\n"
          "  -g, --syslog                    output log messages to syslog\n"
          "  -f, --logging_facility=FACILITY set syslog facility\n"
          "  -h, --help                      display this help and exit\n"
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
  dump( "  config: %#x", phy_port->config );
  dump( "  state: %#x", phy_port->state );
  dump( "  curr: %#x", phy_port->curr );
  dump( "  advertised: %#x", phy_port->advertised );
  dump( "  supported: %#x", phy_port->supported );
  dump( "  peer: %#x", phy_port->peer );
}


static void
dump_packet_queue( const struct ofp_packet_queue *packet_queue ) {
  uint16_t properties_length;
  struct ofp_queue_prop_header *prop_header, *properties_head;
  struct ofp_queue_prop_min_rate *prop_min_rate;

  dump( "queue_id: %#x", packet_queue->queue_id );
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

  dump( "[switch_ready]" );
  dump( "datapath_id: %#" PRIx64, datapath_id );
}


static void
handle_switch_disconnected( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  dump( "[switch_disconnected]" );
  dump( "datapath_id: %#" PRIx64, datapath_id );
}


static void
handle_error( uint64_t datapath_id, uint32_t transaction_id,
              uint16_t type, uint16_t code, const buffer *data,
              void *user_data ) {
  UNUSED( user_data );

  dump( "[error]" );
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
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
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
  dump( "vendor: %#x", vendor );
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
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
  dump( "n_buffers: %u", n_buffers );
  dump( "n_tables: %u", n_tables );
  dump( "capabilities: %#x", capabilities );
  dump( "actions: %#x", actions );

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
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
  dump( "flags: %#x", flags );
  dump( "miss_send_len: %u", miss_send_len );
}


static void
handle_packet_in(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t buffer_id,
  uint16_t total_len,
  uint16_t in_port,
  uint8_t reason,
  const buffer *data,
  void *user_data
) {
  UNUSED( user_data );

  dump( "[packet_in]" );
  dump( "  datapath_id: %#" PRIx64, datapath_id );
  dump( "  transaction_id: %#x", transaction_id );
  dump( "  buffer_id: %#x", buffer_id );
  dump( "  total_len: %u", total_len );
  dump( "  in_port: %u", in_port );
  dump( "  reason: %#x", reason );
  dump( "  data:" );
  dump_buffer( data, dump );
}


static void
handle_flow_removed( uint64_t datapath_id, flow_removed message ) {
  dump( "[flow_removed]" );
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", message.transaction_id );

  dump( "match:" );
  dump( "  wildcards: %#x", message.match.wildcards );
  dump( "  in_port: %u", message.match.in_port );
  dump( "  dl_src: %02x:%02x:%02x:%02x:%02x:%02x",
        message.match.dl_src[ 0 ], message.match.dl_src[ 1 ], message.match.dl_src[ 2 ],
        message.match.dl_src[ 3 ], message.match.dl_src[ 4 ], message.match.dl_src[ 5 ] );
  dump( "  dl_dst: %02x:%02x:%02x:%02x:%02x:%02x",
        message.match.dl_dst[ 0 ], message.match.dl_dst[ 1 ], message.match.dl_dst[ 2 ],
        message.match.dl_dst[ 3 ], message.match.dl_dst[ 4 ], message.match.dl_dst[ 5 ] );
  dump( "  dl_vlan: %u", message.match.dl_vlan );
  dump( "  dl_vlan_pcp: %u", message.match.dl_vlan_pcp );
  dump( "  dl_type: %#x", message.match.dl_type );
  dump( "  nw_tos: %u", message.match.nw_tos );
  dump( "  nw_proto: %#x", message.match.nw_proto );
  dump( "  nw_src: %#x", message.match.nw_src );
  dump( "  nw_dst: %#x", message.match.nw_dst );
  dump( "  tp_src: %u", message.match.tp_src );
  dump( "  tp_dst: %u", message.match.tp_dst );

  dump( "cookie: %#" PRIx64, message.cookie );
  dump( "priority: %u", message.priority );
  dump( "reason: %#x", message.reason );
  dump( "duration_sec: %u", message.duration_sec );
  dump( "duration_nsec: %u", message.duration_nsec );
  dump( "idle_timeout: %u", message.idle_timeout );
  dump( "packet_count: %" PRIu64, message.packet_count );
  dump( "byte_count: %" PRIu64, message.byte_count );
}


static void
handle_port_status( uint64_t datapath_id, uint32_t transaction_id,
                    uint8_t reason, struct ofp_phy_port phy_port,
                    void *user_data ) {
  UNUSED( user_data );

  dump( "[port_status]" );
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
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
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
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
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
}


static void
handle_queue_get_config_reply( uint64_t datapath_id, uint32_t transaction_id,
                               uint16_t port, const list_element *queues,
                               void *user_data ) {
  UNUSED( user_data );

  list_element *queues_head, *element;
  struct ofp_packet_queue *packet_queue;

  dump( "[queue_get_config_reply]" );
  dump( "datapath_id: %#" PRIx64, datapath_id );
  dump( "transaction_id: %#x", transaction_id );
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
