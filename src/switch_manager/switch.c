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


#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <openflow.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "trema.h"
#include "cookie_table.h"
#include "message_queue.h"
#include "messenger.h"
#include "ofpmsg_send.h"
#include "openflow_service_interface.h"
#include "secure_channel_receiver.h"
#include "secure_channel_sender.h"
#include "service_interface.h"
#include "switch.h"
#include "xid_table.h"
#include "switch_option.h"
#include "event_forward_entry_manipulation.h"

#define SUB_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec - ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec - ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec < 0 ) {                         \
      ( _return )->tv_sec--;                                  \
      ( _return )->tv_nsec += 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )


struct switch_info switch_info;

static const time_t COOKIE_TABLE_AGING_INTERVAL = 3600; // sec.
static const time_t ECHO_REQUEST_INTERVAL = 60; // sec.
static const time_t ECHO_REPLY_TIMEOUT = 2; // ses.
static const time_t WARNING_ECHO_RTT = 500; // msec. The value must is less than 1000.


static bool age_cookie_table_enabled = false;

typedef struct {
  uint64_t datapath_id;
  uint32_t sec;
  uint32_t nsec;
} echo_body;

void
usage() {
  printf(
    "OpenFlow Switch Manager.\n"
    "Usage: %s [OPTION]... [DESTINATION-RULE]...\n"
    "\n"
    "  -s, --socket=fd                 secure channnel socket\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "      --no-flow-cleanup           do not cleanup flows on startup\n"
    "      --no-cookie-translation     do not translate cookie values\n"
    "      --no-packet_in              do not allow packet-ins on startup\n"
    "  -h, --help                      display this help and exit\n"
    "\n"
    "DESTINATION-RULE:\n"
    "  openflow-message-type::destination-service-name\n"
    "\n"
    "openflow-message-type:\n"
    "  packet_in                       packet-in openflow message type\n"
    "  port_status                     port-status openflow message type\n"
    "  vendor                          vendor openflow message type\n"
    "  state_notify                    connection status\n"
    "\n"
    "destination-service-name          destination service name\n"
    , get_executable_name()
  );
}


static int
strtofd( const char *str ) {
  char *ep;
  long l;

  l = strtol( str, &ep, 0 );
  if ( l <  0 || l > INT_MAX || *ep != '\0' ) {
    die( "Invalid socket (%s).", str );
    return 0;
  }
  return ( int ) l;
}


static void
option_parser( int argc, char *argv[] ) {
  int c;

  switch_info.secure_channel_fd = 0; // stdin
  switch_info.flow_cleanup = true;
  switch_info.cookie_translation = true;
  switch_info.deny_packet_in_on_startup = false;
  while ( ( c = getopt_long( argc, argv, switch_short_options, switch_long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 's':
        switch_info.secure_channel_fd = strtofd( optarg );
        break;

      case NO_FLOW_CLEANUP_LONG_OPTION_VALUE:
        switch_info.flow_cleanup = false;
        break;

      case NO_COOKIE_TRANSLATION_LONG_OPTION_VALUE:
        switch_info.cookie_translation = false;
        break;

      case NO_PACKET_IN_LONG_OPTION_VALUE:
        switch_info.deny_packet_in_on_startup = true;
        break;

      default:
        usage();
        exit( EXIT_SUCCESS );
        return;
    }
  }
}


static void
service_recv( uint16_t message_type, void *data, size_t data_len ) {
  buffer *buf;
  void *msg;

  buf = alloc_buffer_with_length( data_len );

  msg = append_back_buffer( buf, data_len );
  memcpy( msg, data, data_len );

  service_recv_from_application( message_type, buf );
}


static void
service_send_state( struct switch_info *sw_info, uint64_t *dpid, uint16_t tag ) {
  service_send_to_application( sw_info->state_service_name_list, tag, dpid, NULL );
}


static void
secure_channel_read( int fd, void *data ) {
  UNUSED( fd );
  UNUSED( data );

  if ( recv_from_secure_channel( &switch_info ) < 0 ) {
    switch_event_disconnected( &switch_info );
    return;
  }

  if ( switch_info.recv_queue->length > 0 ) {
    int ret = handle_messages_from_secure_channel( &switch_info );
    if ( ret < 0 ) {
      stop_event_handler();
      stop_messenger();
    }
  }
}


static void
secure_channel_write( int fd, void *data ) {
  UNUSED( fd );
  UNUSED( data );

  if ( flush_secure_channel( &switch_info ) < 0 ) {
    switch_event_disconnected( &switch_info );
    return;
  }
}


static void
switch_set_timeout( long sec, timer_callback callback, void *user_data ) {
  struct itimerspec interval;

  interval.it_value.tv_sec = sec;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 0;
  add_timer_event_callback( &interval, callback, user_data );
  switch_info.running_timer = true;
}


static void
switch_unset_timeout( timer_callback callback, void *user_data ) {
  if ( switch_info.running_timer ) {
    switch_info.running_timer = false;
    delete_timer_event( callback, user_data );
  }
}


static void
switch_event_timeout_hello( void *user_data ) {
  UNUSED( user_data );

  if ( switch_info.state != SWITCH_STATE_WAIT_HELLO ) {
    return;
  }
  switch_info.running_timer = false;

  error( "Hello timeout. state:%d, dpid:%#" PRIx64 ", fd:%d.",
         switch_info.state, switch_info.datapath_id, switch_info.secure_channel_fd );
  switch_event_disconnected( &switch_info );
}


static void
switch_event_timeout_features_reply( void *user_data ) {
  UNUSED( user_data );

  if ( switch_info.state != SWITCH_STATE_WAIT_FEATURES_REPLY ) {
    return;
  }
  switch_info.running_timer = false;

  error( "Features Reply timeout. state:%d, dpid:%#" PRIx64 ", fd:%d.",
         switch_info.state, switch_info.datapath_id, switch_info.secure_channel_fd );
  switch_event_disconnected( &switch_info );
}


int
switch_event_connected( struct switch_info *sw_info ) {
  int ret;

  // send secure channel disconnect state to application
  service_send_state( sw_info, &sw_info->datapath_id, MESSENGER_OPENFLOW_CONNECTED );
  debug( "Send connected state" );
  ret = ofpmsg_send_hello( sw_info );
  if ( ret < 0 ) {
    return ret;
  }
  sw_info->state = SWITCH_STATE_WAIT_HELLO;

  switch_set_timeout( SWITCH_STATE_TIMEOUT_HELLO, switch_event_timeout_hello, NULL );

  return 0;
}


int
switch_event_recv_hello( struct switch_info *sw_info ) {
  int ret;

  if ( sw_info->state == SWITCH_STATE_WAIT_HELLO ) {
    // cancel to hello_wait-timeout timer
    switch_unset_timeout( switch_event_timeout_hello, NULL );

    if ( sw_info->deny_packet_in_on_startup ) {
      ret = ofpmsg_send_deny_all( sw_info );
      if ( ret < 0 ) {
        return ret;
      }
    }

    ret = ofpmsg_send_featuresrequest( sw_info );
    if ( ret < 0 ) {
      return ret;
    }
    sw_info->state = SWITCH_STATE_WAIT_FEATURES_REPLY;

    switch_set_timeout( SWITCH_STATE_TIMEOUT_FEATURES_REPLY,
                        switch_event_timeout_features_reply, NULL );
  }

  return 0;
}


static void
echo_reply_timeout( void *user_data ) {
  UNUSED( user_data );

  error( "Echo request timeout ( datapath id %#" PRIx64 ").", switch_info.datapath_id );
  switch_event_disconnected( &switch_info );
}


int
switch_event_recv_echoreply( struct switch_info *sw_info, buffer *buf ) {
  if ( buf->length != sizeof( struct ofp_header ) + sizeof( echo_body ) ) {
    return 0;
  }
  struct ofp_header *header = buf->data;
  if ( ntohl( header->xid ) != sw_info->echo_request_xid ) {
    return 0;
  }

  echo_body *body = ( echo_body * ) ( header + 1 );
  if ( ntohll( body->datapath_id ) != sw_info->datapath_id ) {
    return 0;
  }
  switch_unset_timeout( echo_reply_timeout, NULL );
  struct timespec now, tim;
  clock_gettime( CLOCK_MONOTONIC, &now );
  tim.tv_sec = ( time_t ) ntohl( body->sec );
  tim.tv_nsec = ( long ) ntohl( body->nsec );

  SUB_TIMESPEC( &now, &tim, &tim );

  if ( tim.tv_sec > 0 || tim.tv_nsec > ( ( long ) WARNING_ECHO_RTT * 1000000 ) ) {
    warn( "echo round-trip time is greater then %ld ms ( round-trip time = %" PRId64 ".%09ld ).",
          ( long ) WARNING_ECHO_RTT, ( int64_t ) tim.tv_sec, tim.tv_nsec );
  }

  return 0;
}


static void
echo_request_interval( void *user_data ) {
  struct switch_info *sw_info = user_data;

  buffer *buf = alloc_buffer();
  echo_body *body = append_back_buffer( buf, sizeof( echo_body ) );
  body->datapath_id = htonll( switch_info.datapath_id );
  struct timespec now;
  clock_gettime( CLOCK_MONOTONIC, &now );
  body->sec = htonl( ( uint32_t ) now.tv_sec );
  body->nsec = htonl( ( uint32_t ) now.tv_nsec );
  sw_info->echo_request_xid = generate_xid();

  int err = ofpmsg_send_echorequest( sw_info, sw_info->echo_request_xid, buf );
  if ( err < 0 ) {
    switch_event_disconnected( &switch_info );
    return;
  }

  switch_set_timeout( ECHO_REPLY_TIMEOUT, echo_reply_timeout, NULL );
}


int
switch_event_recv_featuresreply( struct switch_info *sw_info, uint64_t *dpid ) {
  int ret;
  char new_service_name[ SWITCH_MANAGER_PREFIX_STR_LEN + SWITCH_MANAGER_DPID_STR_LEN + 1 ];
  const uint16_t new_service_name_len = SWITCH_MANAGER_PREFIX_STR_LEN + SWITCH_MANAGER_DPID_STR_LEN + 1;

  switch ( sw_info->state ) {
  case SWITCH_STATE_WAIT_FEATURES_REPLY:

    sw_info->datapath_id = *dpid;
    sw_info->state = SWITCH_STATE_COMPLETED;

    // cancel to features_reply_wait-timeout timer
    switch_unset_timeout( switch_event_timeout_features_reply, NULL );

    // TODO: set keepalive-timeout
    snprintf( new_service_name, new_service_name_len, "%s%#" PRIx64, SWITCH_MANAGER_PREFIX, sw_info->datapath_id );

    // checking duplicate service
    pid_t pid = get_pid_by_trema_name( new_service_name );
    if ( pid > 0 ) {
      // duplicated
      if ( !terminate_trema_process( pid ) ) {
        return -1;
      }
    }
    // rename service_name of messenger
    rename_message_received_callback( get_trema_name(), new_service_name );

    // rename management service name
    char *management_service_name = xstrdup( get_management_service_name( get_trema_name() ) );
    char *new_management_service_name = xstrdup( get_management_service_name( new_service_name ) );
    rename_message_requested_callback( management_service_name, new_management_service_name );
    xfree( management_service_name );
    xfree( new_management_service_name );

    debug( "Rename service name from %s to %s.", get_trema_name(), new_service_name );
    if ( messenger_dump_enabled() ) {
      stop_messenger_dump();
      start_messenger_dump( new_service_name, DEFAULT_DUMP_SERVICE_NAME );
    }
    set_trema_name( new_service_name );

    // notify state and datapath_id
    service_send_state( sw_info, &sw_info->datapath_id, MESSENGER_OPENFLOW_READY );
    debug( "send ready state" );

    ret = ofpmsg_send_setconfig( sw_info );
    if ( ret < 0 ) {
      return ret;
    }
    if ( switch_info.flow_cleanup ) {
      ret = ofpmsg_send_delete_all_flows( sw_info );
      if ( ret < 0 ) {
        return ret;
      }
    }
    add_periodic_event_callback( ECHO_REQUEST_INTERVAL, echo_request_interval, sw_info );
    break;

  case SWITCH_STATE_COMPLETED:
    // NOP
    break;

  default:
    notice( "Invalid event 'features reply' from a switch." );
    return -1;

    break;
  }

  return 0;
}


int
switch_event_disconnected( struct switch_info *sw_info ) {
  int old_state = sw_info->state;

  sw_info->state = SWITCH_STATE_DISCONNECTED;

  if ( old_state == SWITCH_STATE_COMPLETED ) {
    delete_timer_event( echo_request_interval, sw_info );
  }

  if ( sw_info->fragment_buf != NULL ) {
    free_buffer( sw_info->fragment_buf );
    sw_info->fragment_buf = NULL;
  }

  if ( sw_info->send_queue != NULL ) {
    delete_message_queue( sw_info->send_queue );
    sw_info->send_queue = NULL;
  }

  if ( sw_info->recv_queue != NULL ) {
    delete_message_queue( sw_info->recv_queue );
    sw_info->recv_queue = NULL;
  }

  if ( sw_info->secure_channel_fd >= 0 ) {
    set_readable( switch_info.secure_channel_fd, false );
    set_writable( switch_info.secure_channel_fd, false );
    delete_fd_handler( switch_info.secure_channel_fd );

    close( sw_info->secure_channel_fd );
    sw_info->secure_channel_fd = -1;
  }

  if ( old_state != SWITCH_STATE_COMPLETED ) {
    service_send_state( sw_info, &sw_info->datapath_id, MESSENGER_OPENFLOW_FAILD_TO_CONNECT );
  }
  else {
    // send secure channle disconnect state to application
    service_send_state( sw_info, &sw_info->datapath_id, MESSENGER_OPENFLOW_DISCONNECTED );
  }
  flush_messenger();

  // free service name list
  iterate_list( sw_info->vendor_service_name_list, xfree_data, NULL );
  delete_list( sw_info->vendor_service_name_list );
  sw_info->vendor_service_name_list = NULL;
  iterate_list( sw_info->packetin_service_name_list, xfree_data, NULL );
  delete_list( sw_info->packetin_service_name_list );
  sw_info->packetin_service_name_list = NULL;
  iterate_list( sw_info->portstatus_service_name_list, xfree_data, NULL );
  delete_list( sw_info->portstatus_service_name_list );
  sw_info->portstatus_service_name_list = NULL;
  iterate_list( sw_info->state_service_name_list, xfree_data, NULL );
  delete_list( sw_info->state_service_name_list );
  sw_info->state_service_name_list = NULL;

  stop_trema();

  return 0;
}


int
switch_event_recv_from_application( uint64_t *datapath_id, char *application_service_name, buffer *buf ) {

  if ( *datapath_id != switch_info.datapath_id ) {
    error( "Invalid datapath id %#" PRIx64 ".", *datapath_id );
    free_buffer( buf );

    return -1;
  }

  return ofpmsg_send( &switch_info, buf, application_service_name );
}


int
switch_event_disconnect_request( uint64_t *datapath_id ) {

  if ( *datapath_id != switch_info.datapath_id ) {
    error( "Invalid datapath id %#" PRIx64 ".", *datapath_id );
    return -1;
  }
  return switch_event_disconnected( &switch_info );
}


int
switch_event_recv_error( struct switch_info *sw_info ) {
  if ( sw_info->state == SWITCH_STATE_COMPLETED ) {
    return 0;
  }

  return -1;
}


static void
management_event_forward_entry_operation( const messenger_context_handle *handle, uint32_t command, event_forward_operation_request *req, size_t data_len ) {

  debug( "management efi command:%#x, type:%#x, n_services:%d", command, req->type, req->n_services );

  list_element **subject = NULL;
  switch ( req->type ) {
    case EVENT_FORWARD_TYPE_VENDOR:
      info( "Managing vendor event." );
      subject = &switch_info.vendor_service_name_list;
      break;

    case EVENT_FORWARD_TYPE_PACKET_IN:
      info( "Managing packet_in event." );
      subject = &switch_info.packetin_service_name_list;
      break;

    case EVENT_FORWARD_TYPE_PORT_STATUS:
      info( "Managing port_status event." );
      subject = &switch_info.portstatus_service_name_list;
      break;

    case EVENT_FORWARD_TYPE_STATE_NOTIFY:
      info( "Managing state_notify event." );
      subject = &switch_info.state_service_name_list;
      break;

    default:
      error( "Invalid EVENT_FWD_TYPE ( %#x )", req->type );
      event_forward_operation_reply res;
      memset( &res, 0, sizeof( event_forward_operation_reply ) );
      res.type = req->type;
      res.result = EFI_OPERATION_FAILED;
      management_application_reply *reply = create_management_application_reply( MANAGEMENT_REQUEST_FAILED, command, &res, sizeof( event_forward_operation_reply ) );
      send_management_application_reply( handle, reply );
      xfree( reply );
      return;
  }
  assert( subject != NULL );

  switch ( command ) {
    case EVENT_FORWARD_ENTRY_ADD:
      management_event_forward_entry_add( subject, req, data_len );
      break;

    case EVENT_FORWARD_ENTRY_DELETE:
      management_event_forward_entry_delete( subject, req, data_len );
      break;

    case EVENT_FORWARD_ENTRY_DUMP:
      info( "Dumping current event filter." );
      // do nothing
      break;

    case EVENT_FORWARD_ENTRY_SET:
      management_event_forward_entries_set( subject, req, data_len );
      break;
  }

  buffer *buf = create_event_forward_operation_reply( req->type, EFI_OPERATION_SUCCEEDED, *subject );
  management_application_reply *reply = create_management_application_reply( MANAGEMENT_REQUEST_SUCCEEDED, command, buf->data, buf->length );
  free_buffer( buf );
  send_management_application_reply( handle, reply );
  xfree( reply );
}


static void
management_recv( const messenger_context_handle *handle, uint32_t command, void *data, size_t data_len, void *user_data ) {
  UNUSED( user_data );

  switch ( command ) {
    case DUMP_XID_TABLE:
    {
      dump_xid_table();
    }
    break;

    case DUMP_COOKIE_TABLE:
    {
      if ( !switch_info.cookie_translation ) {
        break;
      }
      dump_cookie_table();
    }
    break;

    case TOGGLE_COOKIE_AGING:
    {
      if ( !switch_info.cookie_translation ) {
        break;
      }
      if ( age_cookie_table_enabled ) {
        delete_timer_event( age_cookie_table, NULL );
        age_cookie_table_enabled = false;
      }
      else {
        add_periodic_event_callback( COOKIE_TABLE_AGING_INTERVAL, age_cookie_table, NULL );
        age_cookie_table_enabled = true;
      }
    }
    break;

    case EVENT_FORWARD_ENTRY_ADD:
    case EVENT_FORWARD_ENTRY_DELETE:
    case EVENT_FORWARD_ENTRY_DUMP:
    case EVENT_FORWARD_ENTRY_SET:
    {
      event_forward_operation_request *req = data;
      req->n_services = ntohl( req->n_services );
      management_event_forward_entry_operation( handle, command, req, data_len );
      return;
    }
    break;

    default:
    {
      error( "Undefined management command ( %#x )", command );
      management_application_reply *reply = create_management_application_reply( MANAGEMENT_REQUEST_FAILED, command, NULL, 0 );
      send_management_application_reply( handle, reply );
      xfree( reply );
      return;
    }
  }

  management_application_reply *reply = create_management_application_reply( MANAGEMENT_REQUEST_SUCCEEDED, command, NULL, 0 );
  send_management_application_reply( handle, reply );
  xfree( reply );
}


static void
stop_switch_daemon( void ) {
  switch_event_disconnected( &switch_info );
}


static void
handle_sigterm( int signum ) {
  UNUSED( signum );

  if ( !set_external_callback( stop_switch_daemon ) ) {
    stop_trema();
  }
}


int
main( int argc, char *argv[] ) {
  int ret;
  int i;
  char *service_name;

  init_trema( &argc, &argv );
  option_parser( argc, argv );

  create_list( &switch_info.vendor_service_name_list );
  create_list( &switch_info.packetin_service_name_list );
  create_list( &switch_info.portstatus_service_name_list );
  create_list( &switch_info.state_service_name_list );

  for ( i = optind; i < argc; i++ ) {
    if ( strncmp( argv[ i ], VENDOR_PREFIX, strlen( VENDOR_PREFIX ) ) == 0 ) {
      service_name = xstrdup( argv[ i ] + strlen( VENDOR_PREFIX ) );
      insert_in_front( &switch_info.vendor_service_name_list, service_name );
    }
    else if ( strncmp( argv[ i ], PACKET_IN_PREFIX, strlen( PACKET_IN_PREFIX ) ) == 0 ) {
      service_name = xstrdup( argv[ i ] + strlen( PACKET_IN_PREFIX ) );
      insert_in_front( &switch_info.packetin_service_name_list, service_name );
    }
    else if ( strncmp( argv[ i ], PORTSTATUS_PREFIX, strlen( PORTSTATUS_PREFIX ) ) == 0 ) {
      service_name = xstrdup( argv[ i ] + strlen( PORTSTATUS_PREFIX ) );
      insert_in_front( &switch_info.portstatus_service_name_list, service_name );
    }
    else if ( strncmp( argv[ i ], STATE_PREFIX, strlen( STATE_PREFIX ) ) == 0 ) {
      service_name = xstrdup( argv[ i ] + strlen( STATE_PREFIX ) );
      insert_in_front( &switch_info.state_service_name_list, service_name );
    }
  }

  struct sigaction signal_exit;
  memset( &signal_exit, 0, sizeof( struct sigaction ) );
  signal_exit.sa_handler = handle_sigterm;
  sigaction( SIGINT, &signal_exit, NULL );
  sigaction( SIGTERM, &signal_exit, NULL );

  fcntl( switch_info.secure_channel_fd, F_SETFL, O_NONBLOCK );

  set_fd_handler( switch_info.secure_channel_fd, secure_channel_read, NULL, secure_channel_write, NULL );
  set_readable( switch_info.secure_channel_fd, true );
  set_writable( switch_info.secure_channel_fd, false );

  // default switch configuration
  switch_info.config_flags = OFPC_FRAG_NORMAL;
  switch_info.miss_send_len = UINT16_MAX;

  switch_info.fragment_buf = NULL;
  switch_info.send_queue = create_message_queue();
  switch_info.recv_queue = create_message_queue();
  switch_info.running_timer = false;
  switch_info.echo_request_xid = 0;

  init_xid_table();
  if ( switch_info.cookie_translation ) {
    init_cookie_table();
  }

  add_message_received_callback( get_trema_name(), service_recv );
  set_management_application_request_handler( management_recv, NULL );

  ret = switch_event_connected( &switch_info );
  if ( ret < 0 ) {
    error( "Failed to set connected state." );
    return -1;
  }
  ret = flush_secure_channel( &switch_info );
  if ( ret < 0 ) {
    error( "Failed to flush secure channel. Terminating %s.", argv[ 0 ] );
    return -1;
  }

  start_trema();

  finalize_xid_table();
  if ( switch_info.cookie_translation ) {
    finalize_cookie_table();
  }

  if ( switch_info.secure_channel_fd >= 0 ) {
    delete_fd_handler( switch_info.secure_channel_fd );
  }

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
