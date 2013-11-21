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
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"
#include "daemon.h"
#include "doubly_linked_list.h"
#include "log.h"
#include "management_interface.h"
#include "messenger.h"
#include "openflow_application_interface.h"
#include "packetin_filter_interface.h"
#include "timer.h"
#include "trema_private.h"
#include "utility.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#ifdef init_log
#undef init_log
#endif
#define init_log mock_init_log
bool mock_init_log( const char *ident, const char *log_directory, logging_type type );

#ifdef error
#undef error
#endif
#define error mock_error
void mock_error( const char *format, ... );

#ifdef set_logging_level
#undef set_logging_level
#endif
#define set_logging_level mock_set_logging_level
bool mock_set_logging_level( const char *level );

#ifdef daemonize
#undef daemonize
#endif
#define daemonize mock_daemonize
void mock_daemonize( const char *home );

#ifdef write_pid
#undef write_pid
#endif
#define write_pid mock_write_pid
void mock_write_pid( const char *directory, const char *name );

#ifdef unlink_pid
#undef unlink_pid
#endif
#define unlink_pid mock_unlink_pid
void mock_unlink_pid( const char *directory, const char *name );

#ifdef rename_pid
#undef rename_pid
#endif
#define rename_pid mock_rename_pid
void mock_rename_pid( const char *directory, const char *old, const char *new );

#ifdef read_pid
#undef read_pid
#endif
#define read_pid mock_read_pid
pid_t mock_read_pid( const char *directory, const char *name );

#ifdef kill
#undef kill
#endif
#define kill mock_kill
int mock_kill( pid_t pid, int sig );

#ifdef clock_nanosleep
#undef clock_nanosleep
#endif
#define clock_nanosleep mock_clock_nanosleep
unsigned int mock_clock_nanosleep( clockid_t clock_id, int flags, const struct timespec *request, struct timespec *remain);

#ifdef init_event_handler
#undef init_event_handler
#endif
#define init_event_handler mock_init_event_handler
void mock_init_event_handler( void );

#ifdef start_event_handler
#undef start_event_handler
#endif
#define start_event_handler mock_start_event_handler
void mock_start_event_handler( void );

#ifdef stop_event_handler
#undef stop_event_handler
#endif
#define stop_event_handler mock_stop_event_handler
void mock_stop_event_handler( void );

#ifdef init_messenger
#undef init_messenger
#endif
#define init_messenger mock_init_messenger
void mock_init_messenger( const char *working_directory );

#ifdef start_messenger
#undef start_messenger
#endif
#define start_messenger mock_start_messenger
void mock_start_messenger( void );

#ifdef flush_messenger
#undef flush_messenger
#endif
#define flush_messenger mock_flush_messenger
void mock_flush_messenger( void );

#ifdef stop_messenger
#undef stop_messenger
#endif
#define stop_messenger mock_stop_messenger
void mock_stop_messenger( void );

#ifdef finalize_messenger
#undef finalize_messenger
#endif
#define finalize_messenger mock_finalize_messenger
void mock_finalize_messenger( void );

#ifdef start_messenger_dump
#undef start_messenger_dump
#endif
#define start_messenger_dump mock_start_messenger_dump
void mock_start_messenger_dump( const char *dump_app_name, const char *dump_service_name );

#ifdef stop_messenger_dump
#undef stop_messenger_dump
#endif
#define stop_messenger_dump mock_stop_messenger_dump
void mock_stop_messenger_dump();

#ifdef messenger_dump_enabled
#undef messenger_dump_enabled
#endif
#define messenger_dump_enabled mock_messenger_dump_enabled
bool mock_messenger_dump_enabled();

#ifdef die
#undef die
#endif
#define die mock_die
void mock_die( const char *format, ... );

#ifdef exit
#undef exit
#endif
#define exit mock_exit
void mock_exit( int status );

#ifdef openflow_application_interface_is_initialized
#undef openflow_application_interface_is_initialized
#endif
#define openflow_application_interface_is_initialized mock_openflow_application_interface_is_initialized
bool mock_openflow_application_interface_is_initialized( void );

#ifdef finalize_openflow_application_interface
#undef finalize_openflow_application_interface
#endif
#define finalize_openflow_application_interface mock_finalize_openflow_application_interface
bool mock_finalize_openflow_application_interface( void );

#ifdef printf
#undef printf
#endif
#define printf mock_printf
int mock_printf( const char *format, ...);

typedef struct stat _stat;
#ifdef stat
#undef stat
#endif
#define stat mock_stat
int mock_stat( const char *path, _stat *buf );

#ifdef debug
#undef debug
#endif
#define debug mock_debug
void mock_debug( const char *format, ... );

#ifdef init_stat
#undef init_stat
#endif
#define init_stat mock_init_stat
bool mock_init_stat();

#ifdef finalize_stat
#undef finalize_stat
#endif
#define finalize_stat mock_finalize_stat
bool mock_finalize_stat();

#ifdef init_timer
#undef init_timer
#endif
#define init_timer mock_init_timer
bool mock_init_timer();

#ifdef finalize_timer
#undef finalize_timer
#endif
#define finalize_timer mock_finalize_timer
bool mock_finalize_timer();

#ifdef push_external_callback
#undef push_external_callback
#endif
#define push_external_callback mock_push_external_callback
bool mock_push_external_callback( void ( *callback )( void ) );

#ifdef dump_stats
#undef dump_stats
#endif
#define dump_stats mock_dump_stats
void mock_dump_stats();

#ifdef finalize_packetin_filter_interface
#undef finalize_packetin_filter_interface
#endif
#define finalize_packetin_filter_interface mock_finalize_packetin_filter_interface
bool mock_finalize_packetin_filter_interface();

#ifdef init_management_interface
#undef init_management_interface
#endif
#define init_management_interface mock_init_management_interface
bool mock_init_management_interface();

#ifdef finalize_management_interface
#undef finalize_management_interface
#endif
#define finalize_management_interface mock_finalize_management_interface
bool mock_finalize_management_interface();

#define static

#endif // UNIT_TESTING


#ifndef UNIT_TESTING
#define _stat struct stat
#endif // not UNIT_TESTING


static bool initialized = false;
static bool trema_started = false;
static bool run_as_daemon = false;
static char *trema_name = NULL;
static char *executable_name = NULL;
static char *trema_log = NULL;
static char *trema_pid = NULL;
static char *trema_sock = NULL;
static logging_type log_output_type = LOGGING_TYPE_FILE;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


static struct option long_options[] = {
  { "name", 1, NULL, 'n' },
  { "daemonize", 0, NULL, 'd' },
  { "logging_level", 1, NULL, 'l' },
  { "syslog", 0, NULL, 'g' },
  { "logging_facility", 1, NULL, 'f' },
  { "help", 0, NULL, 'h' },
  { NULL, 0, NULL, 0 },
};
static char short_options[] = "n:dl:gf:h";


/**
 * Default usage function shown on <tt>-h</tt> or <tt>--help</tt>.
 *
 * This can be overridden by defining your own <tt>void usage( void )</tt>
 * in your Trema application.
 */
void
usage() {
  printf(
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n",
    executable_name
  );
}


static const char *
get_trema_log() {
  if ( trema_log == NULL ) {
    char path[ PATH_MAX ];
    sprintf( path, "%s/log", get_trema_tmp() );
    trema_log = xstrdup( path );
  }
  return trema_log;
}


static const char *
get_trema_pid() {
  if ( trema_pid == NULL ) {
    char path[ PATH_MAX ];
    sprintf( path, "%s/pid", get_trema_tmp() );
    trema_pid = xstrdup( path );
  }
  return trema_pid;
}


static const char *
get_trema_sock() {
  if ( trema_sock == NULL ) {
    char path[ PATH_MAX ];
    sprintf( path, "%s/sock", get_trema_tmp() );
    trema_sock = xstrdup( path );
  }
  return trema_sock;
}


static void
maybe_finalize_openflow_application_interface() {
  if ( openflow_application_interface_is_initialized() ) {
    finalize_openflow_application_interface();
  }
}


static void
die_unless_initialized() {
  if ( !initialized ) {
    die( "Trema is not initialized. Call init_trema() first." );
  }
}


void
finalize_trema() {
  die_unless_initialized();

  debug( "Terminating %s...", get_trema_name() );

  maybe_finalize_openflow_application_interface();
  finalize_management_interface();
  finalize_packetin_filter_interface();
  finalize_messenger();
  finalize_stat();
  finalize_timer();
  trema_started = false;
  unlink_pid( get_trema_pid(), get_trema_name() );
  xfree( trema_name );
  trema_name = NULL;
  xfree( executable_name );
  executable_name = NULL;

  unset_trema_home();
  unset_trema_tmp();

  xfree( trema_log );
  trema_log = NULL;

  initialized = false;
}


static void
check_trema_tmp() {
  _stat st;
  int ret = stat( get_trema_tmp(), &st );
  if ( ( ret != 0 ) && ( errno == ENOENT ) ) {
    die( "Trema temporary directory does not exist: %s", get_trema_tmp() );
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

  set_trema_name( basename( ( *argv )[ 0 ] ) );
  executable_name = xstrdup( get_trema_name() );

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
      case 'n':
        set_trema_name( optarg );
        break;
      case 'd':
        run_as_daemon = true;
        break;
      case 'l':
        set_logging_level( optarg );
        break;
      case 'g':
        log_output_type = LOGGING_TYPE_SYSLOG;
        break;
      case 'f':
        set_syslog_facility( optarg );
        break;
      case 'h':
        usage();
        xfree( trema_name );
        xfree( executable_name );
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
set_do_stop_trema_as_external_callback() {
  push_external_callback( stop_trema );
}


static void
set_exit_handler() {
  struct sigaction signal_exit;

  memset( &signal_exit, 0, sizeof( struct sigaction ) );
  signal_exit.sa_handler = ( void * ) set_do_stop_trema_as_external_callback;
  sigaction( SIGINT, &signal_exit, NULL );
  sigaction( SIGTERM, &signal_exit, NULL );
}


static void
do_restart_log() {
  restart_log( NULL );
}


static void
set_do_restart_log_as_external_callback() {
  push_external_callback( do_restart_log );
}


static void
set_hup_handler() {
  struct sigaction signal_hup;

  memset( &signal_hup, 0, sizeof( struct sigaction ) );
  signal_hup.sa_handler = ( void * ) set_do_restart_log_as_external_callback;
  sigaction( SIGHUP, &signal_hup, NULL );
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
toggle_messenger_dump() {
  if ( messenger_dump_enabled() ) {
    stop_messenger_dump();
  }
  else {
    start_messenger_dump( get_trema_name(), DEFAULT_DUMP_SERVICE_NAME );
  }
}


static void
set_usr2_handler() {
  struct sigaction signal_usr2;

  memset( &signal_usr2, 0, sizeof( struct sigaction ) );
  signal_usr2.sa_handler = ( void * ) toggle_messenger_dump;
  sigaction( SIGUSR2, &signal_usr2, NULL );
}


static void
maybe_daemonize() {
  if ( run_as_daemon ) {
    daemonize( get_trema_home() );
  }
}


/**
 * Call this function before using any other Trema functions in your
 * applications.
 *
 * It will initialize everything needed to handle OpenFlow events and
 * parses some standard command line options. <tt>argc</tt> and
 * <tt>argv</tt> are adjusted accordingly so your own code will never
 * see those standard arguments.
 */
void
init_trema( int *argc, char ***argv ) {
  assert( argc != NULL );
  assert( argv != NULL );

  pthread_mutex_lock( &mutex );

  trema_name = NULL;
  trema_log = NULL;
  executable_name = NULL;
  initialized = false;
  trema_started = false;
  run_as_daemon = false;
  log_output_type = LOGGING_TYPE_FILE;

  parse_argv( argc, argv );
  set_trema_home();
  set_trema_tmp();
  check_trema_tmp();
  if ( !run_as_daemon ) {
    log_output_type |= LOGGING_TYPE_STDOUT;
  }
  init_log( get_trema_name(), get_trema_log(), log_output_type );
  ignore_sigpipe();
  set_exit_handler();
  set_hup_handler();
  set_usr1_handler();
  set_usr2_handler();
  init_messenger( get_trema_sock() );
  init_stat();
  init_timer();
  init_management_interface();

  initialized = true;

  pthread_mutex_unlock( &mutex );
}


static void
create_pid_file() {
  write_pid( get_trema_pid(), get_trema_name() );
}


/**
 * Runs the main loop.
 */
void
start_trema() {
  pthread_mutex_lock( &mutex );

  die_unless_initialized();
  debug( "Starting %s ... (TREMA_HOME = %s)", get_trema_name(), get_trema_home() );

  maybe_daemonize();
  create_pid_file();
  trema_started = true;
  start_messenger();
  start_event_handler();

  finalize_trema();

  pthread_mutex_unlock( &mutex );
}


void
flush() {
  flush_messenger();
}


/**
 * Exits from the main loop.
 */
void
stop_trema() {
  stop_event_handler();
  stop_messenger();
}


void
set_trema_name( const char *name ) {
  assert( name != NULL );
  if ( trema_name != NULL ) {
    if ( trema_started ) {
      rename_pid( get_trema_pid(), trema_name, name );
      rename_log( name );
    }
    xfree( trema_name );
  }
  trema_name = xstrdup( name );

  if ( initialized ) {
    restart_log( trema_name );
  }
}


/**
 * Returns the ID of your Trema application.
 */
const char *
get_trema_name() {
  assert( trema_name != NULL );
  return trema_name;
}


/**
 * Returns the basename of <tt>argv[ 0 ]</tt>. This function is useful
 * for your application's <tt>usage()</tt> function.
 */
const char *
get_executable_name() {
  assert( executable_name != NULL );
  return executable_name;
}


pid_t
get_pid_by_trema_name( const char *name ) {
  return read_pid( get_trema_pid(), name );
}


pid_t
get_trema_process_from_name( const char *name ) {
  return get_pid_by_trema_name( name );
}


bool
terminate_trema_process( pid_t pid ) {
  assert( pid > 0 );
  int res = kill( pid, SIGTERM );
  if ( res < 0 ) {
    if ( errno == ESRCH ) {
      return true;
    }
    error( "Failed to kill %d ( %s [%d] ).", pid, strerror( errno ), errno );
    return false;
  }

  int try = 0;
  const int max_try = 10;
  struct timespec tv = { 0, 500000000 };
  while ( kill( pid, 0 ) == 0 && ++try <= max_try ) {
    clock_nanosleep( CLOCK_MONOTONIC, 0, &tv, NULL );
  }
  if ( try > max_try ) {
    error( "Failed to terminate %d.", pid );
    return false;
  }
  return true;
}


void
_free_trema_name() {
  if ( trema_name != NULL ) {
    xfree( trema_name );
    trema_name = NULL;
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
