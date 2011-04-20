/*
 * Unit tests for daemon functions.
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
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bool.h"
#include "checks.h"
#include "cmockery.h"
#include "daemon.h"
#include "unittest.h"


/********************************************************************************
 * Mocks.
 ********************************************************************************/

void
mock_die( char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
  mock_assert( false, "UNREACHABLE", __FILE__, __LINE__ ); } // This hoaxes gcov.


int
mock_chdir( char *path ) {
  UNUSED( path );
  return ( int ) mock();
}


void
mock_exit( int status ) {
  check_expected( status );
}


pid_t
mock_fork() {
  return ( pid_t ) mock();
}


pid_t
mock_setsid() {
  return ( pid_t ) mock();
}


mode_t
mock_umask( mode_t mode ) {
  check_expected( mode );
  return mode;
}


int
mock_close( int fd ) {
  check_expected( fd );
  return 0;
}


int
mock_open( char *pathname, int flags, mode_t mode ) {
  check_expected( pathname );
  check_expected( flags );
  check_expected( mode );
  return ( int ) mock();
}


int
mock_lockf( int fd, int cmd, off_t len ) {
  check_expected( fd );
  check_expected( cmd );
  check_expected( len );
  return ( int ) mock();
}


pid_t
mock_getpid() {
  return 1234;
}


ssize_t
mock_write( int fd, void *buf, size_t count ) {
  check_expected( fd );
  check_expected( buf );
  check_expected( count );
  return ( ssize_t ) count;
}


int
mock_unlink( char *pathname ) {
  check_expected( pathname );
  return ( int ) mock();
}


/********************************************************************************
 * Test functions.
 ********************************************************************************/

static void
test_daemonize_succeed() {
  will_return( mock_chdir, 0 );
  will_return( mock_fork, 1234 );
  will_return( mock_setsid, 4321 );

  expect_value( mock_exit, status, EXIT_SUCCESS );
  expect_value( mock_umask, mode, 0 );
  expect_value( mock_close, fd, STDIN_FILENO );
  expect_value( mock_close, fd, STDOUT_FILENO );
  expect_value( mock_close, fd, STDERR_FILENO );

  daemonize( "/" );
}


static void
test_daemonize_fail_if_chdir_fail() {
  will_return( mock_chdir, -1 );

  errno = EACCES;
  expect_string( mock_die, message, "Could not cd to /: Permission denied." );
  
  expect_assert_failure( daemonize( "/" ) );
  errno = 0;
}


static void
test_daemonize_fail_if_fork_fail() {
  will_return( mock_chdir, 0 );
  will_return( mock_fork, -1 );

  errno = EAGAIN;
  expect_string( mock_die, message, "fork() failed: Resource temporarily unavailable." );

  expect_assert_failure( daemonize( "/" ) );
  errno = 0;
}


static void
test_daemonize_fail_if_setsid_fail() {
  will_return( mock_chdir, 0 );
  will_return( mock_fork, 1234 );
  will_return( mock_setsid, -1 );

  expect_value( mock_exit, status, EXIT_SUCCESS );

  errno = EPERM;
  expect_string( mock_die, message, "setsid() failed: Operation not permitted." );

  expect_assert_failure( daemonize( "/" ) );
  errno = 0;
}


static void
test_write_pid_succeed() {
  will_return( mock_open, 1111 );
  will_return( mock_lockf, 0 );

  // Test if correctly opened.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  char buffer[] = "1234\n";
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, ( O_RDWR | O_CREAT ) );
  expect_value( mock_open, mode, 0600 );

  // Test if correctly locked.
  expect_value( mock_lockf, fd, 1111 );
  expect_value( mock_lockf, cmd, F_TLOCK );
  expect_value( mock_lockf, len, 0 );

  // Test if correctly written.
  expect_value( mock_write, fd, 1111 );
  expect_string( mock_write, buf, buffer );
  expect_value( mock_write, count, strlen( buffer ) );

  write_pid( "/home/yasuhito/trema/tmp", "chess" );
}


static void
test_write_pid_fail_if_open_fail() {
  will_return( mock_open, -1 );

  // Test if correctly opened.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, ( O_RDWR | O_CREAT ) );
  expect_value( mock_open, mode, 0600 );

  expect_string( mock_die, message, "Could not create a PID file: /home/yasuhito/trema/tmp/chess.pid" );

  expect_assert_failure( write_pid( "/home/yasuhito/trema/tmp", "chess" ) );
}


static void
test_write_pid_fail_if_lockf_fail() {
  will_return( mock_open, 1111 );
  will_return( mock_lockf, -1 );

  // Test if correctly locked.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, ( O_RDWR | O_CREAT ) );
  expect_value( mock_open, mode, 0600 );

  // Test if correctly locked.
  expect_value( mock_lockf, fd, 1111 );
  expect_value( mock_lockf, cmd, F_TLOCK );
  expect_value( mock_lockf, len, 0 );

  // lockf_return_value = -1;
  expect_string( mock_die, message, "Could not acquire a lock on a PID file: /home/yasuhito/trema/tmp/chess.pid" );

  expect_assert_failure( write_pid( "/home/yasuhito/trema/tmp", "chess" ) );
}


static void
test_unlink_pid_succeed() {
  will_return( mock_unlink, 0 );

  // Test if correctly unlinked.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_unlink, pathname, path );

  unlink_pid( "/home/yasuhito/trema/tmp", "chess" );
}


static void
test_unlink_pid_fail_if_unlink_fail() {
  will_return( mock_unlink, -1 );

  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_unlink, pathname, path );

  expect_string( mock_die, message, "Could not remove a PID file: /home/yasuhito/trema/tmp/chess.pid" );

  expect_assert_failure( unlink_pid( "/home/yasuhito/trema/tmp", "chess" ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    // daemonize() tests.
    unit_test( test_daemonize_succeed ),
    unit_test( test_daemonize_fail_if_chdir_fail ),
    unit_test( test_daemonize_fail_if_fork_fail ),
    unit_test( test_daemonize_fail_if_setsid_fail ),

    // write_pid() tests.
    unit_test( test_write_pid_succeed ),
    unit_test( test_write_pid_fail_if_open_fail ),
    unit_test( test_write_pid_fail_if_lockf_fail ),

    // unlink_pid() tsets.
    unit_test( test_unlink_pid_succeed ),
    unit_test( test_unlink_pid_fail_if_unlink_fail ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
