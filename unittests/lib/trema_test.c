/*
 * Unit tests for trema.[ch]
 * 
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2011 NEC Corporation
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
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "trema.h"
#include "cmockery.h"
#include "unittest.h"


/********************************************************************************
 * static functions in trema.c
 ********************************************************************************/

void finalize_trema( void );
void parse_argv( int *argc, char ***argv );


/********************************************************************************
 * static data and types in trema.c
 ********************************************************************************/

extern dlist_element *timer_callbacks;


/********************************************************************************
 * Mock and stub functions.
 ********************************************************************************/

extern bool initialized;
extern char *trema_home;
extern char *trema_tmp;
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
static bool messenger_initialized;
static bool messenger_started;
static bool messenger_flushed;
static bool messenger_dump_started;
static bool stat_initialized;


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
mock_notice( const char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
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


bool
mock_set_external_callback( void ( *callback ) ( void ) ) {
  UNUSED( callback );
  return true;
}


void
mock_dump_stats() {
  // do nothing
}


int
mock_clock_gettime( clockid_t clk_id, struct timespec *tp ) {
  UNUSED( clk_id );
  UNUSED( tp );

  return ( int ) mock();
}


/********************************************************************************
 * Setup and teardown.
 ********************************************************************************/

static void
reset_trema() {
  unsetenv( "TREMA_HOME" );
  unsetenv( "TREMA_TMP" );

  logger_initialized = false;
  messenger_initialized = false;
  initialized = false;
  trema_name = NULL;
  executable_name = NULL;
  trema_home = NULL;
  trema_tmp = NULL;
  run_as_daemon = false;

  daemonized = false;
  pid_file_created = false;
  messenger_started = false;
  messenger_flushed = false;

  stat_initialized = false;

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
  assert_string_equal( trema_home, "/" );
  assert_string_equal( trema_tmp, "/tmp" );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
  delete_timer_callbacks();
}


static void
test_init_trema_dies_if_trema_tmp_does_not_exist() {
  will_return( mock_stat, -1 );

  expect_string( mock_die, message, "Trema temporary directory does not exist: /tmp" );

  expect_assert_failure( init_trema( &default_argc, &default_argv ) );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
}


/********************************************************************************
 * start_trema() tests.
 ********************************************************************************/

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
  assert_true( trema_home == NULL );
  assert_true( trema_tmp == NULL );
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
    "  -n, --name=SERVICE_NAME     service name\n"
    "  -d, --daemonize             run in the background\n"
    "  -l, --logging_level=LEVEL   set logging level\n"
    "  -h, --help                  display this help and exit\n"
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
    "  -n, --name=SERVICE_NAME     service name\n"
    "  -d, --daemonize             run in the background\n"
    "  -l, --logging_level=LEVEL   set logging level\n"
    "  -h, --help                  display this help and exit\n"
  );
  expect_value( mock_exit, status, EXIT_SUCCESS );

  parse_argv( &argc, &argv );
}


/********************************************************************************
 * get_trema_home() tests.
 *******************************************************************************/

static void
test_get_trema_home() {
  setenv( "TREMA_HOME", "/var", 1 );

  assert_string_equal( "/var", get_trema_home() );

  xfree( trema_home );
}


static void
test_get_trema_home_when_TREMA_HOME_is_NOT_set() {
  assert_string_equal( "/", get_trema_home() );

  xfree( trema_home );
}


static void
test_get_trema_home_falls_back_to_ROOT_if_TREMA_HOME_is_invalid() {
  setenv( "TREMA_HOME", "NO_SUCH_DIRECTORY", 1 );

  errno = ENOENT;
  expect_string( mock_notice, message, "Could not get the absolute path of NO_SUCH_DIRECTORY: No such file or directory." );
  expect_string( mock_notice, message, "Falling back TREMA_HOME to \"/\"." );

  assert_string_equal( "/", get_trema_home() );

  xfree( trema_home );
}


/********************************************************************************
 * get_trema_tmp() tests.
 *******************************************************************************/

static void
test_get_trema_tmp() {
  setenv( "TREMA_HOME", "/var", 1 );

  assert_string_equal( "/var/tmp", get_trema_tmp() );

  xfree( trema_home );
  xfree( trema_tmp );
}


static void
test_get_trema_tmp_when_TREMA_HOME_is_ROOT() {
  setenv( "TREMA_HOME", "/", 1 );

  assert_string_equal( "/tmp", get_trema_tmp() );

  xfree( trema_home );
  xfree( trema_tmp );
}


static void
test_get_trema_tmp_falls_back_to_default_if_TREMA_HOME_is_invalid() {
  setenv( "TREMA_HOME", "NO_SUCH_DIRECTORY", 1 );

  errno = ENOENT;
  expect_string( mock_notice, message, "Could not get the absolute path of NO_SUCH_DIRECTORY: No such file or directory." );
  expect_string( mock_notice, message, "Falling back TREMA_HOME to \"/\"." );

  assert_string_equal( "/tmp", get_trema_tmp() );

  xfree( trema_home );
  xfree( trema_tmp );
}


static void
test_get_trema_tmp_when_TREMA_HOME_is_NOT_set() {
  assert_string_equal( "/tmp", get_trema_tmp() );

  xfree( trema_home );
  xfree( trema_tmp );
}


static void
test_get_trema_tmp_when_TREMA_TMP_is_set() {
  setenv( "TREMA_TMP", "/", 1 );

  assert_string_equal( "/", get_trema_tmp() );

  xfree( trema_tmp );
}


static void
test_get_trema_tmp_when_TREMA_HOME_and_TREMA_TMP_are_set() {
  setenv( "TREMA_HOME", "/var", 1 );
  setenv( "TREMA_TMP", "/", 1 );

  assert_string_equal( "/var", get_trema_home() );
  assert_string_equal( "/", get_trema_tmp() );

  xfree( trema_home );
  xfree( trema_tmp );
}


static void
test_get_trema_tmp_falls_back_to_default_if_TREMA_TMP_is_invalid() {
  setenv( "TREMA_TMP", "NO_SUCH_DIRECTORY", 1 );

  errno = ENOENT;
  expect_string( mock_notice, message, "Could not get the absolute path of NO_SUCH_DIRECTORY: No such file or directory." );
  expect_string( mock_notice, message, "Falling back TREMA_TMP to \"/tmp\"." );

  assert_string_equal( "/tmp", get_trema_tmp() );

  xfree( trema_tmp );
}


/********************************************************************************
 * get_executable_name() tests.
 *******************************************************************************/

static void
test_get_executable_name() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  assert_string_equal( "trema_cat", get_executable_name() );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
  delete_timer_callbacks();
}


/********************************************************************************
 * Timer event callback tests.
 *******************************************************************************/

static timer_callback *
find_timer_callback( void ( *callback )( void *user_data ) ) {
  dlist_element *e;
  timer_callback *cb;

  cb = NULL;
  for ( e = timer_callbacks->next; e; e = e->next ) {
    cb = e->data;
    if ( cb->function == callback ) {
      return cb;
    }
  }
  return NULL;
}


static void
mock_timer_event_callback( void *user_data ) {
  UNUSED( user_data );
}


static void
test_timer_event_callback() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  will_return_count( mock_clock_gettime, 0, -1 );

  struct itimerspec interval;
  interval.it_value.tv_sec = 1;
  interval.it_value.tv_nsec = 1000;
  interval.it_interval.tv_sec = 2;
  interval.it_interval.tv_nsec = 2000;
  assert_true( add_timer_event_callback( &interval, mock_timer_event_callback, "It's time!!!" ) );

  timer_callback *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->function == mock_timer_event_callback );
  assert_string_equal( callback->user_data, "It's time!!!" );
  assert_int_equal( callback->interval.tv_sec, 2 );
  assert_int_equal( callback->interval.tv_nsec, 2000 );

  delete_timer_event_callback( mock_timer_event_callback );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
  delete_timer_callbacks();
}


static void
test_periodic_event_callback() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  will_return_count( mock_clock_gettime, 0, -1 );
  assert_true( add_periodic_event_callback( 1, mock_timer_event_callback, "It's time!!!" ) );

  timer_callback *callback = find_timer_callback( mock_timer_event_callback );
  assert_true( callback != NULL );
  assert_true( callback->function == mock_timer_event_callback );
  assert_true( callback->user_data == "It's time!!!" );
  assert_int_equal( callback->interval.tv_sec, 1 );
  assert_int_equal( callback->interval.tv_nsec, 0 );

  delete_periodic_event_callback( mock_timer_event_callback );
  assert_true( find_timer_callback( mock_timer_event_callback ) == NULL );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
  delete_timer_callbacks();
}


static void
test_add_timer_event_callback_fail_with_invalid_timespec() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  will_return_count( mock_clock_gettime, 0, -1 );

  struct itimerspec interval;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 0;
  assert_false( add_timer_event_callback( &interval, mock_timer_event_callback, "USER_DATA" ) );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
  delete_timer_callbacks();
}


static void
test_clock_gettime_fail_einval() {
  will_return( mock_stat, 0 );
  init_trema( &default_argc, &default_argv );

  will_return_count( mock_clock_gettime, -1, -1 );
  assert_false( add_periodic_event_callback( 1, mock_timer_event_callback, "USER_DATA" ) );

  xfree( trema_home );
  xfree( trema_tmp );
  xfree( trema_name );
  xfree( executable_name );
  delete_timer_callbacks();
}


static void
test_nonexistent_timer_event_callback() {
  assert_false( delete_timer_event_callback( mock_timer_event_callback ) );
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

    // get_trema_home() tests.
    unit_test_setup_teardown( test_get_trema_home, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_home_when_TREMA_HOME_is_NOT_set, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_home_falls_back_to_ROOT_if_TREMA_HOME_is_invalid, reset_trema, reset_trema ),

    // get_trema_tmp() tests.
    unit_test_setup_teardown( test_get_trema_tmp, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_tmp_when_TREMA_HOME_is_ROOT, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_tmp_falls_back_to_default_if_TREMA_HOME_is_invalid, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_tmp_when_TREMA_HOME_is_NOT_set, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_tmp_when_TREMA_TMP_is_set, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_tmp_when_TREMA_HOME_and_TREMA_TMP_are_set, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_get_trema_tmp_falls_back_to_default_if_TREMA_TMP_is_invalid, reset_trema, reset_trema ),

    // get_executable_name() test.
    unit_test_setup_teardown( test_get_executable_name, reset_trema, reset_trema ),

    // Timer event callback tests.
    unit_test_setup_teardown( test_timer_event_callback, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_periodic_event_callback, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_add_timer_event_callback_fail_with_invalid_timespec, reset_trema, reset_trema  ),
    unit_test_setup_teardown( test_nonexistent_timer_event_callback, reset_trema, reset_trema ),
    unit_test_setup_teardown( test_clock_gettime_fail_einval, reset_trema, reset_trema ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
