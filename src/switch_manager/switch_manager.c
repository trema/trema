/*
 * Author: Kazushi SUGYO
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


#include <limits.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <openflow.h>
#include "trema.h"
#include "secure_channel_listener.h"
#include "switch_manager.h"
#include "dpid_table.h"


#ifdef UNIT_TESTING
#define static

#define main switch_manager_main

#ifdef printf
#undef printf
#endif
#define printf( fmt, args... )  mock_printf2( fmt, ##args )
int mock_printf2(const char *format, ...);

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
int mock_access( const char *pathname, int mode);

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
void mock_set_external_callback( void ( *callback ) ( void ) );

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
	 "Usage: %s [OPTION]... [-- SWITCH_MANAGER_OPTION]...\n"
	 "\n"
	 "  -s, --switch=PATH           the command path of switch\n"
	 "  -n, --name=SERVICE_NAME     service name\n"
         "  -p, --port=PORT             server listen port (default %u)\n"
	 "  -d, --daemonize             run in the background\n"
	 "  -l, --logging_level=LEVEL   set logging level\n"
	 "  -h, --help                  display this help and exit\n"
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
}


static void
finalize_listener_info(  struct listener_info *listener_info ) {
  if ( listener_info->switch_daemon != NULL ) {
    xfree( (void *)( uintptr_t )listener_info->switch_daemon );
    listener_info->switch_daemon = NULL;
  }
  if ( listener_info->listen_fd >= 0 ) {
    set_readable( listener_info->listen_fd, false );
    delete_fd_handler( listener_info->listen_fd );

    close( listener_info->listen_fd );
    listener_info->listen_fd = -1;
  }
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
        xfree( (void *)( uintptr_t )listener_info->switch_daemon );
        listener_info->switch_daemon = xstrdup( optarg );
        break;
      default:
        usage();
        exit( EXIT_SUCCESS );
        return false;
        break;
    }
  }

  listener_info->switch_daemon_argc = argc - optind;
  listener_info->switch_daemon_argv = &argv[ optind ];

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

  init_dpid_table();
  start_service_management();
  start_switch_management();

  switch_daemon = listener_info.switch_daemon;
  listener_info.switch_daemon = absolute_path( startup_dir, switch_daemon );
  xfree( ( void * )( uintptr_t )switch_daemon );
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
