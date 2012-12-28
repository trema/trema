/*
 * Author: Shuji Ishii, Kazushi SUGYO
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
#include <openflow.h>
#include "etherip.h"
#include "trema.h"
#include "topology_table.h"
#include "service_management.h"
#include "topology_management.h"



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
add_port_notification( sw_entry *sw, const struct ofp_phy_port *phy_port ) {
  port_entry *port = update_port_entry( sw, phy_port->port_no, phy_port->name );
  port->up = is_port_up( phy_port );
  port->id = sw->id;
  memcpy( port->mac, phy_port->hw_addr, ETH_ADDRLEN );

  // Port status notification
  notify_port_status_for_all_user( port );
}


static void
delete_port_notification( sw_entry *sw, port_entry *port ) {
  if ( port->link_to != NULL ) {
    port->link_to->up = false;
    // Link status notification
    info( "Link down (%#" PRIx64 ":[%u])->(%#" PRIx64 ":%u)", port->sw->datapath_id, port->port_no, port->link_to->datapath_id, port->link_to->port_no );
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
update_port_notification( sw_entry *sw, port_entry *port, const struct ofp_phy_port *phy_port ) {
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
    info( "Link down (%#" PRIx64 ":[%u])->(%#" PRIx64 ":%u)", port->sw->datapath_id, port->port_no, port->link_to->datapath_id, port->link_to->port_no );
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

  sw_entry *sw = lookup_sw_entry( &datapath_id );
  if ( sw != NULL ) {
    warn( "Received switch-ready event, but switch(%#" PRIx64 ") already exists.", datapath_id );
  } else {
    sw = update_sw_entry( &datapath_id );
  }
  sw->up = true;
  info( "Switch(%#" PRIx64 ") is connected.", datapath_id );

  // switch ready to subscribed users
  notify_switch_status_for_all_user( sw );

  send_features_request( sw );
}


static void
handle_switch_disconnected( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  sw_entry *sw = lookup_sw_entry( &datapath_id );
  if ( sw == NULL ) {
    warn( "Received switch-disconnected event, but switch(%#" PRIx64 ") is not found.", datapath_id );
    return;
  }
  sw->up = false;
  const list_element *list, *next;
  for ( list = sw->port_table; list != NULL; list = next ) {
    next = list->next;
    port_entry *port = list->data;
    info( "Port del ([%#" PRIx64 "]:%u)", datapath_id, port->port_no );
    delete_port_notification( sw, list->data );
  }
  info( "Switch(%#" PRIx64 ") is disconnected.", datapath_id );
  notify_switch_status_for_all_user( sw );
  delete_sw_entry( sw );
}


static void
handle_switch_features_reply( uint64_t datapath_id, uint32_t transaction_id,
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
    port_entry *port = lookup_port_entry_by_port( sw, phy_port->port_no );
    if ( port == NULL ) {
      info( "Port add ([%#" PRIx64 "]:%u)", datapath_id, phy_port->port_no );
      add_port_notification( sw, phy_port );
    } else {
      port->id = transaction_id;
      info( "Port mod ([%#" PRIx64 "]:%u)", datapath_id, phy_port->port_no );
      update_port_notification( sw, port, phy_port );
    }
    debug( "Updated features-reply from switch(%#" PRIx64 "), port_no(%u).",
           datapath_id, phy_port->port_no );
  }
  for ( list = sw->port_table; list != NULL; list = next ) {
    next = list->next;
    port_entry *port = list->data;
    if ( port->id == transaction_id ) {
      continue;
    }
    info( "Port del ([%#" PRIx64 "]:%u)", datapath_id, port->port_no );
    delete_port_notification( sw, port );
    debug( "Deleted port(%u) in switch(%#" PRIx64 ")", port->port_no, datapath_id );
  }
}


static void
handle_port_status( uint64_t datapath_id, uint32_t transaction_id, uint8_t reason,
             struct ofp_phy_port phy_port, void *user_data ) {
  UNUSED( transaction_id );
  UNUSED( user_data );

  sw_entry *sw = lookup_sw_entry( &datapath_id );
  if ( sw == NULL ) {
    warn( "Received port-status, but switch(%#" PRIx64 ") is not found.", datapath_id );
    return;
  }
  port_entry *port = lookup_port_entry_by_port( sw, phy_port.port_no );

  switch ( reason ) {
    case OFPPR_ADD:
      if ( port != NULL ) {
        warn( "Failed to add port(%u, `%s'), switch(%#" PRIx64 ").",
              phy_port.port_no, phy_port.name, datapath_id );
        return;
      }
      info( "Port add (%#" PRIx64 ":%u)", datapath_id, phy_port.port_no );
      add_port_notification( sw, &phy_port );
      break;

    case OFPPR_DELETE:
      if ( port == NULL ) {
        warn( "Failed to delete port(%u, `%s'), switch(%#" PRIx64 ").",
              phy_port.port_no, phy_port.name, datapath_id );
        return;
      }
      info( "Port del (%#" PRIx64 ":%u)", datapath_id, phy_port.port_no );
      delete_port_notification( sw, port );
      break;

    case OFPPR_MODIFY:
      if ( port == NULL ) {
        debug( "Modified port not found by port_no. (%u)", phy_port.port_no );
        port = lookup_port_entry_by_name( sw, phy_port.name );
      }
      if ( port == NULL ) {
        warn( "Failed to modify port(%u, `%s'), switch(%#" PRIx64 ").",
              phy_port.port_no, phy_port.name, datapath_id );
        return;
      }
      if ( port->port_no != phy_port.port_no ) {
        info( "Port mod (%#" PRIx64 ":%u->%u)", datapath_id, port->port_no, phy_port.port_no );
        delete_port_notification( sw, port );
        add_port_notification( sw, &phy_port );
      } else {
        info( "Port mod (%#" PRIx64 ":%u)", datapath_id, phy_port.port_no );
        update_port_notification( sw, port, &phy_port );
      }
      break;
    default:
      warn( "Failed to handle port status: unknown reason(%u) from switch(%#" PRIx64 ").",
            reason, datapath_id );
      return;
  }
}


static char PORT_STATUS[] = "port_status";
static char STATE_NOTIFY[] = "state_notify";
static void
handle_event_forward_entry_to_all_result( enum efi_result result, void *user_data ) {
  if ( result == EFI_OPERATION_FAILED ) {
    error( "Registering topology to switch event  '%s' failed.", ( const char * ) user_data );
  }
}


static void
emulate_initial_switch_ready( uint64_t* dpids, size_t n_dpids, void *user_data ) {
  UNUSED( user_data );
  if( dpids == NULL ) {
    error( "Failed to get initial switch lists" );
    return;
  }

  for ( size_t i = 0 ; i < n_dpids ; ++i ) {
    handle_switch_ready( dpids[i], NULL );
  }
}


bool
init_topology_management( void ) {
  bool result = true;

  set_switch_ready_handler( handle_switch_ready, NULL );
  set_switch_disconnected_handler( handle_switch_disconnected, NULL );
  set_features_reply_handler( handle_switch_features_reply, NULL );
  set_port_status_handler( handle_port_status, NULL );

  return result;
}


void
finalize_topology_management( void ) {
}


bool
start_topology_management( void ) {
  add_event_forward_entry_to_all_switches( EVENT_FORWARD_TYPE_PORT_STATUS, get_trema_name(), handle_event_forward_entry_to_all_result, PORT_STATUS );
  add_event_forward_entry_to_all_switches( EVENT_FORWARD_TYPE_STATE_NOTIFY, get_trema_name(), handle_event_forward_entry_to_all_result, STATE_NOTIFY );
  send_efi_switch_list_request( emulate_initial_switch_ready, NULL );
  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
