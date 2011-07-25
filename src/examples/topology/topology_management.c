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
#include <openflow.h>
#include "trema.h"
#include "topology_table.h"
#include "service_management.h"
#include "topology_management.h"


#ifdef UNIT_TESTING

#ifdef get_transaction_id
#undef get_transaction_id
#endif
#define get_transaction_id mock_get_transaction_id
uint32_t mock_get_transaction_id( void );

#ifdef create_features_request
#undef create_features_request
#endif
#define create_features_request mock_create_features_request
buffer *mock_create_features_request( const uint32_t transaction_id );

#ifdef send_openflow_message
#undef send_openflow_message
#endif
#define send_openflow_message mock_send_openflow_message
bool mock_send_openflow_message( const uint64_t datapath_id, buffer *message );

#ifdef update_port_entry
#undef update_port_entry
#endif
#define update_port_entry mock_update_port_entry
port_entry *mock_update_port_entry( sw_entry *sw, uint16_t port_no, const char *name );

#ifdef notify_port_status_for_all_user
#undef notify_port_status_for_all_user
#endif
#define notify_port_status_for_all_user mock_notify_port_status_for_all_user
void mock_notify_port_status_for_all_user( port_entry *port );

#ifdef notify_link_status_for_all_user
#undef notify_link_status_for_all_user
#endif
#define notify_link_status_for_all_user mock_notify_link_status_for_all_user
void mock_notify_link_status_for_all_user( port_entry *port );

#ifdef delete_link_to
#undef delete_link_to
#endif
#define delete_link_to mock_delete_link_to
void mock_delete_link_to( port_entry *port );

#ifdef delete_port_entry
#undef delete_port_entry
#endif
#define delete_port_entry mock_delete_port_entry
void mock_delete_port_entry( sw_entry *sw, port_entry *port );

#ifdef update_sw_entry
#undef update_sw_entry
#endif
#define update_sw_entry mock_update_sw_entry
sw_entry *mock_update_sw_entry( uint64_t *datapath_id );

#ifdef lookup_sw_entry
#undef lookup_sw_entry
#endif
#define lookup_sw_entry mock_lookup_sw_entry
sw_entry *mock_lookup_sw_entry( uint64_t *datapath_id );

#ifdef delete_sw_entry
#undef delete_sw_entry
#endif
#define delete_sw_entry mock_delete_sw_entry
void delete_sw_entry( sw_entry *sw );


#ifdef lookup_port_entry
#undef lookup_port_entry
#endif
#define lookup_port_entry mock_lookup_port_entry
port_entry *mock_lookup_port_entry( sw_entry *sw, uint16_t port_no, const char *name );

#ifdef get_trema_name
#undef get_trema_name
#endif
#define get_trema_name mock_get_trema_name
const char *mock_get_trema_name( void );

#ifdef init_openflow_application_interface
#undef init_openflow_application_interface
#endif
#define init_openflow_application_interface mock_init_openflow_application_interface
bool mock_init_openflow_application_interface( const char *custom_service_name );

#ifdef set_switch_ready_handler
#undef set_switch_ready_handler
#endif
#define set_switch_ready_handler mock_set_switch_ready_handler
bool mock_set_switch_ready_handler( switch_ready_handler callback, void *user_data );

#ifdef set_switch_disconnected_handler
#undef set_switch_disconnected_handler
#endif
#define set_switch_disconnected_handler mock_set_switch_disconnected_handler
bool mock_set_switch_disconnected_handler( switch_disconnected_handler callback, void *user_data );

#ifdef set_features_reply_handler
#undef set_features_reply_handler
#endif
#define set_features_reply_handler mock_set_features_reply_handler
bool mock_set_features_reply_handler( features_reply_handler callback, void *user_data );

#ifdef set_port_status_handler
#undef set_port_status_handler
#endif
#define set_port_status_handler mock_set_port_status_handler
bool mock_set_port_status_handler( port_status_handler callback, void *user_data );

#define static

#endif  // UNIT_TESTING


static const uint16_t INITIAL_DISCOVERY_PERIOD = 5;


static void
send_flow_mod_receiving_lldp( sw_entry *sw, uint16_t hard_timeout, uint16_t priority ) {
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = ( OFPFW_ALL & ~OFPFW_DL_TYPE );
  match.dl_type = ETH_ETHTYPE_LLDP;

  openflow_actions *actions = create_actions();
  const uint16_t max_len = UINT16_MAX;
  append_action_output( actions, OFPP_CONTROLLER, max_len );

  const uint16_t idle_timeout = 0;
  const uint32_t buffer_id = UINT32_MAX;
  const uint16_t flags = 0;
  buffer *flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(),
                                      OFPFC_ADD, idle_timeout, hard_timeout,
                                      priority, buffer_id,
                                      OFPP_NONE, flags, actions );
  send_openflow_message( sw->datapath_id, flow_mod );
  delete_actions( actions );
  free_buffer( flow_mod );
  debug( "Sent a flow_mod for receiving LLDP frames from %#" PRIx64 ".", sw->datapath_id );
}


static void
send_flow_mod_discarding_all_packets( sw_entry *sw, uint16_t hard_timeout, uint16_t priority ) {
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  const uint16_t idle_timeout = 0;
  const uint32_t buffer_id = UINT32_MAX;
  const uint16_t flags = 0;
  buffer *flow_mod = create_flow_mod( get_transaction_id(), match, get_cookie(),
                                      OFPFC_ADD, idle_timeout, hard_timeout,
                                      priority, buffer_id,
                                      OFPP_NONE, flags, NULL );
  send_openflow_message( sw->datapath_id, flow_mod );
  free_buffer( flow_mod );
  debug( "Sent a flow_mod for discarding all packets received on %#" PRIx64 ".", sw->datapath_id );
}


static void
start_initial_discovery( sw_entry *sw ) {
  send_flow_mod_receiving_lldp( sw, INITIAL_DISCOVERY_PERIOD, UINT16_MAX );
  send_flow_mod_discarding_all_packets( sw, INITIAL_DISCOVERY_PERIOD, UINT16_MAX - 1 );
}


static void
send_features_request( sw_entry *sw ) {
  uint32_t id = get_transaction_id();
  buffer *buf = create_features_request( id );
  send_openflow_message( sw->datapath_id, buf );
  free_buffer( buf );
  debug( "Sent switch features request to %#" PRIx64 ".", sw->datapath_id );
}


static bool
is_port_up( const struct ofp_phy_port *phy_port ) {
  if ( ( phy_port->config & OFPPC_PORT_DOWN ) == OFPPC_PORT_DOWN ) {
    return false;
  }
  if ( ( phy_port->state & OFPPS_LINK_DOWN ) == OFPPS_LINK_DOWN ) {
    return false;
  }

  return true;
}


static void
add_notification( sw_entry *sw, const struct ofp_phy_port *phy_port ) {
  port_entry *port = update_port_entry( sw, phy_port->port_no, phy_port->name );
  port->up = is_port_up( phy_port );
  port->id = sw->id;
  memcpy( port->mac, phy_port->hw_addr, ETH_ADDRLEN );

  // Port status notification
  notify_port_status_for_all_user( port );
}


static void
delete_notification( sw_entry *sw, port_entry *port ) {
  if ( port->link_to != NULL ) {
    port->link_to->up = false;
    // Link status notification
    notify_link_status_for_all_user( port );
    delete_link_to( port );
    port->external = false;
  }
  port->up = false;
  port->id = sw->id;
  // Port status notification
  notify_port_status_for_all_user( port );
  delete_port_entry( sw, port );
}


static void
update_notification( sw_entry *sw, port_entry *port, const struct ofp_phy_port *phy_port ) {
  UNUSED( sw );
  UNUSED( port );

  if ( strncmp( port->name, phy_port->name, sizeof( port->name ) ) != 0 ) {
    strncpy( port->name, phy_port->name, sizeof( port->name ) );
    port->name[ OFP_MAX_PORT_NAME_LEN - 1] = '\0';
  }

  bool up = is_port_up( phy_port );
  if ( port->up == up ) {
    return;
  }
  port->up = up;
  port->id = sw->id;
  memcpy( port->mac, phy_port->hw_addr, ETH_ADDRLEN );

  if ( !port->up && port->link_to != NULL ) {
    port->link_to->up = false;
    // Link status notification
    notify_link_status_for_all_user( port );
    delete_link_to( port );
    port->external = false;
  }
  // Port status notification
  notify_port_status_for_all_user( port );
}


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  sw_entry *sw = update_sw_entry( &datapath_id );
  debug( "Switch(%#" PRIx64 ") is connected.", datapath_id );
  start_initial_discovery( sw );
  send_features_request( sw );
}


static void
switch_disconnected( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  sw_entry *sw = lookup_sw_entry( &datapath_id );
  if ( sw == NULL ) {
    warn( "Received switch-disconnected event, but switch(%#" PRIx64 ") is not found.", datapath_id );
    return;
  }
  const list_element *list, *next;
  for ( list = sw->port_table; list != NULL; list = next ) {
    next = list->next;
    delete_notification( sw, list->data );
  }
  delete_sw_entry( sw );
  debug( "Switch(%#" PRIx64 ") is disconnected.", datapath_id );
}


static void
switch_features_reply( uint64_t datapath_id, uint32_t transaction_id,
                       uint32_t n_buffers, uint8_t n_tables,
                       uint32_t capabilities, uint32_t actions,
                       const list_element *phy_ports, void *user_data ) {

  UNUSED( n_buffers );
  UNUSED( n_tables );
  UNUSED( capabilities );
  UNUSED( actions );
  UNUSED( user_data );

  sw_entry *sw = lookup_sw_entry( &datapath_id );
  if ( sw == NULL ) {
    warn( "Received features-reply from switch(%#" PRIx64 "), but the switch is not found.", datapath_id );
    return;
  }
  debug( "Received features-reply from switch(%#" PRIx64 ").", datapath_id );
  sw->id = transaction_id;

  const list_element *list, *next;
  for ( list = phy_ports; list != NULL; list = list->next ) {
    const struct ofp_phy_port *phy_port = list->data;
    if ( phy_port->port_no > OFPP_MAX && phy_port->port_no != OFPP_LOCAL ) {
      warn( "Ignore phy_port(port:%u) in features-reply from switch(%#" PRIx64 ").",
            phy_port->port_no, datapath_id );
      continue;
    }
    port_entry *port = update_port_entry( sw, phy_port->port_no, phy_port->name );
    port->id = transaction_id;
    update_notification( sw, port, phy_port );
    debug( "Updated features-reply from switch(%#" PRIx64 "), port_no(%u).",
           datapath_id, phy_port->port_no );
  }
  for ( list = sw->port_table; list != NULL; list = next ) {
    next = list->next;
    port_entry *port = list->data;
    if ( port->id == transaction_id ) {
      continue;
    }
    delete_notification( sw, port );
    debug( "Deleted port(%u) in switch(%#" PRIx64 ")", datapath_id, port->port_no );
  }
}


static void
port_status( uint64_t datapath_id, uint32_t transaction_id, uint8_t reason,
             struct ofp_phy_port phy_port, void *user_data ) {
  UNUSED( transaction_id );
  UNUSED( user_data );

  sw_entry *sw = lookup_sw_entry( &datapath_id );
  if ( sw == NULL ) {
    warn( "Received port-status, but switch(%#" PRIx64 ") is not found.", datapath_id );
    return;
  }
  port_entry *port = lookup_port_entry( sw, phy_port.port_no, phy_port.name );
  switch ( reason ) {
    case OFPPR_ADD:
      if ( port != NULL ) {
        warn( "Failed to add port(%u, `%s'), switch(%#" PRIx64 ").",
              phy_port.port_no, phy_port.name, datapath_id );
        return;
      }
      add_notification( sw, &phy_port );
      break;

    case OFPPR_DELETE:
      if ( port == NULL ) {
        warn( "Failed to delete port(%u, `%s'), switch(%#" PRIx64 ").",
              phy_port.port_no, phy_port.name, datapath_id );
        return;
      }
      delete_notification( sw, port );
      break;

    case OFPPR_MODIFY:
      if ( port == NULL ) {
        warn( "Failed to modify port(%u, `%s'), switch(%#" PRIx64 ").",
              phy_port.port_no, phy_port.name, datapath_id );
        return;
      }
      if ( port->port_no != phy_port.port_no ) {
        delete_notification( sw, port );
        add_notification( sw, &phy_port );
      } else {
        update_notification( sw, port, &phy_port );
      }
      break;
    default:
      warn( "Failed to handle port status: unknown reason(%u) from switch(%#" PRIx64 ").",
            reason, datapath_id );
      return;
  }
}


bool
start_topology_management( void ) {
  init_openflow_application_interface( get_trema_name() );
  set_switch_ready_handler( handle_switch_ready, NULL );
  set_switch_disconnected_handler( switch_disconnected, NULL );
  set_features_reply_handler( switch_features_reply, NULL );
  set_port_status_handler( port_status, NULL );

  return true;
}


void
stop_topology_management( void ) {
  // do something here.
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
