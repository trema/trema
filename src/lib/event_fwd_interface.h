/*
 * Copyright (C) 2013 NEC Corporation
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

#ifndef EVENT_FWD_INTERFACE_H_
#define EVENT_FWD_INTERFACE_H_


#include "buffer.h"
#include "linked_list.h"

// management interface application_ids
enum switch_management_command {
  DUMP_XID_TABLE = 0,   // switch only
  DUMP_COOKIE_TABLE,    // switch only
  TOGGLE_COOKIE_AGING,  // switch only
  EVENT_FWD_ENTRY_ADD,
  EVENT_FWD_ENTRY_DELETE,
  EVENT_FWD_ENTRY_DUMP,
  EVENT_FWD_ENTRY_SET,
  EFI_GET_SWLIST,       // switch manager only
};

enum efi_event_type {
  EVENT_FWD_TYPE_VENDOR = 0,
  EVENT_FWD_TYPE_PACKET_IN,
  EVENT_FWD_TYPE_PORT_STATUS,
  EVENT_FWD_TYPE_STATE_NOTIFY
};

enum efi_result {
  EFI_OPERATION_SUCCEEDED = 0,
  EFI_OPERATION_FAILED,
};

// messenger message data structure
typedef struct {
  uint8_t type;  // enum efi_event_type
  uint8_t pad[3];
  uint32_t n_services;  // Number of element in service_list. 0 for dump, 1 for add/delete, N for set operation.
  char service_list[0];  // e.g. "service1\0service2\0"
} __attribute__( ( packed ) ) event_fwd_op_request;

typedef struct {
  uint8_t type;  // enum efi_event_type
  uint8_t result;    // enum efi_result
  uint8_t pad[2];
  uint32_t n_services;
  char service_list[0];  // service list after manipulation
} __attribute__( ( packed ) ) event_fwd_op_reply;


// struct for user API
typedef struct {
  enum efi_result result;
  uint32_t n_services;
  const char **services;  // service list after manipulation.
} event_fwd_op_result;


// High level API callback function signature
typedef void ( *event_fwd_entry_to_all_callback )(
    enum efi_result result,
    void *user_data
);


// Low level API callback function signature
typedef void ( *event_fwd_entry_operation_callback )(
    event_fwd_op_result result,
    void *user_data
);


typedef void ( *switch_list_request_callback )(
    uint64_t* dpids, // NULL if failed
    size_t n_dpids,
    void *user_data
);


// setup/teardown messenger channel
bool init_event_fwd_interface( void );
bool finalize_event_fwd_interface( void );


// High level API (calls Low level API for switch manager and existing switch daemons)
bool add_event_fwd_to_all_switches( enum efi_event_type type, const char* service_name, event_fwd_entry_to_all_callback callback, void* user_data );
bool delete_event_fwd_to_all_switches( enum efi_event_type type, const char* service_name, event_fwd_entry_to_all_callback callback, void* user_data );


// Low level API for switch manager
bool set_switch_manager_event_fwd_entries( enum efi_event_type type, list_element* service_list, event_fwd_entry_operation_callback callback, void* user_data );
bool add_switch_manager_event_fwd_entry( enum efi_event_type type, const char* service_name, event_fwd_entry_operation_callback callback, void* user_data );
bool delete_switch_manager_event_fwd_entry( enum efi_event_type type, const char* service_name, event_fwd_entry_operation_callback callback, void* user_data );
bool dump_switch_manager_event_fwd_entries( enum efi_event_type type, event_fwd_entry_operation_callback callback, void* user_data );


// Low level API for switch daemon
bool set_switch_event_fwd_entries( uint64_t dpid, enum efi_event_type type, list_element* service_list, event_fwd_entry_operation_callback callback, void* user_data );
bool add_switch_event_fwd_entry( uint64_t dpid, enum efi_event_type type, const char* service_name, event_fwd_entry_operation_callback callback, void* user_data );
bool delete_switch_event_fwd_entry( uint64_t dpid, enum efi_event_type type, const char* service_name, event_fwd_entry_operation_callback callback, void* user_data );
bool dump_switch_event_fwd_entries( uint64_t dpid, enum efi_event_type type, event_fwd_entry_operation_callback callback, void* user_data );


// internal utilities
buffer* create_event_fwd_op_request( buffer* buf, enum efi_event_type, list_element* service_list );
buffer* create_event_fwd_op_reply( enum efi_event_type, enum efi_result, list_element* service_list );

bool send_efi_switch_list_request( switch_list_request_callback callback, void* user_data );

#endif /* EVENT_FWD_INTERFACE_H_ */
