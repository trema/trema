/*
 * An OpenFlow message library.
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


#ifndef OPENFLOW_MESSAGE_H
#define OPENFLOW_MESSAGE_H


#include <openflow.h>
#include "bool.h"
#include "buffer.h"
#include "byteorder.h"
#include "linked_list.h"
#include "packet_info.h"


// A structure for storing OpenFlow actions
typedef struct openflow_actions {
  int n_actions;
  list_element *list;
} openflow_actions;


// Initialization
bool init_openflow_message( void );

// Functions for creating OpenFlow messages
buffer *create_hello( const uint32_t transaction_id );
buffer *create_error( const uint32_t transaction_id, const uint16_t type,
                      const uint16_t code, const buffer *data );
buffer *create_echo_request( const uint32_t transaction_id, const buffer *body );
buffer *create_echo_reply( const uint32_t transaction_id, const buffer *body );
buffer *create_vendor( const uint32_t transaction_id, const uint32_t vendor,
                       const buffer *body );
buffer *create_features_request( const uint32_t transaction_id );
buffer *create_features_reply( const uint32_t transaction_id, const uint64_t datapath_id,
                               const uint32_t n_buffers, const uint8_t n_tables,
                               const uint32_t capabilities, const uint32_t actions,
                               const list_element *ports );
buffer *create_get_config_request( const uint32_t transaction_id );
buffer *create_get_config_reply( const uint32_t transaction_id, const uint16_t flags,
                                 const uint16_t miss_send_len );
buffer *create_set_config( const uint32_t transaction_id, const uint16_t flags,
                           const uint16_t miss_send_len );
buffer *create_packet_in( const uint32_t transaction_id, const uint32_t buffer_id,
                          const uint16_t total_len, uint16_t in_port,
                          const uint8_t reason, const buffer *data );
buffer *create_flow_removed( const uint32_t transaction_id, const struct ofp_match match,
                            const uint64_t cookie, const uint16_t priority,
                            const uint8_t reason, const uint32_t duration_sec,
                            const uint32_t duration_nsec, const uint16_t idle_timeout,
                            const uint64_t packet_count, const uint64_t byte_count );
buffer *create_port_status( const uint32_t transaction_id, const uint8_t reason,
                            const struct ofp_phy_port desc );
buffer *create_packet_out( const uint32_t transaction_id, const uint32_t buffer_id,
                           const uint16_t in_port, const openflow_actions *actions,
                           const buffer *data );
buffer *create_flow_mod(
  const uint32_t transaction_id,
  const struct ofp_match match,
  const uint64_t cookie,
  const uint16_t command,
  const uint16_t idle_timeout,
  const uint16_t hard_timeout,
  const uint16_t priority,
  const uint32_t buffer_id,
  const uint16_t out_port,
  const uint16_t flags,
  const openflow_actions *actions
);
buffer *create_port_mod( const uint32_t transaction_id, const uint16_t port_no,
                         const uint8_t hw_addr[ OFP_ETH_ALEN ], const uint32_t config,
                         const uint32_t mask, const uint32_t advertise );
buffer *create_desc_stats_request( const uint32_t transaction_id, const uint16_t flags );
buffer *create_flow_stats_request( const uint32_t transaction_id, const uint16_t flags,
                                   const struct ofp_match match, const uint8_t table_id,
                                   const uint16_t out_port );
buffer *create_aggregate_stats_request( const uint32_t transaction_id,
                                        const uint16_t flags, const struct ofp_match match,
                                        const uint8_t table_id, const uint16_t out_port );
buffer *create_table_stats_request( const uint32_t transaction_id, const uint16_t flags );
buffer *create_port_stats_request( const uint32_t transaction_id, const uint16_t flags,
                                   const uint16_t port_no );
buffer *create_queue_stats_request( const uint32_t transaction_id, const uint16_t flags,
                                    const uint16_t port_no, const uint32_t queue_id );
buffer *create_vendor_stats_request( const uint32_t transaction_id, const uint16_t flags,
                                     const uint32_t vendor, const buffer *body );
buffer *create_desc_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                                 const char mfr_desc[ DESC_STR_LEN ],
                                 const char hw_desc[ DESC_STR_LEN ],
                                 const char sw_desc[ DESC_STR_LEN ],
                                 const char serial_num[ SERIAL_NUM_LEN ],
                                 const char dp_desc[ DESC_STR_LEN ] );
buffer *create_flow_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                                 const list_element *flows_stats_head );
buffer *create_aggregate_stats_reply( const uint32_t transaction_id,
                                      const uint16_t flags,
                                      const uint64_t packet_count, const uint64_t byte_count,
                                      const uint32_t flow_count );
buffer *create_table_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                                  const list_element *table_stats_head );
buffer *create_port_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                                 const list_element *port_stats_head );
buffer *create_queue_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                                  const list_element *queue_stats_head );
buffer *create_vendor_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                                   const uint32_t vendor, const buffer *body );
buffer *create_barrier_request( const uint32_t transaction_id );
buffer *create_barrier_reply( const uint32_t transaction_id );
buffer *create_queue_get_config_request( const uint32_t transaction_id, const uint16_t port );
buffer *create_queue_get_config_reply( const uint32_t transaction_id, const uint16_t port,
                                       const list_element *queues );
uint32_t get_transaction_id( void );
uint64_t get_cookie( void );
openflow_actions *create_actions( void );
bool delete_actions( openflow_actions *actions );
bool append_action_output( openflow_actions *actions, const uint16_t port, const uint16_t max_len );
bool append_action_set_vlan_vid( openflow_actions *actions, const uint16_t vlan_vid );
bool append_action_set_vlan_pcp( openflow_actions *actions, const uint8_t vlan_pcp );
bool append_action_strip_vlan( openflow_actions *actions );
bool append_action_set_dl_src( openflow_actions *actions, const uint8_t hw_addr[ OFP_ETH_ALEN ] );
bool append_action_set_dl_dst( openflow_actions *actions, const uint8_t hw_addr[ OFP_ETH_ALEN ] );
bool append_action_set_nw_src( openflow_actions *actions, const uint32_t nw_addr );
bool append_action_set_nw_dst( openflow_actions *actions, const uint32_t nw_addr );
bool append_action_set_nw_tos( openflow_actions *actions, const uint8_t nw_tos );
bool append_action_set_tp_src( openflow_actions *actions, const uint16_t tp_port );
bool append_action_set_tp_dst( openflow_actions *actions, const uint16_t tp_port );
bool append_action_enqueue( openflow_actions *actions, const uint16_t port,
                            const uint32_t queue_id );
bool append_action_vendor( openflow_actions *actions, const uint32_t vendor,
                           const buffer *data );


// Return code definitions indicating the result of OpenFlow message validation.
enum {
  SUCCESS = 0,
  ERROR_UNSUPPORTED_VERSION = -60,
  ERROR_INVALID_LENGTH,
  ERROR_TOO_SHORT_MESSAGE,
  ERROR_TOO_LONG_MESSAGE,
  ERROR_INVALID_TYPE,
  ERROR_UNDEFINED_TYPE,
  ERROR_UNSUPPORTED_TYPE,
  ERROR_NO_TABLE_AVAILABLE,
  ERROR_INVALID_PORT_NO,
  ERROR_INVALID_PORT_CONFIG,
  ERROR_INVALID_PORT_STATE,
  ERROR_INVALID_PORT_FEATURES,
  ERROR_INVALID_SWITCH_CONFIG,
  ERROR_INVALID_PACKET_IN_REASON,
  ERROR_INVALID_FLOW_REMOVED_REASON,
  ERROR_INVALID_WILDCARDS,
  ERROR_INVALID_VLAN_VID,
  ERROR_INVALID_VLAN_PCP,
  ERROR_INVALID_NW_TOS,
  ERROR_INVALID_PORT_STATUS_REASON,
  ERROR_TOO_SHORT_QUEUE_DESCRIPTION,
  ERROR_TOO_SHORT_QUEUE_PROPERTY,
  ERROR_TOO_LONG_QUEUE_PROPERTY,
  ERROR_UNDEFINED_QUEUE_PROPERTY,
  ERROR_TOO_SHORT_ACTION,
  ERROR_UNDEFINED_ACTION_TYPE,
  ERROR_INVALID_ACTION_TYPE,
  ERROR_TOO_SHORT_ACTION_OUTPUT,
  ERROR_TOO_LONG_ACTION_OUTPUT,
  ERROR_TOO_SHORT_ACTION_VLAN_VID,
  ERROR_TOO_LONG_ACTION_VLAN_VID,
  ERROR_TOO_SHORT_ACTION_VLAN_PCP,
  ERROR_TOO_LONG_ACTION_VLAN_PCP,
  ERROR_TOO_SHORT_ACTION_STRIP_VLAN,
  ERROR_TOO_LONG_ACTION_STRIP_VLAN,
  ERROR_TOO_SHORT_ACTION_DL_SRC,
  ERROR_TOO_LONG_ACTION_DL_SRC,
  ERROR_TOO_SHORT_ACTION_DL_DST,
  ERROR_TOO_LONG_ACTION_DL_DST,
  ERROR_TOO_SHORT_ACTION_NW_SRC,
  ERROR_TOO_LONG_ACTION_NW_SRC,
  ERROR_TOO_SHORT_ACTION_NW_DST,
  ERROR_TOO_LONG_ACTION_NW_DST,
  ERROR_TOO_SHORT_ACTION_NW_TOS,
  ERROR_TOO_LONG_ACTION_NW_TOS,
  ERROR_TOO_SHORT_ACTION_TP_SRC,
  ERROR_TOO_LONG_ACTION_TP_SRC,
  ERROR_TOO_SHORT_ACTION_TP_DST,
  ERROR_TOO_LONG_ACTION_TP_DST,
  ERROR_TOO_SHORT_ACTION_ENQUEUE,
  ERROR_TOO_LONG_ACTION_ENQUEUE,
  ERROR_TOO_SHORT_ACTION_VENDOR,
  ERROR_INVALID_LENGTH_ACTION_VENDOR,
  ERROR_UNSUPPORTED_STATS_TYPE,
  ERROR_INVALID_STATS_REPLY_FLAGS,
  ERROR_INVALID_FLOW_PRIORITY,
  ERROR_INVALID_FLOW_MOD_FLAGS,
  ERROR_INVALID_PORT_MASK,
  ERROR_INVALID_STATS_TYPE,
  ERROR_INVALID_STATS_REQUEST_FLAGS,
  ERROR_UNDEFINED_FLOW_MOD_COMMAND,
  ERROR_UNEXPECTED_ERROR = -255
};


// Functions for validating OpenFlow messages
int validate_hello( const buffer *message );
int validate_error( const buffer *message );
int validate_echo_request( const buffer *message );
int validate_echo_reply( const buffer *message );
int validate_vendor( const buffer *message );
int validate_features_request( const buffer *message );
int validate_features_reply( const buffer *message );
int validate_get_config_request( const buffer *message );
int validate_get_config_reply( const buffer *message );
int validate_set_config( const buffer *message );
int validate_packet_in( const buffer *message );
int validate_flow_removed( const buffer *message );
int validate_port_status( const buffer *message );
int validate_packet_out( const buffer *message );
int validate_flow_mod( const buffer *message );
int validate_port_mod( const buffer *message );
int validate_desc_stats_request( const buffer *message );
int validate_flow_stats_request( const buffer *message );
int validate_aggregate_stats_request( const buffer *message );
int validate_table_stats_request( const buffer *message );
int validate_port_stats_request( const buffer *message );
int validate_queue_stats_request( const buffer *message );
int validate_vendor_stats_request( const buffer *message );
int validate_stats_request( const buffer *message );
int validate_desc_stats_reply( const buffer *message );
int validate_flow_stats_reply( const buffer *message );
int validate_aggregate_stats_reply( const buffer *message );
int validate_table_stats_reply( const buffer *message );
int validate_port_stats_reply( const buffer *message );
int validate_queue_stats_reply( const buffer *message );
int validate_vendor_stats_reply( const buffer *message );
int validate_stats_reply( const buffer *message );
int validate_barrier_request( const buffer *message );
int validate_barrier_reply( const buffer *message );
int validate_queue_get_config_request( const buffer *message );
int validate_queue_get_config_reply( const buffer *message );
int validate_actions( struct ofp_action_header *actions_head, const uint16_t length );
int validate_action_output( const struct ofp_action_output *action );
int validate_action_set_vlan_vid( const struct ofp_action_vlan_vid *action );
int validate_action_set_vlan_pcp( const struct ofp_action_vlan_pcp *action );
int validate_action_strip_vlan( const struct ofp_action_header *action );
int validate_action_set_dl_src( const struct ofp_action_dl_addr *action );
int validate_action_set_dl_dst( const struct ofp_action_dl_addr *action );
int validate_action_set_nw_src( const struct ofp_action_nw_addr *action );
int validate_action_set_nw_dst( const struct ofp_action_nw_addr *action );
int validate_action_set_nw_tos( const struct ofp_action_nw_tos *action );
int validate_action_set_tp_src( const struct ofp_action_tp_port *action );
int validate_action_set_tp_dst( const struct ofp_action_tp_port *action );
int validate_action_enqueue( const struct ofp_action_enqueue *action );
int validate_action_vendor( const struct ofp_action_vendor_header *action );
int validate_openflow_message( const buffer *message );
bool valid_openflow_message( const buffer *message );

// Utility functions
bool get_error_type_and_code( const uint8_t type, const int error_no,
                              uint16_t *error_type, uint16_t *error_code );
void set_match_from_packet( struct ofp_match *match, const uint16_t in_port,
                            const uint32_t wildcards, const buffer *packet );
void normalize_match( struct ofp_match *match );


#endif // OPENFLOW_MESSAGE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
