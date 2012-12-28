/*
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


#include "discovery_management.h"

#include <assert.h>
#include "service_management.h"
#include "lldp.h"
#include "probe_timer_table.h"

#include "utility.h"
#include "wrapper.h"

static bool g_discovery_enabled = false;

static discovery_management_options options;

bool ( *send_probe )( const uint8_t *mac, uint64_t dpid, uint16_t port_no ) = send_lldp;

bool
init_discovery_management( discovery_management_options new_options ) {
  options = new_options;
  bool result = true;

  init_probe_timer_table();
  result = init_lldp( new_options.lldp );

  return result;
}


void
finalize_discovery_management( void ) {
  finalize_lldp();
  finalize_probe_timer_table();
  g_discovery_enabled = false;
}


bool
start_discovery_management( void ){
  if ( options.always_enabled ) {
    enable_discovery();
  }
  return true;
}


static void
set_match_for_lldp( struct ofp_match *match ) {
  assert( match != NULL );
  memset( match, 0, sizeof( struct ofp_match ) );
  if ( !options.lldp.lldp_over_ip ) {
    match->wildcards = OFPFW_ALL & ~OFPFW_DL_TYPE;
    match->dl_type = ETH_ETHTYPE_LLDP;
  }
  else {
    match->wildcards = OFPFW_ALL & ~( OFPFW_DL_TYPE | OFPFW_NW_PROTO | OFPFW_NW_SRC_MASK | OFPFW_NW_DST_MASK );
    match->dl_type = ETH_ETHTYPE_IPV4;
    match->nw_proto = IPPROTO_ETHERIP;
    match->nw_src = options.lldp.lldp_ip_src;
    match->nw_dst = options.lldp.lldp_ip_dst;
  }
}


static void
send_flow_mod_receiving_lldp( const sw_entry *sw, uint16_t hard_timeout, uint16_t priority, bool add ) {
  struct ofp_match match;
  set_match_for_lldp( &match );

  openflow_actions *actions = create_actions();
  const uint16_t max_len = UINT16_MAX;
  append_action_output( actions, OFPP_CONTROLLER, max_len );

  const uint16_t idle_timeout = 0;
  const uint32_t buffer_id = UINT32_MAX;
  const uint16_t flags = 0;
  buffer *flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(),
                                      ( add )? OFPFC_ADD : OFPFC_DELETE, idle_timeout, hard_timeout,
                                      priority, buffer_id,
                                      OFPP_NONE, flags, actions );
  send_openflow_message( sw->datapath_id, flow_mod );
  delete_actions( actions );
  free_buffer( flow_mod );
  debug( "Sent a flow_mod for receiving LLDP frames from %#" PRIx64 ".", sw->datapath_id );
}


static void
send_add_LLDP_flow_mods( const sw_entry *sw ) {
  const bool add = true;
  send_flow_mod_receiving_lldp( sw, 0, UINT16_MAX, add );
}


static void
send_del_LLDP_flow_mods( const sw_entry *sw ) {
  const bool add = false;
  send_flow_mod_receiving_lldp( sw, 0, UINT16_MAX, add );
}


static void
update_port_status( const port_entry *s ) {
  if ( s->port_no > OFPP_MAX ) {
    return;
  }
  probe_timer_entry *entry = delete_probe_timer_entry( &( s->sw->datapath_id ), s->port_no );
  if ( !s->up ) {
    if ( entry != NULL ) {
      probe_request( entry, PROBE_TIMER_EVENT_DOWN, 0, 0 );
      free_probe_timer_entry( entry );
    }
    return;
  }
  if ( entry == NULL ) {
    entry = allocate_probe_timer_entry( &( s->sw->datapath_id ), s->port_no, s->mac );
  }
  probe_request( entry, PROBE_TIMER_EVENT_UP, 0, 0 );
}

static void
port_entry_walker( port_entry *entry, void *user_data ) {
  UNUSED( user_data );
  update_port_status( entry );
}

static void
handle_port_status_updated_callback( void* param, const port_entry *port ) {
  UNUSED( param );
  update_port_status( port );
}

static void
handle_switch_status_updated_callback( void* param, const sw_entry *sw ) {
  UNUSED( param );
  if ( sw->up ) {
    // switch ready
    send_add_LLDP_flow_mods( sw );
  }
}

static void
switch_add_LLDP_flow_mods( sw_entry *sw, void *user_data ) {
  UNUSED( user_data );
  send_add_LLDP_flow_mods( sw );
}

static void
switch_del_LLDP_flow_mods( sw_entry *sw, void *user_data ) {
  UNUSED( user_data );
  send_del_LLDP_flow_mods( sw );
}

static void
ignore_packet_in( uint64_t dst_datapath_id,
                  uint32_t transaction_id,
                  uint32_t buffer_id,
                  uint16_t total_len,
                  uint16_t dst_port_no,
                  uint8_t reason,
                  const buffer *m,
                  void *user_data ) {
  UNUSED( dst_datapath_id );
  UNUSED( transaction_id );
  UNUSED( buffer_id );
  UNUSED( total_len );
  UNUSED( dst_port_no );
  UNUSED( reason );
  UNUSED( m );
  UNUSED( user_data );
}

static void
handle_packet_in( uint64_t dst_datapath_id,
                  uint32_t transaction_id,
                  uint32_t buffer_id,
                  uint16_t total_len,
                  uint16_t dst_port_no,
                  uint8_t reason,
                  const buffer *m,
                  void *user_data ) {
  UNUSED( transaction_id );
  UNUSED( buffer_id );
  UNUSED( total_len );
  UNUSED( reason );
  UNUSED( user_data );
  packet_info *packet_info = m->user_data;
  assert( packet_info != NULL );

  // check if LLDP or not
  if ( packet_info->eth_type != ETH_ETHTYPE_LLDP ) {
    notice( "Non-LLDP packet is received ( type = %#x ).", packet_info->eth_type );
    return;
  }

  uint64_t src_datapath_id;
  uint16_t src_port_no;
  if ( parse_lldp( &src_datapath_id, &src_port_no, m ) == false ) {
    notice( "Failed to parse LLDP packet" );
    return;
  }

  debug( "Receive LLDP Frame (%#" PRIx64 ", %u) from (%#" PRIx64 ", %u).",
         dst_datapath_id, dst_port_no, src_datapath_id, src_port_no );
  probe_timer_entry *entry = delete_probe_timer_entry( &src_datapath_id,
                                                       src_port_no );
  if ( entry == NULL ) {
    debug( "Not found dst datapath_id (%#" PRIx64 ", %u).", src_datapath_id,
            src_port_no );
    return;
  }

  probe_request( entry, PROBE_TIMER_EVENT_RECV_LLDP, &dst_datapath_id, dst_port_no );
}

static char PACKET_IN[] = "packet_in";
static void
handle_event_forward_entry_to_all_result( enum efi_result result, void *user_data ) {
  if ( result == EFI_OPERATION_FAILED ) {
    warn( "Registering/Unregistering topology to switch event  '%s' failed.", ( const char * ) user_data );
  }
}


void
_enable_discovery( void ) {
  info( "Enabling topology discovery." );
  if ( g_discovery_enabled ) {
    warn( "Topology Discovery is already enabled." );
  }
  g_discovery_enabled = true;
  // insert LLDP flow entry
  foreach_sw_entry( switch_add_LLDP_flow_mods, NULL );

  // start receiving packet-in
  set_packet_in_handler( handle_packet_in, NULL );

  // get event from all switches (directly)
  add_event_forward_entry_to_all_switches( EVENT_FORWARD_TYPE_PACKET_IN, get_trema_name(), handle_event_forward_entry_to_all_result, PACKET_IN );

  set_switch_status_updated_hook( handle_switch_status_updated_callback, NULL );
  set_port_status_updated_hook( handle_port_status_updated_callback, NULL );

  // update all port status
  foreach_port_entry( port_entry_walker, NULL );
}
void (* enable_discovery )( void ) = _enable_discovery;

void
_disable_discovery( void ) {
  if ( options.always_enabled ) return;
  info( "Disabling topology discovery." );
  if ( !g_discovery_enabled ) {
    warn( "Topology Discovery was not enabled." );
  }
  g_discovery_enabled = false;

  // stop receiving packet-in
  set_packet_in_handler( ignore_packet_in, NULL );

  // stop getting event from all switches (directly)
  delete_event_forward_entry_to_all_switches( EVENT_FORWARD_TYPE_PACKET_IN, get_trema_name(), handle_event_forward_entry_to_all_result, PACKET_IN );

  // ignore switch/port events
  set_switch_status_updated_hook( NULL, NULL );
  set_port_status_updated_hook( NULL, NULL );

  // remove LLDP flow entry
  foreach_sw_entry( switch_del_LLDP_flow_mods, NULL );
}
void (* disable_discovery )( void ) = _disable_discovery;

