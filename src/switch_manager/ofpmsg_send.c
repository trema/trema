/*
 * OpenFlow Switch Manager
 *
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


#include <inttypes.h>
#include <openflow.h>
#include <stdio.h>
#include <string.h>
#include "cookie_table.h"
#include "ofpmsg_send.h"
#include "secure_channel_sender.h"
#include "switch.h"
#include "trema.h"
#include "xid_table.h"


int
ofpmsg_send_hello( struct switch_info *sw_info ) {
  int ret;
  buffer *buf;

  buf = create_hello( generate_xid() );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'hello' to a switch. fd:%d", sw_info->secure_channel_fd );
  }

  return ret;
}


int
ofpmsg_send_echorequest( struct switch_info *sw_info, uint32_t xid, buffer *body ) {
  int ret;
  buffer *buf;

  buf = create_echo_request( xid, body );
  free_buffer( body );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'echo request' to a switch %#" PRIx64 ".", sw_info->datapath_id );
  }

  return ret;
}


int
ofpmsg_send_echoreply( struct switch_info *sw_info, uint32_t xid, buffer *body ) {
  int ret;
  buffer *buf;

  buf = create_echo_reply( xid, body );
  free_buffer( body );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'echo reply' to a switch %#" PRIx64 ".", sw_info->datapath_id );
  }

  return ret;
}


int
ofpmsg_send_featuresrequest( struct switch_info *sw_info ) {
  int ret;
  buffer *buf;

  buf = create_features_request( generate_xid() );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'features request' to a switch. fd:%d", sw_info->secure_channel_fd );
  }

  return ret;
}


int
ofpmsg_send_setconfig( struct switch_info *sw_info ) {
  int ret;
  buffer *buf;

  buf = create_set_config( generate_xid(), sw_info->config_flags,
                           sw_info->miss_send_len );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'set config request' to a switch %#" PRIx64 ".", sw_info->datapath_id );
  }

  return ret;
}


int
ofpmsg_send_error_msg( struct switch_info *sw_info, uint16_t type, uint16_t code, buffer *data ) {
  int ret;
  buffer *buf;

  if ( data->length > OFP_ERROR_MSG_MAX_DATA ) {
    // FIXME
    data->length = OFP_ERROR_MSG_MAX_DATA;
  }

  buf = create_error( generate_xid(), type, code, data );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'error' to a switch %#" PRIx64 ".", sw_info->datapath_id );
  }

  return ret;
}


static int
update_flowmod_cookie( buffer *buf, char *service_name ) {
  struct ofp_flow_mod *flow_mod = buf->data;
  uint16_t command = ntohs( flow_mod->command );
  uint16_t flags = ntohs( flow_mod->flags );
  uint64_t cookie = ntohll( flow_mod->cookie );

  switch ( command ) {
  case OFPFC_ADD:
  {
    uint64_t *new_cookie = insert_cookie_entry( &cookie, service_name, flags );
    if ( new_cookie == NULL ) {
      return -1;
    }
    flow_mod->cookie = htonll( *new_cookie );
    flow_mod->flags = htons( flags | OFPFF_SEND_FLOW_REM );
  }
  break;

  case OFPFC_MODIFY:
  case OFPFC_MODIFY_STRICT:
  {
    cookie_entry_t *entry = lookup_cookie_entry_by_application( &cookie, service_name );
    if ( entry != NULL ) {
      flow_mod->cookie = htonll( entry->cookie );
    }
    else {
      uint64_t *new_cookie = insert_cookie_entry( &cookie, service_name, flags );
      if ( new_cookie == NULL ) {
        return -1;
      }
      flow_mod->cookie = htonll( *new_cookie );
    }
    flow_mod->flags = htons( flags | OFPFF_SEND_FLOW_REM );
  }
  break;

  case OFPFC_DELETE:
  case OFPFC_DELETE_STRICT:
  {
    cookie_entry_t *entry = lookup_cookie_entry_by_application( &cookie, service_name );
    if ( entry != NULL ) {
      flow_mod->cookie = htonll( entry->cookie );
    }
    else {
      flow_mod->cookie = htonll( RESERVED_COOKIE );
    }
    flow_mod->flags = htons( flags | OFPFF_SEND_FLOW_REM );
  }
  break;

  default:
    return -1;
  }

  return 0;
}


int
ofpmsg_send( struct switch_info *sw_info, buffer *buf, char *service_name ) {
  int ret;
  struct ofp_header *ofp_header;
  uint32_t new_xid;

  ofp_header = buf->data;

  new_xid = insert_xid_entry( ntohl( ofp_header->xid ), service_name );
  ofp_header->xid = htonl( new_xid );

  if ( ofp_header->type == OFPT_FLOW_MOD && sw_info->cookie_translation ) {
    ret = update_flowmod_cookie( buf, service_name );
    if ( ret < 0 ) {
      error( "Failed to update cookie value ( ret = %d ).", ret );
      free_buffer( buf );
      return ret;
    }
  }

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send an OpenFlow message %d to a switch %#" PRIx64 ".",
      ofp_header->type, sw_info->datapath_id );
  }

  return ret;
}


int
ofpmsg_send_delete_all_flows( struct switch_info *sw_info ) {
  int ret;
  struct ofp_match match;
  buffer *buf;

  memset( &match, 0, sizeof( match ) );
  match.wildcards = OFPFW_ALL;

  buf = create_flow_mod( generate_xid(), match, RESERVED_COOKIE,
                         OFPFC_DELETE, 0, 0, 0, UINT32_MAX, OFPP_NONE, 0, NULL );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'flow mod (delete all)' to a switch %#" PRIx64 ".", sw_info->datapath_id );
  }

  return ret;
}


int
ofpmsg_send_deny_all( struct switch_info *sw_info ) {
  int ret;
  struct ofp_match match;
  buffer *buf;

  memset( &match, 0, sizeof( match ) );
  match.wildcards = OFPFW_ALL;
  const uint16_t timeout = 10;

  buf = create_flow_mod( generate_xid(), match, RESERVED_COOKIE,
                         OFPFC_ADD, 0, timeout, UINT16_MAX, UINT32_MAX, OFPP_NONE, 0, NULL );

  ret = send_to_secure_channel( sw_info, buf );
  if ( ret == 0 ) {
    debug( "Send 'flow mod (deny all)' to a switch %#" PRIx64 ".", sw_info->datapath_id );
  }

  return ret;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
