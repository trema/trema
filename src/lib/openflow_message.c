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


#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include "openflow_message.h"
#include "packet_info.h"
#include "wrapper.h"
#include "log.h"


#ifdef UNIT_TESTING

// Allow static functions to be called from unit tests.
#define static

/* Redirect getpid to a function in the test application so it's
 * possible to test if pid value is correctly used. */
#ifdef getpid
#undef getpid
#endif // getpid
#define getpid mock_getpid
extern pid_t mock_getpid( void );

/* Redirect debug to a function in the test application so it's
 * possible to test if debug messages are generated expectedly. */
#ifdef debug
#undef debug
#endif // debug
#define debug mock_debug
extern void mock_debug( const char *format, ... );

#endif // UNIT_TESTING


#define VLAN_VID_MASK 0x0fff  // 12 bits
#define VLAN_PCP_MASK 0x07    // 3 bits
#define NW_TOS_MASK 0xfc      // upper 6 bits
#define ARP_OP_MASK 0x00ff    // 8 bits
#define ICMP_TYPE_MASK 0x00ff // 8 bits
#define ICMP_CODE_MASK 0x00ff // 8 bits

#define PORT_CONFIG ( OFPPC_PORT_DOWN | OFPPC_NO_STP | OFPPC_NO_RECV      \
                      | OFPPC_NO_RECV_STP | OFPPC_NO_FLOOD | OFPPC_NO_FWD \
                      | OFPPC_NO_PACKET_IN )
#define PORT_STATE ( OFPPS_LINK_DOWN | OFPPS_STP_LISTEN | OFPPS_STP_LEARN \
                     | OFPPS_STP_FORWARD | OFPPS_STP_BLOCK | OFPPS_STP_MASK )
#define PORT_FEATURES ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD    \
                        | OFPPF_100MB_FD |  OFPPF_1GB_HD | OFPPF_1GB_FD   \
                        | OFPPF_10GB_FD | OFPPF_COPPER | OFPPF_FIBER      \
                        | OFPPF_AUTONEG | OFPPF_PAUSE | OFPPF_PAUSE_ASYM )
#define FLOW_MOD_FLAGS ( OFPFF_SEND_FLOW_REM | OFPFF_CHECK_OVERLAP | OFPFF_EMERG )


static uint32_t transaction_id = 0;
static pthread_mutex_t transaction_id_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static uint64_t cookie = 0;
static pthread_mutex_t cookie_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


bool
init_openflow_message( void ) {
  pid_t pid;

  pid = getpid();

  pthread_mutex_lock( &transaction_id_mutex );
  transaction_id = ( uint32_t ) pid << 16;
  pthread_mutex_unlock( &transaction_id_mutex );

  pthread_mutex_lock( &cookie_mutex );
  cookie = ( uint64_t ) pid << 48;
  pthread_mutex_unlock( &cookie_mutex );

  debug( "transaction_id and cookie are initialized ( transaction_id = %#x, cookie = %#" PRIx64 " ).",
         transaction_id, cookie );

  return true;
}


static buffer *
create_header( const uint32_t transaction_id, const uint8_t type, const uint16_t length ) {
  debug( "Creating an OpenFlow header ( version = %#x, type = %#x, length = %u, xid = %#x ).",
         OFP_VERSION, type, length, transaction_id );

  assert( length >= sizeof( struct ofp_header ) );

  buffer *buffer = alloc_buffer();
  assert( buffer != NULL );

  struct ofp_header *header = append_back_buffer( buffer, length );
  assert( header != NULL );
  memset( header, 0, length );

  header->version = OFP_VERSION;
  header->type = type;
  header->length = htons( length );
  header->xid = htonl( transaction_id );

  return buffer;
}


buffer *
create_error( const uint32_t transaction_id, const uint16_t type,
              const uint16_t code, const buffer *data ) {
  uint16_t length;
  uint16_t data_len = 0;
  buffer *buffer;
  struct ofp_error_msg *error_msg;

  if ( ( data != NULL ) && ( data->length > 0 ) ) {
    data_len = ( uint16_t ) data->length;
  }

  debug( "Creating an error ( xid = %#x, type = %#x, code = %#x, data length = %u ).",
         transaction_id, type, code, data_len );

  length = ( uint16_t ) ( sizeof( struct ofp_error_msg ) + data_len );
  buffer = create_header( transaction_id, OFPT_ERROR, length );
  assert( buffer != NULL );

  error_msg = ( struct ofp_error_msg * ) buffer->data;
  error_msg->type = htons( type );
  error_msg->code = htons( code );

  if ( data_len > 0 ) {
    memcpy( error_msg->data, data->data, data->length );
  }

  return buffer;
}


buffer *
create_hello( const uint32_t transaction_id ) {
  debug( "Creating a hello ( xid = %#x ).", transaction_id );

  return create_header( transaction_id, OFPT_HELLO, sizeof( struct ofp_header ) );
}


buffer *
create_echo_request( const uint32_t transaction_id, const buffer *body ) {
  uint16_t data_length = 0;

  if ( ( body != NULL ) && ( body->length > 0 ) ) {
    data_length = ( uint16_t ) body->length;
  }

  debug( "Creating an echo request ( xid = %#x, data length = %u ).", transaction_id, data_length );

  buffer *echo_request = create_header( transaction_id, OFPT_ECHO_REQUEST, ( uint16_t ) ( sizeof( struct ofp_header ) + data_length ) );
  assert( echo_request != NULL );

  if ( data_length > 0 ) {
    memcpy( ( char * ) echo_request->data + sizeof( struct ofp_header ), body->data, data_length );
  }

  return echo_request;
}


buffer *
create_echo_reply( const uint32_t transaction_id, const buffer *body ) {
  uint16_t data_length = 0;

  if ( ( body != NULL ) && ( body->length > 0 ) ) {
    data_length = ( uint16_t ) body->length;
  }

  debug( "Creating an echo reply ( xid = %#x, data length = %u ).", transaction_id, data_length );

  buffer *echo_reply = create_header( transaction_id, OFPT_ECHO_REPLY, ( uint16_t ) ( sizeof( struct ofp_header ) + data_length ) );
  assert( echo_reply != NULL );

  if ( data_length > 0 ) {
    memcpy( ( char * ) echo_reply->data + sizeof( struct ofp_header ), body->data, data_length );
  }

  return echo_reply;
}


buffer *
create_vendor( const uint32_t transaction_id, const uint32_t vendor, const buffer *data ) {
  void *d;
  uint16_t length;
  uint16_t data_length = 0;
  buffer *buffer;
  struct ofp_vendor_header *vendor_header;

  if ( ( data != NULL ) && ( data->length > 0 ) ) {
    data_length = ( uint16_t ) data->length;
  }

  debug( "Creating a vendor ( xid = %#x, vendor = %#x, data length = %u ).",
         transaction_id, vendor, data_length );

  length =  ( uint16_t ) ( sizeof( struct ofp_vendor_header ) + data_length );
  buffer = create_header( transaction_id, OFPT_VENDOR, length );
  assert( buffer != NULL );

  vendor_header = ( struct ofp_vendor_header * ) buffer->data;
  vendor_header->vendor = htonl( vendor );

  if ( data_length > 0 ) {
    d = ( void * ) ( ( char * ) buffer->data + sizeof( struct ofp_vendor_header ) );
    memcpy( d, data->data, data_length );
  }

  return buffer;
}


buffer *
create_features_request( const uint32_t transaction_id ) {
  debug( "Creating a features request ( xid = %#x ).", transaction_id );

  return create_header( transaction_id, OFPT_FEATURES_REQUEST, sizeof( struct ofp_header ) );
}


buffer *
create_features_reply( const uint32_t transaction_id, const uint64_t datapath_id,
                       const uint32_t n_buffers, const uint8_t n_tables,
                       const uint32_t capabilities, const uint32_t actions,
                       const list_element *ports ) {
  char port_str[ 1024 ];
  int count = 0;
  uint16_t n_ports = 0;
  buffer *buffer;
  struct ofp_switch_features *switch_features;
  struct ofp_phy_port *phy_port;
  struct ofp_phy_port pn;
  list_element *p = NULL, *port;

  debug( "Creating a features reply "
         "( xid = %#x, datapath_id = %#" PRIx64 ", n_buffers = %u, n_tables = %u, capabilities = %#x, actions = %#x ).",
         transaction_id, datapath_id, n_buffers, n_tables, capabilities, actions );

  if ( ports != NULL ) {
    p = ( list_element * ) xmalloc( sizeof( list_element ) );
    memcpy( p, ports, sizeof( list_element ) );

    port = p;
    while ( port != NULL ) {
      port = port->next;
      n_ports++;
    }
  }

  debug( "# of ports = %u.", n_ports );

  buffer = create_header( transaction_id, OFPT_FEATURES_REPLY,
                          ( uint16_t ) ( offsetof( struct ofp_switch_features, ports )
                                         + sizeof( struct ofp_phy_port ) * n_ports ) );
  assert( buffer != NULL );

  switch_features = ( struct ofp_switch_features * ) buffer->data;
  switch_features->datapath_id = htonll( datapath_id );
  switch_features->n_buffers = htonl( n_buffers );
  switch_features->n_tables = n_tables;
  memset( switch_features->pad, 0, sizeof( switch_features->pad ) );
  switch_features->capabilities = htonl( capabilities );
  switch_features->actions = htonl( actions );

  if ( n_ports ) {
    phy_port = ( struct ofp_phy_port * ) ( ( char * ) buffer->data
                                           + offsetof( struct ofp_switch_features, ports ) );
    port = p;
    while ( port != NULL ) {
      phy_port_to_string( port->data, port_str, sizeof( port_str ) );
      debug( "[%u] %s", count++, port_str );
      hton_phy_port( &pn, ( struct ofp_phy_port * ) port->data );
      memcpy( phy_port, &pn, sizeof( struct ofp_phy_port ) );
      port = port->next;
      phy_port++;
    }

    xfree( p );
  }

  return buffer;
}


buffer *
create_get_config_request( const uint32_t transaction_id ) {
  debug( "Creating a get config request ( xid = %#x ).", transaction_id );

  return create_header( transaction_id, OFPT_GET_CONFIG_REQUEST, sizeof( struct ofp_header ) );
}


buffer *
create_get_config_reply( const uint32_t transaction_id, const uint16_t flags,
                         const uint16_t miss_send_len ) {
  buffer *buffer;
  struct ofp_switch_config *switch_config;

  debug( "Creating a get config reply ( xid = %#x, flags = %#x, miss_send_len = %u ).",
         transaction_id, flags, miss_send_len );

  buffer = create_header( transaction_id, OFPT_GET_CONFIG_REPLY, sizeof( struct ofp_switch_config ) );
  assert( buffer != NULL );

  switch_config = ( struct ofp_switch_config * ) buffer->data;
  switch_config->flags = htons( flags );
  switch_config->miss_send_len = htons( miss_send_len );

  return buffer;
}


buffer *
create_set_config( const uint32_t transaction_id, const uint16_t flags, uint16_t miss_send_len ) {
  debug( "Creating a set config ( xid = %#x, flags = %#x, miss_send_len = %u ).",
         transaction_id, flags, miss_send_len );

  buffer *set_config = create_header( transaction_id, OFPT_SET_CONFIG, sizeof( struct ofp_switch_config ) );
  assert( set_config != NULL );

  struct ofp_switch_config *switch_config = ( struct ofp_switch_config * ) set_config->data;
  switch_config->flags = htons( flags );
  switch_config->miss_send_len = htons( miss_send_len );
  return set_config;
}


buffer *
create_packet_in( const uint32_t transaction_id, const uint32_t buffer_id, const uint16_t total_len,
                  uint16_t in_port, const uint8_t reason, const buffer *data ) {
  uint16_t data_length = 0;
  buffer *buffer;
  struct ofp_packet_in *packet_in;

  if ( ( data != NULL ) && ( data->length > 0 ) ) {
    data_length = ( uint16_t ) data->length;
  }

  debug( "Creating a packet-in "
         "( xid = %#x, buffer_id = %#x, total_len = %u, "
         "in_port = %u, reason = %#x, data length = %u ).",
         transaction_id, buffer_id, total_len,
         in_port, reason, data_length );

  buffer = create_header( transaction_id, OFPT_PACKET_IN,
                          ( uint16_t ) ( offsetof( struct ofp_packet_in, data ) + data_length ) );
  assert( buffer != NULL );

  packet_in = ( struct ofp_packet_in * ) buffer->data;
  packet_in->buffer_id = htonl( buffer_id );
  packet_in->total_len = htons( total_len );
  packet_in->in_port = htons( in_port );
  packet_in->reason = reason;
  packet_in->pad = 0;

  if ( data_length > 0 ) {
    memcpy( packet_in->data, data->data, data_length );
  }

  return buffer;
}


buffer *
create_flow_removed( const uint32_t transaction_id, const struct ofp_match match,
                     const uint64_t cookie, const uint16_t priority,
                     const uint8_t reason, const uint32_t duration_sec,
                     const uint32_t duration_nsec, const uint16_t idle_timeout,
                     const uint64_t packet_count, const uint64_t byte_count ) {
  char match_str[ 1024 ];
  buffer *buffer;
  struct ofp_match m = match;
  struct ofp_flow_removed *flow_removed;

  // Because match_to_string() is costly, we check logging_level first.
  if ( get_logging_level() >= LOG_DEBUG ) {
    match_to_string( &m, match_str, sizeof( match_str ) );
    debug( "Creating a flow removed "
           "( xid = %#x, match = [%s], cookie = %#" PRIx64 ", priority = %u, "
           "reason = %#x, duration_sec = %u, duration_nsec = %u, "
           "idle_timeout = %u, packet_count = %" PRIu64 ", byte_count = %" PRIu64 " ).",
           transaction_id, match_str, cookie, priority,
           reason, duration_sec, duration_nsec,
           idle_timeout, packet_count, byte_count );
  }

  buffer = create_header( transaction_id, OFPT_FLOW_REMOVED, sizeof( struct ofp_flow_removed ) );
  assert( buffer != NULL );

  flow_removed = ( struct ofp_flow_removed * ) buffer->data;
  hton_match( &flow_removed->match, &m );
  flow_removed->cookie = htonll( cookie );
  flow_removed->priority = htons( priority );
  flow_removed->reason = reason;
  memset( &flow_removed->pad, 0, sizeof( flow_removed->pad ) );
  flow_removed->duration_sec = htonl( duration_sec );
  flow_removed->duration_nsec = htonl( duration_nsec );
  flow_removed->idle_timeout = htons( idle_timeout );
  memset( &flow_removed->pad2, 0, sizeof( flow_removed->pad2 ) );
  flow_removed->packet_count = htonll( packet_count );
  flow_removed->byte_count = htonll( byte_count );

  return buffer;
}


buffer *
create_port_status( const uint32_t transaction_id, const uint8_t reason,
                    const struct ofp_phy_port desc ) {
  char desc_str[ 1024 ];
  buffer *buffer;
  struct ofp_phy_port d = desc;
  struct ofp_port_status *port_status;

  phy_port_to_string( &d, desc_str, sizeof( desc_str ) );
  debug( "Creating a port status ( xid = %#x, reason = %#x, desc = [%s] ).",
         transaction_id, reason, desc_str );

  buffer = create_header( transaction_id, OFPT_PORT_STATUS, sizeof( struct ofp_port_status ) );
  assert( buffer != NULL );

  port_status = ( struct ofp_port_status * ) buffer->data;
  port_status->reason = reason;
  memset( &port_status->pad, 0, sizeof( port_status->pad ) );
  hton_phy_port( &port_status->desc, &d );

  return buffer;
}


static uint16_t
get_actions_length( const openflow_actions *actions ) {
  int actions_length = 0;
  struct ofp_action_header *action_header;
  list_element *action;

  debug( "Calculating the total length of actions." );

  assert( actions != NULL );

  action = actions->list;
  while ( action != NULL ) {
    action_header = ( struct ofp_action_header * ) action->data;

    if ( ( action_header->type <= OFPAT_ENQUEUE ) || ( action_header->type == OFPAT_VENDOR ) ) {
      actions_length += action_header->len;
    }
    else {
      critical( "Undefined action type ( type = %#x ).", action_header->type );
      assert( 0 );
    }

    action = action->next;
  }

  debug( "Total length of actions = %d.", actions_length );

  if ( actions_length > UINT16_MAX ) {
    critical( "Too many actions ( # of actions = %d, actions length = %d ).",
              actions->n_actions, actions_length );
    assert( 0 );
  }

  return ( uint16_t ) actions_length;
}


buffer *
create_packet_out( const uint32_t transaction_id, const uint32_t buffer_id, const uint16_t in_port,
                   const openflow_actions *actions, const buffer *data ) {
  void *a, *d;
  uint16_t length;
  uint16_t data_length = 0;
  uint16_t action_length = 0;
  uint16_t actions_length = 0;
  buffer *buffer;
  struct ofp_packet_out *packet_out;
  struct ofp_action_header *action_header;
  list_element *action;

  if ( ( data != NULL ) && ( data->length > 0 ) ) {
    data_length = ( uint16_t ) data->length;
  }

  debug( "Creating a packet-out ( xid = %#x, buffer_id = %#x, in_port = %u, data length = %u ).",
         transaction_id, buffer_id, in_port, data_length );

  if ( buffer_id == UINT32_MAX ) {
    if ( data == NULL ) {
      die( "An Ethernet frame must be provided if buffer_id is equal to %#x", UINT32_MAX );
    }
    if ( data->length + ETH_FCS_LENGTH < ETH_MINIMUM_LENGTH ) {
      die( "The length of the provided Ethernet frame is shorter than the minimum length of an Ethernet frame (= %d bytes).", ETH_MINIMUM_LENGTH );
    }
  }

  if ( actions != NULL ) {
    debug( "# of actions = %d.", actions->n_actions );
    actions_length = get_actions_length( actions );
  }

  length = ( uint16_t ) ( offsetof( struct ofp_packet_out, actions ) + actions_length + data_length );
  buffer = create_header( transaction_id, OFPT_PACKET_OUT, length );
  assert( buffer != NULL );

  packet_out = ( struct ofp_packet_out * ) buffer->data;
  packet_out->buffer_id = htonl( buffer_id );
  packet_out->in_port = htons( in_port );
  packet_out->actions_len = htons( actions_length );

  if ( actions_length > 0 ) {
    a = ( void * ) ( ( char * ) buffer->data + offsetof( struct ofp_packet_out, actions ) );

    action = actions->list;
    while ( action != NULL ) {
      action_header = ( struct ofp_action_header * ) action->data;
      action_length = action_header->len;
      hton_action( ( struct ofp_action_header * ) a, action_header );
      a = ( void * ) ( ( char * ) a + action_length );
      action = action->next;
    }
  }

  if ( data_length > 0 ) {
    d = ( void * ) ( ( char * ) buffer->data
                     + offsetof( struct ofp_packet_out, actions ) + actions_length );
    memcpy( d, data->data, data_length );
  }

  return buffer;
}


buffer *
create_flow_mod( const uint32_t transaction_id, const struct ofp_match match,
                 const uint64_t cookie, const uint16_t command,
                 const uint16_t idle_timeout, const uint16_t hard_timeout,
                 const uint16_t priority, const uint32_t buffer_id,
                 const uint16_t out_port, const uint16_t flags,
                 const openflow_actions *actions ) {
  void *a;
  char match_str[ 1024 ];
  uint16_t length;
  uint16_t action_length = 0;
  uint16_t actions_length = 0;
  buffer *buffer;
  struct ofp_match m = match;
  struct ofp_flow_mod *flow_mod;
  struct ofp_action_header *action_header;
  list_element *action;

  // Because match_to_string() is costly, we check logging_level first.
  if ( get_logging_level() >= LOG_DEBUG ) {
    match_to_string( &m, match_str, sizeof( match_str ) );
    debug( "Creating a flow modification "
           "( xid = %#x, match = [%s], cookie = %#" PRIx64 ", command = %#x, "
           "idle_timeout = %u, hard_timeout = %u, priority = %u, "
           "buffer_id = %#x, out_port = %u, flags = %#x ).",
           transaction_id, match_str, cookie, command,
           idle_timeout, hard_timeout, priority,
           buffer_id, out_port, flags );
  }

  if ( actions != NULL ) {
    debug( "# of actions = %d.", actions->n_actions );
    actions_length = get_actions_length( actions );
  }

  length = ( uint16_t ) ( offsetof( struct ofp_flow_mod, actions ) + actions_length );
  buffer = create_header( transaction_id, OFPT_FLOW_MOD, length );
  assert( buffer != NULL );

  flow_mod = ( struct ofp_flow_mod * ) buffer->data;
  hton_match( &flow_mod->match, &m );
  flow_mod->cookie = htonll( cookie );
  flow_mod->command = htons( command );
  flow_mod->idle_timeout = htons( idle_timeout );
  flow_mod->hard_timeout = htons( hard_timeout );
  flow_mod->priority = htons( priority );
  flow_mod->buffer_id = htonl( buffer_id );
  flow_mod->out_port = htons( out_port );
  flow_mod->flags = htons( flags );

  if ( actions_length > 0 ) {
    a = ( void * ) ( ( char * ) buffer->data + offsetof( struct ofp_flow_mod, actions ) );

    action = actions->list;
    while ( action != NULL ) {
      action_header = ( struct ofp_action_header * ) action->data;
      action_length = action_header->len;
      hton_action( ( struct ofp_action_header * ) a, action_header );
      a = ( void * ) ( ( char * ) a + action_length );
      action = action->next;
    }
  }

  return buffer;
}


buffer *
create_port_mod( const uint32_t transaction_id, const uint16_t port_no,
                 const uint8_t hw_addr[ OFP_ETH_ALEN ], const uint32_t config,
                 const uint32_t mask, const uint32_t advertise ) {
  buffer *buffer;
  struct ofp_port_mod *port_mod;

  debug( "Creating a port modification "
         "( xid = %#x, port_no = %u, hw_addr = %02x:%02x:%02x:%02x:%02x:%02x, "
         "config = %#x, mask = %#x, advertise = %#x ).",
         transaction_id, port_no,
         hw_addr[ 0 ], hw_addr[ 1 ], hw_addr[ 2 ], hw_addr[ 3 ], hw_addr[ 4 ], hw_addr[ 5 ],
         config, mask, advertise );

  buffer = create_header( transaction_id, OFPT_PORT_MOD, sizeof( struct ofp_port_mod ) );
  assert( buffer != NULL );

  port_mod = ( struct ofp_port_mod * ) buffer->data;
  port_mod->port_no = htons( port_no );
  memcpy( port_mod->hw_addr, hw_addr, OFP_ETH_ALEN );
  port_mod->config = htonl( config );
  port_mod->mask = htonl( mask );
  port_mod->advertise = htonl( advertise );
  memset( port_mod->pad, 0, sizeof( port_mod->pad ) );

  return buffer;
}


static buffer *
create_stats_request( const uint32_t transaction_id, const uint16_t type,
                      const uint16_t length, const uint16_t flags ) {
  buffer *buffer;
  struct ofp_stats_request *stats_request;

  debug( "Creating a stats request ( xid = %#x, type = %#x, length = %u, flags = %#x ).",
         transaction_id, type, length, flags );

  buffer = create_header( transaction_id, OFPT_STATS_REQUEST, length );
  assert( buffer != NULL );

  stats_request = ( struct ofp_stats_request * ) buffer->data;
  stats_request->type = htons( type );
  stats_request->flags = htons( flags );

  return buffer;
}


buffer *
create_desc_stats_request( const uint32_t transaction_id, const uint16_t flags ) {
  debug( "Creating a description stats request ( xid = %#x, flags = %#x ).",
         transaction_id, flags );

  return create_stats_request( transaction_id, OFPST_DESC,
                               sizeof( struct ofp_stats_request ), flags );
}


buffer *
create_flow_stats_request( const uint32_t transaction_id, const uint16_t flags,
                           const struct ofp_match match, const uint8_t table_id,
                           const uint16_t out_port ) {
  char match_str[ 1024 ];
  uint16_t length;
  buffer *buffer;
  struct ofp_match m = match;
  struct ofp_flow_stats_request *flow_stats_request;

  // Because match_to_string() is costly, we check logging_level first.
  if ( get_logging_level() >= LOG_DEBUG ) {
    match_to_string( &m, match_str, sizeof( match_str ) );
    debug( "Creating a flow stats request ( xid = %#x, flags = %#x, match = [%s], table_id = %u, out_port = %u ).",
           transaction_id, flags, match_str, table_id, out_port );
  }

  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_flow_stats_request ) );
  buffer = create_stats_request( transaction_id, OFPST_FLOW, length, flags );
  assert( buffer != NULL );

  flow_stats_request = ( struct ofp_flow_stats_request * ) ( ( char * ) buffer->data
                       + offsetof( struct ofp_stats_request, body ) );
  hton_match( &flow_stats_request->match, &m );
  flow_stats_request->table_id = table_id;
  flow_stats_request->pad = 0;
  flow_stats_request->out_port = htons( out_port );

  return buffer;
}


buffer *
create_aggregate_stats_request( const uint32_t transaction_id,
                                const uint16_t flags, const struct ofp_match match,
                                const uint8_t table_id, const uint16_t out_port ) {
  char match_str[ 1024 ];
  uint16_t length;
  buffer *buffer;
  struct ofp_match m = match;
  struct ofp_aggregate_stats_request *aggregate_stats_request;

  // Because match_to_string() is costly, we check logging_level first.
  if ( get_logging_level() >= LOG_DEBUG ) {
    match_to_string( &m, match_str, sizeof( match_str ) );
    debug( "Creating an aggregate stats request ( xid = %#x, flags = %#x, match = [%s], table_id = %u, out_port = %u ).",
           transaction_id, flags, match_str, table_id, out_port );
  }

  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body )
           + sizeof( struct ofp_aggregate_stats_request ) );
  buffer = create_stats_request( transaction_id, OFPST_AGGREGATE, length, flags );
  assert( buffer != NULL );

  aggregate_stats_request = ( struct ofp_aggregate_stats_request * ) ( ( char * ) buffer->data
                            + offsetof( struct ofp_stats_request, body ) );
  hton_match( &aggregate_stats_request->match, &m );
  aggregate_stats_request->table_id = table_id;
  aggregate_stats_request->pad = 0;
  aggregate_stats_request->out_port = htons( out_port );

  return buffer;
}


buffer *
create_table_stats_request( const uint32_t transaction_id, const uint16_t flags ) {
  debug( "Creating a table stats request ( xid = %#x, flags = %#x ).", transaction_id, flags );

  return create_stats_request( transaction_id, OFPST_TABLE,
                               sizeof( struct ofp_stats_request ), flags );
}


buffer *
create_port_stats_request( const uint32_t transaction_id, const uint16_t flags,
                           const uint16_t port_no ) {
  uint16_t length;
  buffer *buffer;
  struct ofp_port_stats_request *port_stats_request;

  debug( "Creating a port stats request ( xid = %#x, flags = %#x, port_no = %u ).",
         transaction_id, flags, port_no );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body )
           + sizeof( struct ofp_port_stats_request ) );
  buffer = create_stats_request( transaction_id, OFPST_PORT, length, flags );
  assert( buffer != NULL );

  port_stats_request = ( struct ofp_port_stats_request * ) ( ( char * ) buffer->data
                       + offsetof( struct ofp_stats_request, body ) );
  port_stats_request->port_no = htons( port_no );
  memset( &port_stats_request->pad, 0, sizeof( port_stats_request->pad ) );

  return buffer;
}


buffer *
create_queue_stats_request( const uint32_t transaction_id, const uint16_t flags,
                            const uint16_t port_no, const uint32_t queue_id ) {
  uint16_t length;
  buffer *buffer;
  struct ofp_queue_stats_request *queue_stats_request;

  debug( "Creating a queue stats request ( xid = %#x, flags = %#x, port_no = %u, queue_id = %u ).",
         transaction_id, flags, port_no, queue_id );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body )
                        + sizeof( struct ofp_queue_stats_request ) );
  buffer = create_stats_request( transaction_id, OFPST_QUEUE, length, flags );
  assert( buffer != NULL );

  queue_stats_request = ( struct ofp_queue_stats_request * ) ( ( char * ) buffer->data
                        + offsetof( struct ofp_stats_request, body ) );
  queue_stats_request->port_no = htons( port_no );
  memset( &queue_stats_request->pad, 0, sizeof( queue_stats_request->pad ) );
  queue_stats_request->queue_id = htonl( queue_id );

  return buffer;
}


buffer *
create_vendor_stats_request( const uint32_t transaction_id, const uint16_t flags,
                             const uint32_t vendor, const buffer *body ) {
  void *b;
  uint16_t length;
  uint16_t data_length = 0;
  uint32_t *v;
  buffer *buffer;

  if ( ( body != NULL ) && ( body->length > 0 ) ) {
    data_length = ( uint16_t ) body->length;
  }

  debug( "Creating a vendor stats request ( xid = %#x, flags = %#x, vendor = %#x, data length = %u ).",
         transaction_id, flags, vendor, data_length );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t )
                        + data_length );
  buffer = create_stats_request( transaction_id, OFPST_VENDOR, length, flags );
  assert( buffer != NULL );

  v = ( uint32_t * ) ( ( char * ) buffer->data + offsetof( struct ofp_stats_request, body ) );
  *v = htonl( vendor );

  if ( data_length > 0 ) {
    b = ( void * ) ( ( char * ) buffer->data
                   + offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t ) );

    memcpy( b, body->data, data_length );
  }

  return buffer;
}


static buffer *
create_stats_reply( const uint32_t transaction_id, const uint16_t type,
                    const uint16_t length, const uint16_t flags ) {
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;

  debug( "Creating a stats reply ( xid = %#x, type = %#x, length = %u, flags = %#x ).",
         transaction_id, type, length, flags );

  buffer = create_header( transaction_id, OFPT_STATS_REPLY, length );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  stats_reply->type = htons( type );
  stats_reply->flags = htons( flags );

  return buffer;
}


buffer *
create_desc_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                         const char mfr_desc[ DESC_STR_LEN ],
                         const char hw_desc[ DESC_STR_LEN ],
                         const char sw_desc[ DESC_STR_LEN ],
                         const char serial_num[ SERIAL_NUM_LEN ],
                         const char dp_desc[ DESC_STR_LEN ] ) {
  uint16_t length;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;
  struct ofp_desc_stats *desc_stats;

  debug( "Creating a description stats reply "
         "( xid = %#x, flags = %#x, mfr_desc = %s, hw_desc = %s, sw_desc = %s, serial_num = %s, dp_desc = %s ).",
         transaction_id, flags, mfr_desc, hw_desc, sw_desc, serial_num, dp_desc );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
                        + sizeof( struct ofp_desc_stats ) );
  buffer = create_stats_reply( transaction_id, OFPST_DESC, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  desc_stats = ( struct ofp_desc_stats * ) stats_reply->body;
  memcpy( desc_stats->mfr_desc, mfr_desc, DESC_STR_LEN );
  memcpy( desc_stats->hw_desc, hw_desc, DESC_STR_LEN );
  memcpy( desc_stats->sw_desc, sw_desc, DESC_STR_LEN );
  memcpy( desc_stats->serial_num, serial_num, DESC_STR_LEN );
  memcpy( desc_stats->dp_desc, dp_desc, DESC_STR_LEN );

  return buffer;
}


buffer *
create_flow_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                         const list_element *flows_stats_head ) {
  int n_flows = 0;
  uint16_t length = 0;
  buffer *buffer;
  list_element *f = NULL;
  list_element *flow = NULL;
  struct ofp_stats_reply *stats_reply;
  struct ofp_flow_stats *fs, *flow_stats;

  debug( "Creating a flow stats reply ( xid = %#x, flags = %#x ).", transaction_id, flags );

  if ( flows_stats_head != NULL ) {
    f = ( list_element * ) xmalloc( sizeof( list_element ) );
    memcpy( f, flows_stats_head, sizeof( list_element ) );
  }

  flow = f;
  while ( flow != NULL ) {
    flow_stats = flow->data;
    length = ( uint16_t ) ( length + flow_stats->length );
    n_flows++;
    flow = flow->next;
  }

  debug( "# of flows = %d.", n_flows );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body ) + length );

  buffer = create_stats_reply( transaction_id, OFPST_FLOW, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  flow_stats = ( struct ofp_flow_stats * ) stats_reply->body;

  flow = f;
  while ( flow != NULL ) {
    fs = ( struct ofp_flow_stats * ) flow->data;
    hton_flow_stats( flow_stats, fs );
    flow_stats = ( struct ofp_flow_stats * ) ( ( char * ) flow_stats + fs->length );
    flow = flow->next;
  }

  if ( f != NULL ) {
    xfree( f );
  }

  return buffer;
}


buffer *
create_aggregate_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                              const uint64_t packet_count, const uint64_t byte_count,
                              const uint32_t flow_count ) {
  uint16_t length;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;
  struct ofp_aggregate_stats_reply *aggregate_stats_reply;

  debug( "Creating an aggregate stats reply "
         "( xid = %#x, flags = %#x, packet_count = %" PRIu64 ", byte_count = %" PRIu64 ", flow_count = %u ).",
         transaction_id, flags, packet_count, byte_count, flow_count );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
                        + sizeof( struct ofp_aggregate_stats_reply ) );
  buffer = create_stats_reply( transaction_id, OFPST_AGGREGATE, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  aggregate_stats_reply = ( struct ofp_aggregate_stats_reply * ) stats_reply->body;
  aggregate_stats_reply->packet_count = htonll( packet_count );
  aggregate_stats_reply->byte_count = htonll( byte_count );
  aggregate_stats_reply->flow_count = htonl( flow_count );
  memset( &aggregate_stats_reply->pad, 0, sizeof( aggregate_stats_reply->pad ) );

  return buffer;
}


buffer *
create_table_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                          const list_element *table_stats_head ) {
  uint16_t length;
  uint16_t n_tables = 0;
  buffer *buffer;
  list_element *t = NULL;
  list_element *table = NULL;
  struct ofp_stats_reply *stats_reply;
  struct ofp_table_stats *ts, *table_stats;

  debug( "Creating a table stats reply ( xid = %#x, flags = %#x ).", transaction_id, flags );

  if ( table_stats_head != NULL ) {
    t = ( list_element * ) xmalloc( sizeof( list_element ) );
    memcpy( t, table_stats_head, sizeof( list_element ) );
  }

  table = t;
  while ( table != NULL ) {
    n_tables++;
    table = table->next;
  }

  debug( "# of tables = %u.", n_tables );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
                        + sizeof( struct ofp_table_stats ) * n_tables );
  buffer = create_stats_reply( transaction_id, OFPST_TABLE, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  table_stats = ( struct ofp_table_stats * ) stats_reply->body;

  table = t;
  while ( table != NULL ) {
    ts = ( struct ofp_table_stats * ) table->data;
    hton_table_stats( table_stats, ts );
    table = table->next;
    table_stats++;
  }

  if ( t != NULL ) {
    xfree( t );
  }

  return buffer;
}


buffer *
create_port_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                         const list_element *port_stats_head ) {
  uint16_t length;
  uint16_t n_ports = 0;
  buffer *buffer;
  list_element *p = NULL;
  list_element *port = NULL;
  struct ofp_stats_reply *stats_reply;
  struct ofp_port_stats *ps, *port_stats;

  debug( "Creating a port stats reply ( xid = %#x, flags = %#x ).", transaction_id, flags );

  if ( port_stats_head != NULL ) {
    p = ( list_element * ) xmalloc( sizeof( list_element ) );
    memcpy( p, port_stats_head, sizeof( list_element ) );
  }

  port = p;
  while ( port != NULL ) {
    n_ports++;
    port = port->next;
  }

  debug( "# of ports = %u.", n_ports );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
                          + sizeof( struct ofp_port_stats ) * n_ports );
  buffer = create_stats_reply( transaction_id, OFPST_PORT, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  port_stats = ( struct ofp_port_stats * ) stats_reply->body;

  port = p;
  while ( port != NULL ) {
    ps = ( struct ofp_port_stats * ) port->data;
    hton_port_stats( port_stats, ps );
    port = port->next;
    port_stats++;
  }

  if ( p != NULL ) {
    xfree( p );
  }

  return buffer;
}


buffer *
create_queue_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                          const list_element *queue_stats_head ) {
  uint16_t length;
  uint16_t n_queues = 0;
  buffer *buffer;
  list_element *q = NULL;
  list_element *queue = NULL;
  struct ofp_stats_reply *stats_reply;
  struct ofp_queue_stats *qs, *queue_stats;

  debug( "Creating a queue stats reply ( xid = %#x, flags = %#x ).", transaction_id, flags );

  if ( queue_stats_head != NULL ) {
    q = ( list_element * ) xmalloc( sizeof( list_element ) );
    memcpy( q, queue_stats_head, sizeof( list_element ) );
  }

  queue = q;
  while ( queue != NULL ) {
    n_queues++;
    queue = queue->next;
  }

  debug( "# of queues = %u.", n_queues );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
                          + sizeof( struct ofp_queue_stats ) * n_queues );
  buffer = create_stats_reply( transaction_id, OFPST_QUEUE, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  queue_stats = ( struct ofp_queue_stats * ) stats_reply->body;

  queue = q;
  while ( queue != NULL ) {
    qs = ( struct ofp_queue_stats * ) queue->data;
    hton_queue_stats( queue_stats, qs );
    queue = queue->next;
    queue_stats++;
  }

  if ( q != NULL ) {
    xfree( q );
  }

  return buffer;
}


buffer *
create_vendor_stats_reply( const uint32_t transaction_id, const uint16_t flags,
                           const uint32_t vendor, const buffer *body ) {
  void *b;
  uint16_t length;
  uint16_t data_length = 0;
  uint32_t *v;
  buffer *buffer;
  struct ofp_stats_reply *stats_reply;

  if ( ( body != NULL ) && ( body->length > 0 ) ) {
    data_length = ( uint16_t ) body->length;
  }

  debug( "Creating a vendor stats reply ( xid = %#x, flags = %#x, vendor = %#x, data length = %u ).",
         transaction_id, flags, vendor, data_length );

  length = ( uint16_t ) ( offsetof( struct ofp_stats_reply, body )
                        + sizeof( uint32_t ) + data_length );
  buffer = create_stats_reply( transaction_id, OFPST_VENDOR, length, flags );
  assert( buffer != NULL );

  stats_reply = ( struct ofp_stats_reply * ) buffer->data;
  v = ( uint32_t * ) stats_reply->body;
  *v = htonl( vendor );

  if ( data_length > 0 ) {
    b = ( void * ) ( ( char * ) v + sizeof( uint32_t ) );
    memcpy( b, body->data, data_length );
  }

  return buffer;
}


buffer *
create_barrier_request( const uint32_t transaction_id ) {
  debug( "Creating a barrier request ( xid = %#x ).", transaction_id );

  return create_header( transaction_id, OFPT_BARRIER_REQUEST, sizeof( struct ofp_header ) );
}


buffer *
create_barrier_reply( const uint32_t transaction_id ) {
  debug( "Creating a barrier reply ( xid = %#x ).", transaction_id );

  return create_header( transaction_id, OFPT_BARRIER_REPLY, sizeof( struct ofp_header ) );
}


buffer *
create_queue_get_config_request( const uint32_t transaction_id, const uint16_t port ) {
  buffer *buffer;
  struct ofp_queue_get_config_request *queue_get_config_request;

  debug( "Creating a queue get config request ( xid = %#x, port = %u ).", transaction_id, port );

  buffer = create_header( transaction_id, OFPT_QUEUE_GET_CONFIG_REQUEST,
                          sizeof( struct ofp_queue_get_config_request ) );
  assert( buffer != NULL );

  queue_get_config_request = ( struct ofp_queue_get_config_request * ) buffer->data;
  queue_get_config_request->port = htons( port );
  memset( queue_get_config_request->pad, 0, sizeof( queue_get_config_request->pad ) );

  return buffer;
}


buffer *
create_queue_get_config_reply( const uint32_t transaction_id, const uint16_t port,
                               const list_element *queues ) {
  uint16_t length;
  uint16_t n_queues = 0;
  uint16_t queues_length = 0;
  buffer *buffer;
  list_element *q, *queue;
  struct ofp_queue_get_config_reply *queue_get_config_reply;
  struct ofp_packet_queue *pq, *packet_queue;

  debug( "Creating a queue get config reply ( xid = %#x, port = %u ).", transaction_id, port );

#ifndef UNIT_TESTING
  assert( queues != NULL );
#endif

  if ( queues != NULL ) {
    q = ( list_element * ) xmalloc( sizeof( list_element ) );
    memcpy( q, queues, sizeof( list_element ) );

    queue = q;
    while ( queue != NULL ) {
      packet_queue = ( struct ofp_packet_queue * ) queue->data;
      queues_length = ( uint16_t ) ( queues_length + packet_queue->len );
      n_queues++;
      queue = queue->next;
    }
  }

  debug( "# of queues = %u.", n_queues );

  length = ( uint16_t ) ( offsetof( struct ofp_queue_get_config_reply, queues ) + queues_length );
  buffer = create_header( transaction_id, OFPT_QUEUE_GET_CONFIG_REPLY, length );
  assert( buffer != NULL );

  queue_get_config_reply = ( struct ofp_queue_get_config_reply * ) buffer->data;
  queue_get_config_reply->port = htons( port );
  memset( &queue_get_config_reply->pad, 0, sizeof( queue_get_config_reply->pad ) );
  packet_queue = ( struct ofp_packet_queue * ) queue_get_config_reply->queues;

  if ( n_queues ) {
    queue = q;
    while ( queue != NULL ) {
      pq = ( struct ofp_packet_queue * ) queue->data;

      hton_packet_queue( packet_queue, pq );

      packet_queue = ( struct ofp_packet_queue * ) ( ( char * ) packet_queue + pq->len );
      queue = queue->next;
    }

    xfree( q );
  }

  return buffer;
}


uint32_t
get_transaction_id( void ) {
  debug( "Generating a transaction id." );

  pthread_mutex_lock( &transaction_id_mutex );

  if ( ( transaction_id & 0xffff ) == 0xffff ) {
    transaction_id = transaction_id & 0xffff0000;
  }
  else {
    transaction_id++;
  }

  pthread_mutex_unlock( &transaction_id_mutex );

  debug( "Transaction id = %#x.", transaction_id );

  return transaction_id;
}


uint64_t
get_cookie( void ) {
  debug( "Generating a cookie." );

  pthread_mutex_lock( &cookie_mutex );

  if ( ( cookie & 0x0000ffffffffffffULL ) == 0x0000ffffffffffffULL ) {
    cookie = cookie & 0xffff000000000000ULL;
  }
  else {
    cookie++;
  }

  pthread_mutex_unlock( &cookie_mutex );

  debug( "Cookie = %#" PRIx64 ".", cookie );

  return cookie;
}


openflow_actions *
create_actions() {
  openflow_actions *actions;

  debug( "Creating an empty actions list." );

  actions = ( openflow_actions * ) xmalloc( sizeof( openflow_actions ) );

  if ( create_list( &actions->list ) == false ) {
    assert( 0 );
  }

  actions->n_actions = 0;

  return actions;
}


bool
delete_actions( openflow_actions *actions ) {
  list_element *element;

  debug( "Deleting an actions list." );

  assert( actions != NULL );

  debug( "# of actions = %d.", actions->n_actions );

  element = actions->list;
  while ( element != NULL ) {
    xfree( element->data );
    element = element->next;
  }

  delete_list( actions->list );
  xfree( actions );

  return true;
}


bool
append_action_output( openflow_actions *actions, const uint16_t port, const uint16_t max_len ) {
  bool ret;
  struct ofp_action_output *action_output;

  debug( "Appending an output action ( port = %u, max_len = %u ).", port, max_len );

  assert( actions != NULL );

  action_output = ( struct ofp_action_output * ) xcalloc( 1, sizeof( struct ofp_action_output ) );
  action_output->type = OFPAT_OUTPUT;
  action_output->len = sizeof( struct ofp_action_output );
  action_output->port = port;
  action_output->max_len = max_len;

  ret = append_to_tail( &actions->list, ( void * ) action_output );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_set_vlan_vid( openflow_actions *actions, const uint16_t vlan_vid ) {
  bool ret;
  struct ofp_action_vlan_vid *action_vlan_vid;

  debug( "Appending a set vlan action ( vlan_vid = %#x ).", vlan_vid );

  assert( actions != NULL );
  assert( ( vlan_vid & ~VLAN_VID_MASK ) == 0 );

  action_vlan_vid = ( struct ofp_action_vlan_vid * ) xcalloc( 1, sizeof( struct ofp_action_vlan_vid ) );
  action_vlan_vid->type = OFPAT_SET_VLAN_VID;
  action_vlan_vid->len = sizeof( struct ofp_action_vlan_vid );
  action_vlan_vid->vlan_vid = vlan_vid;

  ret = append_to_tail( &actions->list, ( void * ) action_vlan_vid );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_set_vlan_pcp( openflow_actions *actions, const uint8_t vlan_pcp ) {
  bool ret;
  struct ofp_action_vlan_pcp *action_vlan_pcp;

  debug( "Appending a set vlan pcp action ( vlan_pcp = %#x ).", vlan_pcp );

  assert( actions != NULL );
  assert( ( vlan_pcp & ~VLAN_PCP_MASK ) == 0 );

  action_vlan_pcp = ( struct ofp_action_vlan_pcp * ) xcalloc( 1, sizeof( struct ofp_action_vlan_pcp ) );
  action_vlan_pcp->type = OFPAT_SET_VLAN_PCP;
  action_vlan_pcp->len = sizeof( struct ofp_action_vlan_pcp );
  action_vlan_pcp->vlan_pcp = vlan_pcp;

  ret = append_to_tail( &actions->list, ( void * ) action_vlan_pcp );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_strip_vlan( openflow_actions *actions ) {
  bool ret;
  struct ofp_action_header *action_strip_vlan;

  debug( "Appending a strip vlan action." );

  assert( actions != NULL );

  action_strip_vlan = ( struct ofp_action_header * ) xcalloc( 1, sizeof( struct ofp_action_header ) );
  action_strip_vlan->type = OFPAT_STRIP_VLAN;
  action_strip_vlan->len = sizeof( struct ofp_action_header );

  ret = append_to_tail( &actions->list, ( void * ) action_strip_vlan );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


static bool
append_action_set_dl_addr( openflow_actions *actions, const uint16_t type,
                           const uint8_t hw_addr[ OFP_ETH_ALEN ] ) {
  bool ret;
  struct ofp_action_dl_addr *action_dl_addr;

  debug( "Appending a set dl_src/dl_dst action ( type = %#x, hw_addr = %02x:%02x:%02x:%02x:%02x:%02x ).",
         type, hw_addr[ 0 ], hw_addr[ 1 ], hw_addr[ 2 ], hw_addr[ 3 ], hw_addr[ 4 ], hw_addr[ 5 ] );

  assert( actions != NULL );

  action_dl_addr = ( struct ofp_action_dl_addr * ) xcalloc( 1, sizeof( struct ofp_action_dl_addr ) );
  action_dl_addr->type = type;
  action_dl_addr->len = sizeof( struct ofp_action_dl_addr );
  memcpy( action_dl_addr->dl_addr, hw_addr, OFP_ETH_ALEN );

  ret = append_to_tail( &actions->list, ( void * ) action_dl_addr );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_set_dl_src( openflow_actions *actions, const uint8_t hw_addr[ OFP_ETH_ALEN ] ) {
  debug( "Appending a set dl_src action ( hw_addr = %02x:%02x:%02x:%02x:%02x:%02x ).",
         hw_addr[ 0 ], hw_addr[ 1 ], hw_addr[ 2 ], hw_addr[ 3 ], hw_addr[ 4 ], hw_addr[ 5 ] );

  assert( actions != NULL );
  return append_action_set_dl_addr( actions, OFPAT_SET_DL_SRC, hw_addr );
}


bool
append_action_set_dl_dst( openflow_actions *actions, const uint8_t hw_addr[ OFP_ETH_ALEN ] ) {
  debug( "Appending a set dl_dst action ( hw_addr = %02x:%02x:%02x:%02x:%02x:%02x ).",
         hw_addr[ 0 ], hw_addr[ 1 ], hw_addr[ 2 ], hw_addr[ 3 ], hw_addr[ 4 ], hw_addr[ 5 ] );

  assert( actions != NULL );
  return append_action_set_dl_addr( actions, OFPAT_SET_DL_DST, hw_addr );
}


static bool
append_action_set_nw_addr( openflow_actions *actions, const uint16_t type, const uint32_t nw_addr ) {
  bool ret;
  char addr_str[ 16 ];
  struct in_addr addr;
  struct ofp_action_nw_addr *action_nw_addr;

  addr.s_addr = htonl( nw_addr );
  memset( addr_str, '\0', sizeof( addr_str ) );
  inet_ntop( AF_INET, &addr, addr_str, sizeof( addr_str ) );
  debug( "Appending a set nw_src/nw_dst action ( type = %#x, nw_addr = %s ).", type, addr_str );

  assert( actions != NULL );

  action_nw_addr = ( struct ofp_action_nw_addr * ) xcalloc( 1, sizeof( struct ofp_action_nw_addr ) );
  action_nw_addr->type = type;
  action_nw_addr->len = sizeof( struct ofp_action_nw_addr );
  action_nw_addr->nw_addr = nw_addr;

  ret = append_to_tail( &actions->list, ( void * ) action_nw_addr );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_set_nw_src( openflow_actions *actions, const uint32_t nw_addr ) {
  char addr_str[ 16 ];
  struct in_addr addr;

  addr.s_addr = htonl( nw_addr );
  memset( addr_str, '\0', sizeof( addr_str ) );
  inet_ntop( AF_INET, &addr, addr_str, sizeof( addr_str ) );
  debug( "Appending a set nw_src action ( nw_addr = %s ).", addr_str );

  assert( actions != NULL );
  return append_action_set_nw_addr( actions, OFPAT_SET_NW_SRC, nw_addr );
}


bool
append_action_set_nw_dst( openflow_actions *actions, const uint32_t nw_addr ) {
  char addr_str[ 16 ];
  struct in_addr addr;

  addr.s_addr = htonl( nw_addr );
  memset( addr_str, '\0', sizeof( addr_str ) );
  inet_ntop( AF_INET, &addr, addr_str, sizeof( addr_str ) );
  debug( "Appending a set nw_dst action ( nw_addr = %s ).", addr_str );

  assert( actions != NULL );
  return append_action_set_nw_addr( actions, OFPAT_SET_NW_DST, nw_addr );
}


bool
append_action_set_nw_tos( openflow_actions *actions, const uint8_t nw_tos ) {
  bool ret;
  struct ofp_action_nw_tos *action_nw_tos;

  debug( "Appending a set nw_tos action ( nw_tos = %#x ).", nw_tos );

  assert( actions != NULL );
  assert( ( nw_tos & ~NW_TOS_MASK ) == 0 );

  action_nw_tos = ( struct ofp_action_nw_tos * ) xcalloc( 1, sizeof( struct ofp_action_nw_tos ) );
  action_nw_tos->type = OFPAT_SET_NW_TOS;
  action_nw_tos->len = sizeof( struct ofp_action_nw_tos );
  action_nw_tos->nw_tos = nw_tos;

  ret = append_to_tail( &actions->list, ( void * ) action_nw_tos );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


static bool
append_action_set_tp_port( openflow_actions *actions, const uint16_t type, const uint16_t tp_port ) {
  bool ret;
  struct ofp_action_tp_port *action_tp_port;

  debug( "Appending a set tp_src/tp_dst action ( type = %#x, tp_port = %u ).", type, tp_port );

  assert( actions != NULL );

  action_tp_port = ( struct ofp_action_tp_port * ) xcalloc( 1, sizeof( struct ofp_action_tp_port ) );
  action_tp_port->type = type;
  action_tp_port->len = sizeof( struct ofp_action_tp_port );
  action_tp_port->tp_port = tp_port;

  ret = append_to_tail( &actions->list, ( void * ) action_tp_port );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_set_tp_src( openflow_actions *actions, const uint16_t tp_port ) {
  debug( "Appending a set tp_src action ( tp_port = %u ).", tp_port );

  assert( actions != NULL );
  return append_action_set_tp_port( actions, OFPAT_SET_TP_SRC, tp_port );
}


bool
append_action_set_tp_dst( openflow_actions *actions, const uint16_t tp_port ) {
  debug( "Appending a set tp_dst action ( tp_port = %u ).", tp_port );

  assert( actions != NULL );
  return append_action_set_tp_port( actions, OFPAT_SET_TP_DST, tp_port );
}


bool
append_action_enqueue( openflow_actions *actions, const uint16_t port, const uint32_t queue_id ) {
  bool ret;
  struct ofp_action_enqueue *action_enqueue;

  debug( "Appending an enqueue action ( port = %u, queue_id = %u ).", port, queue_id );

  assert( actions != NULL );

  action_enqueue = ( struct ofp_action_enqueue * ) xcalloc( 1, sizeof( struct ofp_action_enqueue ) );
  action_enqueue->type = OFPAT_ENQUEUE;
  action_enqueue->len = sizeof( struct ofp_action_enqueue );
  action_enqueue->port = port;
  action_enqueue->queue_id = queue_id;

  ret = append_to_tail( &actions->list, ( void * ) action_enqueue );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


bool
append_action_vendor( openflow_actions *actions, const uint32_t vendor, const buffer *body ) {
  bool ret;
  uint16_t body_length = 0;
  struct ofp_action_vendor_header *action_vendor;

  if ( ( body != NULL ) && ( body->length > 0 ) ) {
    body_length = ( uint16_t ) body->length;
  }

  debug( "Appending a vendor action ( vendor = %#" PRIx32 ", body length = %u ).", vendor, body_length );

  assert( actions != NULL );
  assert( ( body_length % 8 ) == 0 );

  action_vendor = ( struct ofp_action_vendor_header * )
                  xcalloc( 1, sizeof( struct ofp_action_vendor_header ) + body_length );
  action_vendor->type = OFPAT_VENDOR;
  action_vendor->len = ( uint16_t ) ( sizeof( struct ofp_action_vendor_header ) + body_length );
  action_vendor->vendor = vendor;

  if ( body_length > 0 ) {
    memcpy( ( char * ) action_vendor + sizeof( struct ofp_action_vendor_header ), body->data, body_length );
  }

  ret = append_to_tail( &actions->list, ( void * ) action_vendor );
  if ( ret ) {
    actions->n_actions++;
  }

  return ret;
}


static int
validate_header( const buffer *message, const uint8_t type,
                 const uint16_t min_length, const uint16_t max_length ) {
  struct ofp_header *header;

  assert( message != NULL );
  if ( message->length < sizeof( struct ofp_header ) ) {
    return ERROR_TOO_SHORT_MESSAGE;
  }

  header = ( struct ofp_header * ) message->data;
  if ( header->version != OFP_VERSION ) {
    return ERROR_UNSUPPORTED_VERSION;
  }
  if ( header->type > OFPT_QUEUE_GET_CONFIG_REPLY ) {
    return ERROR_UNDEFINED_TYPE;
  }
  if ( header->type != type ) {
    return ERROR_INVALID_TYPE;
  }
  if ( ntohs( header->length ) > max_length ) {
    return ERROR_TOO_LONG_MESSAGE;
  }
  else if ( ntohs( header->length ) < min_length ) {
    return ERROR_TOO_SHORT_MESSAGE;
  }
  if ( ntohs( header->length ) < message->length ) {
    return ERROR_TOO_LONG_MESSAGE;
  }
  else if ( ntohs( header->length ) > message->length ) {
    return ERROR_TOO_SHORT_MESSAGE;
  }

  if ( message->length > max_length ) {
    return ERROR_TOO_LONG_MESSAGE;
  }

  return 0;
}


int
validate_hello( const buffer *message ) {
  assert( message != NULL );
  return validate_header( message, OFPT_HELLO, sizeof( struct ofp_header ), sizeof( struct ofp_header ) );
}


int
validate_error( const buffer *message ) {
  int ret;

  assert( message != NULL );

  ret = validate_header( message, OFPT_ERROR, sizeof( struct ofp_error_msg ), UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_echo_request( const buffer *message ) {
  int ret;
  struct ofp_header *header;

  assert( message != NULL );

  ret = validate_header( message, OFPT_ECHO_REQUEST, sizeof( struct ofp_header ), UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  header = ( struct ofp_header * ) message->data;
  if ( message->length != ntohs( header->length ) ) {
    return ERROR_INVALID_LENGTH;
  }

  return 0;
}


int
validate_echo_reply( const buffer *message ) {
  int ret;
  struct ofp_header *header;

  assert( message != NULL );

  ret = validate_header( message, OFPT_ECHO_REPLY, sizeof( struct ofp_header ), UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  header = ( struct ofp_header * ) message->data;
  if ( message->length != ntohs( header->length ) ) {
    return ERROR_INVALID_LENGTH;
  }

  return 0;
}


int
validate_vendor( const buffer *message ) {
  int ret;
  struct ofp_vendor_header *vendor_header;

  assert( message != NULL );

  ret = validate_header( message, OFPT_VENDOR, sizeof( struct ofp_vendor_header ), UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  vendor_header = ( struct ofp_vendor_header * ) message->data;
  if ( message->length != ntohs( vendor_header->header.length ) ) {
    return ERROR_INVALID_LENGTH;
  }

  return 0;
}


int
validate_features_request( const buffer *message ) {
  assert( message != NULL );
  return validate_header( message, OFPT_FEATURES_REQUEST, sizeof( struct ofp_header ),
                          sizeof( struct ofp_header ) );
}


static int
validate_phy_port_no( const uint16_t port_no ) {
  if ( ( port_no == 0 ) || ( ( port_no > OFPP_MAX ) && ( port_no < OFPP_IN_PORT ) ) ) {
    return ERROR_INVALID_PORT_NO;
  }

  return 0;
}


static int
validate_phy_port( struct ofp_phy_port *port ) {
  int ret;
  struct ofp_phy_port port_h;

  assert( port != NULL );

  ntoh_phy_port( &port_h, port );

  ret = validate_phy_port_no( port_h.port_no );
  if ( ret < 0 ) {
    return ret;
  }

  if ( ( port_h.config & ( uint32_t ) ~PORT_CONFIG ) != 0 ) {
    return ERROR_INVALID_PORT_CONFIG;
  }
  if ( ( port_h.state & ( uint32_t ) ~PORT_STATE ) != 0 ) {
    return ERROR_INVALID_PORT_STATE;
  }
  if ( ( port_h.curr & ( uint32_t ) ~PORT_FEATURES ) != 0
       || ( port_h.advertised & ( uint32_t ) ~PORT_FEATURES ) != 0
       || ( port_h.supported & ( uint32_t ) ~PORT_FEATURES ) != 0
       || ( port_h.peer & ( uint32_t ) ~PORT_FEATURES ) != 0 ) {
    return ERROR_INVALID_PORT_FEATURES;
  }

  return 0;
}


static int
validate_phy_ports( struct ofp_phy_port *ports, const int n_ports ) {
  int i;
  int ret;
  struct ofp_phy_port *port;

  assert( ports != NULL );
  assert( n_ports );

  port = ports;
  for ( i = 0; i < n_ports; i++ ) {
    ret = validate_phy_port( port );
    if ( ret < 0 ) {
      return ret;
    }
    port++;
  }

  return 0;
}


int
validate_features_reply( const buffer *message ) {
  void *p;
  int ret;
  int n_ports;
  uint16_t port_length;
  struct ofp_switch_features *switch_features;

  assert( message != NULL );

  ret = validate_header( message, OFPT_FEATURES_REPLY, sizeof( struct ofp_switch_features ), UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  switch_features = ( struct ofp_switch_features * ) message->data;

  // switch_features->datapath_id
  // switch_features->n_buffers

  if ( switch_features->n_tables == 0 ) {
    return ERROR_NO_TABLE_AVAILABLE;
  }

  port_length = ( uint16_t ) ( ntohs( switch_features->header.length )
                             - sizeof( struct ofp_switch_features ) );
  if ( port_length % sizeof( struct ofp_phy_port ) != 0 ) {
    return ERROR_INVALID_LENGTH;
  }

  if ( port_length > 0 ) {
    p = ( void * ) ( ( char * ) message->data + offsetof( struct ofp_switch_features, ports ) );
    n_ports = port_length / sizeof( struct ofp_phy_port );

    ret = validate_phy_ports( p, n_ports );
    if ( ret < 0 ) {
      return ret;
    }
  }

  return 0;
}


int
validate_get_config_request( const buffer *message ) {
  assert( message != NULL );
  return validate_header( message, OFPT_GET_CONFIG_REQUEST, sizeof( struct ofp_header ),
                          sizeof( struct ofp_header ) );
}


static int
validate_switch_config( const buffer *message, const uint8_t type ) {
  int ret;
  struct ofp_switch_config *switch_config;

  assert( message != NULL );
  assert( ( type == OFPT_GET_CONFIG_REPLY ) || ( type == OFPT_SET_CONFIG ) );

  ret = validate_header( message, type, sizeof( struct ofp_switch_config ),
                         sizeof( struct ofp_switch_config ) );
  if ( ret < 0 ) {
    return ret;
  }

  switch_config = ( struct ofp_switch_config * ) message->data;
  if ( ntohs( switch_config->flags ) > OFPC_FRAG_MASK ) {
    return ERROR_INVALID_SWITCH_CONFIG;
  }

  // switch_config->miss_send_len

  return 0;
}


int
validate_get_config_reply( const buffer *message ) {
  assert( message != NULL );
  return validate_switch_config( message, OFPT_GET_CONFIG_REPLY );
}


int
validate_set_config( const buffer *message ) {
  assert( message != NULL );
  return validate_switch_config( message, OFPT_SET_CONFIG );
}


int
validate_packet_in( const buffer *message ) {
  int ret;
  uint16_t data_length;
  struct ofp_packet_in *packet_in;

  assert( message != NULL );

  ret = validate_header( message, OFPT_PACKET_IN, offsetof( struct ofp_packet_in, data ), UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  packet_in = ( struct ofp_packet_in * ) message->data;

  // packet_in->buffer_id
  // packet_in->total_len
  // packet_in->in_port

  ret = validate_phy_port_no( ntohs( packet_in->in_port ) );
  if ( ret < 0 ) {
    return ret;
  }

  if ( packet_in->reason > OFPR_ACTION ) {
    return ERROR_INVALID_PACKET_IN_REASON;
  }

  data_length = ( uint16_t ) ( ntohs( packet_in->header.length ) - offsetof( struct ofp_packet_in, data ) );
  if ( data_length > 0 ) {
    // FIXME: it may be better to check if this is a valid Ethernet frame or not.
  }

  return 0;
}


static int
validate_wildcards( const uint32_t wildcards ) {
  if ( ( wildcards & ( uint32_t ) ~OFPFW_ALL ) != 0 ) {
    return ERROR_INVALID_WILDCARDS;
  }

  return 0;
}


static int
validate_vlan_vid( const uint16_t vid ) {
  if ( ( vid != UINT16_MAX ) && ( ( vid & ~VLAN_VID_MASK ) != 0 ) ) {
    return ERROR_INVALID_VLAN_VID;
  }

  return 0;
}


static int
validate_vlan_pcp( const uint8_t pcp ) {
  if ( ( pcp & ~VLAN_PCP_MASK ) != 0 ) {
    return ERROR_INVALID_VLAN_PCP;
  }

  return 0;
}


static int
validate_nw_tos( const uint8_t tos ) {
  if ( ( tos & ~NW_TOS_MASK ) != 0 ) {
    return ERROR_INVALID_NW_TOS;
  }

  return 0;
}


static int
validate_match( const struct ofp_match match ) {
  int ret;

  ret = validate_wildcards( match.wildcards );
  if ( ret < 0 ) {
    return ret;
  }

  ret = validate_vlan_vid( match.dl_vlan );
  if ( ret < 0 ) {
    return ret;
  }

  ret = validate_vlan_pcp( match.dl_vlan_pcp );
  if ( ret < 0 ) {
    return ret;
  }

  ret = validate_nw_tos( match.nw_tos );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_flow_removed( const buffer *message ) {
  int ret;
  struct ofp_match match;
  struct ofp_flow_removed *flow_removed;

  assert( message != NULL );

  ret = validate_header( message, OFPT_FLOW_REMOVED, sizeof( struct ofp_flow_removed ),
                         sizeof( struct ofp_flow_removed ) );
  if ( ret < 0 ) {
    return ret;
  }

  flow_removed = ( struct ofp_flow_removed * ) message->data;

  ntoh_match( &match, &flow_removed->match );

  ret = validate_match( match );
  if ( ret < 0 ) {
    return ret;
  }

  // flow_removed->cookie

  if ( ( ( match.wildcards & OFPFW_ALL ) == 0 ) && ( ntohs( flow_removed->priority ) != UINT16_MAX ) ) {
    return ERROR_INVALID_FLOW_PRIORITY;
  }

  if ( flow_removed->reason > OFPRR_DELETE ) {
    return ERROR_INVALID_FLOW_REMOVED_REASON;
  }

  // flow_removed->duration_sec
  // flow_removed->duration_nsec
  // flow_removed->idle_timeout
  // flow_removed->packet_count
  // flow_removed->byte_count

  return 0;
}


int
validate_port_status( const buffer *message ) {
  int ret;
  struct ofp_port_status *port_status;

  assert( message != NULL );

  ret = validate_header( message, OFPT_PORT_STATUS, sizeof( struct ofp_port_status ),
                         sizeof( struct ofp_port_status ) );
  if ( ret < 0 ) {
    return ret;
  }

  port_status = ( struct ofp_port_status * ) message->data;
  if ( port_status->reason > OFPPR_MODIFY ) {
    return ERROR_INVALID_PORT_STATUS_REASON;
  }

  ret = validate_phy_port( &port_status->desc );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_packet_out( const buffer *message ) {
  int ret;
  uint16_t data_length;
  struct ofp_packet_out *packet_out;

  assert( message != NULL );

  ret = validate_header( message, OFPT_PACKET_OUT, offsetof( struct ofp_packet_out, actions ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  packet_out = ( struct ofp_packet_out * ) message->data;

  ret = validate_phy_port_no( ntohs( packet_out->in_port ) );
  if ( ret < 0 ) {
    return ret;
  }

  if ( ntohs( packet_out->actions_len ) > 0 ) {
    ret = validate_actions( packet_out->actions, ntohs( packet_out->actions_len ) );
    if ( ret < 0 ) {
      return ret;
    }
  }

  data_length = ( uint16_t ) ( ntohs( packet_out->header.length )
                             - offsetof( struct ofp_packet_out, actions )
                             - ntohs( packet_out->actions_len ) );

  if ( data_length > 0 ) {
    // FIXME: it may be better to check if this is a valid Ethernet frame or not.
  }

  return 0;
}


int
validate_flow_mod( const buffer *message ) {
  int ret;
  uint16_t actions_length;
  struct ofp_match match;
  struct ofp_flow_mod *flow_mod;

  assert( message != NULL );

  ret = validate_header( message, OFPT_FLOW_MOD, offsetof( struct ofp_flow_mod, actions ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  flow_mod = ( struct ofp_flow_mod * ) message->data;

  ntoh_match( &match, &flow_mod->match );

  ret = validate_match( match );
  if ( ret < 0 ) {
    return ret;
  }

  // flow_mod->cookie

  if ( ntohs( flow_mod->command ) > OFPFC_DELETE_STRICT ) {
    return ERROR_UNDEFINED_FLOW_MOD_COMMAND;
  }

  // flow_mod->idle_timeout
  // flow_mod->hard_timeout

  if ( ( ( match.wildcards & OFPFW_ALL ) == 0 ) && ( ntohs( flow_mod->priority ) != UINT16_MAX ) ) {
    return ERROR_INVALID_FLOW_PRIORITY;
  }

  // flow_mod->buffer_id

  if ( ( ntohs( flow_mod->command ) == OFPFC_DELETE )
       || ( ntohs( flow_mod->command ) == OFPFC_DELETE_STRICT ) ) {
    if ( ntohs( flow_mod->out_port ) != OFPP_NONE ) {
      ret = validate_phy_port_no( ntohs( flow_mod->out_port ) );
      if ( ret < 0 ) {
        return ret;
      }
    }
  }

  if ( ( ntohs( flow_mod->flags ) & ~FLOW_MOD_FLAGS ) != 0 ) {
    return ERROR_INVALID_FLOW_MOD_FLAGS;
  }

  actions_length = ( uint16_t ) ( ntohs( flow_mod->header.length )
                                - offsetof( struct ofp_flow_mod, actions ) );

  if ( actions_length > 0 ) {
    ret = validate_actions( flow_mod->actions, actions_length );
    if ( ret < 0 ) {
      return ret;
    }

  }

  return 0;
}


int
validate_port_mod( const buffer *message ) {
  int ret;
  struct ofp_port_mod *port_mod;

  assert( message != NULL );

  ret = validate_header( message, OFPT_PORT_MOD, sizeof( struct ofp_port_mod ),
                         sizeof( struct ofp_port_mod ) );
  if ( ret < 0 ) {
    return ret;
  }

  port_mod = ( struct ofp_port_mod * ) message->data;

  ret = validate_phy_port_no( ntohs( port_mod->port_no ) );
  if ( ret < 0 ) {
    return ret;
  }
  if ( ( ntohs( port_mod->port_no ) > OFPP_MAX ) && ( ntohs( port_mod->port_no ) != OFPP_LOCAL ) ) {
    return ERROR_INVALID_PORT_NO;
  }

  // port_mod->hw_addr

  if ( ( ntohl( port_mod->config ) & ( uint32_t ) ~PORT_CONFIG ) != 0 ) {
    return ERROR_INVALID_PORT_CONFIG;
  }
  if ( ( ntohl( port_mod->mask ) & ( uint32_t ) ~PORT_CONFIG ) != 0 ) {
    return ERROR_INVALID_PORT_MASK;
  }
  if ( ( ntohl( port_mod->advertise ) & ( uint32_t ) ~PORT_CONFIG ) != 0 ) {
    return ERROR_INVALID_PORT_FEATURES;
  }

  return 0;
}


int
validate_desc_stats_request( const buffer *message ) {
  int ret;
  struct ofp_stats_request *stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST, sizeof( struct ofp_stats_request ),
                         sizeof( struct ofp_stats_request ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_DESC ) {
    return ERROR_INVALID_STATS_TYPE;
  }
  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  return 0;
}


int
validate_flow_stats_request( const buffer *message ) {
  int ret;
  struct ofp_match match;
  struct ofp_stats_request *stats_request;
  struct ofp_flow_stats_request *flow_stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST,
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_flow_stats_request ),
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_flow_stats_request ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_FLOW ) {
    return ERROR_INVALID_STATS_TYPE;
  }

  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  flow_stats_request = ( struct ofp_flow_stats_request * ) stats_request->body;
  ntoh_match( &match, &flow_stats_request->match );

  ret = validate_match( match );
  if ( ret < 0 ) {
    return ret;
  }

  // flow_stats_request->table_id

  ret = validate_phy_port_no( ntohs( flow_stats_request->out_port ) );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_aggregate_stats_request( const buffer *message ) {
  int ret;
  struct ofp_match match;
  struct ofp_stats_request *stats_request;
  struct ofp_aggregate_stats_request *aggregate_stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST,
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_aggregate_stats_request ),
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_aggregate_stats_request ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_AGGREGATE ) {
    return ERROR_INVALID_STATS_TYPE;
  }
  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  aggregate_stats_request = ( struct ofp_aggregate_stats_request * ) stats_request->body;
  ntoh_match( &match, &aggregate_stats_request->match );

  ret = validate_match( match );
  if ( ret < 0 ) {
    return ret;
  }

  // aggregate_stats_request->table_id

  ret = validate_phy_port_no( ntohs( aggregate_stats_request->out_port ) );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_table_stats_request( const buffer *message ) {
  int ret;
  struct ofp_stats_request *stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST, offsetof( struct ofp_stats_request, body ),
                         offsetof( struct ofp_stats_request, body ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_TABLE ) {
    return ERROR_INVALID_STATS_TYPE;
  }
  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  return 0;
}


int
validate_port_stats_request( const buffer *message ) {
  int ret;
  struct ofp_stats_request *stats_request;
  struct ofp_port_stats_request *port_stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST,
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_port_stats_request ),
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_port_stats_request ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_PORT ) {
    return ERROR_INVALID_STATS_TYPE;
  }
  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  port_stats_request = ( struct ofp_port_stats_request * ) stats_request->body;

  ret = validate_phy_port_no( ntohs( port_stats_request->port_no ) );
  if ( ret < 0 ) {
    return ret;
  }

  if ( ntohs( port_stats_request->port_no ) > OFPP_MAX
       && ntohs( port_stats_request->port_no ) != OFPP_NONE
       && ntohs( port_stats_request->port_no ) != OFPP_LOCAL ) {
    return ERROR_INVALID_PORT_NO;
  }

  return 0;
}


int
validate_queue_stats_request( const buffer *message ) {
  int ret;
  struct ofp_stats_request *stats_request;
  struct ofp_queue_stats_request *queue_stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST,
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_queue_stats_request ),
                         offsetof( struct ofp_stats_request, body )
                         + sizeof( struct ofp_queue_stats_request ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_QUEUE ) {
    return ERROR_INVALID_STATS_TYPE;
  }
  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  queue_stats_request = ( struct ofp_queue_stats_request * ) stats_request->body;

  ret = validate_phy_port_no( ntohs( queue_stats_request->port_no ) );
  if ( ret < 0 ) {
    return ret;
  }

  // queue_stats_request->queue_id
  return 0;
}


int
validate_vendor_stats_request( const buffer *message ) {
  int ret;
  struct ofp_stats_request *stats_request;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REQUEST,
                         offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  stats_request = ( struct ofp_stats_request * ) message->data;

  if ( ntohs( stats_request->type ) != OFPST_VENDOR ) {
    return ERROR_INVALID_STATS_TYPE;
  }
  if ( ntohs( stats_request->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REQUEST_FLAGS;
  }

  // vendor_id
  return 0;
}


int
validate_stats_request( const buffer *message ) {
  struct ofp_stats_request *request;

  assert( message != NULL );

  request = ( struct ofp_stats_request * ) message->data;

  // TODO: if ( request->header.type != OFPT_STATS_REQUEST ) { ... }

  switch ( ntohs( request->type ) ) {
  case OFPST_DESC:
    return validate_desc_stats_request( message );
  case OFPST_FLOW:
    return validate_flow_stats_request( message );
  case OFPST_AGGREGATE:
    return validate_aggregate_stats_request( message );
  case OFPST_TABLE:
    return validate_table_stats_request( message );
  case OFPST_PORT:
    return validate_port_stats_request( message );
  case OFPST_QUEUE:
    return validate_queue_stats_request( message );
  case OFPST_VENDOR:
    return validate_vendor_stats_request( message );
  default:
    break;
  }

  return ERROR_UNSUPPORTED_STATS_TYPE;
}


int
validate_desc_stats_reply( const buffer *message ) {
  int ret;
  struct ofp_stats_reply *stats_reply;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY,
                         offsetof( struct ofp_stats_reply, body ),
                         offsetof( struct ofp_stats_reply, body ) + sizeof( struct ofp_desc_stats ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;
  if ( ntohs( stats_reply->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  return 0;
}


int
validate_flow_stats_reply( const buffer *message ) {
  int ret;
  uint16_t offset;
  uint16_t flow_length;
  uint16_t actions_length;
  struct ofp_stats_reply *stats_reply;
  struct ofp_flow_stats *flow_stats;
  struct ofp_action_header *actions_head;
  struct ofp_match match;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY, offsetof( struct ofp_stats_reply, body ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;
  if ( ( ntohs( stats_reply->flags ) & ~OFPSF_REPLY_MORE ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  flow_length = ( uint16_t ) ( ntohs( stats_reply->header.length )
                              - offsetof( struct ofp_stats_reply, body ) );
  offset = offsetof( struct ofp_stats_reply, body );
  flow_stats = ( struct ofp_flow_stats * ) ( ( char * ) message->data + offset );

  while ( flow_length > 0 ) {
    // flow_stats->length
    // flow_stats->table_id

    ntoh_match( &match, &flow_stats->match );

    ret = validate_match( match );
    if ( ret < 0 ) {
      return ret;
    }

    // flow_stats->duration_sec
    // flow_stats->duration_nsec

    if ( ( ( match.wildcards & OFPFW_ALL ) == 0 ) && ( ntohs( flow_stats->priority ) < UINT16_MAX ) ) {
      return ERROR_INVALID_FLOW_PRIORITY;
    }

    // flow_stats->idle_timeout
    // flow_stats->hard_timeout
    // flow_stats->cookie
    // flow_stats->packet_count
    // flow_stats->byte_count

    actions_length = ( uint16_t ) ( ntohs( flow_stats->length )
                                   - offsetof( struct ofp_flow_stats, actions ) );

    if ( actions_length > 0 ) {
      actions_head = ( struct ofp_action_header * ) ( ( char * ) flow_stats
                                                    + offsetof( struct ofp_flow_stats, actions ) );

      ret = validate_actions( actions_head, actions_length );
      if ( ret < 0 ) {
        return ret;
      }
    }

    flow_length = ( uint16_t ) ( flow_length - ntohs( flow_stats->length ) );
    flow_stats = ( struct ofp_flow_stats * ) ( ( char * ) flow_stats + ntohs( flow_stats->length ) );
  }

  return 0;
}


int
validate_aggregate_stats_reply( const buffer *message ) {
  int ret;
  struct ofp_stats_reply *stats_reply;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY,
                         offsetof( struct ofp_stats_reply, body ),
                         offsetof( struct ofp_stats_reply, body ) + sizeof( struct ofp_aggregate_stats_reply ) );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;
  if ( ntohs( stats_reply->flags ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  // uint16_t offset = offsetof( struct ofp_stats_reply, body );
  // struct ofp_aggregate_stats_reply *aggregate_stats = ( struct ofp_aggregate_stats_reply * ) ( ( char * ) message->data + offset );

  // aggregate_stats->packet_count
  // aggregate_stats->byte_count
  // aggregate_stats->flow_count

  return 0;
}


int
validate_table_stats_reply( const buffer *message ) {
  int i;
  int ret;
  uint16_t tables_length;
  uint16_t n_tables;
  uint16_t offset;
  struct ofp_stats_reply *stats_reply;
  struct ofp_table_stats *table_stats;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY,
                         offsetof( struct ofp_stats_reply, body ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;
  if ( ( ntohs( stats_reply->flags ) & ~OFPSF_REPLY_MORE ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  tables_length = ( uint16_t ) ( ntohs( stats_reply->header.length )
                                - offsetof( struct ofp_stats_reply, body ) );
  if ( tables_length % sizeof( struct ofp_table_stats ) != 0 ) {
    return ERROR_INVALID_LENGTH;
  }

  offset = offsetof( struct ofp_stats_reply, body );
  table_stats = ( struct ofp_table_stats * ) ( ( char * ) message->data + offset );

  n_tables = tables_length / sizeof( struct ofp_table_stats );

  for ( i = 0; i < n_tables; i++ ) {
    // table_stats->table_id

    ret = validate_wildcards( ntohl( table_stats->wildcards ) );
    if ( ret < 0 ) {
      return ret;
    }

    // table_stats->max_entries
    // table_stats->active_count
    // table_stats->lookup_count
    // table_stats->matched_count

    table_stats++;
  }

  return 0;
}


int
validate_port_stats_reply( const buffer *message ) {
  int i;
  int ret;
  uint16_t ports_length;
  uint16_t n_ports;
  uint16_t offset;
  struct ofp_stats_reply *stats_reply;
  struct ofp_port_stats *port_stats;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY,
                         offsetof( struct ofp_stats_reply, body ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;
  if ( ( ntohs( stats_reply->flags ) & ~OFPSF_REPLY_MORE ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  ports_length = ( uint16_t ) ( ntohs( stats_reply->header.length )
                              - offsetof( struct ofp_stats_reply, body ) );
  if ( ports_length % sizeof( struct ofp_port_stats ) != 0 ) {
    return ERROR_INVALID_LENGTH;
  }

  offset = offsetof( struct ofp_stats_reply, body );
  port_stats = ( struct ofp_port_stats * ) ( ( char * ) message->data + offset );

  n_ports = ports_length / sizeof( struct ofp_port_stats );
  for ( i = 0; i < n_ports; i++ ) {
    ret = validate_phy_port_no( ntohs( port_stats->port_no ) );

    if ( ret < 0 ) {
      return ret;
    }

    // port_stats->rx_packets
    // port_stats->tx_packets
    // port_stats->rx_bytes
    // port_stats->tx_bytes
    // port_stats->rx_dropped
    // port_stats->tx_dropped
    // port_stats->rx_errors
    // port_stats->tx_errors
    // port_stats->rx_frame_err
    // port_stats->rx_over_err
    // port_stats->rx_crc_err
    // port_stats->collisions

    port_stats++;
  }

  return 0;
}


int
validate_queue_stats_reply( const buffer *message ) {
  int i;
  int ret;
  uint16_t queues_length;
  uint16_t n_queues;
  uint16_t offset;
  struct ofp_stats_reply *stats_reply;
  struct ofp_queue_stats *queue_stats;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY,
                         offsetof( struct ofp_stats_reply, body ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;
  if ( ( ntohs( stats_reply->flags ) & ~OFPSF_REPLY_MORE ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  queues_length = ( uint16_t ) ( ntohs( stats_reply->header.length )
                                 - offsetof( struct ofp_stats_reply, body ) );
  if ( queues_length % sizeof( struct ofp_queue_stats ) != 0 ) {
    return ERROR_INVALID_LENGTH;
  }

  offset = offsetof( struct ofp_stats_reply, body );
  queue_stats = ( struct ofp_queue_stats * ) ( ( char * ) message->data + offset );

  n_queues = queues_length / sizeof( struct ofp_queue_stats );
  for ( i = 0; i < n_queues; i++ ) {
    ret = validate_phy_port_no( ntohs( queue_stats->port_no ) );
    if ( ret < 0 ) {
      return ret;
    }

    // queue_stats->queue_id
    // queue_stats->tx_bytes
    // queue_stats->tx_packets
    // queue_stats->tx_errors

    queue_stats++;
  }

  return 0;
}


int
validate_vendor_stats_reply( const buffer *message ) {
  void *body;
  int ret;
  uint16_t body_length;
  uint16_t offset;
  struct ofp_stats_reply *stats_reply;

  assert( message != NULL );

  ret = validate_header( message, OFPT_STATS_REPLY,
                         offsetof( struct ofp_stats_reply, body ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  stats_reply = ( struct ofp_stats_reply * ) message->data;

  if ( ( ntohs( stats_reply->flags ) & ~OFPSF_REPLY_MORE ) != 0 ) {
    return ERROR_INVALID_STATS_REPLY_FLAGS;
  }

  body_length = ( uint16_t ) ( ntohs( stats_reply->header.length )
                             - offsetof( struct ofp_stats_reply, body ) );

  offset = offsetof( struct ofp_stats_reply, body );
  body = ( void * ) ( ( char * ) message->data + offset );
  if ( ( body_length > 0 ) && ( body != NULL ) ) {
    // FIXME: validate body here
  }

  return 0;
}


int
validate_stats_reply( const buffer *message ) {
  struct ofp_stats_reply *reply;

  assert( message != NULL );
  assert( message->data != NULL );

  reply = ( struct ofp_stats_reply * ) message->data;

  // TODO: if ( reply->header.type != OFPT_STATS_REPLY ) { ... }

  switch ( ntohs( reply->type ) ) {
  case OFPST_DESC:
    return validate_desc_stats_reply( message );
  case OFPST_FLOW:
    return validate_flow_stats_reply( message );
  case OFPST_AGGREGATE:
    return validate_aggregate_stats_reply( message );
  case OFPST_TABLE:
    return validate_table_stats_reply( message );
  case OFPST_PORT:
    return validate_port_stats_reply( message );
  case OFPST_QUEUE:
    return validate_queue_stats_reply( message );
  case OFPST_VENDOR:
    return validate_vendor_stats_reply( message );
  default:
    break;
  }

  return ERROR_UNSUPPORTED_STATS_TYPE;
}


int
validate_barrier_request( const buffer *message ) {
  return validate_header( message, OFPT_BARRIER_REQUEST, sizeof( struct ofp_header ),
                          sizeof( struct ofp_header ) );
}


int
validate_barrier_reply( const buffer *message ) {
  return validate_header( message, OFPT_BARRIER_REPLY, sizeof( struct ofp_header ),
                          sizeof( struct ofp_header ) );
}


int
validate_queue_get_config_request( const buffer *message ) {
  int ret;
  struct ofp_queue_get_config_request *queue_get_config_request;

  ret = validate_header( message, OFPT_QUEUE_GET_CONFIG_REQUEST,
                         sizeof( struct ofp_queue_get_config_request ),
                         sizeof( struct ofp_queue_get_config_request ) );
  if ( ret < 0 ) {
    return ret;
  }

  queue_get_config_request = ( struct ofp_queue_get_config_request * ) message->data;

  ret = validate_phy_port_no( ntohs( queue_get_config_request->port ) );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


static int
validate_queue_property( const struct ofp_queue_prop_header *property ) {
  uint16_t property_length = ntohs( property->len );

  if ( property_length < sizeof( struct ofp_queue_prop_header ) ) {
    return ERROR_TOO_SHORT_QUEUE_PROPERTY;
  }

  switch ( ntohs( property->property ) ) {
  case OFPQT_NONE:
    if ( property_length < sizeof( struct ofp_queue_prop_header ) ) {
      return ERROR_TOO_SHORT_QUEUE_PROPERTY;
    }
    else if ( property_length > sizeof( struct ofp_queue_prop_header ) ) {
      return ERROR_TOO_LONG_QUEUE_PROPERTY;
    }
    break;
  case OFPQT_MIN_RATE:
    if ( property_length < sizeof( struct ofp_queue_prop_min_rate ) ) {
      return ERROR_TOO_SHORT_QUEUE_PROPERTY;
    }
    else if ( property_length > sizeof( struct ofp_queue_prop_min_rate ) ) {
      return ERROR_TOO_LONG_QUEUE_PROPERTY;
    }
    break;
  default:
    return ERROR_UNDEFINED_QUEUE_PROPERTY;
  }

  return 0;
}


static int
validate_queue_properties( struct ofp_queue_prop_header *prop_head,
                           const uint16_t properties_length ) {
  int ret;
  uint16_t offset = 0;
  struct ofp_queue_prop_header *property;

  property = prop_head;
  while ( offset < properties_length ) {
    ret = validate_queue_property( property );
    if ( ret < 0 ) {
      return ret;
    }

    offset = ( uint16_t ) ( offset + ntohs( property->len ) );
    property = ( struct ofp_queue_prop_header * ) ( ( char * ) prop_head + offset );
  }

  return 0;
}


static int
validate_packet_queue( struct ofp_packet_queue *queue ) {
  int ret;
  uint16_t properties_length;
  struct ofp_queue_prop_header *prop_head;

  assert( queue != NULL );

  // queue->queue_id

  if ( ntohs( queue->len ) < ( offsetof( struct ofp_packet_queue, properties )
                             + sizeof( struct ofp_queue_prop_header ) ) ) {
    return ERROR_TOO_SHORT_QUEUE_DESCRIPTION;
  }

  prop_head =  ( struct ofp_queue_prop_header * ) ( ( char * ) queue
               + offsetof( struct ofp_packet_queue, properties ) );
  properties_length = ( uint16_t ) ( ntohs( queue->len )
                                   - offsetof( struct ofp_packet_queue, properties ) );

  ret = validate_queue_properties( prop_head, properties_length );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


static int
validate_packet_queues( struct ofp_packet_queue *queue_head, const int n_queues ) {
  int i;
  int ret;
  struct ofp_packet_queue *queue;

  assert( queue_head != NULL );

  queue = queue_head;
  for ( i = 0; i < n_queues; i++ ) {
    ret = validate_packet_queue( queue );
    if ( ret < 0 ) {
      return ret;
    }
    queue = ( struct ofp_packet_queue * ) ( ( char * ) queue + ntohs( queue->len ) );
  }

  return 0;
}


int
validate_queue_get_config_reply( const buffer *message ) {
  int ret;
  int n_queues = 0;
  uint16_t queues_length;
  struct ofp_queue_get_config_reply *queue_get_config_reply;
  struct ofp_packet_queue *queue_head, *queue;

  assert( message != NULL );

  ret = validate_header( message, OFPT_QUEUE_GET_CONFIG_REPLY,
                         sizeof( struct ofp_queue_get_config_reply ) + sizeof( struct ofp_packet_queue ),
                         UINT16_MAX );
  if ( ret < 0 ) {
    return ret;
  }

  queue_get_config_reply = ( struct ofp_queue_get_config_reply * ) message->data;

  ret = validate_phy_port_no( ntohs( queue_get_config_reply->port ) );
  if ( ret < 0 ) {
    return ret;
  }

  queues_length = ( uint16_t ) ( ntohs( queue_get_config_reply->header.length )
                 - offsetof( struct ofp_queue_get_config_reply, queues ) );

  queue_head = ( struct ofp_packet_queue * ) ( ( char * ) message->data
               + offsetof( struct ofp_queue_get_config_reply, queues ) );

  queue = queue_head;
  while ( queues_length > offsetof( struct ofp_packet_queue, properties ) ) {
    queues_length = ( uint16_t ) ( queues_length - ntohs( queue->len ) );
    queue = ( struct ofp_packet_queue * ) ( ( char * ) queue + ntohs( queue->len ) );
    n_queues++;
  }

  if ( queues_length != 0 ) {
    return ERROR_INVALID_LENGTH;
  }

  if ( n_queues > 0 ) {
    ret = validate_packet_queues( queue_head, n_queues );
    if ( ret < 0 ) {
      return ret;
    }
  }

  return 0;
}


static int
validate_action( struct ofp_action_header *action ) {
  if ( ntohs( action->len ) < sizeof( struct ofp_action_header ) ) {
    return ERROR_TOO_SHORT_ACTION;
  }

  switch ( ntohs( action->type ) ) {
  case OFPAT_OUTPUT:
    return validate_action_output( ( struct ofp_action_output * ) action );
  case OFPAT_SET_VLAN_VID:
    return validate_action_set_vlan_vid( ( struct ofp_action_vlan_vid * ) action );
  case OFPAT_SET_VLAN_PCP:
    return validate_action_set_vlan_pcp( ( struct ofp_action_vlan_pcp * ) action );
  case OFPAT_STRIP_VLAN:
    return validate_action_strip_vlan( ( struct ofp_action_header * ) action );
  case OFPAT_SET_DL_SRC:
    return validate_action_set_dl_src( ( struct ofp_action_dl_addr * ) action );
  case OFPAT_SET_DL_DST:
    return validate_action_set_dl_dst( ( struct ofp_action_dl_addr * ) action );
  case OFPAT_SET_NW_SRC:
    return validate_action_set_nw_src( ( struct ofp_action_nw_addr * ) action );
  case OFPAT_SET_NW_DST:
    return validate_action_set_nw_dst( ( struct ofp_action_nw_addr * ) action );
  case OFPAT_SET_NW_TOS:
    return validate_action_set_nw_tos( ( struct ofp_action_nw_tos * ) action );
  case OFPAT_SET_TP_SRC:
    return validate_action_set_tp_src( ( struct ofp_action_tp_port * ) action );
  case OFPAT_SET_TP_DST:
    return validate_action_set_tp_dst( ( struct ofp_action_tp_port * ) action );
  case OFPAT_ENQUEUE:
    return validate_action_enqueue( ( struct ofp_action_enqueue * ) action );
  case OFPAT_VENDOR:
    return validate_action_vendor( ( struct ofp_action_vendor_header * ) action );
  default:
    break;
  }

  return ERROR_UNDEFINED_ACTION_TYPE;
}


int
validate_actions( struct ofp_action_header *actions_head, const uint16_t length ) {
  int ret;
  uint16_t offset = 0;
  struct ofp_action_header *action;

  action = actions_head;
  while ( offset < length ) {
    ret = validate_action( action );
    if ( ret < 0 ) {
      return ret;
    }

    offset = ( uint16_t ) ( offset + ntohs( action->len ) );
    action = ( struct ofp_action_header * ) ( ( char * ) actions_head + offset );
  }

  return 0;
}


int
validate_action_output( const struct ofp_action_output *action ) {
  int ret;
  struct ofp_action_output output;

  ntoh_action_output( &output, action );
  if ( output.type != OFPAT_OUTPUT ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( output.len < sizeof( struct ofp_action_output ) ) {
    return ERROR_TOO_SHORT_ACTION_OUTPUT;
  }
  else if ( output.len > sizeof( struct ofp_action_output ) ) {
    return ERROR_TOO_LONG_ACTION_OUTPUT;
  }

  ret = validate_phy_port_no( output.port );
  if ( ret < 0 ) {
    return ret;
  }

  // output.max_len

  return 0;
}


int
validate_action_set_vlan_vid( const struct ofp_action_vlan_vid *action ) {
  int ret;
  struct ofp_action_vlan_vid vlan_vid;

  ntoh_action_vlan_vid( &vlan_vid, action );

  if ( vlan_vid.type != OFPAT_SET_VLAN_VID ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( vlan_vid.len < sizeof( struct ofp_action_vlan_vid ) ) {
    return ERROR_TOO_SHORT_ACTION_VLAN_VID;
  }
  else if ( vlan_vid.len > sizeof( struct ofp_action_vlan_vid ) ) {
    return ERROR_TOO_LONG_ACTION_VLAN_VID;
  }

  ret = validate_vlan_vid( vlan_vid.vlan_vid );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_action_set_vlan_pcp( const struct ofp_action_vlan_pcp *action ) {
  int ret;
  struct ofp_action_vlan_pcp vlan_pcp;

  ntoh_action_vlan_pcp( &vlan_pcp, action );

  if ( vlan_pcp.type != OFPAT_SET_VLAN_PCP ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( vlan_pcp.len < sizeof( struct ofp_action_vlan_pcp ) ) {
    return ERROR_TOO_SHORT_ACTION_VLAN_PCP;
  }
  else if ( vlan_pcp.len > sizeof( struct ofp_action_vlan_pcp ) ) {
    return ERROR_TOO_LONG_ACTION_VLAN_PCP;
  }

  ret = validate_vlan_pcp( vlan_pcp.vlan_pcp );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_action_strip_vlan( const struct ofp_action_header *action ) {
  struct ofp_action_header strip_vlan;

  ntoh_action_strip_vlan( &strip_vlan, action );

  if ( strip_vlan.type != OFPAT_STRIP_VLAN ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( strip_vlan.len < sizeof( struct ofp_action_header ) ) {
    return ERROR_TOO_SHORT_ACTION_STRIP_VLAN;
  }
  else if ( strip_vlan.len > sizeof( struct ofp_action_header ) ) {
    return ERROR_TOO_LONG_ACTION_STRIP_VLAN;
  }

  return 0;
}


int
validate_action_set_dl_src( const struct ofp_action_dl_addr *action ) {
  struct ofp_action_dl_addr dl_src;

  ntoh_action_dl_addr( &dl_src, action );

  if ( dl_src.type != OFPAT_SET_DL_SRC ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( dl_src.len < sizeof( struct ofp_action_dl_addr ) ) {
    return ERROR_TOO_SHORT_ACTION_DL_SRC;
  }
  else if ( dl_src.len > sizeof( struct ofp_action_dl_addr ) ) {
    return ERROR_TOO_LONG_ACTION_DL_SRC;
  }

  return 0;
}


int
validate_action_set_dl_dst( const struct ofp_action_dl_addr *action ) {
  struct ofp_action_dl_addr dl_dst;

  ntoh_action_dl_addr( &dl_dst, action );

  if ( dl_dst.type != OFPAT_SET_DL_DST ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( dl_dst.len < sizeof( struct ofp_action_dl_addr ) ) {
    return ERROR_TOO_SHORT_ACTION_DL_DST;
  }
  else if ( dl_dst.len > sizeof( struct ofp_action_dl_addr ) ) {
    return ERROR_TOO_LONG_ACTION_DL_DST;
  }

  return 0;
}


int
validate_action_set_nw_src( const struct ofp_action_nw_addr *action ) {
  struct ofp_action_nw_addr nw_src;

  ntoh_action_nw_addr( &nw_src, action );

  if ( nw_src.type != OFPAT_SET_NW_SRC ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( nw_src.len < sizeof( struct ofp_action_nw_addr ) ) {
    return ERROR_TOO_SHORT_ACTION_NW_SRC;
  }
  else if ( nw_src.len > sizeof( struct ofp_action_nw_addr ) ) {
    return ERROR_TOO_LONG_ACTION_NW_SRC;
  }

  return 0;
}


int
validate_action_set_nw_dst( const struct ofp_action_nw_addr *action ) {
  struct ofp_action_nw_addr nw_dst;

  ntoh_action_nw_addr( &nw_dst, action );

  if ( nw_dst.type != OFPAT_SET_NW_DST ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( nw_dst.len < sizeof( struct ofp_action_nw_addr ) ) {
    return ERROR_TOO_SHORT_ACTION_NW_DST;
  }
  else if ( nw_dst.len > sizeof( struct ofp_action_nw_addr ) ) {
    return ERROR_TOO_LONG_ACTION_NW_DST;
  }

  return 0;
}


int
validate_action_set_nw_tos( const struct ofp_action_nw_tos *action ) {
  int ret;
  struct ofp_action_nw_tos nw_tos;

  ntoh_action_nw_tos( &nw_tos, action );

  if ( nw_tos.type != OFPAT_SET_NW_TOS ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( nw_tos.len < sizeof( struct ofp_action_nw_addr ) ) {
    return ERROR_TOO_SHORT_ACTION_NW_TOS;
  }
  else if ( nw_tos.len > sizeof( struct ofp_action_nw_addr ) ) {
    return ERROR_TOO_LONG_ACTION_NW_TOS;
  }

  ret = validate_nw_tos( nw_tos.nw_tos );
  if ( ret < 0 ) {
    return ret;
  }

  return 0;
}


int
validate_action_set_tp_src( const struct ofp_action_tp_port *action ) {
  struct ofp_action_tp_port tp_src;

  ntoh_action_tp_port( &tp_src, action );

  if ( tp_src.type != OFPAT_SET_TP_SRC ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( tp_src.len < sizeof( struct ofp_action_tp_port ) ) {
    return ERROR_TOO_SHORT_ACTION_TP_SRC;
  }
  else if ( tp_src.len > sizeof( struct ofp_action_tp_port ) ) {
    return ERROR_TOO_LONG_ACTION_TP_SRC;
  }

  return 0;
}


int
validate_action_set_tp_dst( const struct ofp_action_tp_port *action ) {
  struct ofp_action_tp_port tp_dst;

  ntoh_action_tp_port( &tp_dst, action );

  if ( tp_dst.type != OFPAT_SET_TP_DST ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( tp_dst.len < sizeof( struct ofp_action_tp_port ) ) {
    return ERROR_TOO_SHORT_ACTION_TP_DST;
  }
  else if ( tp_dst.len > sizeof( struct ofp_action_tp_port ) ) {
    return ERROR_TOO_LONG_ACTION_TP_DST;
  }

  return 0;
}


int
validate_action_enqueue( const struct ofp_action_enqueue *action ) {
  int ret;
  struct ofp_action_enqueue enqueue;

  ntoh_action_enqueue( &enqueue, action );

  if ( enqueue.type != OFPAT_ENQUEUE ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( enqueue.len < sizeof( struct ofp_action_enqueue ) ) {
    return ERROR_TOO_SHORT_ACTION_ENQUEUE;
  }
  else if ( enqueue.len > sizeof( struct ofp_action_enqueue ) ) {
    return ERROR_TOO_LONG_ACTION_ENQUEUE;
  }

  ret = validate_phy_port_no( enqueue.port );
  if ( ret < 0 ) {
    return ret;
  }

  // enqueue.queue_id

  return 0;
}


int
validate_action_vendor( const struct ofp_action_vendor_header *action ) {
  if ( ntohs( action->type ) != OFPAT_VENDOR ) {
    return ERROR_INVALID_ACTION_TYPE;
  }
  if ( ntohs( action->len ) < sizeof( struct ofp_action_vendor_header ) ) {
    return ERROR_TOO_SHORT_ACTION_VENDOR;
  }
  else if ( ntohs( action->len ) % 8 != 0 ) {
    return ERROR_INVALID_LENGTH_ACTION_VENDOR;
  }

  // action->vendor

  return 0;
}


int
validate_openflow_message( const buffer *message ) {
  int ret;

  assert( message != NULL );
  assert( message->data != NULL );

  struct ofp_header *header = ( struct ofp_header * ) message->data;

  debug( "Validating an OpenFlow message ( version = %#x, type = %#x, length = %u, xid = %#x ).",
         header->version, header->type, ntohs( header->length ), ntohl( header->xid ) );

  switch ( header->type ) {
  case OFPT_HELLO:
    ret = validate_hello( message );
    break;
  case OFPT_ERROR:
    ret = validate_error( message );
    break;
  case OFPT_ECHO_REQUEST:
    ret = validate_echo_request( message );
    break;
  case OFPT_ECHO_REPLY:
    ret = validate_echo_reply( message );
    break;
  case OFPT_VENDOR:
    ret = validate_vendor( message );
    break;
  case OFPT_FEATURES_REQUEST:
    ret = validate_features_request( message );
    break;
  case OFPT_FEATURES_REPLY:
    ret = validate_features_reply( message );
    break;
  case OFPT_GET_CONFIG_REQUEST:
    ret = validate_get_config_request( message );
    break;
  case OFPT_GET_CONFIG_REPLY:
    ret = validate_get_config_reply( message );
    break;
  case OFPT_SET_CONFIG:
    ret = validate_set_config( message );
    break;
  case OFPT_PACKET_IN:
    ret = validate_packet_in( message );
    break;
  case OFPT_FLOW_REMOVED:
    ret = validate_flow_removed( message );
    break;
  case OFPT_PORT_STATUS:
    ret = validate_port_status( message );
    break;
  case OFPT_PACKET_OUT:
    ret = validate_packet_out( message );
    break;
  case OFPT_FLOW_MOD:
    ret = validate_flow_mod( message );
    break;
  case OFPT_PORT_MOD:
    ret = validate_port_mod( message );
    break;
  case OFPT_STATS_REQUEST:
    ret = validate_stats_request( message );
    break;
  case OFPT_STATS_REPLY:
    ret = validate_stats_reply( message );
    break;
  case OFPT_BARRIER_REQUEST:
    ret = validate_barrier_request( message );
    break;
  case OFPT_BARRIER_REPLY:
    ret = validate_barrier_reply( message );
    break;
  case OFPT_QUEUE_GET_CONFIG_REQUEST:
    ret = validate_queue_get_config_request( message );
    break;
  case OFPT_QUEUE_GET_CONFIG_REPLY:
    ret = validate_queue_get_config_reply( message );
    break;
  default:
    ret = ERROR_UNDEFINED_TYPE;
    break;
  }

  debug( "Validation completed ( ret = %d ).", ret );

  return ret;
}


bool
valid_openflow_message( const buffer *message ) {
  if ( validate_openflow_message( message ) < 0 ) {
    return false;
  }

  return true;
}


static struct error_map {
  uint8_t type; // One of the OFPT_ constants.
  struct map {
    int error_no; // Internal error number.
    uint16_t error_type; // OpenFlow error type.
    uint16_t error_code; // OpenFlow error code.
  } maps[ 64 ];
} error_maps[] = {
  {
    OFPT_HELLO,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_HELLO_FAILED, OFPHFC_INCOMPATIBLE },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_ERROR,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_ECHO_REQUEST,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_ECHO_REPLY,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_VENDOR,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_FEATURES_REQUEST,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_FEATURES_REPLY,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_GET_CONFIG_REQUEST,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_GET_CONFIG_REPLY,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_SET_CONFIG,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_SWITCH_CONFIG, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_PACKET_IN, // FIXME: Should we return an error for packet_in ?
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_PACKET_IN_REASON, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_FLOW_REMOVED,  // FIXME: Should we return an error for flow_removed ?
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_FLOW_PRIORITY, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_FLOW_REMOVED_REASON, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_PORT_STATUS,  // FIXME: Should we return an error for port_status ?
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_PORT_STATUS_REASON, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_PACKET_OUT,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_ACTION_TYPE, OFPET_BAD_ACTION, OFPBAC_BAD_TYPE },
      { ERROR_TOO_SHORT_ACTION_OUTPUT, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_OUTPUT, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_PORT_NO, OFPET_BAD_ACTION, OFPBAC_BAD_OUT_PORT },
      { ERROR_TOO_SHORT_ACTION_VLAN_VID, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_VLAN_VID, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_VLAN_VID, OFPET_BAD_ACTION, OFPBAC_BAD_ARGUMENT },
      { ERROR_TOO_SHORT_ACTION_VLAN_PCP, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_VLAN_PCP, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_VLAN_PCP, OFPET_BAD_ACTION, OFPBAC_BAD_ARGUMENT },
      { ERROR_TOO_SHORT_ACTION_STRIP_VLAN, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_STRIP_VLAN, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_DL_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_DL_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_DL_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_DL_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_NW_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_NW_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_NW_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_NW_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_NW_TOS, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_NW_TOS, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_TP_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_TP_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_TP_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_TP_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_ENQUEUE, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_ENQUEUE, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_PORT_NO, OFPET_BAD_ACTION, OFPBAC_BAD_OUT_PORT },
      { ERROR_TOO_SHORT_ACTION_VENDOR, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_LENGTH_ACTION_VENDOR, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_UNDEFINED_ACTION_TYPE, OFPET_BAD_ACTION, OFPBAC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_FLOW_MOD,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_ACTION_TYPE, OFPET_BAD_ACTION, OFPBAC_BAD_TYPE },
      { ERROR_TOO_SHORT_ACTION_OUTPUT, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_OUTPUT, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_PORT_NO, OFPET_BAD_ACTION, OFPBAC_BAD_OUT_PORT },
      { ERROR_TOO_SHORT_ACTION_VLAN_VID, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_VLAN_VID, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_VLAN_VID, OFPET_BAD_ACTION, OFPBAC_BAD_ARGUMENT },
      { ERROR_TOO_SHORT_ACTION_VLAN_PCP, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_VLAN_PCP, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_VLAN_PCP, OFPET_BAD_ACTION, OFPBAC_BAD_ARGUMENT },
      { ERROR_TOO_SHORT_ACTION_STRIP_VLAN, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_STRIP_VLAN, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_DL_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_DL_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_DL_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_DL_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_NW_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_NW_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_NW_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_NW_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_NW_TOS, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_NW_TOS, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_TP_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_TP_SRC, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_TP_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_TP_DST, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_SHORT_ACTION_ENQUEUE, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_TOO_LONG_ACTION_ENQUEUE, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_PORT_NO, OFPET_BAD_ACTION, OFPBAC_BAD_OUT_PORT },
      { ERROR_TOO_SHORT_ACTION_VENDOR, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_INVALID_LENGTH_ACTION_VENDOR, OFPET_BAD_ACTION, OFPBAC_BAD_LEN },
      { ERROR_UNDEFINED_ACTION_TYPE, OFPET_BAD_ACTION, OFPBAC_BAD_TYPE },
      { ERROR_INVALID_WILDCARDS, OFPET_FLOW_MOD_FAILED, OFPFMFC_EPERM }, // FIXME
      { ERROR_UNDEFINED_FLOW_MOD_COMMAND, OFPET_FLOW_MOD_FAILED, OFPFMFC_BAD_COMMAND },
      { ERROR_INVALID_FLOW_PRIORITY, OFPET_FLOW_MOD_FAILED, OFPFMFC_EPERM }, // FIXME
      { ERROR_INVALID_FLOW_MOD_FLAGS, OFPET_FLOW_MOD_FAILED, OFPFMFC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_PORT_MOD,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_PORT_NO, OFPET_PORT_MOD_FAILED, OFPPMFC_BAD_PORT },
      { ERROR_INVALID_PORT_CONFIG, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_PORT_MASK, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_PORT_FEATURES, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_STATS_REQUEST,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_UNSUPPORTED_STATS_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_STAT },
      { ERROR_INVALID_STATS_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_STAT },
      { ERROR_INVALID_STATS_REQUEST_FLAGS, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_PORT_NO, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_WILDCARDS, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_VLAN_VID, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_VLAN_PCP, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { ERROR_INVALID_NW_TOS, OFPET_BAD_REQUEST, OFPBRC_EPERM }, // FIXME
      { 0, 0, 0 },
    }
  },
  {
    OFPT_STATS_REPLY,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_BARRIER_REQUEST,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_BARRIER_REPLY,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_QUEUE_GET_CONFIG_REQUEST,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_PORT_NO, OFPET_QUEUE_OP_FAILED, OFPQOFC_BAD_PORT },
      { 0, 0, 0 },
    }
  },
  {
    OFPT_QUEUE_GET_CONFIG_REPLY,
    {
      { ERROR_UNSUPPORTED_VERSION, OFPET_BAD_REQUEST, OFPBRC_BAD_VERSION },
      { ERROR_TOO_SHORT_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_TOO_LONG_MESSAGE, OFPET_BAD_REQUEST, OFPBRC_BAD_LEN },
      { ERROR_UNDEFINED_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { ERROR_INVALID_TYPE, OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE },
      { 0, 0, 0 },
    }
  },
};


bool
get_error_type_and_code( const uint8_t type, const int error_no,
                         uint16_t *error_type, uint16_t *error_code ) {
  if ( type > OFPT_QUEUE_GET_CONFIG_REPLY ) {
    *error_type = OFPET_BAD_REQUEST;
    *error_code = OFPBRC_BAD_TYPE;
    debug( "Undefined OpenFlow message type ( type = %u ).", type );
    return true;
  }

  int i = 0;
  for ( i = 0; error_maps[ type ].maps[ i ].error_no != 0; i++ ) {
    if ( error_no == error_maps[ type ].maps[ i ].error_no ) {
      *error_type = error_maps[ type ].maps[ i ].error_type;
      *error_code = error_maps[ type ].maps[ i ].error_code;
      return true;
    }
  }

  return false;
}


void
set_match_from_packet( struct ofp_match *match, const uint16_t in_port,
                       const uint32_t wildcards, const buffer *packet ) {
  // Note that wildcards must be filled before calling this function.

  assert( match != NULL );
  assert( packet != NULL );
  assert( packet->user_data != NULL );

  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = wildcards;

  if ( !( wildcards & OFPFW_IN_PORT ) ) {
    match->in_port = in_port;
  }
  if ( !( wildcards & OFPFW_DL_SRC ) ) {
    memcpy( match->dl_src, ( ( packet_info * ) packet->user_data )->eth_macsa, OFP_ETH_ALEN );
  }
  if ( !( wildcards & OFPFW_DL_DST ) ) {
    memcpy( match->dl_dst, ( ( packet_info * ) packet->user_data )->eth_macda, OFP_ETH_ALEN );
  }
  if ( !( wildcards & OFPFW_DL_VLAN ) ) {
    if ( packet_type_eth_vtag( packet ) ) {
      match->dl_vlan = ( ( packet_info * ) packet->user_data )->vlan_vid;
      if ( ( match->dl_vlan & ~VLAN_VID_MASK ) != 0 ) {
        warn( "Invalid vlan id ( change %#x to %#x )", match->dl_vlan, match->dl_vlan & VLAN_VID_MASK );
        match->dl_vlan = ( uint16_t ) ( match->dl_vlan & VLAN_VID_MASK );
      }
    }
    else {
      match->dl_vlan = UINT16_MAX;
    }
  }
  if ( !( wildcards & OFPFW_DL_VLAN_PCP ) ) {
    if ( packet_type_eth_vtag( packet ) ) {
      match->dl_vlan_pcp = ( ( packet_info * ) packet->user_data )->vlan_prio;
      if ( ( match->dl_vlan_pcp & ~VLAN_PCP_MASK ) != 0 ) {
        warn( "Invalid vlan pcp ( change %u to %u )", match->dl_vlan_pcp, match->dl_vlan_pcp & VLAN_PCP_MASK );
        match->dl_vlan_pcp = ( uint8_t ) ( match->dl_vlan_pcp & VLAN_PCP_MASK );
      }
    }
  }
  if ( !( wildcards & OFPFW_DL_TYPE ) ) {
    match->dl_type = ( ( packet_info * ) packet->user_data )->eth_type;
  }
  if ( match->dl_type == ETH_ETHTYPE_IPV4 ) {
    if ( !( wildcards & OFPFW_NW_TOS ) ) {
      match->nw_tos = ( ( packet_info * ) packet->user_data )->ipv4_tos;
      if ( ( match->nw_tos & ~NW_TOS_MASK ) != 0 ) {
        warn( "Invalid ipv4 tos ( change %#x to %#x )", match->nw_tos, match->nw_tos & NW_TOS_MASK );
        match->nw_tos = ( uint8_t ) ( match->nw_tos & NW_TOS_MASK );
      }
    }
    if ( !( wildcards & OFPFW_NW_PROTO ) ) {
      match->nw_proto = ( ( packet_info * ) packet->user_data )->ipv4_protocol;
    }
    unsigned int nw_src_mask_len = ( wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;
    if ( nw_src_mask_len < 32 ) {
      match->nw_src = ( ( packet_info * ) packet->user_data )->ipv4_saddr & ( 0xffffffff << nw_src_mask_len );
    }
    unsigned int nw_dst_mask_len = ( wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;
    if ( nw_dst_mask_len < 32 ) {
      match->nw_dst = ( ( packet_info * ) packet->user_data )->ipv4_daddr & ( 0xffffffff << nw_dst_mask_len );
    }

    switch ( match->nw_proto ) {
    case IPPROTO_ICMP:
      if ( !( wildcards & OFPFW_ICMP_TYPE ) ) {
        match->icmp_type = ( ( packet_info * ) packet->user_data )->icmpv4_type;
      }
      if ( !( wildcards & OFPFW_ICMP_CODE ) ) {
        match->icmp_code = ( ( packet_info * ) packet->user_data )->icmpv4_code;
      }
      break;
    case IPPROTO_TCP:
      if ( !( wildcards & OFPFW_TP_SRC ) ) {
        match->tp_src = ( ( packet_info * ) packet->user_data )->tcp_src_port;
      }
      if ( !( wildcards & OFPFW_TP_DST ) ) {
        match->tp_dst = ( ( packet_info * ) packet->user_data )->tcp_dst_port;
      }
      break;
    case IPPROTO_UDP:
      if ( !( wildcards & OFPFW_TP_SRC ) ) {
        match->tp_src = ( ( packet_info * ) packet->user_data )->udp_src_port;
      }
      if ( !( wildcards & OFPFW_TP_DST ) ) {
        match->tp_dst = ( ( packet_info * ) packet->user_data )->udp_dst_port;
      }
      break;

    default:
      break;
    }
  }
  if ( match->dl_type == ETH_ETHTYPE_ARP ) {
    if ( !( wildcards & OFPFW_NW_PROTO ) ) {
      match->nw_proto = ( uint8_t ) ( ( ( packet_info * ) packet->user_data )->arp_ar_op & ARP_OP_MASK );
    }
    unsigned int nw_src_mask_len = ( wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;
    if ( nw_src_mask_len < 32 ) {
      match->nw_src = ( ( packet_info * ) packet->user_data )->arp_spa & ( 0xffffffff << nw_src_mask_len );
    }
    unsigned int nw_dst_mask_len = ( wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;
    if ( nw_dst_mask_len < 32 ) {
      match->nw_dst = ( ( packet_info * ) packet->user_data )->arp_tpa & ( 0xffffffff << nw_dst_mask_len );
    }
  }
}


void
normalize_match( struct ofp_match *match ) {

  assert( match != NULL );

  char match_string[ 1024 ];
  match_to_string( match, match_string, sizeof( match_string ) );
  debug( "Normalizing match structure ( original match = [%s] ).", match_string );

  memset( match->pad1, 0, sizeof( match->pad1 ) );
  memset( match->pad2, 0, sizeof( match->pad2 ) );

  match->wildcards &= OFPFW_ALL;

  if ( match->wildcards & OFPFW_IN_PORT ) {
    match->in_port = 0;
  }
  if ( match->wildcards & OFPFW_DL_VLAN ) {
    match->dl_vlan = 0;
  }
  else {
    if ( match->dl_vlan == UINT16_MAX ) {
      match->wildcards |= ( uint32_t ) OFPFW_DL_VLAN_PCP;
      match->dl_vlan_pcp = 0;
    }
    else {
      match->dl_vlan &= VLAN_VID_MASK;
    }
  }
  if ( match->wildcards & OFPFW_DL_SRC ) {
    memset( match->dl_src, 0, sizeof( match->dl_src ) );
  }
  if ( match->wildcards & OFPFW_DL_DST ) {
    memset( match->dl_dst, 0, sizeof( match->dl_dst ) );
  }
  if ( match->wildcards & OFPFW_DL_TYPE ) {
    match->dl_type = 0;
    match->wildcards |= ( uint32_t ) OFPFW_NW_TOS;
    match->wildcards |= ( uint32_t ) OFPFW_NW_PROTO;
    match->wildcards |= ( uint32_t ) OFPFW_NW_SRC_MASK;
    match->wildcards |= ( uint32_t ) OFPFW_NW_DST_MASK;
    match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
    match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
    match->nw_tos = 0;
    match->nw_proto = 0;
    match->nw_src = 0;
    match->nw_dst = 0;
    match->tp_src = 0;
    match->tp_dst = 0;
  }
  else {
    if ( match->dl_type == ETH_ETHTYPE_ARP ) {
      match->wildcards |= ( uint32_t ) OFPFW_NW_TOS;
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->nw_tos = 0;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
    else if ( match->dl_type != ETH_ETHTYPE_IPV4 ) {
      match->wildcards |= ( uint32_t ) OFPFW_NW_TOS;
      match->wildcards |= ( uint32_t ) OFPFW_NW_PROTO;
      match->wildcards |= ( uint32_t ) OFPFW_NW_SRC_MASK;
      match->wildcards |= ( uint32_t ) OFPFW_NW_DST_MASK;
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->nw_tos = 0;
      match->nw_proto = 0;
      match->nw_src = 0;
      match->nw_dst = 0;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
  }
  if ( match->wildcards & OFPFW_NW_PROTO ) {
    match->nw_proto = 0;
    if ( match->dl_type != ETH_ETHTYPE_IPV4 ) {
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
  }
  else {
    if ( match->dl_type == ETH_ETHTYPE_ARP ) {
      match->nw_proto &= ARP_OP_MASK;
    }
    if ( match->dl_type == ETH_ETHTYPE_IPV4 &&
         match->nw_proto != IPPROTO_TCP && match->nw_proto != IPPROTO_UDP && match->nw_proto != IPPROTO_ICMP ) {
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
  }

  unsigned int nw_src_mask_len = ( match->wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;
  if ( nw_src_mask_len >= 32 ) {
    match->wildcards &= ( uint32_t ) ~OFPFW_NW_SRC_MASK;
    match->wildcards |= OFPFW_NW_SRC_ALL;
    match->nw_src = 0;
  }
  else {
    match->nw_src &= ( 0xffffffff << nw_src_mask_len );
  }
  unsigned int nw_dst_mask_len = ( match->wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;
  if ( nw_dst_mask_len >= 32 ) {
    match->wildcards &= ( uint32_t ) ~OFPFW_NW_DST_MASK;
    match->wildcards |= OFPFW_NW_DST_ALL;
    match->nw_dst = 0;
  }
  else {
    match->nw_dst &= ( 0xffffffff << nw_dst_mask_len );
  }
  if ( match->wildcards & OFPFW_TP_SRC ) {
    match->tp_src = 0;
  }
  else {
    if ( match->nw_proto == IPPROTO_ICMP ) {
      match->tp_src &= ICMP_TYPE_MASK;
    }
  }
  if ( match->wildcards & OFPFW_TP_DST ) {
    match->tp_dst = 0;
  }
  else {
    if ( match->nw_proto == IPPROTO_ICMP ) {
      match->tp_dst &= ICMP_CODE_MASK;
    }
  }

  if ( match->wildcards & OFPFW_DL_VLAN_PCP ) {
    match->dl_vlan_pcp = 0;
  }
  else {
    match->dl_vlan_pcp &= VLAN_PCP_MASK;
  }
  if ( match->wildcards & OFPFW_NW_TOS ) {
    match->nw_tos = 0;
  }
  else {
    match->nw_tos &= NW_TOS_MASK;
  }

  match_to_string( match, match_string, sizeof( match_string ) );
  debug( "Normalization completed ( updated match = [%s] ).", match_string );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
