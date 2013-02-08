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
#include <inttypes.h>
#include <openflow.h>
#include "openflow_message.h"
#include "cookie_table.h"
#include "ofpmsg_recv.h"
#include "ofpmsg_send.h"
#include "service_interface.h"
#include "switch.h"
#include "xid_table.h"


static int ofpmsg_recv_hello( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_error( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_echorequest( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_echoreply( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_vendor( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_featuresreply( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_getconfigreply( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_packetin( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_flowremoved( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_portstatus( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_statsreply( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_barrierreply( struct switch_info *sw_info, buffer *buf );
static int ofpmsg_recv_queue_getconfigreply( struct switch_info *sw_info, buffer *buf );


#define ofpmsg_debug( format, args... )                      \
  do {                                                       \
    debug( format " state:%d, dpid:%#" PRIx64 ", fd:%d.",    \
      ## args, sw_info->state, sw_info->datapath_id,         \
      sw_info->secure_channel_fd );                          \
  }                                                          \
  while ( 0 )


static int
send_transaction_reply( struct switch_info *sw_info, buffer *buf ) {
  struct ofp_header *header;
  uint32_t xid;
  xid_entry_t *xid_entry;

  header = buf->data;
  xid = ntohl( header->xid );

  xid_entry = lookup_xid_entry( xid );
  if ( xid_entry == NULL ) {
    free_buffer( buf );
    return -1;
  }
  header->xid = htonl( xid_entry->original_xid );
  service_send_to_reply( xid_entry->service_name, MESSENGER_OPENFLOW_MESSAGE,
                         &sw_info->datapath_id, buf );
  delete_xid_entry( xid_entry );
  free_buffer( buf );

  return 0;
}


int
ofpmsg_recv_hello( struct switch_info *sw_info, buffer *buf ) {
  int ret;

  ofpmsg_debug( "Receive 'hello' from a switch." );

  free_buffer( buf );

  ret = switch_event_recv_hello( sw_info );
  if ( ret < 0 ) {
    return ret;
  }

  return ret;
}


int
ofpmsg_recv_error( struct switch_info *sw_info, buffer *buf ) {
  int ret;
  struct ofp_error_msg *error_msg;

  ofpmsg_debug( "Receive 'error' from a switch." );

  error_msg = buf->data;

  uint16_t type = ntohs( error_msg->type );
  uint16_t code = ntohs( error_msg->code );

  notice( "Receive 'error' from a switch. xid:%#x, type:%d, code:%d.",
          ntohl( error_msg->header.xid ), type, code );

  ret = switch_event_recv_error( sw_info );
  if ( ret < 0 ) {
    free_buffer( buf );
    return ret;
  }

  if ( type == OFPET_FLOW_MOD_FAILED ) {
    size_t length = ntohs( error_msg->header.length ) - offsetof( struct ofp_error_msg, data );
    if ( length >= offsetof( struct ofp_flow_mod, command ) ) {
      struct ofp_flow_mod *flow_mod = ( struct ofp_flow_mod * ) error_msg->data;
      uint32_t xid = ntohl( flow_mod->header.xid );
      xid_entry_t *xid_entry = lookup_xid_entry( xid );
      if ( xid_entry != NULL ) {
        flow_mod->header.xid = htonl( xid_entry->original_xid );
      }
      uint64_t cookie = ntohll( flow_mod->cookie );
      if ( cookie == RESERVED_COOKIE ) {
        free_buffer( buf );
        return 0;
      }
      if ( sw_info->cookie_translation ) {
        cookie_entry_t *entry = lookup_cookie_entry_by_cookie( &cookie );
        if ( entry != NULL ) {
          flow_mod->cookie = htonll( entry->application.cookie );
          if ( length >= offsetof( struct ofp_flow_mod, actions ) ) {
            flow_mod->flags = htons( entry->application.flags );
          }
        }

        if ( length >= offsetof( struct ofp_flow_mod, idle_timeout ) ) {
          uint16_t command = ntohs( flow_mod->command );
          switch ( command ) {
          case OFPFC_ADD:
          {
            if ( entry != NULL ) {
              delete_cookie_entry( entry );
            }
            else {
              error( "No cookie entry found ( cookie = %#" PRIx64 " ).", cookie );
            }
          }
          break;

          case OFPFC_MODIFY:
          case OFPFC_MODIFY_STRICT:
          case OFPFC_DELETE:
          case OFPFC_DELETE_STRICT:
            break;

          default:
            error( "Undefined flow_mod command ( command = %#x ).", command );
          }
        }
      }
    }
  }

  send_transaction_reply( sw_info, buf );

  return 0;
}


int
ofpmsg_recv_echorequest( struct switch_info *sw_info, buffer *buf ) {
  int ret;
  struct ofp_header *ofp_header;

  ofpmsg_debug( "Receive 'echo request' from a switch." );

  ofp_header = buf->data;
  remove_front_buffer( buf, sizeof( struct ofp_header ) );
  ret = ofpmsg_send_echoreply( sw_info, ntohl( ofp_header->xid ), buf );

  return ret;
}


int
ofpmsg_recv_echoreply( struct switch_info *sw_info,  buffer *buf ) {
  ofpmsg_debug( "Receive 'echo reply' from a switch." );

  int ret = switch_event_recv_echoreply( sw_info, buf );
  if ( ret < 0 ) {
    free_buffer( buf );
    return ret;
  }

  send_transaction_reply( sw_info, buf );

  return 0;
}


int
ofpmsg_recv_vendor( struct switch_info *sw_info, buffer *buf ) {
  ofpmsg_debug( "Receive 'vendor' from a switch." );

  service_send_to_application( sw_info->vendor_service_name_list,
                               MESSENGER_OPENFLOW_MESSAGE,
                               &sw_info->datapath_id, buf );
  free_buffer( buf );

  return 0;
}


int
ofpmsg_recv_featuresreply( struct switch_info *sw_info, buffer *buf ) {
  int ret;
  struct ofp_switch_features *feat_rep;
  uint64_t datapath_id;

  ofpmsg_debug( "Receive 'features reply' from a switch." );

  feat_rep = buf->data;

  datapath_id = ntohll( feat_rep->datapath_id );

  ret = switch_event_recv_featuresreply( sw_info, &datapath_id );
  if ( ret < 0 ) {
    free_buffer( buf );
    return ret;
  }

  send_transaction_reply( sw_info, buf );

  return 0;
}


int
ofpmsg_recv_getconfigreply( struct switch_info *sw_info, buffer *buf ) {
  ofpmsg_debug( "Receive 'get config reply' from a switch." );

  send_transaction_reply( sw_info, buf );

  return 0;
}


int
ofpmsg_recv_packetin( struct switch_info *sw_info, buffer *buf ) {
  ofpmsg_debug( "Receive 'packet in' from a switch." );

  service_send_to_application( sw_info->packetin_service_name_list,
                               MESSENGER_OPENFLOW_MESSAGE,
                               &sw_info->datapath_id, buf );
  free_buffer( buf );

  return 0;
}


int
ofpmsg_recv_flowremoved( struct switch_info *sw_info, buffer *buf ) {
  struct ofp_flow_removed *flow_removed;
  uint64_t cookie;
  cookie_entry_t *entry;

  ofpmsg_debug( "Receive 'flow removed' from a switch." );

  flow_removed = buf->data;

  cookie = ntohll( flow_removed->cookie );
  if ( cookie == RESERVED_COOKIE ) {
    free_buffer( buf );
    return 0;
  }

  if ( !sw_info->cookie_translation ) {
    service_send_to_application( sw_info->state_service_name_list, MESSENGER_OPENFLOW_MESSAGE,
                                 &sw_info->datapath_id, buf );
    free_buffer( buf );
    return 0;
  }

  entry = lookup_cookie_entry_by_cookie( &cookie );
  if ( entry == NULL ) {
    error( "No cookie entry found ( cookie = %#" PRIx64 " ).", cookie );
    free_buffer( buf );
    return 0;
  }

  debug( "Cookie found ( cookie = %#" PRIx64 ", application = [ cookie = %#" PRIx64
         ", service name = %s, flags = %#x ], reference_count = %d, expire_at = %" PRIu64 " ).",
         cookie, entry->application.cookie, entry->application.service_name, entry->application.flags,
         entry->reference_count, ( int64_t ) entry->expire_at );

  if ( entry->application.flags & OFPFF_SEND_FLOW_REM ) {
    flow_removed->cookie = htonll( entry->application.cookie );

    service_send_to_reply( entry->application.service_name, MESSENGER_OPENFLOW_MESSAGE,
                           &sw_info->datapath_id, buf );
  }

  delete_cookie_entry( entry );
  free_buffer( buf );

  return 0;
}


int
ofpmsg_recv_portstatus( struct switch_info *sw_info, buffer *buf ) {
  ofpmsg_debug( "Receive 'port status' from a switch." );

  service_send_to_application( sw_info->portstatus_service_name_list,
                               MESSENGER_OPENFLOW_MESSAGE,
                               &sw_info->datapath_id, buf );
  free_buffer( buf );

  return 0;
}


int
ofpmsg_recv_statsreply( struct switch_info *sw_info, buffer *buf ) {
  struct ofp_stats_reply *stats_reply = buf->data;
  uint16_t type = ntohs( stats_reply->type );

  ofpmsg_debug( "Receive 'statistics reply' from a switch." );

  if ( type == OFPST_FLOW && sw_info->cookie_translation ) {
    size_t body_offset = offsetof( struct ofp_stats_reply, body );
    int body_length = ntohs( stats_reply->header.length ) - ( int ) body_offset;
    struct ofp_flow_stats *flow_stats = ( void * ) ( ( char * ) stats_reply + body_offset );
    while ( body_length > 0 ) {
      uint64_t cookie = ntohll( flow_stats->cookie );
      cookie_entry_t *entry = lookup_cookie_entry_by_cookie( &cookie );
      if ( entry != NULL ) {
        debug( "Cookie entry found ( cookie = %#" PRIx64 ", application = [ cookie = %#" PRIx64 ", service name = %s ] ).",
               cookie, entry->application.cookie, entry->application.service_name );
        flow_stats->cookie = htonll( entry->application.cookie );
      }
      else {
        warn( "No cookie entry found ( cookie = %#" PRIx64 " ).", cookie );
      }

      body_length = body_length - ntohs( flow_stats->length );
      flow_stats = ( void * ) ( ( char * ) flow_stats + ntohs( flow_stats->length ) );
    }
  }

  // since we may receive multiple replies, we cannot call send_transaction_reply().
  uint32_t xid = ntohl( stats_reply->header.xid );
  xid_entry_t *xid_entry = lookup_xid_entry( xid );
  if ( xid_entry == NULL ) {
    error( "No transaction id entry found ( transaction_id = %#" PRIx32 " ).", xid );
    free_buffer( buf );
    return -1;
  }
  stats_reply->header.xid = htonl( xid_entry->original_xid );
  service_send_to_reply( xid_entry->service_name, MESSENGER_OPENFLOW_MESSAGE,
                         &sw_info->datapath_id, buf );

  if ( ( ntohs( stats_reply->flags ) & OFPSF_REPLY_MORE ) == 0 ) {
    delete_xid_entry( xid_entry );
  }
  free_buffer( buf );

  return 0;
}


int
ofpmsg_recv_barrierreply( struct switch_info *sw_info, buffer *buf ) {
  ofpmsg_debug( "Receive 'barrier reply' from a switch." );

  send_transaction_reply( sw_info, buf );

  return 0;
}


int
ofpmsg_recv_queue_getconfigreply( struct switch_info *sw_info, buffer *buf ) {
  ofpmsg_debug( "Receive 'queue get config reply' from a switch." );

  send_transaction_reply( sw_info, buf );

  return 0;
}


int
ofpmsg_recv( struct switch_info *sw_info, buffer *buf ) {
  int ret;
  struct ofp_header *header;
  uint16_t error_type, error_code;

  ofpmsg_debug( "Receive a message from a switch." );

  header = buf->data;
  ret = validate_openflow_message( buf );
  if ( ret != 0 ) {
    notice( "Invalid openflow message. type:%d, errno:%d", header->type, ret );

    error_type = OFPET_BAD_REQUEST;
    error_code = OFPBRC_BAD_TYPE;
    get_error_type_and_code( header->type, ret, &error_type, &error_code );
    debug( "Validation error. type %u, errno %d, error type %u, error code %u",
           header->type, ret, error_type, error_code );

    ofpmsg_send_error_msg( sw_info, error_type, error_code, buf );
    free_buffer( buf );

    return -1;
  }

  switch ( header->type ) {
  // Immutable messages.
  case OFPT_HELLO:
    ret = ofpmsg_recv_hello( sw_info, buf );
    break;

  case OFPT_ERROR:
    ret = ofpmsg_recv_error( sw_info, buf );
    break;

  case OFPT_ECHO_REQUEST:
    ret = ofpmsg_recv_echorequest( sw_info, buf );
    break;

  case OFPT_ECHO_REPLY:
    ret = ofpmsg_recv_echoreply( sw_info, buf );
    break;

  case OFPT_VENDOR:
    ret = ofpmsg_recv_vendor( sw_info, buf );
    break;

  // Switch configuration messages.
  case OFPT_FEATURES_REPLY:
    ret = ofpmsg_recv_featuresreply( sw_info, buf );
    break;

  case OFPT_GET_CONFIG_REPLY:
    ret = ofpmsg_recv_getconfigreply( sw_info, buf );
    break;

  // Asynchronous messages.
  case OFPT_PACKET_IN:
    ret = ofpmsg_recv_packetin( sw_info, buf );
    break;

  case OFPT_FLOW_REMOVED:
    ret = ofpmsg_recv_flowremoved( sw_info, buf );
    break;

  case OFPT_PORT_STATUS:
    ret = ofpmsg_recv_portstatus( sw_info, buf );
    break;

  // Statistics messages.
  case OFPT_STATS_REPLY:
    ret = ofpmsg_recv_statsreply( sw_info, buf );
    break;

  // Barrier messages.
  case OFPT_BARRIER_REPLY:
    ret = ofpmsg_recv_barrierreply( sw_info, buf );
    break;

  // Queue Configuration messages.
  case OFPT_QUEUE_GET_CONFIG_REPLY:
    ret = ofpmsg_recv_queue_getconfigreply( sw_info, buf );
    break;

  default:
    assert( 0 );
    break;
  }

  return ret;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
