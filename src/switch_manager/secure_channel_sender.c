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


#include <assert.h>
#include <errno.h>
#include <limits.h>
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


typedef struct {
  struct iovec *iov;
  int iovcnt;
} writev_args;


static bool
append_to_writev_args( buffer *message, void *user_data ) {
  writev_args *args = user_data;

  args->iov[ args->iovcnt ].iov_base = message->data;
  args->iov[ args->iovcnt ].iov_len = message->length;
  args->iovcnt++;

  if ( args->iovcnt >= IOV_MAX ) {
    return false;
  }

  return true;
}


int
flush_secure_channel( struct switch_info *sw_info ) {
  assert( sw_info != NULL );
  assert( sw_info->send_queue != NULL );
  assert( sw_info->secure_channel_fd >= 0 );

  buffer *buf;
  ssize_t write_length;

  if ( sw_info->send_queue->length == 0 ) {
    return 0;
  }
  set_writable( sw_info->secure_channel_fd, false );
  writev_args args;
  args.iov = xmalloc( sizeof( struct iovec ) * ( size_t ) sw_info->send_queue->length );
  args.iovcnt = 0;
  foreach_message_queue( sw_info->send_queue, append_to_writev_args, &args );
  if ( args.iovcnt == 0 ) {
    xfree( args.iov );
    return 0;
  }
  write_length = writev( sw_info->secure_channel_fd, args.iov, args.iovcnt );
  xfree( args.iov );
  if ( write_length < 0 ) {
    if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
      set_writable( sw_info->secure_channel_fd, true );
      return 0;
    }
    error( "Failed to send a message to secure channel ( errno = %s [%d] ).",
           strerror( errno ), errno );
    return -1;
  }
  if ( write_length == 0 ) {
    return 0;
  }
  while ( ( buf = peek_message( sw_info->send_queue ) ) != NULL ) {
    if ( write_length == 0 ) {
      set_writable( sw_info->secure_channel_fd, true );
      return 0;
    }
    if ( ( size_t ) write_length < buf->length ) {
      remove_front_buffer( buf, ( size_t ) write_length );
      set_writable( sw_info->secure_channel_fd, true );
      return 0;
    }
    write_length -= ( ssize_t ) buf->length;
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
