/*
 * Author: Hiroyasu OHYAMA
 *
 * Copyright (C) 2012 Univ. of tsukuba
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


#include "trema.h"

#include "discover_queue.h"
#include "ofnode.h"

#define BUFLEN ( 100 )

#define DISCOVERING_INTERVAL ( 3 )

static void 
send_discover_packet( uint64_t dpid ) {
  buffer *buf = alloc_buffer_with_length( BUFLEN );
  buf->length = BUFLEN;

  /* setting ethernet header infomation */
  ether_header_t *l2hdr = ( ether_header_t * ) buf->data;
  memcpy( l2hdr->macsa, ( uint8_t * ) &dpid, ETH_ADDRLEN );
  l2hdr->type = ETH_ETHTYPE_UKNOWN;

  buf->user_data = ( void * ) malloc(sizeof(uint64_t));
  *( ( uint64_t * ) buf->user_data ) = dpid;

  /* setting forwarding action */
  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_FLOOD, 0 );

  /* making a message of Send-Packet and send it to switch */
  buffer *packet_out = create_packet_out( get_transaction_id(), UINT32_MAX, OFPP_NONE, actions, buf );
  send_openflow_message( dpid, packet_out );
  
  free_buffer( buf );
}

static void
send_feature_request( uint64_t dpid ) {
  buffer *message = create_features_request( get_transaction_id() );
  send_openflow_message( dpid, message );
  free_buffer( message );
}

static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  send_feature_request( datapath_id );
}

static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  packet_info pinfo = get_packet_info( message.data );

  if ( pinfo.eth_type == ETH_ETHTYPE_UKNOWN ) {
    uint64_t from_dpid = 0;
  
    memcpy( ( uint8_t * ) &from_dpid, pinfo.eth_macsa, ETH_ADDRLEN );
  
    discover_queue_push_switch( datapath_id, from_dpid, message.in_port );
  } else {
    ofnode_create_host( pinfo.eth_macsa, pinfo.ipv4_saddr );

    discover_queue_push_host( datapath_id, pinfo.eth_macsa, pinfo.ipv4_saddr, message.in_port );
  }
}

static void
handle_features_reply (
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t n_buffers,
  uint8_t n_tables,
  uint32_t capabilities,
  uint32_t actions,
  const list_element *phy_ports,
  void *user_data
) {
  UNUSED( transaction_id );
  UNUSED( n_buffers );
  UNUSED( n_tables );
  UNUSED( capabilities );
  UNUSED( actions );
  UNUSED( user_data );

  uint16_t port_len = ( uint16_t ) list_length_of( phy_ports );
  
  ofnode_create_switch( datapath_id, port_len );

  send_discover_packet( datapath_id );
}

static void 
discover_path( void *data ) {
  UNUSED( data );

  bool is_update = false;
  discover_queue_entry *queue_entry;

  do {
    queue_entry = discover_queue_pop();

    if ( queue_entry != NULL ) {
      struct ofnode *target_node;
      struct ofnode *from_node = NULL;
      int in_port = queue_entry->in_port;

      /* The case of receving discovery packet before 
       * receving features reply. */
      target_node = ofnode_get_switch( queue_entry->target_dpid );
      if ( target_node == NULL ) {
        discover_queue_push( queue_entry );
        break;
      }
  
      if ( queue_entry->type == TYPE_SWITCH ) {
        /* append a switch infomation to target ofnode */

        /* The case of receving discovery packet before 
         * receving features reply. */
        from_node = ofnode_get_switch( queue_entry->from_dpid );
        if ( from_node == NULL ) {
          discover_queue_push( queue_entry );
          break;
        }

      } else if ( queue_entry->type == TYPE_HOST ) {
        /* append a host infomation to target ofnode */
        from_node = ofnode_get_host( queue_entry->nw_addr );
      }
  
      target_node->switch_info.ports[ in_port ] = from_node;
  
      is_update = true;
  
      xfree( queue_entry );
    } else if ( is_update ) {
      is_update = false;
  
      ofnode_show_table();
    }
  } while ( queue_entry != NULL );
}

int
main( int argc, char **argv ) {
  discover_queue_init();
  ofnode_init_table();

  init_trema( &argc, &argv );

  add_periodic_event_callback( DISCOVERING_INTERVAL, discover_path, NULL );

  set_switch_ready_handler( handle_switch_ready, NULL );
  set_packet_in_handler( handle_packet_in, NULL );
  set_features_reply_handler( handle_features_reply, NULL );

  start_trema();

  ofnode_destroy();
  discover_queue_destroy();

  return 0;
}
