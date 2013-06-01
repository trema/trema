/*
 * Management service interface.
 *
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


#ifndef MANAGEMENT_SERVICE_INTERFACE_H
#define MANAGEMENT_SERVICE_INTERFACE_H


#include <inttypes.h>
#include <time.h>
#include "messenger.h"
#include "stat.h"


enum {
  MESSENGER_MANAGEMENT_REQUEST = 0x0020,
  MESSENGER_MANAGEMENT_REPLY,
};

enum {
  MANAGEMENT_REQUEST_SUCCEEDED,
  MANAGEMENT_REQUEST_FAILED,
};

enum {
  MANAGEMENT_ECHO_REQUEST,
  MANAGEMENT_ECHO_REPLY,
  MANAGEMENT_SET_LOGGING_LEVEL_REQUEST,
  MANAGEMENT_SET_LOGGING_LEVEL_REPLY,
  MANAGEMENT_SHOW_STATS_REQUEST,
  MANAGEMENT_SHOW_STATS_REPLY,
  MANAGEMENT_APPLICATION_REQUEST,
  MANAGEMENT_APPLICATION_REPLY,
};


typedef struct {
  uint16_t type;
  uint32_t length;
} __attribute__( ( packed ) ) management_request_header;

typedef struct {
  uint32_t tv_sec;
  uint32_t tv_nsec;
} timespec_n;

typedef struct {
  management_request_header header;
  timespec_n sent_at;
} __attribute__( ( packed ) ) management_echo_request;

typedef struct {
  management_request_header header;
  char level[ LOGGING_LEVEL_STR_LENGTH ];
} __attribute__( ( packed ) ) management_set_logging_level_request;

typedef struct {
  management_request_header header;
} __attribute__( ( packed ) ) management_show_stats_request;

typedef struct {
  management_request_header header;
  uint32_t application_id;
  uint8_t data[ 0 ];
} __attribute__( ( packed ) ) management_application_request;

typedef struct {
  uint16_t type;
  uint8_t status;
  uint8_t flags;
  uint32_t length;
} __attribute__( ( packed ) ) management_reply_header;

typedef struct {
  management_reply_header header;
  timespec_n sent_at;
  timespec_n received_at;
} __attribute__( ( packed ) ) management_echo_reply;

typedef struct {
  management_reply_header header;
} __attribute__( ( packed ) ) management_set_logging_level_reply;

typedef struct {
  management_reply_header header;
  stat_entry entries[ 0 ];
} __attribute__( ( packed ) ) management_show_stats_reply;

typedef struct {
  management_reply_header header;
  uint32_t application_id;
  uint8_t data[ 0 ];
} __attribute__( ( packed ) ) management_application_reply;

typedef void ( *management_application_request_handler )(
  const messenger_context_handle *handle,
  uint32_t application_id,
  void *data,
  size_t data_length,
  void *user_data
);


extern const char *( *get_management_service_name )( const char *service_name );
extern void ( *set_management_application_request_handler )( management_application_request_handler callback, void *user_data );
extern management_application_reply *( *create_management_application_reply )( uint8_t status, uint32_t application_id, void *data, size_t data_length );
extern bool ( *send_management_application_reply )( const messenger_context_handle *handle, const management_application_reply *reply );


#endif // MANAGEMENT_SERVICE_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
