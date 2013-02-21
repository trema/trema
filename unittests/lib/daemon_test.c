/*
 * Unit tests for daemon functions.
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
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bool.h"
#include "checks.h"
#include "cmockery_trema.h"
#include "daemon.h"


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


int
mock_access( char *pathname, int mode ) {
  check_expected( pathname );
  check_expected( mode );
  return ( int ) mock();
}


static size_t read_length = 0;
static char *read_buffer = NULL;

ssize_t
mock_read( int fd, void *buf, size_t count ) {
  check_expected( fd );
  check_expected( buf );
  check_expected( count );
  if ( read_length > 0 ) {
    memcpy( buf, read_buffer, read_length );
  }
  return ( int ) mock();
}


int
mock_kill( pid_t pid, int sig ) {
  check_expected( pid );
  check_expected( sig );
  return ( int ) mock();
}


static size_t link_length = 0;
static char *link_buffer = NULL;

ssize_t
mock_readlink( const char *path, char *buf, size_t bufsiz ) {
  check_expected( path );
  check_expected( buf );
  check_expected( bufsiz );
  if ( link_length > 0 ) {
    memcpy( buf, link_buffer, link_length );
  }
  return ( ssize_t ) mock();
}


char *
mock_basename( char *path ) {
  check_expected( path );
  return ( char * )( intptr_t )  mock();
}


int
mock_rename( char *oldpath, char *newpath ) {
  check_expected( oldpath );
  check_expected( newpath );
  return ( int ) mock();
}


void
mock_warn( const char *format, ... ) {
  UNUSED( format );
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


static void
test_read_pid_successed() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  char valid_pid_string[] = "123\n";
  pid_t valid_pid = 123;
  read_buffer = valid_pid_string;
  read_length = strlen( valid_pid_string );
  will_return( mock_read, read_length );

  // Test if correctly kill.
  expect_value( mock_kill, pid, valid_pid );
  expect_value( mock_kill, sig, 0 );
  will_return( mock_kill, 0 );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Test if correctly readlink.
  char proc_path[] = "/proc/123/exe";
  expect_string( mock_readlink, path, proc_path );
  expect_not_value( mock_readlink, buf, NULL );
  expect_value( mock_readlink, bufsiz, PATH_MAX - 1 );
  char valid_exe_path[] = "/home/yasuhito/trema/bin/chess";
  link_buffer = valid_exe_path;
  link_length = strlen( valid_exe_path );
  will_return( mock_readlink, link_length );

  // Test if correctly basename.
  expect_string( mock_basename, path, valid_exe_path );
  will_return( mock_basename, strdup( "chess" ) );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == valid_pid );
}


static void
test_read_pid_fail_if_access_fail() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, -1 );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_open_fail() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, -1 );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_read_fail() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  read_length = 0;
  will_return( mock_read, -1 );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_pid_is_invalid() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  char INVALID_pid_string[] = "not number\n";
  read_buffer = INVALID_pid_string;
  read_length = strlen( INVALID_pid_string );
  will_return( mock_read, read_length );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_pid_is_zero() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  char ZERO_pid_string[] = "0\n";
  read_buffer = ZERO_pid_string;
  read_length = strlen( ZERO_pid_string );
  will_return( mock_read, read_length );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_kill_fail_with_ESRCH() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  char valid_pid_string[] = "123\n";
  pid_t valid_pid = 123;
  read_buffer = valid_pid_string;
  read_length = strlen( valid_pid_string );
  will_return( mock_read, read_length );

  // Test if correctly read.
  expect_value( mock_kill, pid, valid_pid );
  expect_value( mock_kill, sig, 0 );
  errno = ESRCH;
  will_return( mock_kill, -1 );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Test if correctly unlink.
  expect_string( mock_unlink, pathname, path );
  will_return( mock_unlink, 0 );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_kill_fail_with_EPERM() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  char valid_pid_string[] = "123\n";
  pid_t valid_pid = 123;
  read_buffer = valid_pid_string;
  read_length = strlen( valid_pid_string );
  will_return( mock_read, read_length );

  // Test if correctly read.
  expect_value( mock_kill, pid, valid_pid );
  expect_value( mock_kill, sig, 0 );
  errno = EPERM;
  will_return( mock_kill, -1 );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_read_pid_fail_if_readlink_fail() {
  // Test if correctly access.
  char path[] = "/home/yasuhito/trema/tmp/chess.pid";
  expect_string( mock_access, pathname, path );
  expect_value( mock_access, mode, R_OK );
  will_return( mock_access, 0 );

  // Test if correctly opened.
  int pid_file_fd = 111;
  expect_string( mock_open, pathname, path );
  expect_value( mock_open, flags, O_RDONLY );
  expect_value( mock_open, mode, 0 );
  will_return( mock_open, pid_file_fd );

  // Test if correctly read.
  expect_value( mock_read, fd, pid_file_fd );
  expect_not_value( mock_read, buf, NULL );
  expect_value( mock_read, count, 10 - 1 );
  char valid_pid_string[] = "123\n";
  pid_t valid_pid = 123;
  read_buffer = valid_pid_string;
  read_length = strlen( valid_pid_string );
  will_return( mock_read, read_length );

  // Test if correctly kill.
  expect_value( mock_kill, pid, valid_pid );
  expect_value( mock_kill, sig, 0 );
  will_return( mock_kill, 0 );

  // Test if correctly close.
  expect_value( mock_close, fd, pid_file_fd );

  // Test if correctly readlink.
  char proc_path[] = "/proc/123/exe";
  expect_string( mock_readlink, path, proc_path );
  expect_not_value( mock_readlink, buf, NULL );
  expect_value( mock_readlink, bufsiz, PATH_MAX - 1 );
  will_return( mock_readlink, -1 );

  // Go
  pid_t pid = read_pid( "/home/yasuhito/trema/tmp", "chess" );
  assert_true( pid == -1 );
}


static void
test_rename_pid_successed() {
  // Test if correctly unlink.
  expect_string( mock_unlink, pathname, "/home/yasuhito/trema/tmp/hello.pid" );
  errno = ENOENT;
  will_return( mock_unlink, -1 );
  // Test if correctly rename.
  expect_string( mock_rename, oldpath, "/home/yasuhito/trema/tmp/bye.pid" );
  expect_string( mock_rename, newpath, "/home/yasuhito/trema/tmp/hello.pid" );
  will_return( mock_rename, 0 );

  // Go
  rename_pid( "/home/yasuhito/trema/tmp", "bye", "hello" );
}


static void
test_rename_pid_fail_if_rename_fail() {
  // Test if correctly unlink.
  expect_string( mock_unlink, pathname, "/home/yasuhito/trema/tmp/hello.pid" );
  errno = ENOENT;
  will_return( mock_unlink, -1 );
  // Test if correctly rename.
  expect_string( mock_rename, oldpath, "/home/yasuhito/trema/tmp/bye.pid" );
  expect_string( mock_rename, newpath, "/home/yasuhito/trema/tmp/hello.pid" );
  errno = ENOENT;
  will_return( mock_rename, -1 );

  expect_string( mock_die, message, "Could not rename a PID file from "
                                    "/home/yasuhito/trema/tmp/bye.pid"
                                    " to "
                                    "/home/yasuhito/trema/tmp/hello.pid." );

  // Go
  expect_assert_failure( rename_pid( "/home/yasuhito/trema/tmp", "bye", "hello" ) );
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

    // read_pid() tests.
    unit_test( test_read_pid_successed ),
    unit_test( test_read_pid_fail_if_access_fail ),
    unit_test( test_read_pid_fail_if_open_fail ),
    unit_test( test_read_pid_fail_if_read_fail ),
    unit_test( test_read_pid_fail_if_pid_is_invalid ),
    unit_test( test_read_pid_fail_if_pid_is_zero ),
    unit_test( test_read_pid_fail_if_kill_fail_with_ESRCH ),
    unit_test( test_read_pid_fail_if_kill_fail_with_EPERM ),
    unit_test( test_read_pid_fail_if_readlink_fail ),

    // rename_pid() tests.
    unit_test( test_rename_pid_successed ),
    unit_test( test_rename_pid_fail_if_rename_fail ),

  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
