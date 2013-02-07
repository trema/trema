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


#ifndef PACKETIN_FILTER_INTERFACE_H
#define PACKETIN_FILTER_INTERFACE_H


#include <inttypes.h>
#include <openflow.h>
#include "bool.h"
#include "messenger.h"


#define PACKETIN_FILTER_MANAGEMENT_SERVICE "packetin_filter_management"


enum {
  MESSENGER_ADD_PACKETIN_FILTER_REQUEST = 0x0010,
  MESSENGER_ADD_PACKETIN_FILTER_REPLY,
  MESSENGER_DELETE_PACKETIN_FILTER_REQUEST,
  MESSENGER_DELETE_PACKETIN_FILTER_REPLY,
  MESSENGER_DUMP_PACKETIN_FILTER_REQUEST,
  MESSENGER_DUMP_PACKETIN_FILTER_REPLY,
};

enum {
  PACKETIN_FILTER_OPERATION_SUCCEEDED,
  PACKETIN_FILTER_OPERATION_FAILED,
};

enum {
  PACKETIN_FILTER_FLAG_MATCH_LOOSE = 0x00,
  PACKETIN_FILTER_FLAG_MATCH_STRICT = 0x01,
};


typedef struct {
  struct ofp_match match;
  uint16_t priority;
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
} __attribute__( ( packed ) ) packetin_filter_entry;

typedef struct {
  packetin_filter_entry entry;
} __attribute__( ( packed ) ) add_packetin_filter_request;

typedef struct {
  uint8_t status;
} __attribute__( ( packed ) ) add_packetin_filter_reply;

typedef struct {
  packetin_filter_entry criteria;
  uint8_t flags;
} __attribute__( ( packed ) ) delete_packetin_filter_request;

typedef struct {
  uint8_t status;
  uint32_t n_deleted;
} __attribute__( ( packed ) ) delete_packetin_filter_reply;

typedef struct {
  packetin_filter_entry criteria;
  uint8_t flags;
} __attribute__( ( packed ) ) dump_packetin_filter_request;

typedef struct {
  uint8_t status;
  uint32_t n_entries;
  packetin_filter_entry entries[ 0 ];
} __attribute__( ( packed ) ) dump_packetin_filter_reply;


typedef void ( *add_packetin_filter_handler )(
  int status,
  void *user_data
);

typedef void ( *delete_packetin_filter_handler )(
  int status,
  int n_deleted,
  void *user_data
);

typedef void ( *dump_packetin_filter_handler )(
  int status,
  int n_entries,
  packetin_filter_entry *entries,
  void *user_data
);


bool add_packetin_filter( struct ofp_match match, uint16_t priority, char *service_name,
                          add_packetin_filter_handler callback, void *user_data );
bool delete_packetin_filter( struct ofp_match match, uint16_t priority, char *service_name, bool strict,
                             delete_packetin_filter_handler callback, void *user_data );
bool dump_packetin_filter( struct ofp_match match, uint16_t priority, char *service_name, bool strict,
                           dump_packetin_filter_handler callback, void *user_data );
bool init_packetin_filter_interface( void );
bool finalize_packetin_filter_interface( void );


#endif // PACKETIN_FILTER_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
