/*
 * Management service interface.
 *
 * Author: Yasunobu Chiba
 *
 * Copyright (C) 2012 NEC Corporation
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
  MANAGEMENT_APPLICATION_REQUEST,
  MANAGEMENT_APPLICATION_REPLY,
};


typedef struct {
  uint16_t type;
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
  uint16_t type;
  uint8_t status;
} __attribute__( ( packed ) ) management_reply_header;

typedef struct {
  management_reply_header header;
  timespec_n sent_at;
  timespec_n received_at;
} __attribute__( ( packed ) ) management_echo_reply;

typedef struct {
  management_reply_header header;
} __attribute__( ( packed ) ) management_set_logging_level_reply;


extern const char *( *get_management_service_name )( const char *service_name );


#endif // MANAGEMENT_SERVICE_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
