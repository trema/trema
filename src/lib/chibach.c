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
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "chibach.h"
#include "chibach_private.h"
#include "external_callback.h"
#include "daemon.h"
#include "doubly_linked_list.h"
#include "log.h"
#include "messenger.h"
#include "openflow_switch_interface.h"
#include "stat.h"
#include "timer.h"
#include "utility.h"
#include "wrapper.h"


static bool initialized = false;
static bool chibach_started = false;
static bool run_as_daemon = false;
static char *chibach_log = NULL;
static char *chibach_pid = NULL;
static char *chibach_name = NULL;
static uint64_t datapath_id = 0;

static struct {
  uint32_t ip;
  uint16_t port;
} controller;


static struct option long_options[] = {
  { "datapath_id", required_argument, NULL, 'i' },
  { "controller", required_argument, NULL, 'c' },
  { "port", required_argument, NULL, 'p' },
  { "daemonize", no_argument, NULL, 'd' },
  { "logging_level", required_argument, NULL, 'l' },
  { "help", no_argument, NULL, 'h' },
  { NULL, 0, NULL, 0 },
};

static char short_options[] = "i:c:p:dl:h";


void
usage() {
  printf(
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -i, --datapath_id=DATAPATH_ID   set datapath id\n"
    "  -c, --controller=IP_ADDR        set controller host\n"
    "  -p, --port=TCP_PORT             set controller TCP port\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n",
    get_chibach_name()
  );
}


static const char *
get_chibach_log() {
  if ( chibach_log == NULL ) {
    char path[ PATH_MAX ];
    sprintf( path, "%s/log", get_chibach_tmp() );
    chibach_log = xstrdup( path );
  }
  return chibach_log;
}


static const char *
get_chibach_pid() {
  if ( chibach_pid == NULL ) {
    char path[ PATH_MAX ];
    sprintf( path, "%s/pid", get_chibach_tmp() );
    chibach_pid = xstrdup( path );
  }
  return chibach_pid;
}


const char *
get_chibach_name() {
  if ( chibach_name == NULL ) {
    return "";
  }

  return chibach_name;
}


static void
maybe_finalize_openflow_switch_interface() {
  if ( openflow_switch_interface_is_initialized() ) {
    finalize_openflow_switch_interface();
  }
}


static void
die_unless_initialized() {
  if ( !initialized ) {
    die( "Chibach is not initialized. Call init_chibach() first." );
  }
}


static void
finalize_chibach() {
  die_unless_initialized();

  debug( "Terminating chibach..." );

  maybe_finalize_openflow_switch_interface();

  finalize_messenger();
  finalize_stat();
  finalize_timer();
  chibach_started = false;
  unlink_pid( get_chibach_pid(), get_chibach_name() );

  unset_chibach_home();
  unset_chibach_tmp();

  xfree( chibach_name );
  chibach_name = NULL;

  xfree( chibach_log );
  chibach_log = NULL;

  initialized = false;
}


static void
check_chibach_tmp() {
  struct stat st;
  int ret = stat( get_chibach_tmp(), &st );
  if ( ( ret != 0 ) && ( errno == ENOENT ) ) {
    die( "Chibach temporary directory does not exist: %s", get_chibach_tmp() );
  }
}


static void
reset_getopt() {
  optind = 0;
  opterr = 1;
}


static void
parse_argv( int *argc, char ***argv ) {
  assert( argc != NULL );
  assert( argv != NULL );

  int argc_tmp = *argc;
  char *new_argv[ *argc ];

  run_as_daemon = false;
  controller.ip = 0x7f000001;
  controller.port = 6653;

  for ( int i = 0; i < *argc; ++i ) {
    new_argv[ i ] = ( *argv )[ i ];
  }

  for ( ;; ) {
    opterr = 0;
    int c = getopt_long( *argc, *argv, short_options, long_options, NULL );

    if ( c == -1 ) {
      break;
    }

    switch ( c ) {
      case 'i':
        if ( optarg != NULL ) {
          string_to_datapath_id( optarg, &datapath_id );
        }
        break;
      case 'c':
        if ( optarg != NULL ) {
          struct in_addr addr;
          inet_aton( optarg, &addr );
          controller.ip = ntohl( addr.s_addr );
        }
        break;
      case 'p':
        if ( optarg != NULL && atoi( optarg ) <= UINT16_MAX ) {
          controller.port = ( uint16_t ) atoi( optarg );
        }
        break;
      case 'd':
        run_as_daemon = true;
        break;
      case 'l':
        set_logging_level( optarg );
        break;
      case 'h':
        usage();
        exit( EXIT_SUCCESS );
        break;
      default:
        continue;
    }

    if ( optarg == NULL || strchr( new_argv[ optind - 1 ], '=' ) != NULL ) {
      argc_tmp -= 1;
      new_argv[ optind - 1 ] = NULL;
    }
    else {
      argc_tmp -= 2;
      new_argv[ optind - 1 ] = NULL;
      new_argv[ optind - 2 ] = NULL;
    }
  }

  for ( int i = 0, j = 0; i < *argc; ++i ) {
    if ( new_argv[ i ] != NULL ) {
      ( *argv )[ j ] = new_argv[ i ];
      j++;
    }
  }
  if ( argc_tmp < *argc ) {
    ( *argv )[ argc_tmp ] = NULL;
  }
  *argc = argc_tmp;

  reset_getopt();
}


static void
ignore_sigpipe() {
  struct sigaction sigpipe_ignore;

  memset( &sigpipe_ignore, 0, sizeof( struct sigaction ) );
  sigpipe_ignore.sa_handler = SIG_IGN;
  sigaction( SIGPIPE, &sigpipe_ignore, NULL );
}


static void
set_exit_handler() {
  struct sigaction signal_exit;

  memset( &signal_exit, 0, sizeof( struct sigaction ) );
  signal_exit.sa_handler = ( void * ) stop_chibach;
  sigaction( SIGINT, &signal_exit, NULL );
  sigaction( SIGTERM, &signal_exit, NULL );
}


static void
set_dump_stats_as_external_callback() {
  push_external_callback( dump_stats );
}


static void
set_usr1_handler() {
  struct sigaction signal_usr1;

  memset( &signal_usr1, 0, sizeof( struct sigaction ) );
  signal_usr1.sa_handler = ( void * ) set_dump_stats_as_external_callback;
  sigaction( SIGUSR1, &signal_usr1, NULL );
}


static void
maybe_daemonize() {
  if ( run_as_daemon ) {
    daemonize( get_chibach_home() );
  }
}


void
init_chibach( int *argc, char ***argv ) {
  assert( argc != NULL );
  assert( argv != NULL );

  chibach_log = NULL;
  initialized = false;
  chibach_started = false;
  run_as_daemon = false;

  chibach_name = xstrdup( basename( *argv[ 0 ] ) );

  parse_argv( argc, argv );
  set_chibach_home();
  set_chibach_tmp();
  check_chibach_tmp();
  if ( run_as_daemon ) {
    init_log( get_chibach_name(), get_chibach_log(), LOGGING_TYPE_FILE );
  }
  else {
    init_log( get_chibach_name(), get_chibach_log(), LOGGING_TYPE_FILE | LOGGING_TYPE_STDOUT );
  }
  ignore_sigpipe();
  set_exit_handler();
  set_usr1_handler();
  init_stat();
  init_timer();
  init_messenger( get_chibach_tmp() );
  if ( datapath_id != 0 ) {
    init_openflow_switch_interface( datapath_id, controller.ip, controller.port );
  }

  initialized = true;
}


static void
start_chibach_up() {
  debug( "Starting chibach ... (CHIBACH_HOME = %s)", get_chibach_home() );

  maybe_daemonize();
  write_pid( get_chibach_pid(), get_chibach_name() );
  chibach_started = true;

  start_messenger();
}


static void
start_chibach_down() {
  finalize_chibach();
}


void
start_chibach() {
  start_chibach_up();
  start_event_handler();
  start_chibach_down();
}


void
stop_chibach() {
  stop_event_handler();
  stop_messenger();
}


uint64_t
get_datapath_id() {
  return datapath_id;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
