/*
 * Author: Kazushi SUGYO
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


#include <assert.h>
#include <errno.h>
#include <openflow.h>
#include <string.h>
#include <unistd.h>
#include "event_handler.h"
#include "message_queue.h"
#include "ofpmsg_send.h"
#include "secure_channel_sender.h"
#include "trema.h"


int
send_to_secure_channel( struct switch_info *sw_info, buffer *buf ) {
  assert( sw_info != NULL );
  assert( buf != NULL );
  assert( buf->length > 0 );

  if ( sw_info->send_queue == NULL ) {
    return -1;
  }

  bool res = enqueue_message( sw_info->send_queue, buf );
  if ( res ) {
    set_writable( sw_info->secure_channel_fd, true );
  }
  return res ? 0 : -1;
}


int
flush_secure_channel( struct switch_info *sw_info ) {
  assert( sw_info != NULL );
  assert( sw_info->send_queue != NULL );
  assert( sw_info->secure_channel_fd >= 0 );

  buffer *buf;
  ssize_t write_length;

  set_writable( sw_info->secure_channel_fd, false );
  while ( ( buf = peek_message( sw_info->send_queue ) ) != NULL ) {
    write_length = write( sw_info->secure_channel_fd, buf->data, buf->length );
    if ( write_length < 0 ) {
      if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
        set_writable( sw_info->secure_channel_fd, true );
        return 0;
      }
      error( "Failed to send a message to secure channel ( errno = %s [%d] ).",
             strerror( errno ), errno );
      return -1;
    }
    if ( ( size_t ) write_length < buf->length ) {
      remove_front_buffer( buf, ( size_t ) write_length );
      set_writable( sw_info->secure_channel_fd, true );
      return 0;
    }
    buf = dequeue_message( sw_info->send_queue );
    free_buffer( buf );
  }

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
