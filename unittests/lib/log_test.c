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


#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "checks.h"
#include "cmockery.h"
#include "log.h"
#include "unittest.h"


extern int level;


int logging_level_from( const char *name );


/********************************************************************************
 * Mocks
 ********************************************************************************/

static int times_vsyslog_called;


void
mock_die( char *format, ... ) {
  check_expected( format );
  mock_assert( false, "mock_die", __FILE__, __LINE__ );
}


/*
 * A mock vsyslog() that checks priority and the value of strings
 * logged to the syslog.
 */
void
mock_vsyslog( int priority, const char *format, va_list ap ) {
  char output[ 256 ];
  vsnprintf( output, sizeof( output ), format, ap );

  check_expected( priority );
  check_expected( output );

  times_vsyslog_called++;
}


/*
 * A mock vprintf() that checks the value of strings logged to the
 * stdout.
 */
void
mock_vprintf( const char *format, va_list ap ) {
  char output[ 256 ];
  vsnprintf( output, sizeof( output ), format, ap );

  check_expected( output );
}


int
mock_printf( const char *format, ... ) {
  UNUSED( format );

  return 0;
}


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

void
reset_logging_level() {
  unsetenv( "LOGGING_LEVEL" );
  set_logging_level( "info" );
}


void
reset_times_syslog_called() {
  times_vsyslog_called = 0;
}


/********************************************************************************
 * Initialization tests.
 ********************************************************************************/

void
test_init_log_reads_LOGING_LEVEL_environment_variable() {
  setenv( "LOGGING_LEVEL", "CRITICAL", 1 );
  assert_true( init_log( "tetris", true ) );
  assert_int_equal( level, LOG_CRIT );
}


/********************************************************************************
 * Logging level tests.
 ********************************************************************************/

void
test_default_logging_level_is_INFO() {
  assert_int_equal( level, LOG_INFO );
}


void
test_set_logging_level_succeed() {
  set_logging_level( "critical" );
  assert_int_equal( level, LOG_CRIT );
}


void
test_set_logging_level_fail_with_invalid_value() {
  expect_string( mock_die, format, "Invalid logging level: %s" );

  expect_assert_failure( set_logging_level( "INVALID_LEVEL" ) );
}


void
test_get_logging_level_succeed() {
  assert_int_equal( get_logging_level(), LOG_INFO );
}


void
test_logging_level_from() {
  assert_int_equal( LOG_CRIT, logging_level_from( "CRITICAL" ) );
  assert_int_equal( LOG_CRIT, logging_level_from( "critical" ) );
  assert_int_equal( LOG_CRIT, logging_level_from( "CRIT" ) );
  assert_int_equal( LOG_CRIT, logging_level_from( "crit" ) );

  assert_int_equal( LOG_ERR, logging_level_from( "ERROR" ) );
  assert_int_equal( LOG_ERR, logging_level_from( "error" ) );
  assert_int_equal( LOG_ERR, logging_level_from( "ERR" ) );
  assert_int_equal( LOG_ERR, logging_level_from( "err" ) );

  assert_int_equal( LOG_WARNING, logging_level_from( "WARNING" ) );
  assert_int_equal( LOG_WARNING, logging_level_from( "warning" ) );
  assert_int_equal( LOG_WARNING, logging_level_from( "WARN" ) );
  assert_int_equal( LOG_WARNING, logging_level_from( "warn" ) );

  assert_int_equal( LOG_NOTICE, logging_level_from( "NOTICE" ) );
  assert_int_equal( LOG_NOTICE, logging_level_from( "notice" ) );

  assert_int_equal( LOG_INFO, logging_level_from( "INFORMATION" ) );
  assert_int_equal( LOG_INFO, logging_level_from( "information" ) );
  assert_int_equal( LOG_INFO, logging_level_from( "INFO" ) );
  assert_int_equal( LOG_INFO, logging_level_from( "info" ) );

  assert_int_equal( LOG_DEBUG, logging_level_from( "DEBUG" ) );
  assert_int_equal( LOG_DEBUG, logging_level_from( "debug" ) );
  assert_int_equal( LOG_DEBUG, logging_level_from( "DBG" ) );
  assert_int_equal( LOG_DEBUG, logging_level_from( "dbg" ) );
}


void
test_logging_level_from_NULL_fail() {
  expect_assert_failure( logging_level_from( NULL ) );
}


void
test_logging_level_from_fail_with_invalid_logging_level() {
  assert_int_equal( -1, logging_level_from( "INVALID_LOGGING_LEVEL_STRING" ) );
}


/********************************************************************************
 * critical() tests.
 ********************************************************************************/

void
test_critical_logs_if_logging_level_is_CRITICAL() {
  expect_value( mock_vsyslog, priority, LOG_CRIT );
  expect_string( mock_vsyslog, output, "CRITICAL message." );

  set_logging_level( "critical" );
  critical( "CRITICAL message." );
}


void
test_critical_logs_if_logging_level_is_ERROR() {
  expect_value( mock_vsyslog, priority, LOG_CRIT );
  expect_string( mock_vsyslog, output, "CRITICAL message." );

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
  error( "This message should not be logged." );

  assert_true( times_vsyslog_called == 0 );
}


void
test_error_logs_if_logging_level_is_ERROR() {
  expect_value( mock_vsyslog, priority, LOG_ERR );
  expect_string( mock_vsyslog, output, "ERROR message." );

  set_logging_level( "error" );
  error( "ERROR message." );
}


void
test_error_logs_if_logging_level_is_WARNING() {
  expect_value( mock_vsyslog, priority, LOG_ERR );
  expect_string( mock_vsyslog, output, "ERROR message." );

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
  warn( "This message should not be logged." );

  assert_true( times_vsyslog_called == 0 );
}


void
test_warn_logs_if_logging_level_is_WARNING() {
  expect_value( mock_vsyslog, priority, LOG_WARNING );
  expect_string( mock_vsyslog, output, "WARN message." );

  set_logging_level( "warning" );
  warn( "WARN message." );
}


void
test_warn_logs_if_logging_level_is_NOTICE() {
  expect_value( mock_vsyslog, priority, LOG_WARNING );
  expect_string( mock_vsyslog, output, "WARN message." );

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
  notice( "This message should not be logged." );

  assert_true( times_vsyslog_called == 0 );
}


void
test_notice_logs_if_logging_level_is_NOTICE() {
  expect_value( mock_vsyslog, priority, LOG_NOTICE );
  expect_string( mock_vsyslog, output, "NOTICE message." );

  set_logging_level( "notice" );
  notice( "NOTICE message." );
}


void
test_notice_logs_if_logging_level_is_INFO() {
  expect_value( mock_vsyslog, priority, LOG_NOTICE );
  expect_string( mock_vsyslog, output, "NOTICE message." );

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
  info( "This message should not be logged." );

  assert_true( times_vsyslog_called == 0 );
}


void
test_info_logs_if_logging_level_is_INFO() {
  expect_value( mock_vsyslog, priority, LOG_INFO );
  expect_string( mock_vsyslog, output, "INFO message." );

  set_logging_level( "info" );
  info( "INFO message." );
}


void
test_info_logs_if_logging_level_is_DEBUG() {
  expect_value( mock_vsyslog, priority, LOG_INFO );
  expect_string( mock_vsyslog, output, "INFO message." );

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
  debug( "This message should not be logged." );

  assert_true( times_vsyslog_called == 0 );
}


void
test_DEBUG_logs_if_logging_level_is_DEBUG() {
  expect_value( mock_vsyslog, priority, LOG_DEBUG );
  expect_string( mock_vsyslog, output, "DEBUG message." );

  set_logging_level( "debug" );
  debug( "DEBUG message." );
}


void
test_debug_fail_if_NULL() {
  expect_assert_failure( debug( NULL ) );
}


/********************************************************************************
 * No daemon
 ********************************************************************************/

void
test_output_to_stdout() {
  expect_string( mock_vprintf, output, "Hello World\n" );

  init_log( "tetris", false );

  info( "Hello World" );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_default_logging_level_is_INFO,
			      reset_logging_level, reset_logging_level ),
    unit_test_setup_teardown( test_set_logging_level_succeed,
			      reset_logging_level, reset_logging_level ),
    unit_test_setup_teardown( test_set_logging_level_fail_with_invalid_value,
			      reset_logging_level, reset_logging_level ),
    unit_test_setup_teardown( test_get_logging_level_succeed,
			      reset_logging_level, reset_logging_level ),

    unit_test_setup_teardown( test_init_log_reads_LOGING_LEVEL_environment_variable,
			      reset_logging_level, reset_logging_level ),

    unit_test( test_logging_level_from_NULL_fail ),
    unit_test_setup_teardown( test_logging_level_from,
			      reset_logging_level, reset_logging_level ),
    unit_test_setup_teardown( test_logging_level_from_fail_with_invalid_logging_level,
			      reset_logging_level, reset_logging_level ),

    unit_test_setup_teardown( test_critical_logs_if_logging_level_is_CRITICAL,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_critical_logs_if_logging_level_is_ERROR,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test( test_critical_fail_if_NULL ),

    unit_test_setup_teardown( test_error_donothing_if_logging_level_is_CRITICAL,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_error_logs_if_logging_level_is_ERROR,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_error_logs_if_logging_level_is_WARNING,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test( test_error_fail_if_NULL ),

    unit_test_setup_teardown( test_warn_donothing_if_logging_level_is_ERROR,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_warn_logs_if_logging_level_is_WARNING,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_warn_logs_if_logging_level_is_NOTICE,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test( test_warn_fail_if_NULL ),

    unit_test_setup_teardown( test_notice_donothing_if_logging_level_is_WARNING,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_notice_logs_if_logging_level_is_NOTICE,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_notice_logs_if_logging_level_is_INFO,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test( test_notice_fail_if_NULL ),

    unit_test_setup_teardown( test_info_logs_if_logging_level_is_DEBUG,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_info_logs_if_logging_level_is_INFO,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_info_donothing_if_logging_level_is_NOTICE,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test( test_info_fail_if_NULL ),

    unit_test_setup_teardown( test_DEBUG_donothing_if_logging_level_is_INFO,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test_setup_teardown( test_DEBUG_logs_if_logging_level_is_DEBUG,
        		      reset_times_syslog_called, reset_times_syslog_called ),
    unit_test( test_debug_fail_if_NULL ),

    unit_test( test_output_to_stdout ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
