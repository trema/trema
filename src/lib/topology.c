/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "trema.h"
#include "topology.h"

bool disable_auto_start_topology_daemon = false;

static char *libtopology_queue_name = NULL;
static char *topology_name = NULL;


static void ( *link_status_updated_callback )( void *,
                                               const topology_link_status * ) = NULL;
static void *link_status_updated_callback_param = NULL;

static void ( *port_status_updated_callback )( void *,
                                              const topology_port_status * ) = NULL;
static void *port_status_updated_callback_param = NULL;

static void ( *switch_status_updated_callback )( void *, const topology_switch_status* ) = NULL;
static void *switch_status_updated_callback_param = NULL;

static hash_table *transaction_table = NULL;

static bool is_subscribed = false;

struct send_request_param {
  void ( *callback )();
  void *user_data;
  uint32_t transaction_id;
  uint16_t message_type;
  struct timeval called_at;
};


static buffer*
create_request_message( const char *name ) {
  size_t req_len = strlen( name ) + 1;
  buffer *buf = alloc_buffer_with_length( req_len );
  topology_request *req = append_back_buffer( buf, req_len );
  strcpy( req->name, name );
  return buf;
}


static uint32_t
get_request_transaction_id() {
  static uint32_t transaction_id = 0;

  return transaction_id++;
}


static void
mark_transaction( struct send_request_param *param, uint16_t message_type ) {
  gettimeofday( &param->called_at, NULL );
  param->transaction_id = get_request_transaction_id();
  param->message_type = message_type;
  void *duplicated = insert_hash_entry( transaction_table, &param->transaction_id, param );
  assert( duplicated == NULL );
}


static void
unmark_transaction( struct send_request_param *param ) {
  void *deleted = delete_hash_entry( transaction_table, &param->transaction_id );
  assert( deleted != NULL );
}


static struct send_request_param *
create_request_param( void ( *callback )(), void *user_data ) {
  struct send_request_param *param = xmalloc( sizeof( *param ) );
  param->callback = callback;
  param->user_data = user_data;

  return param;
}


static bool
send_request( uint16_t message_type, void ( *callback )(), void *user_data ) {
  // register transaction and continuation
  // make request header
  buffer *buf = create_request_message( libtopology_queue_name );

  struct send_request_param *param = create_request_param( callback, user_data );
  mark_transaction( param, message_type );

  bool ret = send_request_message( topology_name, libtopology_queue_name,
                              message_type, buf->data, buf->length,
                              param );

  if ( !ret ) {
    warn("Failed to send a request message %d to %s.", message_type, topology_name);
    unmark_transaction( param );
    xfree( param );
  }
  free_buffer( buf );
  return ret;
}


static buffer *
create_update_link_status_message( const topology_update_link_status *link_status ) {
  buffer *buf = alloc_buffer_with_length( sizeof( topology_update_link_status ) );
  topology_update_link_status *req = append_back_buffer( buf, sizeof( topology_update_link_status ) );

  req->from_dpid = htonll( link_status->from_dpid );
  req->from_portno = htons( link_status->from_portno );
  req->to_dpid = htonll( link_status->to_dpid );
  req->to_portno = htons( link_status->to_portno );
  req->status = link_status->status;

  return buf;
}


static bool
send_update_link_status( const topology_update_link_status *link_status,
                         void ( *callback )(), void *user_data ) {
  // register transaction and continuation
  // make request header
  buffer *buf = create_update_link_status_message( link_status );

  struct send_request_param *param = create_request_param( callback, user_data );
  mark_transaction( param, TD_MSGTYPE_UPDATE_LINK_STATUS_REQUEST );

  bool ret = send_request_message( topology_name, libtopology_queue_name,
                              TD_MSGTYPE_UPDATE_LINK_STATUS_REQUEST,
                              buf->data, buf->length, param );

  if ( !ret ) {
    warn( "Failed to send a set link status request to %s.", topology_name );
    unmark_transaction( param );
    xfree( param );
  }
  free_buffer( buf );

  return ret;
}


// handle reply from topology
static void
recv_query_link_status_reply( uint16_t tag,
                              void *data, size_t len,
                              void *param0 ) {
  UNUSED( tag );
  topology_link_status *const link_status = data;
  const int number_of_links =( int ) ( len / sizeof( topology_link_status ) );
  int i;
  struct send_request_param *param = param0;

  if ( param->callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
  } else {
    // rearrange byte order
    for ( i = 0; i < number_of_links; i++ ) {
      topology_link_status *s = &link_status[ i ];
      s->from_dpid = ntohll( s->from_dpid );
      s->from_portno = ntohs( s->from_portno );
      s->to_dpid = ntohll( s->to_dpid );
      s->to_portno = ntohs( s->to_portno );
    }

    // rebuild link topology
    ( *param->callback )( param->user_data, number_of_links, link_status );
  }
  xfree( param );
}


// handle reply from topology
static void
recv_query_port_status_reply( uint16_t tag,
                              void *data, size_t len,
                              void *param0 ) {
  UNUSED( tag );
  topology_port_status *const port_status = data;
  const int number_of_ports =( int ) ( len / sizeof( topology_port_status ) );
  int i;
  struct send_request_param *param = param0;

  if ( param->callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
  } else {
    // rearrange byte order
    for ( i = 0; i < number_of_ports; i++ ) {
      topology_port_status *s = &port_status[ i ];
      s->dpid = ntohll( s->dpid );
      s->port_no = ntohs( s->port_no );
    }

    // rebuild link topology
    ( *param->callback )( param->user_data, number_of_ports, port_status );
  }
  xfree( param );
}


// handle reply from topology
static void
recv_query_switch_status_reply( uint16_t tag,
                                void *data, size_t len, void *param0 ) {
  UNUSED( tag );
  topology_switch_status *const switch_status = data;
  const int number_of_switches = ( int ) ( len / sizeof( topology_switch_status ) );
  int i;
  struct send_request_param *param = param0;

  if ( param->callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
  } else {
    // rearrange byte order
    for ( i = 0; i < number_of_switches; i++ ) {
      topology_switch_status *s = &switch_status[ i ];
      s->dpid = ntohll( s->dpid );
    }

    // rebuild link topology
    ( *param->callback )( param->user_data, number_of_switches, switch_status );
  }
  xfree( param );
}


// handle asynchronouse notification from topology
static void
recv_link_status_notification( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  topology_link_status *const link_status = data;
  const int number_of_links = ( int ) ( len / sizeof( topology_link_status ) );
  int i;

  if ( link_status_updated_callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
    return;
  }

  // (re)build topology db
  for ( i = 0; i < number_of_links; i++ ) {
    topology_link_status *s = &link_status[ i ];
    s->from_dpid = ntohll( s->from_dpid );
    s->from_portno = ntohs( s->from_portno );
    s->to_dpid = ntohll( s->to_dpid );
    s->to_portno = ntohs( s->to_portno );

    // TODO update link cache when implementing Topology cache in C

    ( *link_status_updated_callback )( link_status_updated_callback_param, s );
  }
}


// handle asynchronous notification from topology
static void
recv_port_status_notification( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  UNUSED( len );
  topology_port_status *const port_status = data;

  if ( port_status_updated_callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
    return;
  }

  // arrange byte order
  topology_port_status *s = port_status;
  s->dpid = ntohll( s->dpid );
  s->port_no = ntohs( s->port_no );

  // TODO update port cache when implementing Topology cache in C

  // (re)build port db
  ( *port_status_updated_callback )( port_status_updated_callback_param, s );
}


static void
recv_switch_status_notification( uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  UNUSED( len );
  topology_switch_status* switch_status = data;

  if( switch_status_updated_callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
    return;
  }

  // arrange byte order
  switch_status->dpid = ntohll( switch_status->dpid );

  // TODO update switch cache when implementing Topology cache in C

  (* switch_status_updated_callback )( switch_status_updated_callback_param, switch_status );
}


static void
recv_ping_request( const messenger_context_handle *handle, void *data, size_t len ) {
  UNUSED( len );
  UNUSED( data );

  // respond to topology ping
  const size_t name_bytes = strlen( libtopology_queue_name ) + 1;
  const size_t req_len = sizeof(topology_ping_response) + name_bytes;

  buffer *buf = alloc_buffer_with_length( req_len );
  topology_ping_response *res = append_back_buffer( buf, req_len );
  strncpy( res->name, libtopology_queue_name, name_bytes );
//  assert( res->name[name_bytes] == '\0' );

  bool ret = send_reply_message( handle, TD_MSGTYPE_PING_RESPONSE,
                      buf->data, buf->length );
  if ( !ret ) {
    warn( "Failed to respond to ping from topology." );
  }
  free_buffer( buf );
}


static void
recv_topology_response( uint16_t tag, void *data, size_t len, void *param0 ) {
  UNUSED( tag );
  if( len < sizeof(topology_response) ) {
    error( "Invalid data length. (tag=%#x len=%zu, where > %zu expected)", tag, len, sizeof(topology_response) );
    return;
  }
  topology_response *res = data;
  struct send_request_param *param = param0;

  if( res->status != TD_RESPONSE_OK ){
    warn( "Response other than TD_RESPONSE_OK received. (tag=%#x, status=%#x)", tag, res->status );
  }

  if ( param->callback == NULL ) {
    debug( "%s: callback is NULL", __FUNCTION__ );
  } else {
    ( *param->callback )( param->user_data, res );
  }
  xfree( param );
}


static void
recv_subscribe_reply( uint16_t tag, void *data, size_t len, void *param0 ) {
  UNUSED( tag );
  topology_response *res = data;

  switch ( res->status ) {
  case TD_RESPONSE_OK:
    is_subscribed = true;
    break;
  case TD_RESPONSE_ALREADY_SUBSCRIBED:
    warn( "%s: Already subscribed.", __func__ );
    is_subscribed = true;
    break;
  }

  recv_topology_response(tag, data, len, param0);
}


static void
recv_unsubscribe_reply( uint16_t tag, void *data, size_t len, void *param0 ) {
  UNUSED( tag );
  topology_response *res = data;

  switch ( res->status ) {
  case TD_RESPONSE_OK:
    is_subscribed = false;
    break;
  case TD_RESPONSE_NO_SUCH_SUBSCRIBER:
    warn( "%s: Was not subscribed.", __func__ );
    is_subscribed = false;
    break;
  }

  recv_topology_response(tag, data, len, param0);
}


bool
subscribe_topology( void ( *callback )( void *user_data, topology_response *res ), void *user_data ) {
  if( is_subscribed ){
      debug( "%s: Already subscribed.", __func__ );
  }
  return send_request( TD_MSGTYPE_SUBSCRIBE_REQUEST, callback, user_data );
}


bool
unsubscribe_topology( void ( *callback )( void *user_data, topology_response *res ), void *user_data ) {
  if( !is_subscribed ){
      debug( "%s: Not subscribed.", __func__ );
  }
  return send_request( TD_MSGTYPE_UNSUBSCRIBE_REQUEST, callback, user_data );
}


bool
enable_topology_discovery( void ( *callback )( void *user_data, const topology_response *res ), void *user_data ) {
  return send_request( TD_MSGTYPE_ENABLE_DISCOVERY_REQUEST, callback, user_data );
}


bool
disable_topology_discovery( void ( *callback )( void *user_data, const topology_response *res ), void *user_data ) {
  return send_request( TD_MSGTYPE_DISABLE_DISCOVERY_REQUEST, callback, user_data );
}


static void
recv_reply( uint16_t tag, void *data, size_t len, void *user_data ) {
  unmark_transaction( (struct send_request_param *)user_data );
  debug( "%s: %#x", __func__, (unsigned int)tag );

  switch ( tag ) {
  case TD_MSGTYPE_SUBSCRIBE_RESPONSE:
    recv_subscribe_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_UNSUBSCRIBE_RESPONSE:
    recv_unsubscribe_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_QUERY_LINK_STATUS_RESPONSE:
    recv_query_link_status_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_QUERY_PORT_STATUS_RESPONSE:
    recv_query_port_status_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_QUERY_SWITCH_STATUS_RESPONSE:
    recv_query_switch_status_reply( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_UPDATE_LINK_STATUS_RESPONSE:
    // nothing to do. using default handler.
    recv_topology_response( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_PING_RESPONSE:
    // TODO add handler for client -> server ping response
    recv_topology_response( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_ENABLE_DISCOVERY_RESPONSE:
    // nothing to do. using default handler.
    recv_topology_response( tag, data, len, user_data );
    break;

  case TD_MSGTYPE_DISABLE_DISCOVERY_RESPONSE:
    // nothing to do. using default handler.
    recv_topology_response( tag, data, len, user_data );
    break;

  default:
    warn( "%s: Unknown message type: %#x", __func__, (unsigned int)tag );
  }
}


static void
recv_status_notification( uint16_t tag, void *data, size_t len ) {
  debug( "%s: %#x", __func__, (unsigned int)tag );
  switch ( tag ) {
  case TD_MSGTYPE_LINK_STATUS_NOTIFICATION:
    recv_link_status_notification( tag, data, len );
    break;

  case TD_MSGTYPE_PORT_STATUS_NOTIFICATION:
    recv_port_status_notification( tag, data, len );
    break;

  case TD_MSGTYPE_SWITCH_STATUS_NOTIFICATION:
    recv_switch_status_notification( tag, data, len );
    break;

  default:
    warn( "%s: Unknown message type: %#x", __func__, (unsigned int)tag );
  }
}


static void
recv_request( const messenger_context_handle *handle,
              uint16_t tag, void *data, size_t len ) {
  debug( "%s: %#x", __func__, (unsigned int)tag );
  switch ( tag ) {
  case TD_MSGTYPE_PING_REQUEST:
    // ping: topology service -> libtopology
    recv_ping_request( handle, data, len );
    break;

  default:
    warn( "%s: Unknown message type: %#x", __func__, (unsigned int)tag );
  }
}


static void
check_transaction_table( void *user_data ) {
  hash_table *transaction_table = user_data;

  // TODO implement client -> server heart beat check here?

  hash_iterator iter;
  init_hash_iterator( transaction_table, &iter );
  hash_entry *e;
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    struct send_request_param *param = e->value;
    debug( "Outstanding transaction still remains: %#x", param->transaction_id );
    debug( " message type is %#x", param->message_type );
    char buf[ 32 ];
    debug( " called at %s", ctime_r( ( time_t * ) &param->called_at.tv_sec, buf ) );
  }
}


static const char TOPOLOGY_DAEMON_PATH[] = "objects/topology/topology";

static void
maybe_start_topology_daemon( const char* service_name ) {
  if ( disable_auto_start_topology_daemon ) return;

  // topology daemon process check
  if ( get_trema_process_from_name( service_name ) > 0 ) {
    return;
  }
  // topology daemon not running

  pid_t pid = fork();
  if ( pid < 0 ) {
    error( "Failed to fork. %s.", strerror( errno ) );
    return;
  }

  if ( pid == 0 ) {
    // child process

    // close all open fd
    DIR* dir = opendir( "/proc/self/fd" );
    if ( dir == NULL ) {
      error( "Failed to opendir. %s.", strerror( errno ) );
    } else {
      struct dirent *entry;
      while ( ( entry = readdir( dir ) ) != NULL ) {
        int openfd = atoi( entry->d_name );
        close( openfd );
      }
      closedir( dir );
      dir = NULL;
    }

    // exec
    int in_fd = open( "/dev/null", O_RDONLY );
    if ( in_fd != 0 ) {
      dup2( in_fd, 0 );
      close( in_fd );
    }
    int out_fd = open( "/dev/null", O_WRONLY );
    if ( out_fd != 1 ) {
      dup2( out_fd, 1 );
      close( out_fd );
    }
    int err_fd = open( "/dev/null", O_WRONLY );
    if ( err_fd != 2 ) {
      dup2( err_fd, 2 );
      close( err_fd );
    }
    char* daemon_path = xasprintf( "%s/%s", get_trema_home(),
                                   TOPOLOGY_DAEMON_PATH );
    char arg_daemonize[ ] = "-d";
    char arg_name[ ] = "-n";
    char* arg_name_opt = xstrdup( service_name );

    char* argv[ 5 ];
    argv[ 0 ] = daemon_path;
    argv[ 1 ] = arg_daemonize;
    argv[ 2 ] = arg_name;
    argv[ 3 ] = arg_name_opt;
    argv[ 4 ] = NULL; 

    execvp( daemon_path, argv );
    int err = errno;
    error( "Failed to execvp: %s %s %s %s. %s.", argv[ 0 ], argv[ 1 ],
           argv[ 2 ], argv[ 3 ], strerror( err ) );

//    xfree( daemon_path );
//    xfree( arg_name_opt );
    UNREACHABLE();
  } else {
    // parent process
    // wait for topology daemon to start
    int try = 0;
    const int max_try = 5;
    while ( get_trema_process_from_name( service_name ) == -1 && ++try <= max_try ) {
      sleep( 1 );
    }
  }
}


bool
init_libtopology( const char *service_name ) {
  if ( topology_name != NULL || libtopology_queue_name != NULL ) {
    debug( "already initialized" );
    return false;
  }

  maybe_start_topology_daemon( service_name );

  const size_t server_name_len = strlen( service_name ) + strlen( ".t" ) + 1;
  if ( server_name_len > MESSENGER_SERVICE_NAME_LENGTH ) {
    die( "Service name too long to create topology service name ( %s ).", service_name );
  }
  topology_name = xcalloc( 1, server_name_len );
  snprintf( topology_name, server_name_len, "%s.t", service_name );

  const size_t client_name_len = strlen( service_name ) + strlen( "-c-4294967295" ) + 1;
  if ( client_name_len > MESSENGER_SERVICE_NAME_LENGTH ) {
    die( "Service name too long to create topology client name ( %s ).", service_name );
  }
  libtopology_queue_name = xcalloc( 1, client_name_len );
  snprintf( libtopology_queue_name, client_name_len, "%s-c-%d",
            service_name, getpid() );

  add_message_requested_callback( libtopology_queue_name, recv_request );
  add_message_replied_callback( libtopology_queue_name, recv_reply );
  add_message_received_callback( libtopology_queue_name, recv_status_notification );

  transaction_table = create_hash( compare_uint32, hash_uint32 );
  add_periodic_event_callback( 60, check_transaction_table, transaction_table );

  return true;
}


bool
add_callback_switch_status_updated( void ( *callback )( void *user_data,
                                                           const topology_switch_status *link_status ),
                                                           void *user_data ) {

  switch_status_updated_callback = callback;
  switch_status_updated_callback_param = user_data;

  return true;
}


bool
add_callback_port_status_updated(
  void ( *callback )( void *, const topology_port_status * ), void *param ) {

  port_status_updated_callback = callback;
  port_status_updated_callback_param = param;

  return true;
}


bool
add_callback_link_status_updated(
  void ( *callback )( void *, const topology_link_status * ), void *param ) {

  link_status_updated_callback = callback;
  link_status_updated_callback_param = param;

  return true;
}


static void
free_all_transaction_table( void *key, void *value, void *user_data ) {
  UNUSED( key );
  UNUSED( user_data );
  xfree( value );
}


bool
finalize_libtopology() {
  if ( topology_name == NULL || libtopology_queue_name == NULL ) {
    debug( "already finalized" );
    return false;
  }
  xfree( topology_name );
  topology_name = NULL;

  delete_message_received_callback( libtopology_queue_name, recv_status_notification );
  delete_message_replied_callback( libtopology_queue_name, recv_reply );
  delete_message_requested_callback( libtopology_queue_name, recv_request );

  xfree( libtopology_queue_name );
  libtopology_queue_name = NULL;

  delete_timer_event( check_transaction_table, transaction_table );

  assert( transaction_table != NULL );
  if( transaction_table->length > 0 ) {
    debug( "%u outstanding transaction left.", transaction_table->length );
    check_transaction_table( transaction_table );
    foreach_hash( transaction_table, free_all_transaction_table, NULL );
  }
  delete_hash( transaction_table );
  transaction_table = NULL;

  return true;
}


bool
get_all_link_status( void ( *callback )(), void *user_data ) {
  // send request message
  return send_request( TD_MSGTYPE_QUERY_LINK_STATUS_REQUEST, callback, user_data );
}


bool
get_all_port_status( void ( *callback )(), void *user_data ) {
  // send request message
  return send_request( TD_MSGTYPE_QUERY_PORT_STATUS_REQUEST, callback, user_data );
}


bool
get_all_switch_status( void ( *callback )(), void *user_data ) {
  // send request message
  return send_request( TD_MSGTYPE_QUERY_SWITCH_STATUS_REQUEST, callback, user_data );
}


bool
set_link_status( const topology_update_link_status *link_status,
                 void ( *callback )( void *user_data, const topology_response *res ), void *user_data ) {
  // send request message
  return send_update_link_status( link_status, callback, user_data );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
