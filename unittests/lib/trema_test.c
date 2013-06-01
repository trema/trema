/*
 * Unit tests for trema.[ch]
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


#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cmockery_trema.h"
#include "trema.h"
#include "trema_private.h"


/********************************************************************************
 * static functions in trema.c
 ********************************************************************************/

void finalize_trema( void );
void parse_argv( int *argc, char ***argv );


/********************************************************************************
 * Mock and stub functions.
 ********************************************************************************/

extern bool initialized;
extern bool trema_started;
extern char *trema_log;
extern char *trema_name;
extern char *executable_name;
extern bool run_as_daemon;

static int default_argc = 1;
static char trema_app[] = "/usr/bin/trema_cat";
static char *default_args[] = { trema_app, NULL };
static char **default_argv = default_args;

static bool logger_initialized;
static bool daemonized;
static bool pid_file_created;
static bool event_handler_initialized;
static bool event_handler_started;
static bool messenger_initialized;
static bool messenger_started;
static bool messenger_flushed;
static bool messenger_dump_started;
static bool stat_initialized;
static bool management_interface_initialized;


bool
mock_init_log() {
  assert_true( !logger_initialized );

  assert_true( !daemonized );
  assert_true( !pid_file_created );
  assert_true( !messenger_started );

  logger_initialized = true;
  return true;
}


void
mock_error( const char *format, ... ) {
  UNUSED( format );
}


bool
mock_set_logging_level( char *level ) {
  check_expected( level );
  return true;
}


void
mock_daemonize() {
  assert_true( logger_initialized );

  daemonized = true;
}


void
mock_write_pid( const char *directory, const char *name ) {
  UNUSED( directory );
  UNUSED( name );

  assert_true( logger_initialized );

  pid_file_created = true;
}


void
mock_unlink_pid( char *directory, char *name ) {
  UNUSED( directory );
  UNUSED( name );
}


void
mock_rename_pid( char *directory, char *old, char *new ) {
  check_expected( directory );
  check_expected( old );
  check_expected( new );
}


pid_t
mock_read_pid( char *directory, char *name ) {
  check_expected( directory );
  check_expected( name );

  return ( pid_t ) mock();
}


int
mock_kill( pid_t pid, int sig ) {
  check_expected( pid );
  check_expected( sig );

  return ( int ) mock();
}


unsigned int
mock_sleep( unsigned int seconds ) {
  check_expected( seconds );
  return ( unsigned int ) mock();
}


void
mock_init_event_handler( void ) {
  assert_true( logger_initialized );
  assert_true( !messenger_initialized );

  event_handler_initialized = true;
}


void
mock_start_event_handler() {
  event_handler_started = true;
}


void
mock_stop_event_handler() {
  event_handler_started = false;
}


void
mock_init_messenger( const char *working_directory ) {
  UNUSED( working_directory );

  assert_true( logger_initialized );
  assert_true( !messenger_initialized );

  assert_true( !daemonized );
  assert_true( !pid_file_created );
  assert_true( !messenger_started );

  messenger_initialized = true;
}


void
mock_start_messenger() {
  messenger_started = true;
}


void
mock_flush_messenger() {
  messenger_flushed = true;
}


void
mock_stop_messenger() {
  messenger_started = false;
}


void
mock_finalize_messenger() {
  messenger_initialized = false;
}


void
mock_start_messenger_dump( const char *dump_app_name, const char *dump_service_name ) {
  UNUSED( dump_app_name );
  UNUSED( dump_service_name );
  messenger_dump_started = true;
}


void
mock_stop_messenger_dump() {
  messenger_dump_started = false;
}


bool
mock_messenger_dump_enabled() {
  return messenger_dump_started;
}


void
mock_die( char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
  mock_assert( false, "mock_die", __FILE__, __LINE__ ); } // This hoaxes gcov.


void
mock_exit( int status ) {
  check_expected( status );
}


bool
mock_openflow_application_interface_is_initialized() {
  return true;
}


bool
mock_finalize_openflow_application_interface() {
  return true;
}


int
mock_printf( char *format, ... ) {
  check_expected( format );
  return 0;
}


int
mock_stat( const char *path, struct stat *buf ) {
  UNUSED( path );
  UNUSED( buf );

  // Set errno to fail with ENOENT when mock() returns non-zero.
  errno = ENOENT;
  return ( int ) mock();
}


void
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


void
mock_warn( const char *format, ... ) {
  UNUSED( format );
}


bool
mock_init_stat() {
  assert_false( stat_initialized );

  stat_initialized = true;

  return true;
}


bool
mock_finalize_stat() {
  assert_true( stat_initialized );

  stat_initialized = false;

  return true;
}


void
mock_execute_timer_events() {
  // Do nothing.
}


int
mock_clock_gettime( clockid_t clk_id, struct timespec *tp ) {
  UNUSED( clk_id );
  UNUSED( tp );

  return ( int ) mock();
}


bool
mock_set_external_callback( void ( *callback )( void ) ) {
  UNUSED( callback );
  return true;
}


void
mock_dump_stats() {
  // do nothing
}


bool
mock_init_timer() {
  // Do nothing.
  return true;
}


bool
mock_finalize_timer() {
  // Do nothing.
  return true;
}


bool
mock_finalize_packetin_filter_interface() {
  // Do nothing.
  return true;
}


bool
mock_init_management_interface() {
  assert_false( management_interface_initialized );

  management_interface_initialized = true;

  return true;
}


bool
mock_finalize_management_interface() {
  assert_true( management_interface_initialized );

  management_interface_initialized = false;

  return true;
}



/********************************************************************************
 * Setup and teardown.
 ********************************************************************************/

static void
reset_trema() {
  unsetenv( "TREMA_HOME" );
  unsetenv( "TREMA_TMP" );

  unset_trema_home();
  unset_trema_tmp();

  logger_initialized = false;
  event_handler_initialized = false;
  messenger_initialized = false;
  initialized = false;
  trema_name = NULL;
  executable_name = NULL;
  run_as_daemon = false;

  daemonized = false;
  pid_file_created = false;
  event_handler_started = false;
  messenger_started = false;
  messenger_flushed = false;

  stat_initialized = false;

  management_interface_initialized = false;

  errno = 0;
}


/********************************************************************************
 * init_trema() tests.
 ********************************************************************************/

static void
test_init_trema_initializes_submodules_in_right_order() {
  will_return( mock_stat, 0 );

  init_trema( &default_argc, &default_argv );

  assert_true( logger_initialized );
  assert_true( messenger_initialized );
  assert_true( initialized );
  assert_true( stat_initialized );
  assert_true( management_interface_initialized );
  assert_string_equal( _get_trema_home(), "/" );
  assert_string_equal( _get_trema_tmp(), "/tmp" );

  unset_trema_home();
  unset_trema_tmp();
  xfree( trema_log );
  xfree( trema_name );
  xfree( executable_name );
}


static void
test_init_trema_dies_if_trema_tmp_does_not_exist() {
  will_return( mock_stat, -1 );

  expect_string( mock_die, message, "Trema temporary directory does not exist: /tmp" );

  expect_assert_failure( init_trema( &default_argc, &default_argv ) );

  unset_trema_home();
  unset_trema_tmp();
  xfree( trema_name );
  xfree( executable_name );
}


/********************************************************************************
 * start_trema() tests.
 ********************************************************************************/

#include <unistd.h>

static void
test_start_trema_daemonizes_if_d_option_is_ON() {
  char opt_d[] = "-d";
  char *args[] = { trema_app, opt_d, NULL };
  int argc = 2;
  char **argv = args;

  will_return( mock_stat, 0 );
  init_trema( &argc, &argv );

  start_trema();

  assert_true( daemonized );
}


static void
test_start_trema_donot_daemonize_if_d_option_is_OFF() {
  int argc = 1;
  char *args[] = { trema_app, NULL };
  char **argv = args;

  will_return( mock_stat, 0 );
  init_trema( &argc, &argv );

  start_trema();

  assert_false( run_as_daemon );
}


static void
test_start_trema_creates_pid_file() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  start_trema();

  assert_true( pid_file_created );
}


static void
test_start_trema_runs_messenger() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  start_trema();

  assert_true( messenger_started );
  assert_false( messenger_dump_started );
}


static void
test_start_trema_dies_if_not_initialized() {
  expect_string( mock_die, message, "Trema is not initialized. Call init_trema() first." );

  expect_assert_failure( start_trema() );
}


/********************************************************************************
 * stop_trema() tests.
 *******************************************************************************/

static void
test_stop_trema_stops_messenger() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );
  start_trema();

  stop_trema();
  assert_false( messenger_started );
}


/********************************************************************************
 * finalize_trema() tests.
 ********************************************************************************/

static void
test_finalize_trema_finalizes_submodules_in_right_order() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  finalize_trema();

  assert_false( messenger_initialized );
  assert_false( initialized );
  assert_false( stat_initialized );
  assert_false( management_interface_initialized );
  assert_true( _get_trema_home() == NULL );
  assert_true( _get_trema_tmp() == NULL );
}


static void
test_finalize_trema_dies_if_not_initialized() {
  expect_string( mock_die, message, "Trema is not initialized. Call init_trema() first." );

  expect_assert_failure( finalize_trema() );
}


/********************************************************************************
 * parse_argv() tests.
 *******************************************************************************/

static void
test_parse_argv_rewrites_argc_argv() {
  char name[] = "trema";
  char arg[] = "HELLO";
  char opt_d[] = "-d";
  char opt_n[] = "-n";
  char unknown_opt[] = "-u";

  int argc = 6;
  char *args[] = { trema_app, arg, opt_d, opt_n, name, unknown_opt, NULL };
  char **argv = args;

  parse_argv( &argc, &argv );

  assert_int_equal( argc, 3 );
  assert_string_equal( argv[ 0 ], "/usr/bin/trema_cat" );
  assert_string_equal( argv[ 1 ], "HELLO" );
  assert_string_equal( argv[ 2 ], "-u" );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_name_option() {
  char opt_name[] = "--name";
  char name[] = "trema";
  int argc = 3;
  char *args[] = { trema_app, opt_name, name, NULL };
  char **argv = args;

  parse_argv( &argc, &argv );

  assert_string_equal( "trema", get_trema_name() );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_name_equal_option() {
  char opt_name[] = "--name=trema";
  int argc = 2;
  char *args[] = { trema_app, opt_name, NULL };
  char **argv = args;

  parse_argv( &argc, &argv );

  assert_string_equal( "trema", get_trema_name() );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_n_option() {
  char opt_n[] = "-n";
  char name[] = "trema";
  int argc = 3;
  char *args[] = { trema_app, opt_n, name, NULL };
  char **argv = args;

  parse_argv( &argc, &argv );

  assert_string_equal( "trema", get_trema_name() );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_daemonize_option() {
  char opt_daemonize[] = "--daemonize";
  int argc = 2;
  char *args[] = { trema_app, opt_daemonize, NULL };
  char **argv = args;

  parse_argv( &argc, &argv );

  assert_true( run_as_daemon );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_d_option() {
  char opt_d[] = "-d";
  int argc = 2;
  char *args[] = { trema_app, opt_d, NULL };
  char **argv = args;

  parse_argv( &argc, &argv );

  assert_true( run_as_daemon );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_logging_level_option() {
  char opt_logging_level[] = "--logging_level";
  char level[] = "notice";
  int argc = 3;
  char *args[] = { trema_app, opt_logging_level, level, NULL };
  char **argv = args;

  expect_string( mock_set_logging_level, level, "notice" );

  parse_argv( &argc, &argv );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_logging_level_equal_option() {
  char opt_logging_level[] = "--logging_level=notice";
  int argc = 2;
  char *args[] = { trema_app, opt_logging_level, NULL };
  char **argv = args;

  expect_string( mock_set_logging_level, level, "notice" );

  parse_argv( &argc, &argv );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_l_option() {
  char opt_l[] = "-l";
  char level[] = "notice";
  int argc = 3;
  char *args[] = { trema_app, opt_l, level, NULL };
  char **argv = args;

  expect_string( mock_set_logging_level, level, "notice" );

  parse_argv( &argc, &argv );

  xfree( trema_name );
  xfree( executable_name );
}


static void
test_parse_help_option() {
  char opt_help[] = "--help";
  int argc = 2;
  char *args[] = { trema_app, opt_help, NULL };
  char **argv = args;

  expect_string(
    mock_printf,
    format,
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
  );
  expect_value( mock_exit, status, EXIT_SUCCESS );

  parse_argv( &argc, &argv );
}


static void
test_parse_h_option() {
  char opt_h[] = "-h";
  int argc = 2;
  char *args[] = { trema_app, opt_h, NULL };
  char **argv = args;

  expect_string(
    mock_printf,
    format,
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
  );
  expect_value( mock_exit, status, EXIT_SUCCESS );

  parse_argv( &argc, &argv );
}


/********************************************************************************
 * set_trema_name() tests.
 *******************************************************************************/

static void
test_set_trema_name_when_first_call() {
  char NAME[] = "test_name";
  trema_name = NULL;
  trema_started = false;

  // Go
  set_trema_name( NAME );

  assert_string_equal( NAME, get_trema_name() );
  assert_false( logger_initialized );

  xfree( trema_name );
}


static void
test_set_trema_name_when_called_before_write_pid() {
  char NAME[] = "new_name";
  trema_name = xstrdup( "old_name" );
  trema_started = false;

  // Go
  set_trema_name( NAME );
  assert_string_equal( NAME, get_trema_name() );
  assert_false( logger_initialized );

  xfree( trema_name );
}


/********************************************************************************
 * get_executable_name() tests.
 *******************************************************************************/

static void
test_get_executable_name() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  assert_string_equal( "trema_cat", get_executable_name() );

  unset_trema_home();
  unset_trema_tmp();
  xfree( trema_log );
  xfree( trema_name );
  xfree( executable_name );
}


/********************************************************************************
 * get_pid_by_trema_name() tests.
 *******************************************************************************/

static void
test_get_pid_by_trema_name() {
  char NAME[] = "test_name";
  char TEMP_DIRECTORY[] = "/tmp";
  char PID_DIRECTORY[] = "/tmp/pid";
  int PID = 123;
  setenv( "TREMA_TMP", TEMP_DIRECTORY, 1 );
  expect_string( mock_read_pid, directory, PID_DIRECTORY );
  expect_string( mock_read_pid, name, NAME );
  will_return( mock_read_pid, PID );

  // Go
  pid_t pid = get_pid_by_trema_name( NAME );
  assert_true( pid == PID );
  unset_trema_tmp();
}


/********************************************************************************
 * terminate_trema_process() tests.
 *******************************************************************************/

static void
test_terminate_trema_process_when_was_found() {
  int PID = 123;
  expect_value_count( mock_kill, pid, PID, 2 );
  expect_value_count( mock_kill, sig, SIGTERM, 1 );
  expect_value_count( mock_kill, sig, 0, 1 );
  will_return_count( mock_kill, 0, 1 );
  will_return_count( mock_kill, -1, 1 );

  // Go
  assert_true( terminate_trema_process( PID ) );
}


static void
test_terminate_trema_process_when_was_not_found() {
  int PID = 123;
  errno = ESRCH;
  expect_value_count( mock_kill, pid, PID, 1 );
  expect_value_count( mock_kill, sig, SIGTERM, 1 );
  will_return_count( mock_kill, -1, 1 );

  // Go
  assert_true( terminate_trema_process( PID ) );
}


static void
test_terminate_trema_process_when_does_not_have_permissio_to_kill() {
  int PID = 123;
  errno = EPERM;
  expect_value_count( mock_kill, pid, PID, 1 );
  expect_value_count( mock_kill, sig, SIGTERM, 1 );
  will_return_count( mock_kill, -1, 1 );

  // Go
  assert_true( !terminate_trema_process( PID ) );
}


static void
test_terminate_trema_process_when_retry_count_1() {
  int PID = 123;
  expect_value_count( mock_kill, pid, PID, 3 );
  expect_value_count( mock_kill, sig, SIGTERM, 1 );
  expect_value_count( mock_kill, sig, 0, 2 );
  will_return_count( mock_kill, 0, 2 );
  will_return_count( mock_kill, -1, 1 );
  expect_value_count( mock_sleep, seconds, 1, 1 );
  will_return_count( mock_sleep, 0, 1 );

  // Go
  assert_true( terminate_trema_process( PID ) );
}


static void
test_terminate_trema_process_when_retry_count_10() {
  int PID = 123;
  expect_value_count( mock_kill, pid, PID, 12 );
  expect_value_count( mock_kill, sig, SIGTERM, 1 );
  expect_value_count( mock_kill, sig, 0, 11 );
  will_return_count( mock_kill, 0, 11 );
  will_return_count( mock_kill, -1, 1 );
  expect_value_count( mock_sleep, seconds, 1, 10 );
  will_return_count( mock_sleep, 0, 10 );

  // Go
  assert_true( terminate_trema_process( PID ) );
}


static void
test_terminate_trema_process_when_over_max_retry_count() {
  int PID = 123;
  expect_value_count( mock_kill, pid, PID, 12 );
  expect_value_count( mock_kill, sig, SIGTERM, 1 );
  expect_value_count( mock_kill, sig, 0, 11 );
  will_return_count( mock_kill, 0, 12 );
  expect_value_count( mock_sleep, seconds, 1, 10 );
  will_return_count( mock_sleep, 0, 10 );

  // Go
  assert_true( !terminate_trema_process( PID ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // init_trema() tests.
    unit_test_setup_teardown( test_init_trema_initializes_submodules_in_right_order, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_init_trema_dies_if_trema_tmp_does_not_exist, reset_trema, reset_trema ),

    // start_trema() tests.
    unit_test_setup_teardown( test_start_trema_daemonizes_if_d_option_is_ON, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_start_trema_donot_daemonize_if_d_option_is_OFF, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_start_trema_creates_pid_file, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_start_trema_runs_messenger, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_start_trema_dies_if_not_initialized, reset_trema, reset_trema ),

    // stop_trema() tests.
    unit_test_setup_teardown( test_stop_trema_stops_messenger, reset_trema, reset_trema ),

    // finalize_trema() tests.
    unit_test_setup_teardown( test_finalize_trema_finalizes_submodules_in_right_order, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_finalize_trema_dies_if_not_initialized, reset_trema, reset_trema ),

    // parse_argv() tests.
    unit_test_setup_teardown( test_parse_argv_rewrites_argc_argv, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_name_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_name_equal_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_n_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_daemonize_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_d_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_logging_level_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_logging_level_equal_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_l_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_help_option, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_parse_h_option, reset_trema, reset_trema ),

    // set_trema_name() test.
    unit_test_setup_teardown( test_set_trema_name_when_first_call, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_set_trema_name_when_called_before_write_pid, reset_trema, reset_trema ),

    // get_executable_name() test.
    unit_test_setup_teardown( test_get_executable_name, reset_trema, reset_trema ),

    // get_pid_by_trema_name() test.
    unit_test_setup_teardown( test_get_pid_by_trema_name, reset_trema, reset_trema ),

    // terminate_trema_process() test.
    unit_test_setup_teardown( test_terminate_trema_process_when_was_found, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_terminate_trema_process_when_was_not_found, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_terminate_trema_process_when_does_not_have_permissio_to_kill, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_terminate_trema_process_when_retry_count_1, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_terminate_trema_process_when_retry_count_10, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_terminate_trema_process_when_over_max_retry_count, reset_trema, reset_trema ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
