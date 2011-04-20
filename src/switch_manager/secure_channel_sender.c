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


#include <errno.h>
#include <openflow.h>
#include <string.h>
#include <unistd.h>
#include "ofpmsg_send.h"
#include "secure_channel_sender.h"
#include "trema.h"


int
send_to_secure_channel( struct switch_info *sw_info, buffer *buf ) {
  int ret;

retry:
  ret = write( sw_info->secure_channel_fd, buf->data, buf->length );
  if ( ret < 0 ) {
    if ( errno == EINTR ) {
      goto retry;
    }
    free_buffer( buf );
    error( "Failed to send. %s", strerror( errno ) );

    return -1;
  }
  free_buffer( buf );

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
