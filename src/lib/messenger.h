/*
 * Trema messenger library.
 *
 * Author: Toshio Koide
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

/**
* @file
*
* @brief Implementation of messaging in OpenFlow
*
*/

#ifndef MESSENGER_H
#define MESSENGER_H


#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "checks.h"
#include "bool.h"


#define MESSENGER_SERVICE_NAME_LENGTH 32


typedef struct messenger_context_handle {
  uint32_t transaction_id;
  uint16_t service_name_len;
  uint16_t pad;
  char service_name[ 0 ];
} messenger_context_handle;


/* message dump format:
 * +-------------------+--------+------------+----+
 * |message_dump_header|app_name|service_name|data|
 * +-------------------+--------+------------+----+
 */

typedef struct message_dump_header {
  struct {   // same as struct timespec but fixed length
    uint32_t sec;
    uint32_t nsec;
  } sent_time;
  uint16_t app_name_length;
  uint16_t service_name_length;
  uint32_t data_length;
} message_dump_header;

enum {
  MESSENGER_DUMP_SENT,
  MESSENGER_DUMP_RECEIVED,
  MESSENGER_DUMP_RECV_CONNECTED,
  MESSENGER_DUMP_RECV_OVERFLOW,
  MESSENGER_DUMP_RECV_CLOSED,
  MESSENGER_DUMP_SEND_CONNECTED,
  MESSENGER_DUMP_SEND_REFUSED,
  MESSENGER_DUMP_SEND_OVERFLOW,
  MESSENGER_DUMP_SEND_CLOSED,
};


typedef void ( *callback_message_received )( uint16_t tag, void *data, size_t len );


bool init_messenger( const char *working_directory );
bool add_message_received_callback( const char *service_name, const callback_message_received function );
bool add_message_requested_callback( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
bool add_message_replied_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) );
bool add_timer_event_callback( struct itimerspec *interval, void ( *callback )( void *user_data ), void *user_data );
bool add_periodic_event_callback( const time_t seconds, void ( *callback )( void *user_data ), void *user_data );
bool delete_message_received_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len ) );
bool delete_message_requested_callback( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
bool delete_message_replied_callback( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) );
bool delete_timer_event_callback( void ( *callback )( void *user_data ) );
bool delete_periodic_event_callback( void ( *callback )( void *user_data ) );
bool rename_message_received_callback( const char *old_service_name, const char *new_service_name );
bool send_message( const char *service_name, const uint16_t tag, const void *data, size_t len );
bool send_request_message( const char *to_service_name, const char *from_service_name, const uint16_t tag, const void *data, size_t len, void *user_data );
bool send_reply_message( const messenger_context_handle *handle, const uint16_t tag, const void *data, size_t len );
int flush_messenger( void );
bool start_messenger( void );
bool stop_messenger( void );
bool finalize_messenger( void );
void start_messenger_dump( const char *dump_app_name, const char *dump_service_name );
void stop_messenger_dump( void );
bool messenger_dump_enabled( void );
void set_fd_set_callback( void ( *callback )( fd_set *read_set, fd_set *write_set ) );
void set_check_fd_isset_callback( void ( *callback )( fd_set *read_set, fd_set *write_set ) );
bool set_external_callback( void ( *callback ) ( void ) );


#endif // MESSENGER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
