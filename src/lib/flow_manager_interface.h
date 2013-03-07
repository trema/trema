/*
 * Author: Yasunobu Chiba
 *
 * Copyright (C) 2011 NEC Corporation
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


#ifndef FLOW_MANAGER_INTERFACE_H
#define FLOW_MANAGER_INTERFACE_H


#include <inttypes.h>
#include "trema.h"


#define FLOW_MANAGEMENT_SERVICE "fm_service"


enum {
  MESSENGER_FLOW_ENTRY_GROUP_SETUP_REQUEST = 0x9000,
  MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
  MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REQUEST,
  MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
  MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN,
};

enum {
  SUCCEEDED,
  CONFLICTED_ID,
  CONFLICTED_ENTRY,
  SWITCH_ERROR,
  NO_ID_FOUND,
  UNDEFINED_ERROR = 255,
};

typedef struct {
  uint64_t datapath_id;
  struct ofp_match match;
  uint16_t priority;
  uint16_t idle_timeout;
  uint16_t hard_timeout;
  uint16_t actions_length;
  struct ofp_action_header actions[ 0 ];
} __attribute__( ( packed ) ) flow_entry;

typedef struct {
  uint64_t id;
  char owner[ MESSENGER_SERVICE_NAME_LENGTH ];
  uint16_t n_entries;
  uint32_t entries_length;
  flow_entry entries[ 0 ];
} __attribute__( ( packed ) ) flow_entry_group_setup_request;

typedef struct {
  uint64_t id;
  uint8_t status;
} __attribute__( ( packed ) ) flow_entry_group_setup_reply;

typedef struct {
  uint64_t id;
} __attribute__( ( packed ) ) flow_entry_group_teardown_request;

typedef struct {
  uint64_t id;
  uint8_t status;
} __attribute__( ( packed ) ) flow_entry_group_teardown_reply;

enum {
  TIMEOUT,
  MANUALLY_REQUESTED,
};

typedef struct {
  uint64_t id;
  uint8_t reason;
} __attribute__( ( packed ) ) flow_entry_group_teardown;


buffer *create_flow_entry( uint64_t datapath_id, struct ofp_match match,
                           uint16_t priority, uint16_t idle_timeout,
                           uint16_t hard_timeout, openflow_actions *actions );

uint64_t get_flow_entry_group_id( void );

buffer *create_flow_entry_group_setup_request( uint64_t id, const char *owner,
                                               uint16_t n_entries, buffer *entries );
buffer *create_flow_entry_group_setup_reply( uint64_t id, uint8_t status );
buffer *create_flow_entry_group_teardown_request( uint64_t id );
buffer *create_flow_entry_group_teardown_reply( uint64_t id, uint8_t status );
buffer *create_flow_entry_group_teardown( uint64_t id, uint8_t reason );


#endif // FLOW_MANAGER_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
