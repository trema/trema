/*
 * An OpenFlow switch interface library.
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


#ifndef OPENFLOW_SWITCH_INTERFACE_H
#define OPENFLOW_SWITCH_INTERFACE_H


#include "bool.h"
#include "buffer.h"
#include "openflow.h"
#include "openflow_message.h"


/********************************************************************************
 * Functions for initializing/finalizing the OpenFlow switch interface.
 ********************************************************************************/

bool init_openflow_switch_interface( const uint64_t datapath_id, uint32_t controller_ip, uint16_t controller_port );
bool finalize_openflow_switch_interface( void );
bool openflow_switch_interface_is_initialized( void );


/********************************************************************************
 * Event handler definitions.
 ********************************************************************************/

typedef void ( *controller_connected_handler )(
  void *user_data
);


typedef void ( *controller_disconnected_handler )(
  void *user_data
);


typedef void ( *hello_handler )(
  uint32_t transaction_id,
  uint8_t version,
  void *user_data
);


typedef void ( *error_handler )(
  uint32_t transaction_id,
  uint16_t type,
  uint16_t code,
  const buffer *data,
  void *user_data
);


typedef void ( *echo_request_handler )(
  uint32_t transaction_id,
  const buffer *body,
  void *user_data
);


typedef void ( *echo_reply_handler )(
  uint32_t transaction_id,
  const buffer *body,
  void *user_data
);


typedef void ( *vendor_handler )(
  uint32_t transaction_id,
  uint32_t vendor,
  const buffer *data,
  void *user_data
);


typedef void ( *features_request_handler )(
  uint32_t transaction_id,
  void *user_data
);


typedef void ( *get_config_request_handler )(
  uint32_t transaction_id,
  void *user_data
);


typedef void ( *set_config_handler )(
  uint32_t transaction_id,
  uint16_t flags,
  uint16_t miss_send_len,
  void *user_data
);


typedef void ( *packet_out_handler )(
  uint32_t transaction_id,
  uint32_t buffer_id,
  uint16_t in_port,
  const openflow_actions *actions,
  const buffer *data,
  void *user_data
);


typedef void ( *flow_mod_handler )(
  uint32_t transaction_id,
  struct ofp_match match,
  uint64_t cookie,
  uint16_t command,
  uint16_t idle_timeout,
  uint16_t hard_timeout,
  uint16_t priority,
  uint32_t buffer_id,
  uint16_t out_port,
  uint16_t flags,
  const openflow_actions *actions,
  void *user_data
);


typedef void ( *port_mod_handler )(
  uint32_t transaction_id,
  uint16_t port_no,
  uint8_t hw_addr[ OFP_ETH_ALEN ],
  uint32_t config,
  uint32_t mask,
  uint32_t advertise,
  void *user_data
);


typedef void ( *stats_request_handler )(
  uint32_t transaction_id,
  uint16_t type,
  uint16_t flags,
  const buffer *body,
  void *user_data
);


typedef void ( *barrier_request_handler )(
  uint32_t transaction_id,
  void *user_data
);


typedef void ( *queue_get_config_request_handler )(
  uint32_t transaction_id,
  uint16_t port,
  void *user_data
);


typedef struct {
  controller_connected_handler controller_connected_callback;
  void *controller_connected_user_data;

  controller_disconnected_handler controller_disconnected_callback;
  void *controller_disconnected_user_data;

  hello_handler hello_callback;
  void *hello_user_data;

  error_handler error_callback;
  void *error_user_data;

  echo_request_handler echo_request_callback;
  void *echo_request_user_data;

  echo_reply_handler echo_reply_callback;
  void *echo_reply_user_data;

  vendor_handler vendor_callback;
  void *vendor_user_data;

  features_request_handler features_request_callback;
  void *features_request_user_data;

  get_config_request_handler get_config_request_callback;
  void *get_config_request_user_data;

  set_config_handler set_config_callback;
  void *set_config_user_data;

  packet_out_handler packet_out_callback;
  void *packet_out_user_data;

  flow_mod_handler flow_mod_callback;
  void *flow_mod_user_data;

  port_mod_handler port_mod_callback;
  void *port_mod_user_data;

  stats_request_handler stats_request_callback;
  void *stats_request_user_data;

  barrier_request_handler barrier_request_callback;
  void *barrier_request_user_data;

  queue_get_config_request_handler queue_get_config_request_callback;
  void *queue_get_config_request_user_data;
} openflow_event_handlers;


/********************************************************************************
 * Functions for setting callback functions for OpenFlow related events.
 ********************************************************************************/

bool switch_set_openflow_event_handlers( const openflow_event_handlers handlers );
bool set_controller_connected_handler( controller_connected_handler callback, void *user_data );
bool set_controller_disconnected_handler( controller_disconnected_handler callback, void *user_data );
bool set_hello_handler( hello_handler callback, void *user_data );
bool switch_set_error_handler( error_handler callback, void *user_data );
bool set_echo_request_handler( echo_request_handler callback, void *user_data );
bool switch_set_echo_reply_handler( echo_reply_handler callback, void *user_data );
bool switch_set_vendor_handler( vendor_handler callback, void *user_data );
bool set_features_request_handler( features_request_handler callback, void *user_data );
bool set_get_config_request_handler( get_config_request_handler callback, void *user_data );
bool set_set_config_handler( set_config_handler callback, void *user_data );
bool set_packet_out_handler( packet_out_handler callback, void *user_data );
bool set_flow_mod_handler( flow_mod_handler callback, void *user_data );
bool set_port_mod_handler( port_mod_handler callback, void *user_data );
bool set_stats_request_handler( stats_request_handler callback, void *user_data );
bool set_barrier_request_handler( barrier_request_handler callback, void *user_data );
bool set_queue_get_config_request_handler( queue_get_config_request_handler callback, void *user_data );


/********************************************************************************
 * Function for sending/receiving OpenFlow messages.
 ********************************************************************************/

bool switch_send_openflow_message( buffer *message );
bool handle_secure_channel_message( buffer *message );
bool send_error_message( uint32_t transaction_id, uint16_t type, uint16_t code );


#endif // OPENFLOW_SWITCH_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
