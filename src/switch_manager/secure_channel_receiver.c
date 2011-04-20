/*
 * Author: Kazusi Sugyo
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


#include <assert.h>
#include <errno.h>
#include <openflow.h>
#include <string.h>
#include <unistd.h>
#include "trema.h"
#include "ofpmsg_recv.h"
#include "ofpmsg_send.h"
#include "secure_channel_receiver.h"


#define MBUFALLOC_USERHEADROOM_LEN 128


int
recv_from_secure_channel( struct switch_info *sw_info ) {
  int ret;
  ssize_t recv_len;
  char *recv_data;
  size_t recv_remain;
  uint16_t ofp_length;
  struct ofp_header *ofp_header;

  if ( sw_info->fragment_buf == NULL ) {
    sw_info->fragment_buf = alloc_buffer_with_length( MBUFALLOC_USERHEADROOM_LEN + sizeof( struct ofp_header ) );
    assert( sw_info->fragment_buf != NULL );
  }
  ofp_header = ( struct ofp_header * ) sw_info->fragment_buf->data;
  if ( sw_info->fragment_buf->length < sizeof( struct ofp_header ) ) {
    ofp_length = sizeof( struct ofp_header );
  }
  else {
    ofp_length = ntohs( ofp_header->length );
  }

  while ( ( recv_remain = ofp_length - sw_info->fragment_buf->length ) > 0 ) {
    recv_data = ( char * ) sw_info->fragment_buf->data + sw_info->fragment_buf->length;
    recv_len = read( sw_info->secure_channel_fd, recv_data, recv_remain );
    if ( recv_len < 0 ) {
        if ( ( errno == EINTR ) || ( errno == EAGAIN ) ) {
          // TODO: start fragmentation timer

        return 0;
      }
      error( "Receive error: %s(%d)", strerror( errno ), errno );
      goto closed;
    }
    if ( recv_len == 0 ) {
      error( "Receive error: peer shutdown" );
      goto closed;
    }
    sw_info->fragment_buf->length += ( uint ) recv_len;
    if ( sw_info->fragment_buf->length == sizeof( struct ofp_header ) ) {
      if ( ofp_header->version != OFP_VERSION ) {
        error( "Receive error: invalid version (version %d)", ofp_header->version );
        ofpmsg_send_error_msg( sw_info,
          OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION, sw_info->fragment_buf );

        goto closed;
      }
      ofp_length = ntohs( ofp_header->length );
      if ( ofp_length < sizeof( struct ofp_header ) ) {
        error( "Receive error: invalid length (length %d)", ofp_length );
        ofpmsg_send_error_msg( sw_info,
          OFPET_BAD_REQUEST, OFPBRC_BAD_LEN, sw_info->fragment_buf );

        goto closed;
      }

      if ( ofp_length > sizeof( struct ofp_header ) ) {
        append_back_buffer( sw_info->fragment_buf, ofp_length - sizeof( ofp_header ) );
        sw_info->fragment_buf->length = sizeof( struct ofp_header );
      }
    }
  }


  ret = ofpmsg_recv( sw_info, sw_info->fragment_buf );
  if ( ret < 0 ) {
    error( "Failed to handle message to application." );
    sw_info->fragment_buf = NULL;
    goto closed;
  }
  sw_info->fragment_buf = NULL;

  return 0;

closed:
  if ( sw_info->fragment_buf != NULL ) {
    free_buffer( sw_info->fragment_buf );
    sw_info->fragment_buf = NULL;
  }

  close( sw_info->secure_channel_fd );
  sw_info->secure_channel_fd = -1;

  return -1;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
