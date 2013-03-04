/*
 * An OpenFlow application interface library.
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


#ifndef OPENFLOW_APPLICATION_INTERFACE_H
#define OPENFLOW_APPLICATION_INTERFACE_H


#include <arpa/inet.h>
#include "buffer.h"
#include "linked_list.h"
#include "openflow.h"
#include "openflow_service_interface.h"


/********************************************************************************
 * Functions for initializing/finalizing the OpenFlow application interface.
 ********************************************************************************/

bool init_openflow_application_interface( const char *service_name );
bool finalize_openflow_application_interface( void );
bool openflow_application_interface_is_initialized( void );


/********************************************************************************
 * Event handler definitions.
 ********************************************************************************/

typedef struct {
  uint64_t datapath_id;
  void *user_data;
} switch_ready;

typedef void ( simple_switch_ready_handler )( switch_ready event );

typedef void ( switch_ready_handler )(
  uint64_t datapath_id,
  void *user_data
);


typedef void ( *switch_disconnected_handler )(
  uint64_t datapath_id,
  void *user_data
);


typedef void ( *error_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t type,
  uint16_t code,
  const buffer *data,
  void *user_data
);


typedef void ( *echo_reply_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  const buffer *data,
  void *user_data
);


typedef void ( *vendor_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t vendor,
  const buffer *data,
  void *user_data
);


typedef void ( *features_reply_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t n_buffers,
  uint8_t n_tables,
  uint32_t capabilities,
  uint32_t actions,
  const list_element *phy_ports,
  void *user_data
);


typedef void ( *get_config_reply_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t flags,
  uint16_t miss_send_len,
  void *user_data
);


typedef struct {
  uint64_t datapath_id;
  uint32_t transaction_id;
  uint32_t buffer_id;
  uint16_t total_len;
  uint16_t in_port;
  uint8_t reason;
  const buffer *data;
  void *user_data;
} packet_in;

typedef void ( simple_packet_in_handler )( uint64_t datapath_id, packet_in message );
typedef void ( packet_in_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t buffer_id,
  uint16_t total_len,
  uint16_t in_port,
  uint8_t reason,
  const buffer *data,
  void *user_data
);


typedef struct {
  uint64_t datapath_id;
  uint32_t transaction_id;
  struct ofp_match match;
  uint64_t cookie;
  uint16_t priority;
  uint8_t reason;
  uint32_t duration_sec;
  uint32_t duration_nsec;
  uint16_t idle_timeout;
  uint64_t packet_count;
  uint64_t byte_count;
  void *user_data;
} flow_removed;

typedef void ( simple_flow_removed_handler )( uint64_t datapath_id, flow_removed message );
typedef void ( flow_removed_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  struct ofp_match match,
  uint64_t cookie,
  uint16_t priority,
  uint8_t reason,
  uint32_t duration_sec,
  uint32_t duration_nsec,
  uint16_t idle_timeout,
  uint64_t packet_count,
  uint64_t byte_count,
  void *user_data
);


typedef void ( *port_status_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint8_t reason,
  struct ofp_phy_port phy_port,
  void *user_data
);


typedef void ( *stats_reply_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t type,
  uint16_t flags,
  const buffer *data,
  void *user_data
);


typedef void ( *barrier_reply_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  void *user_data
);


typedef void ( *queue_get_config_reply_handler )(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t port,
  const list_element *queues,
  void *user_data
);


typedef void ( *list_switches_reply_handler )(
  const list_element *switches,
  void *user_data
);


typedef struct openflow_event_handlers {
  bool simple_switch_ready_callback;
  void *switch_ready_callback;
  void *switch_ready_user_data;

  switch_disconnected_handler switch_disconnected_callback;
  void *switch_disconnected_user_data;

  error_handler error_callback;
  void *error_user_data;

  echo_reply_handler echo_reply_callback;
  void *echo_reply_user_data;

  vendor_handler vendor_callback;
  void *vendor_user_data;

  features_reply_handler features_reply_callback;
  void *features_reply_user_data;

  get_config_reply_handler get_config_reply_callback;
  void *get_config_reply_user_data;

  bool simple_packet_in_callback;
  void *packet_in_callback;
  void *packet_in_user_data;

  bool simple_flow_removed_callback;
  void *flow_removed_callback;
  void *flow_removed_user_data;

  port_status_handler port_status_callback;
  void *port_status_user_data;

  stats_reply_handler stats_reply_callback;
  void *stats_reply_user_data;

  barrier_reply_handler barrier_reply_callback;
  void *barrier_reply_user_data;

  queue_get_config_reply_handler queue_get_config_reply_callback;
  void *queue_get_config_reply_user_data;

  list_switches_reply_handler list_switches_reply_callback;
} openflow_event_handlers_t;


/********************************************************************************
 * Functions for setting callback functions for OpenFlow related events.
 ********************************************************************************/

bool set_openflow_event_handlers( const openflow_event_handlers_t handlers );

#define set_switch_ready_handler( callback, user_data )                                      \
  {                                                                                          \
    if ( __builtin_types_compatible_p( typeof( callback ), simple_switch_ready_handler ) ) { \
      _set_switch_ready_handler( true, ( void * ) callback, user_data );                     \
    }                                                                                        \
    else if ( __builtin_types_compatible_p( typeof( callback ), switch_ready_handler ) ) {   \
      _set_switch_ready_handler( false, ( void * ) callback, user_data );                    \
    }                                                                                        \
    else {                                                                                   \
      _set_switch_ready_handler( false, NULL, NULL );                                        \
    }                                                                                        \
  }
bool _set_switch_ready_handler( bool simple_callback, void *callback, void *user_data );


bool set_switch_disconnected_handler( switch_disconnected_handler callback, void *user_data );
bool set_error_handler( error_handler callback, void *user_data );
bool set_echo_reply_handler( echo_reply_handler callback, void *user_data );
bool set_vendor_handler( vendor_handler callback, void *user_data );
bool set_features_reply_handler( features_reply_handler callback, void *user_data );
bool set_get_config_reply_handler( get_config_reply_handler callback, void *user_data );


#define set_packet_in_handler( callback, user_data )                                      \
  {                                                                                       \
    if ( __builtin_types_compatible_p( typeof( callback ), simple_packet_in_handler ) ) { \
      _set_packet_in_handler( true, ( void * ) callback, user_data );                     \
    }                                                                                     \
    else if ( __builtin_types_compatible_p( typeof( callback ), packet_in_handler ) ) {   \
      _set_packet_in_handler( false, ( void * ) callback, user_data );                    \
    }                                                                                     \
    else {                                                                                \
      _set_packet_in_handler( false, NULL, NULL );                                        \
    }                                                                                     \
  }
bool _set_packet_in_handler( bool simple_callback, void *callback, void *user_data );


#define set_flow_removed_handler( callback, user_data )                                      \
  {                                                                                          \
    if ( __builtin_types_compatible_p( typeof( callback ), simple_flow_removed_handler ) ) { \
      _set_flow_removed_handler( true, ( void * ) callback, user_data );                     \
    }                                                                                        \
    else if ( __builtin_types_compatible_p( typeof( callback ), flow_removed_handler ) ) {   \
      _set_flow_removed_handler( false, ( void * ) callback, user_data );                    \
    }                                                                                        \
    else {                                                                                   \
      _set_flow_removed_handler( false, NULL, NULL );                                        \
    }                                                                                        \
  }
bool _set_flow_removed_handler( bool simple_callback, void *callback, void *user_data );


bool set_port_status_handler( port_status_handler callback, void *user_data );
bool set_stats_reply_handler( stats_reply_handler callback, void *user_data );
bool set_barrier_reply_handler( barrier_reply_handler callback, void *user_data );
bool set_queue_get_config_reply_handler( queue_get_config_reply_handler callback, void *user_data );

bool set_list_switches_reply_handler( list_switches_reply_handler callback );


/********************************************************************************
 * Function for sending an OpenFlow message to an OpenFlow switch.
 ********************************************************************************/

bool send_openflow_message( const uint64_t datapath_id, buffer *message );


/********************************************************************************
 * Function for retrieving the list of switches from Switch Manager.
 ********************************************************************************/

bool send_list_switches_request( void *user_data );


/********************************************************************************
 * Function for deleting OpenFlow messages in a send queue
 ********************************************************************************/

bool delete_openflow_messages( uint64_t datapath_id );


/********************************************************************************
 * Function for disconnecting a switch
 ********************************************************************************/

bool disconnect_switch( uint64_t datapath_id );


#endif // OPENFLOW_APPLICATION_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
