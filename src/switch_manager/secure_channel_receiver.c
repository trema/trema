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
#include "trema.h"
#include "message_queue.h"
#include "ofpmsg_recv.h"
#include "ofpmsg_send.h"
#include "secure_channel_receiver.h"


static const size_t RECEIVE_BUFFFER_SIZE = UINT16_MAX + sizeof( struct ofp_packet_in ) - 2;


int
recv_from_secure_channel( struct switch_info *sw_info ) {
  assert( sw_info != NULL );
  assert( sw_info->recv_queue != NULL );

  // all queued messages should be processed before receiving new messages from remote
  if ( sw_info->recv_queue->length > 0 ) {
    return 0;
  }

  if ( sw_info->fragment_buf == NULL ) {
    sw_info->fragment_buf = alloc_buffer_with_length( RECEIVE_BUFFFER_SIZE );
  }

  size_t remaining_length = RECEIVE_BUFFFER_SIZE - sw_info->fragment_buf->length;
  char *recv_buf = ( char * ) sw_info->fragment_buf->data + sw_info->fragment_buf->length;
  ssize_t recv_length = read( sw_info->secure_channel_fd, recv_buf, remaining_length );
  if ( recv_length < 0 ) {
    if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
      return 0;
    }
    error( "Receive error:%s(%d)", strerror( errno ), errno );
    return -1;
  }
  if ( recv_length == 0 ) {
    debug( "Connection closed by peer." );
    return -1;
  }
  sw_info->fragment_buf->length += ( size_t ) recv_length;

  size_t read_total = 0;
  while ( sw_info->fragment_buf->length >= sizeof( struct ofp_header ) ) {
    struct ofp_header *header = sw_info->fragment_buf->data;
    if ( ! valid_message_version( header->type, header->version ) ) {
      error( "Receive error: invalid version (version %d)", header->version );
      ofpmsg_send_error_msg( sw_info,
                             OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION, sw_info->fragment_buf );
      return -1;
    }
    uint16_t message_length = ntohs( header->length );
    if ( message_length > sw_info->fragment_buf->length ) {
      break;
    }
    buffer *message = alloc_buffer_with_length( message_length );
    char *p = append_back_buffer( message, message_length );
    memcpy( p, sw_info->fragment_buf->data, message_length );
    remove_front_buffer( sw_info->fragment_buf, message_length );
    enqueue_message( sw_info->recv_queue, message );
    read_total += message_length;
  }

  // remove headroom manually for next call
  if ( read_total > 0 ) {
    memmove( ( char * ) sw_info->fragment_buf->data - read_total,
             sw_info->fragment_buf->data,
             sw_info->fragment_buf->length );
    sw_info->fragment_buf->data = ( char * ) sw_info->fragment_buf->data - read_total;
  }

  return 0;
}


int
handle_messages_from_secure_channel( struct switch_info *sw_info ) {
  assert( sw_info != NULL );
  assert( sw_info->recv_queue != NULL );

  int ret;
  int errors = 0;
  buffer *message;

  while ( ( message = dequeue_message( sw_info->recv_queue ) ) != NULL ) {
    ret = ofpmsg_recv( sw_info, message );
    if ( ret < 0 ) {
      error( "Failed to handle message to application." );
      errors++;
    }
  }

  return errors == 0 ? 0 : -1;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
