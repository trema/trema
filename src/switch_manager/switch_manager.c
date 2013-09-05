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


#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <openflow.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include "trema.h"
#include "secure_channel_listener.h"
#include "switch_manager.h"
#include "dpid_table.h"
#include "switch_option.h"
#include "event_forward_entry_manipulation.h"


#ifdef UNIT_TESTING
#define static

#define main switch_manager_main

#ifdef printf
#undef printf
#endif
#define printf( fmt, args... )  mock_printf2( fmt, ##args )
int mock_printf2( const char *format, ... );

#ifdef die
#undef die
#endif
#define die( fmt, ... )                          \
  do {                                           \
  }                                              \
  while ( 0 )

#ifdef exit
#undef exit
#endif
#define exit mock_exit
void mock_exit( int status );

#ifdef wait3
#undef wait3
#endif
#define wait3 mock_wait3
pid_t mock_wait3( int *status, int options, struct rusage *rusage );

#ifdef secure_channel_accept
#undef secure_channel_accept
#endif
#define secure_channel_accept mock_secure_channel_accept
void mock_secure_channel_accept( struct listener_info *listener_info );

#ifdef access
#undef access
#endif
#define access mock_access
int mock_access( const char *pathname, int mode );

#ifdef get_current_dir_name
#undef get_current_dir_name
#endif
#define get_current_dir_name mock_get_current_dir_name
char *mock_get_current_dir_name( void );

#ifdef init_trema
#undef init_trema
#endif
#define init_trema mock_init_trema
void mock_init_trema( int *argc, char ***argv );

#ifdef set_fd_set_callback
#undef set_fd_set_callback
#endif
#define set_fd_set_callback mock_set_fd_set_callback
void mock_set_fd_set_callback( void ( *callback )( fd_set *read_set, fd_set *write_set ) );

#ifdef set_check_fd_isset_callback
#undef set_check_fd_isset_callback
#endif
#define set_check_fd_isset_callback mock_set_check_fd_isset_callback
void mock_set_check_fd_isset_callback( void ( *callback )( fd_set *read_set, fd_set *write_set ) );

#ifdef secure_channel_accept
#undef secure_channel_accept
#endif
#define secure_channel_accept mock_secure_channel_accept
void mock_secure_channel_accept( struct listener_info *listener_info );

#ifdef secure_channel_listen_start
#undef secure_channel_listen_start
#endif
#define secure_channel_listen_start mock_secure_channel_listen_start
bool mock_secure_channel_listen_start( struct listener_info *listener_info );

#ifdef get_trema_home
#undef get_trema_home
#endif
#define get_trema_home mock_get_trema_home
char *mock_get_trema_home( void );

#ifdef start_trema
#undef start_trema
#endif
#define start_trema mock_start_trema
void mock_start_trema( void );

#ifdef get_executable_name
#undef get_executable_name
#endif
#define get_executable_name mock_get_executable_name
const char *mock_get_executable_name( void );

#ifdef set_external_callback
#undef set_external_callback
#endif
#define set_external_callback mock_set_external_callback
void mock_set_external_callback( void ( *callback )( void ) );

#endif // UNIT_TESTING


struct listener_info listener_info;


static struct option long_options[] = {
  { "port", 1, NULL, 'p' },
  { "switch", 1, NULL, 's' },
  { NULL, 0, NULL, 0  },
};

static char short_options[] = "p:s:";


void
usage() {
  printf(
    "OpenFlow Switch Manager.\n"
    "Usage: %s [OPTION]... [-- SWITCH_DAEMON_OPTION]...\n"
    "\n"
    "  -s, --switch=PATH               the command path of switch\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -p, --port=PORT                 server listen port (default %u)\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
    , get_executable_name(), OFP_TCP_PORT
  );
}


static void
wait_child( void ) {
  int status;
  pid_t pid;

  while ( 1 ) {
    pid = wait3( &status, WNOHANG, NULL );
    if ( pid <= 0 ) {
      break;
    }
    if ( WIFEXITED( status ) ) {
      debug( "Child process is exited. pid:%d, status:%d", pid, WEXITSTATUS( status ) );
    }
    else if ( WIFSIGNALED( status ) ) {
      if ( WCOREDUMP( status ) ) {
        debug( "Child process dumped core. pid:%d, signal:%d", pid, WTERMSIG( status ) );
      }
      else {
        debug( "Child process is terminated. pid:%d, signal:%d", pid, WTERMSIG( status ) );
      }
    }
  }
}


static void
handle_sigchld( int signum ) {
  UNUSED( signum );

  // because wait_child() is not signal safe, we call it later.
  if ( set_external_callback != NULL ) {
    set_external_callback( wait_child );
  }
}


static char *
xconcatenate_path( const char *dir, const char *file ) {
  size_t len;
  const char *sp = "/";
  char *p;

  len = strlen( dir ) + strlen( file ) + 2;
  if ( dir[ strlen( dir ) - 1 ] == '/' ) {
    sp = "";
  }
  p = xmalloc( len );
  snprintf( p, len, "%s%s%s", dir, sp, file );

  return p;
}


static char *
absolute_path( const char *dir, const char *file ) {
  char *p;
  int ret;

  if ( *file == '/' ) {
    p = xstrdup( file );
  }
  else {
    p = xconcatenate_path( dir, file );
  }

  // x bit check
  ret = access( p, X_OK );
  if ( ret != 0 ) {
    die( "%s: %s", p, strerror( errno ) );
    xfree( p );
    return NULL;
  }

  return p;
}


static void
init_listener_info( struct listener_info *listener_info ) {
  memset( listener_info, 0, sizeof( struct listener_info ) );
  listener_info->switch_daemon = xconcatenate_path( get_trema_home(), SWITCH_MANAGER_PATH );
  listener_info->listen_port = OFP_TCP_PORT;
  listener_info->listen_fd = -1;
  create_list( &listener_info->vendor_service_name_list );
  create_list( &listener_info->packetin_service_name_list );
  create_list( &listener_info->portstatus_service_name_list );
  create_list( &listener_info->state_service_name_list );
}


static void
finalize_listener_info( struct listener_info *listener_info ) {
  if ( listener_info->switch_daemon != NULL ) {
    xfree( ( void * ) ( uintptr_t ) listener_info->switch_daemon );
    listener_info->switch_daemon = NULL;
  }
  if ( listener_info->listen_fd >= 0 ) {
    set_readable( listener_info->listen_fd, false );
    delete_fd_handler( listener_info->listen_fd );

    close( listener_info->listen_fd );
    listener_info->listen_fd = -1;
  }
  for ( int i = 0; i < listener_info->switch_daemon_argc; ++i ) {
    xfree( listener_info->switch_daemon_argv[ i ] );
    listener_info->switch_daemon_argv[ i ] = NULL;
  }
  xfree( listener_info->switch_daemon_argv );
  listener_info->switch_daemon_argv = NULL;

  iterate_list( listener_info->vendor_service_name_list, xfree_data, NULL );
  delete_list( listener_info->vendor_service_name_list );
  listener_info->vendor_service_name_list = NULL;
  iterate_list( listener_info->packetin_service_name_list, xfree_data, NULL );
  delete_list( listener_info->packetin_service_name_list );
  listener_info->packetin_service_name_list = NULL;
  iterate_list( listener_info->portstatus_service_name_list, xfree_data, NULL );
  delete_list( listener_info->portstatus_service_name_list );
  listener_info->portstatus_service_name_list = NULL;
  iterate_list( listener_info->state_service_name_list, xfree_data, NULL );
  delete_list( listener_info->state_service_name_list );
  listener_info->state_service_name_list = NULL;
}


static uint16_t
strtoport( const char *str ) {
  char *ep;
  long l;

  l = strtol( str, &ep, 0 );
  if ( l <= 0 || l > USHRT_MAX || *ep != '\0' ) {
    die( "Invalid port. %s", str );
    return 0;
  }
  return ( uint16_t ) l;
}


static bool
parse_argument( struct listener_info *listener_info, int argc, char *argv[] ) {
  int c;

  while ( ( c = getopt_long( argc, argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'p':
        listener_info->listen_port = strtoport( optarg );
        if ( listener_info->listen_port == 0 ) {
          return false;
        }
        break;
      case 's':
        xfree( ( void * ) ( uintptr_t ) listener_info->switch_daemon );
        listener_info->switch_daemon = xstrdup( optarg );
        break;
      default:
        usage();
        exit( EXIT_SUCCESS );
        return false;
        break;
    }
  }

  debug( "Start parsing switch daemon arguments." );
  // index of first switch daemon arguments in argv
  const int switch_arg_begin = optind;

  // reorder switch daemon arguments
  opterr = 0; // disable error messages of trema common options
  while ( getopt_long( argc, argv, switch_short_options, switch_long_options, NULL ) != -1 );

  // index of first packet_in::..., etc. in argv
  const int switch_arg_option_begin = optind;

  // deep copy switch daemon arguments excluding packet_in::..., etc.
  listener_info->switch_daemon_argc = switch_arg_option_begin - switch_arg_begin;
  listener_info->switch_daemon_argv = xcalloc( ( size_t ) listener_info->switch_daemon_argc + 1,
                                               sizeof( char * ) );
  for ( int i = 0; i < listener_info->switch_daemon_argc; ++i ) {
    debug( "switch daemon option: %s\n", argv[switch_arg_begin + i] );
    listener_info->switch_daemon_argv[ i ] =  xstrdup( argv[ switch_arg_begin + i ] );
  }

  //  set service lists for each event type
  for ( int i = switch_arg_option_begin; i < argc; i++ ) {
    if ( strncmp( argv[ i ], VENDOR_PREFIX, strlen( VENDOR_PREFIX ) ) == 0 ) {
      char *service_name = xstrdup( argv[ i ] + strlen( VENDOR_PREFIX ) );
      debug( "event: " VENDOR_PREFIX "%s", service_name );
      insert_in_front( &listener_info->vendor_service_name_list, service_name );
    }
    else if ( strncmp( argv[ i ], PACKET_IN_PREFIX, strlen( PACKET_IN_PREFIX ) ) == 0 ) {
      char *service_name = xstrdup( argv[ i ] + strlen( PACKET_IN_PREFIX ) );
      debug( "event: " PACKET_IN_PREFIX "%s", service_name );
      insert_in_front( &listener_info->packetin_service_name_list, service_name );
    }
    else if ( strncmp( argv[ i ], PORTSTATUS_PREFIX, strlen( PORTSTATUS_PREFIX ) ) == 0 ) {
      char *service_name = xstrdup( argv[ i ] + strlen( PORTSTATUS_PREFIX ) );
      debug( "event: " PORTSTATUS_PREFIX "%s", service_name );
      insert_in_front( &listener_info->portstatus_service_name_list, service_name );
    }
    else if ( strncmp( argv[ i ], STATE_PREFIX, strlen( STATE_PREFIX ) ) == 0 ) {
      char *service_name = xstrdup( argv[ i ] + strlen( STATE_PREFIX ) );
      debug( "event: " STATE_PREFIX "%s", service_name );
      insert_in_front( &listener_info->state_service_name_list, service_name );
    }
  }

  return true;
}


static void
catch_sigchild( void ) {
  struct sigaction sigchld_handler;

  memset( &sigchld_handler, 0, sizeof( struct sigaction ) );
  sigchld_handler.sa_handler = ( void * ) handle_sigchld;
  sigchld_handler.sa_flags = SA_RESTART;
  sigaction( SIGCHLD, &sigchld_handler, NULL );
}


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  debug( "Switch (dpid = %#" PRIx64 ") is connected.", datapath_id );
  insert_dpid_entry( &datapath_id );
}


static void
handle_switch_disconnected( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  debug( "Switch (dpid = %#" PRIx64 ") is disconnected.", datapath_id );
  delete_dpid_entry( &datapath_id );
}


static void
recv_request( const messenger_context_handle *handle,
              uint16_t tag, void *data, size_t len ) {
  UNUSED( tag );
  UNUSED( data );
  UNUSED( len );

  buffer *reply = get_switches();
  send_reply_message( handle, 0, reply->data, reply->length );
  free_buffer( reply );
}


static void
management_event_forward_entry_operation( const messenger_context_handle *handle, uint32_t command, event_forward_operation_request *req, size_t data_len ) {

  debug( "management efi command:%#x, type:%#x, n_services:%d", command, req->type, req->n_services );

  list_element **subject = NULL;
  switch ( req->type ) {
    case EVENT_FORWARD_TYPE_VENDOR:
      info( "Managing vendor event." );
      subject = &listener_info.vendor_service_name_list;
      break;

    case EVENT_FORWARD_TYPE_PACKET_IN:
      info( "Managing packet_in event." );
      subject = &listener_info.packetin_service_name_list;
      break;

    case EVENT_FORWARD_TYPE_PORT_STATUS:
      info( "Managing port_status event." );
      subject = &listener_info.portstatus_service_name_list;
      break;

    case EVENT_FORWARD_TYPE_STATE_NOTIFY:
      info( "Managing state_notify event." );
      subject = &listener_info.state_service_name_list;
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

    case EFI_GET_SWLIST:
    {
      buffer *buf = get_switches();
      management_application_reply *reply = create_management_application_reply( MANAGEMENT_REQUEST_SUCCEEDED, command, buf->data, buf->length );
      free_buffer( buf );
      send_management_application_reply( handle, reply );
      xfree( reply );
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
}


static bool
start_service_management( void ) {
  return add_message_requested_callback( get_trema_name(), recv_request );
}


static void
stop_service_management( void ) {
}


static bool
start_switch_management( void ) {
  init_openflow_application_interface( get_trema_name() );
  set_switch_ready_handler( handle_switch_ready, NULL );
  set_switch_disconnected_handler( handle_switch_disconnected, NULL );

  return true;
}


static void
stop_switch_management( void ) {
  // do something here.
}


int
main( int argc, char *argv[] ) {
  bool ret;
  const char *switch_daemon = NULL;
  char *startup_dir;

  // get startup directory using absolute_path()
  startup_dir = get_current_dir_name();
  if ( startup_dir == NULL ) {
    die( "Failed to get_current_dir_name." );
  }

  init_trema( &argc, &argv ); // changes the current working directory

  init_listener_info( &listener_info );
  ret = parse_argument( &listener_info, argc, argv );
  if ( !ret ) {
    finalize_listener_info( &listener_info );
    exit( EXIT_FAILURE );
  }
  set_management_application_request_handler( management_recv, NULL );

  init_dpid_table();
  start_service_management();
  start_switch_management();

  switch_daemon = listener_info.switch_daemon;
  listener_info.switch_daemon = absolute_path( startup_dir, switch_daemon );
  xfree( ( void * ) ( uintptr_t ) switch_daemon );
  // free returned buffer of get_current_dir_name()
  free( startup_dir );

  catch_sigchild();

  // listener start (listen socket binding and listen)
  ret = secure_channel_listen_start( &listener_info );
  if ( !ret ) {
    finalize_listener_info( &listener_info );
    exit( EXIT_FAILURE );
  }

  set_fd_handler( listener_info.listen_fd, secure_channel_accept, &listener_info, NULL, NULL );
  set_readable( listener_info.listen_fd, true );

  start_trema();

  finalize_listener_info( &listener_info );
  stop_switch_management();
  stop_service_management();
  finalize_dpid_table();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
