/*
 * Unit tests for logging functions and macros.
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


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "log.h"
#include "trema.h"
#include "trema_wrapper.h"
#include "utility.h"


/********************************************************************************
 * Mocks
 ********************************************************************************/

static void ( *original_die )( const char *format, ... );

static void
mock_die( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );

  mock_assert( false, "mock_die", __FILE__, __LINE__ ); } // Hoaxes gcov.


static void
mock_abort() {
  mock_assert( false, "mock_abort", __FILE__, __LINE__ ); } // Hoaxes gcov.


static int
mock_vprintf( const char *format, va_list ap ) {
  char output[ 256 ];
  vsnprintf( output, sizeof( output ), format, ap );

  check_expected( output );

  return 0;
}


static int
mock_fprintf( FILE *stream, const char *format, ... ) {
  UNUSED( stream );

  char line[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( line, format, args );
  va_end( args );

  char *output = strchr( line, ']' ) + 2;
  check_expected( output );

  return 0;
}


static bool syslog_initialized = false;

static void
mock_openlog( const char *ident, int option, int facility ) {
  check_expected( ident );
  check_expected( option );
  check_expected( facility );

  syslog_initialized = true;
}


static void
mock_closelog( void ) {
  syslog_initialized = false;
}


static void
mock_vsyslog( int priority, const char *format, va_list ap ) {
  check_expected( priority );

  char output[ 256 ];
  vsnprintf( output, sizeof( output ), format, ap );

  check_expected( output );
}


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

static void
reset_LOGGING_LEVEL() {
  unsetenv( "LOGGING_LEVEL" );
}


static void
reset_LOGGING_FACILITY() {
  unsetenv( "LOGGING_FACILITY" );
}


static void
setup() {
  finalize_log();
  reset_LOGGING_LEVEL();
  reset_LOGGING_FACILITY();

  setup_leak_detector();

  original_die = die;
  die = mock_die;

  trema_abort = mock_abort;
  trema_vprintf = mock_vprintf;
  trema_fprintf = mock_fprintf;
  trema_openlog = mock_openlog;
  trema_closelog = mock_closelog;
  trema_vsyslog = mock_vsyslog;
}


static void
setup_logger_stdout() {
  setup();
  init_log( "log_test.c", get_trema_tmp(), LOGGING_TYPE_STDOUT );
}


static void
setup_logger_file() {
  setup();
  init_log( "log_test.c", get_trema_tmp(), LOGGING_TYPE_FILE );
}


static void
setup_logger_file_stdout() {
  setup();
  init_log( "log_test.c", get_trema_tmp(), LOGGING_TYPE_FILE | LOGGING_TYPE_STDOUT );
}


static void
setup_logger_syslog() {
  setup();
  const char *ident = "log_test.c";
  expect_string( mock_openlog, ident, ident );
  expect_value( mock_openlog, option, LOG_NDELAY );
  expect_value( mock_openlog, facility, LOG_USER );
  init_log( ident, get_trema_tmp(), LOGGING_TYPE_SYSLOG );
}


static void
teardown() {
  finalize_log();
  reset_LOGGING_LEVEL();
  reset_LOGGING_FACILITY();

  teardown_leak_detector();

  die = original_die;
  trema_abort = abort;
  trema_vprintf = vprintf;
  trema_fprintf = fprintf;
  trema_openlog = openlog;
  trema_closelog = closelog;
  trema_vsyslog = vsyslog;
}


/********************************************************************************
 * Initialization tests.
 ********************************************************************************/

void
test_init_log_reads_LOGING_LEVEL_environment_variable() {
  setenv( "LOGGING_LEVEL", "CRITICAL", 1 );
  init_log( "tetris", get_trema_tmp(), LOGGING_TYPE_FILE );
  assert_int_equal( LOG_CRIT, get_logging_level() );
}


void
test_init_log_opens_syslog() {
  const char *ident = "tetris";
  expect_string( mock_openlog, ident, ident );
  expect_value( mock_openlog, option, LOG_NDELAY );
  expect_value( mock_openlog, facility, LOG_USER );
  init_log( ident, get_trema_tmp(), LOGGING_TYPE_SYSLOG );
  assert_true( syslog_initialized );
}


void
test_init_log_reads_LOGGING_FACILITY_environment_variable() {
  const char *ident = "tetris";
  setenv( "LOGGING_FACILITY", "LOCAL7", 1 );

  expect_string( mock_openlog, ident, ident );
  expect_value( mock_openlog, option, LOG_NDELAY );
  expect_value( mock_openlog, facility, LOG_LOCAL7 );
  init_log( ident, get_trema_tmp(), LOGGING_TYPE_SYSLOG );
  assert_true( syslog_initialized );
}


/********************************************************************************
 * Finalization test.
 ********************************************************************************/

void
test_finalize_log_closes_syslog() {
  finalize_log();
  assert_false( syslog_initialized );
}


/********************************************************************************
 * Logging level tests.
 ********************************************************************************/

void
test_default_logging_level_is_INFO() {
  assert_int_equal( LOG_INFO, get_logging_level() );
}


void
test_set_logging_level_succeeds() {
  set_logging_level( "critical" );
  assert_int_equal( LOG_CRIT, get_logging_level() );
}


void
test_set_logging_level_fails_with_invalid_value() {
  expect_assert_failure( set_logging_level( "INVALID_LEVEL" ) );
}


void
test_set_logging_level_is_called_before_init_log() {
  set_logging_level( "critical" );
  init_log( "tetris", get_trema_tmp(), LOGGING_TYPE_FILE );
  assert_int_equal( LOG_CRIT, get_logging_level() );
}


void
test_LOGGING_LEVEL_overrides_logging_level() {
  setenv( "LOGGING_LEVEL", "DEBUG", 1 );
  set_logging_level( "critical" );
  init_log( "tetris", get_trema_tmp(), LOGGING_TYPE_FILE );
  assert_int_equal( LOG_DEBUG, get_logging_level() );
}


void
test_valid_logging_level_returns_true_with_valid_logging_level() {
  assert_true( valid_logging_level( "information" ) );
}


void
test_valid_logging_level_returns_false_with_invalid_logging_level() {
  assert_false( valid_logging_level( "INVALID_LOGGING_LEVEL" ) );
}


/********************************************************************************
 * Syslog facility tests.
 ********************************************************************************/

void
test_default_faciliity_is_USER() {
  const char *ident = "tetris";
  expect_string( mock_openlog, ident, ident );
  expect_value( mock_openlog, option, LOG_NDELAY );
  expect_value( mock_openlog, facility, LOG_USER );
  init_log( ident, get_trema_tmp(), LOGGING_TYPE_SYSLOG );
}


void
test_set_syslog_facility_succeeds() {
  const char *ident = "tetris";
  set_syslog_facility( "LOCAL0" );

  expect_string( mock_openlog, ident, ident );
  expect_value( mock_openlog, option, LOG_NDELAY );
  expect_value( mock_openlog, facility, LOG_LOCAL0 );
  init_log( ident, get_trema_tmp(), LOGGING_TYPE_SYSLOG );
}


void
test_set_syslog_facility_fails_with_invalid_value() {
  expect_assert_failure( set_syslog_facility( "INVALID_FACILITY" ) );
}


void
test_LOGGING_FACILITY_overrides_logging_facility() {
  const char *ident = "tetris";
  setenv( "LOGGING_FACILITY", "LOCAL0", 1 );
  set_syslog_facility( "LOCAL7" );

  expect_string( mock_openlog, ident, ident );
  expect_value( mock_openlog, option, LOG_NDELAY );
  expect_value( mock_openlog, facility, LOG_LOCAL0 );
  init_log( ident, get_trema_tmp(), LOGGING_TYPE_SYSLOG );
}


/********************************************************************************
 * critical() tests.
 ********************************************************************************/

void
test_critical_logs_if_logging_level_is_CRITICAL() {
  expect_string( mock_fprintf, output, "CRITICAL message.\n" );

  set_logging_level( "critical" );
  critical( "CRITICAL message." );
}


void
test_critical_logs_if_logging_level_is_ERROR() {
  expect_string( mock_fprintf, output, "CRITICAL message.\n" );

  set_logging_level( "error" );
  critical( "CRITICAL message." );
}


void
test_critical_fail_if_NULL() {
  expect_assert_failure( critical( NULL ) );
}


/********************************************************************************
 * error() tests.
 ********************************************************************************/

void
test_error_donothing_if_logging_level_is_CRITICAL() {
  set_logging_level( "critical" );
  error( "This message must not be logged." );
}


void
test_error_logs_if_logging_level_is_ERROR() {
  expect_string( mock_fprintf, output, "ERROR message.\n" );

  set_logging_level( "error" );
  error( "ERROR message." );
}


void
test_error_logs_if_logging_level_is_WARNING() {
  expect_string( mock_fprintf, output, "ERROR message.\n" );

  set_logging_level( "warning" );
  error( "ERROR message." );
}


void
test_error_fail_if_NULL() {
  expect_assert_failure( error( NULL ) );
}


/********************************************************************************
 * warn() tests.
 ********************************************************************************/

void
test_warn_donothing_if_logging_level_is_ERROR() {
  set_logging_level( "error" );
  warn( "This message must not be logged." );
}


void
test_warn_logs_if_logging_level_is_WARNING() {
  expect_string( mock_fprintf, output, "WARN message.\n" );

  set_logging_level( "warning" );
  warn( "WARN message." );
}


void
test_warn_logs_if_logging_level_is_NOTICE() {
  expect_string( mock_fprintf, output, "WARN message.\n" );

  set_logging_level( "notice" );
  warn( "WARN message." );
}


void
test_warn_fail_if_NULL() {
  expect_assert_failure( warn( NULL ) );
}


/********************************************************************************
 * notice() tests.
 ********************************************************************************/

void
test_notice_donothing_if_logging_level_is_WARNING() {
  set_logging_level( "warning" );
  notice( "This message must not be logged." );
}


void
test_notice_logs_if_logging_level_is_NOTICE() {
  expect_string( mock_fprintf, output, "NOTICE message.\n" );

  set_logging_level( "notice" );
  notice( "NOTICE message." );
}


void
test_notice_logs_if_logging_level_is_INFO() {
  expect_string( mock_fprintf, output, "NOTICE message.\n" );

  set_logging_level( "info" );
  notice( "NOTICE message." );
}


void
test_notice_fail_if_NULL() {
  expect_assert_failure( notice( NULL ) );
}


/********************************************************************************
 * info() tests.
 ********************************************************************************/

void
test_info_donothing_if_logging_level_is_NOTICE() {
  set_logging_level( "notice" );
  info( "This message must not be logged." );
}


void
test_info_logs_if_logging_level_is_INFO() {
  expect_string( mock_fprintf, output, "INFO message.\n" );

  set_logging_level( "info" );
  info( "INFO message." );
}


void
test_info_logs_if_logging_level_is_DEBUG() {
  expect_string( mock_fprintf, output, "INFO message.\n" );

  set_logging_level( "debug" );
  info( "INFO message." );
}


void
test_info_fail_if_NULL() {
  expect_assert_failure( info( NULL ) );
}


/********************************************************************************
 * debug() tests.
 ********************************************************************************/

void
test_DEBUG_donothing_if_logging_level_is_INFO() {
  set_logging_level( "info" );
  debug( "This message must not be logged." );
}


void
test_DEBUG_logs_if_logging_level_is_DEBUG() {
  expect_string( mock_fprintf, output, "DEBUG message.\n" );

  set_logging_level( "debug" );
  debug( "DEBUG message." );
}


void
test_debug_fail_if_NULL() {
  expect_assert_failure( debug( NULL ) );
}


/********************************************************************************
 * Output type tests.
 ********************************************************************************/

void
test_output_to_stdout() {
  expect_string( mock_vprintf, output, "Hello World\n" );

  info( "Hello World" );
}


void
test_output_to_file_stdout() {
  expect_string( mock_vprintf, output, "Hello World\n" );
  expect_string( mock_fprintf, output, "Hello World\n" );

  info( "Hello World" );
}


void
test_output_to_syslog() {
  expect_value( mock_vsyslog, priority, LOG_INFO );
  expect_string( mock_vsyslog, output, "Hello World" );

  info( "Hello World" );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_init_log_reads_LOGING_LEVEL_environment_variable,
                              reset_LOGGING_LEVEL, reset_LOGGING_LEVEL ),
    unit_test_setup_teardown( test_init_log_opens_syslog,
                              setup, teardown ),
    unit_test_setup_teardown( test_init_log_reads_LOGGING_FACILITY_environment_variable,
                              setup, teardown ),

    unit_test_setup_teardown( test_finalize_log_closes_syslog,
                              setup_logger_syslog, teardown ),

    unit_test_setup_teardown( test_default_logging_level_is_INFO,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_set_logging_level_succeeds,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_set_logging_level_fails_with_invalid_value,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_set_logging_level_is_called_before_init_log,
                              setup, teardown ),
    unit_test_setup_teardown( test_LOGGING_LEVEL_overrides_logging_level,
                              setup, teardown ),
    unit_test( test_valid_logging_level_returns_true_with_valid_logging_level ),
    unit_test( test_valid_logging_level_returns_false_with_invalid_logging_level ),

    unit_test_setup_teardown( test_default_faciliity_is_USER,
                              setup, teardown ),
    unit_test_setup_teardown( test_set_syslog_facility_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_set_syslog_facility_fails_with_invalid_value,
                              setup, teardown ),
    unit_test_setup_teardown( test_LOGGING_FACILITY_overrides_logging_facility,
                              setup, teardown ),

    unit_test_setup_teardown( test_critical_logs_if_logging_level_is_CRITICAL,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_critical_logs_if_logging_level_is_ERROR,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_critical_fail_if_NULL,
                              setup_logger_file, teardown ),

    unit_test_setup_teardown( test_error_donothing_if_logging_level_is_CRITICAL,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_error_logs_if_logging_level_is_ERROR,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_error_logs_if_logging_level_is_WARNING,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_error_fail_if_NULL,
                              setup_logger_file, teardown ),

    unit_test_setup_teardown( test_warn_donothing_if_logging_level_is_ERROR,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_warn_logs_if_logging_level_is_WARNING,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_warn_logs_if_logging_level_is_NOTICE,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_warn_fail_if_NULL,
                              setup_logger_file, teardown ),

    unit_test_setup_teardown( test_notice_donothing_if_logging_level_is_WARNING,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_notice_logs_if_logging_level_is_NOTICE,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_notice_logs_if_logging_level_is_INFO,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_notice_fail_if_NULL,
                              setup_logger_file, teardown ),

    unit_test_setup_teardown( test_info_logs_if_logging_level_is_DEBUG,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_info_logs_if_logging_level_is_INFO,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_info_donothing_if_logging_level_is_NOTICE,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_info_fail_if_NULL,
                              setup_logger_file, teardown ),

    unit_test_setup_teardown( test_DEBUG_donothing_if_logging_level_is_INFO,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_DEBUG_logs_if_logging_level_is_DEBUG,
                              setup_logger_file, teardown ),
    unit_test_setup_teardown( test_debug_fail_if_NULL,
                              setup_logger_file, teardown ),

    unit_test_setup_teardown( test_output_to_stdout,
                              setup_logger_stdout, teardown ),
    unit_test_setup_teardown( test_output_to_file_stdout,
                              setup_logger_file_stdout, teardown ),
    unit_test_setup_teardown( test_output_to_syslog,
                              setup_logger_syslog, teardown ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
