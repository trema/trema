/*
 * Unit tests for logging functions and macros.
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


#include <stdio.h>
#include <stdlib.h>
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


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

static void
reset_LOGGING_LEVEL() {
  unsetenv( "LOGGING_LEVEL" );
}


static void
setup() {
  finalize_log();
  reset_LOGGING_LEVEL();

  setup_leak_detector();

  original_die = die;
  die = mock_die;

  trema_abort = mock_abort;
  trema_vprintf = mock_vprintf;
  trema_fprintf = mock_fprintf;
}


static void
setup_logger() {
  setup();
  init_log( "log_test.c", get_trema_tmp(), false );
}


static void
setup_daemon_logger() {
  setup();
  init_log( "log_test.c", get_trema_tmp(), true );
}


static void
teardown() {
  finalize_log();
  reset_LOGGING_LEVEL();

  teardown_leak_detector();

  die = original_die;
  trema_abort = abort;
  trema_vprintf = vprintf;
  trema_fprintf = fprintf;
}


/********************************************************************************
 * Initialization tests.
 ********************************************************************************/

void
test_init_log_reads_LOGING_LEVEL_environment_variable() {
  setenv( "LOGGING_LEVEL", "CRITICAL", 1 );
  init_log( "tetris", get_trema_tmp(), false );
  assert_int_equal( LOG_CRITICAL, get_logging_level() );
}


/********************************************************************************
 * Logging level tests.
 ********************************************************************************/

void
test_default_logging_level_is_INFO() {
  assert_int_equal( LOG_INFO, get_logging_level() );
}


void
test_set_logging_level_succeed() {
  set_logging_level( "critical" );
  assert_int_equal( LOG_CRITICAL, get_logging_level() );
}


void
test_set_logging_level_fail_with_invalid_value() {
  expect_assert_failure( set_logging_level( "INVALID_LEVEL" ) );
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
 * Misc.
 ********************************************************************************/

void
test_output_to_stdout() {
  expect_string( mock_vprintf, output, "Hello World\n" );
  expect_string( mock_fprintf, output, "Hello World\n" );

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

    unit_test_setup_teardown( test_default_logging_level_is_INFO,
                              setup_logger, teardown ),
    unit_test_setup_teardown( test_set_logging_level_succeed,
                              setup_logger, teardown ),
    unit_test_setup_teardown( test_set_logging_level_fail_with_invalid_value,
                              setup_logger, teardown ),

    unit_test_setup_teardown( test_critical_logs_if_logging_level_is_CRITICAL,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_critical_logs_if_logging_level_is_ERROR,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_critical_fail_if_NULL,
                              setup_daemon_logger, teardown ),

    unit_test_setup_teardown( test_error_donothing_if_logging_level_is_CRITICAL,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_error_logs_if_logging_level_is_ERROR,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_error_logs_if_logging_level_is_WARNING,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_error_fail_if_NULL,
                              setup_daemon_logger, teardown ),

    unit_test_setup_teardown( test_warn_donothing_if_logging_level_is_ERROR,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_warn_logs_if_logging_level_is_WARNING,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_warn_logs_if_logging_level_is_NOTICE,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_warn_fail_if_NULL,
                              setup_daemon_logger, teardown ),

    unit_test_setup_teardown( test_notice_donothing_if_logging_level_is_WARNING,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_notice_logs_if_logging_level_is_NOTICE,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_notice_logs_if_logging_level_is_INFO,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_notice_fail_if_NULL,
                              setup_daemon_logger, teardown ),

    unit_test_setup_teardown( test_info_logs_if_logging_level_is_DEBUG,
        		      setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_info_logs_if_logging_level_is_INFO,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_info_donothing_if_logging_level_is_NOTICE,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_info_fail_if_NULL,
                              setup_daemon_logger, teardown ),

    unit_test_setup_teardown( test_DEBUG_donothing_if_logging_level_is_INFO,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_DEBUG_logs_if_logging_level_is_DEBUG,
                              setup_daemon_logger, teardown ),
    unit_test_setup_teardown( test_debug_fail_if_NULL,
                              setup_daemon_logger, teardown ),

    unit_test_setup_teardown( test_output_to_stdout,
                              setup_logger, teardown ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
