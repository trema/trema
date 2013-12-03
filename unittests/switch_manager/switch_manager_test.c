/*
 * Unit tests for switch_manager_test.
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
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "unittest.h"
#include "trema.h"
#include "switch_manager.h"
#include "secure_channel_listener.h"

extern struct listener_info listener_info;

void usage();
void handle_sigchld( int signum );
void secure_channel_fd_set( fd_set *read_set, fd_set *write_set );
void secure_channel_fd_isset( fd_set *read_set, fd_set *write_set );
char *absolute_path( const char *dir, const char *file );
int switch_manager_main( int argc, char *argv[] );
void wait_child( void );


/*************************************************************************
 * Setup and teardown function.
 *************************************************************************/

static void
setup() {
  init_log( "switch_manager_test", false );
}


static void
teardown() {
}


/******************************************************************************
 * Mock
 ******************************************************************************/

void
mock_die( char *format, ... ) {
  UNUSED( format );
}


int
mock_printf2( char *format, ... ) {
  UNUSED( format );

  return ( int ) mock();
}


/*************************************************************************
 * Mock for switch_manager.c
 *************************************************************************/

int mock_wait3_status = 0;

pid_t
mock_wait3( int *status, int options, struct rusage *rusage ) {
  check_expected( status );
  check_expected( options );
  check_expected( rusage );

  *status = mock_wait3_status;

  return ( pid_t ) mock();
}


void
mock_secure_channel_accept( struct listener_info *listener_info ) {
  check_expected( listener_info );

  ( void ) mock();
}


int
mock_access( const char *pathname, int mode ) {
  UNUSED( pathname );
  UNUSED( mode );

  return ( int ) mock();
}


char *
mock_get_current_dir_name( void ) {
  return ( char * ) mock();
}


void
mock_init_trema( int *argc, char ***argv ) {
  UNUSED( argc );
  UNUSED( argv );

  ( void ) mock();
}


void
mock_set_fd_set_callback( void ( *callback )( fd_set *read_set, fd_set *write_set ) ) {
  UNUSED( callback );

  ( void ) mock();
}


void
mock_set_check_fd_isset_callback( void ( *callback )( fd_set *read_set, fd_set *write_set ) ) {
  UNUSED( callback );

  ( void ) mock();
}


bool
mock_secure_channel_listen_start( struct listener_info *listener_info ) {
  UNUSED( listener_info );

  return ( bool ) mock();
}


char *
mock_get_trema_home( void ) {
  return ( char * ) mock();
}


void
mock_start_trema( void ) {
  ( void ) mock();
}


const char *
mock_get_executable_name( void ) {
  return "switch_manager";
}


void
mock_push_external_callback( void ( *callback )( void ) ) {
  check_expected( callback );
}

/*************************************************************************
 * Mock for secure_channel_listener.c
 *************************************************************************/

int
mock_close( int fd ) {
  check_expected( fd );

  return ( int ) mock();
}


int
mock_socket( int domain, int type, int protocol ) {
  check_expected( domain );
  check_expected( type );
  check_expected( protocol );

  return ( int ) mock();
}


int
mock_bind( int sockfd, /* const */ struct sockaddr *addr, socklen_t addrlen ) {
  struct sockaddr_in *addr_in = ( struct sockaddr_in * ) addr;
  uint32_t sin_family32;
  uint32_t sin_port32;

  check_expected( sockfd );
  check_expected( addr_in );
  check_expected( addrlen );
  sin_family32 = addr_in->sin_family;
  check_expected( sin_family32 );
  sin_port32 = addr_in->sin_port;
  check_expected( sin_port32 );
  check_expected( addr_in->sin_addr.s_addr );

  return ( int ) mock();
}


int
mock_listen( int sockfd, int backlog ) {
  check_expected( sockfd );
  check_expected( backlog );

  return ( int ) mock();
}


int
mock_accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen ) {
  check_expected( sockfd );
  check_expected( addr );
  check_expected( addrlen );
  check_expected( *addrlen );

  return ( int ) mock();
}


int
mock_setsockopt( int sockfd, int level, int optname, /* const */ void *optval,
  socklen_t optlen ) {

  check_expected( sockfd );
  check_expected( level );
  check_expected( optname );
  check_expected( optval );
  check_expected( optlen );

  return ( int ) mock();
}


pid_t
mock_fork( void ) {
  return ( pid_t ) mock();
}


int
mock_dup2( int oldfd, int newfd ) {
  check_expected( oldfd );
  check_expected( newfd );

  return ( int ) mock();
}


int
mock_open( /* const */ char *pathname, int flags ) {
  check_expected( pathname );
  check_expected( flags );

  return ( int ) mock();
}


int
mock_execvp( /* const */ char *file, char * /* const */ argv[] ) {
  check_expected( file );
  check_expected( argv );
  check_expected( argv[ 0 ] );
  check_expected( argv[ 1 ] );

  return ( int ) mock();
}


void
mock_exit( int status ) {
  check_expected( status );

  ( void ) mock();
}


/*************************************************************************
 * Test functions for switch_manager.c
 *************************************************************************/

static void
test_usage() {
  setup();

  will_return( mock_printf2, 1 );

  usage();

  teardown();
}


static void
test_handle_sigchld_succeeded() {
  setup();

  expect_value( mock_push_external_callback, callback, wait_child );

  handle_sigchld( 3 );

  teardown();
}


static void
test_wait_child_succeeded() {
  setup();

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  mock_wait3_status = 0;

  will_return( mock_wait3, 1234 );

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  will_return( mock_wait3, 0 );

  wait_child();

  teardown();
}


static void
test_wait_child_wait3_failed() {
  setup();

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  mock_wait3_status = 0;

  will_return( mock_wait3, -1 );

  wait_child();

  teardown();
}


static void
test_wait_child_wait3_zero_return() {
  setup();

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  mock_wait3_status = 0;

  will_return( mock_wait3, 0 );

  wait_child();

  teardown();
}


static void
test_wait_child_wait3_exit() {
  setup();

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  mock_wait3_status = 1 << 8;

  will_return( mock_wait3, 1234 );

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  will_return( mock_wait3, 0 );

  wait_child();

  teardown();
}


static void
test_wait_child_wait3_coredump() {
  setup();

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  mock_wait3_status = 0x80 | 11;

  will_return( mock_wait3, 5678 );

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  will_return( mock_wait3, 0 );

  wait_child();

  teardown();
}


static void
test_wait_child_wait3_signaled() {
  setup();

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  mock_wait3_status = 3;

  will_return( mock_wait3, 4444 );

  expect_not_value( mock_wait3, status, NULL );
  expect_value( mock_wait3, options, WNOHANG );
  expect_value( mock_wait3, rusage, NULL );

  will_return( mock_wait3, 0 );

  wait_child();

  teardown();
}


static void
test_secure_channel_fd_set_succeeded() {
  setup();

  fd_set read_set, write_set;

  listener_info.listen_fd = 1;
  FD_ZERO( &read_set );
  FD_ZERO( &write_set );
  secure_channel_fd_set( &read_set, &write_set );

  assert_true( FD_ISSET( listener_info.listen_fd, &read_set ) );
  FD_CLR( listener_info.listen_fd, &read_set );
  assert_true( FD_ISZERO( &read_set ) );
  assert_true( FD_ISZERO( &write_set ) );

  teardown();
}


static void
test_secure_channel_fd_set_failed() {
  setup();

  fd_set read_set, write_set;

  listener_info.listen_fd = -1;
  FD_ZERO( &read_set );
  FD_ZERO( &write_set );
  secure_channel_fd_set( &read_set, &write_set );

  assert_true( FD_ISZERO( &read_set ) );
  assert_true( FD_ISZERO( &write_set ) );

  teardown();
}


static void
test_secure_channel_fd_isset_succeeded() {
  setup();

  fd_set read_set, write_set;

  expect_value( mock_secure_channel_accept, listener_info, &listener_info );
  will_return_void( mock_secure_channel_accept );

  listener_info.listen_fd = 1;
  FD_ZERO( &read_set );
  FD_ZERO( &write_set );
  FD_SET( listener_info.listen_fd, &read_set );
  secure_channel_fd_isset( &read_set, &write_set );

  teardown();
}


static void
test_secure_channel_fd_isset_failed() {
  setup();

  fd_set read_set, write_set;

  listener_info.listen_fd = -1;
  FD_ZERO( &read_set );
  FD_ZERO( &write_set );
  FD_SET( 1, &read_set );
  secure_channel_fd_isset( &read_set, &write_set );

  teardown();
}


static void
test_absolute_path_absolute() {
  setup();

  char *path;

  will_return( mock_access, 0 );

  path = absolute_path( "/tmp", "/file" );
  assert_string_equal( path, "/file" );
  xfree( path );

  teardown();
}


static void
test_absolute_path_access_failed() {
  setup();

  char *path;

  will_return( mock_access, -1 );

  path = absolute_path( "/tmp", "/file" );
  assert_true( path == NULL );

  teardown();
}


static void
test_absolute_path_relative() {
  setup();

  char *path;

  will_return( mock_access, 0 );

  path = absolute_path( "/tmp", "file" );
  assert_string_equal( path, "/tmp/file" );
  xfree( path );

  will_return( mock_access, 0 );

  path = absolute_path( "/tmp/", "./file" );
  assert_string_equal( path, "/tmp/./file" );
  xfree( path );

  will_return( mock_access, 0 );

  path = absolute_path( "/tmp", "../tmp/file" );
  assert_string_equal( path, "/tmp/../tmp/file" );
  xfree( path );

  will_return( mock_access, 0 );

  path = absolute_path( "/tmp/", "../../tmp/file" );
  assert_string_equal( path, "/tmp/../../tmp/file" );
  xfree( path );

  teardown();
}


static void
test_switch_manager_main_succeeded() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "switch_manager",
      ( char * ) ( uintptr_t ) "--switch_manager=switch_manager",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_get_current_dir_name, strdup( "/tmp" ) );
  will_return_void( mock_init_trema );
  will_return( mock_access, 0 );

  will_return_void( mock_set_fd_set_callback );
  will_return_void( mock_set_check_fd_isset_callback );

  will_return( mock_secure_channel_listen_start, true );
  will_return( mock_get_trema_home, strdup( "/tmp" ) );
  will_return_void( mock_start_trema );

  optind = 1;
  ret = switch_manager_main( argc, argv );

  assert_true( ret == EXIT_SUCCESS );

  teardown();
}


static void
test_switch_manager_main_and_port_option_succeeded() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "switch_manager",
      ( char * ) ( uintptr_t ) "--port=6653",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_get_current_dir_name, strdup( "/tmp" ) );
  will_return_void( mock_init_trema );
  will_return( mock_access, 0 );

  will_return_void( mock_set_fd_set_callback );
  will_return_void( mock_set_check_fd_isset_callback );

  will_return( mock_secure_channel_listen_start, true );
  will_return( mock_get_trema_home, strdup( "/tmp" ) );
  will_return_void( mock_start_trema );

  optind = 1;
  ret = switch_manager_main( argc, argv );

  assert_true( ret == EXIT_SUCCESS );

  teardown();
}


static void
test_switch_manager_main_and_port_option_failed() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "switch_manager",
      ( char * ) ( uintptr_t ) "--port=0",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_get_current_dir_name, strdup( "/tmp" ) );
  will_return_void( mock_init_trema );
  will_return( mock_get_trema_home, strdup( "/tmp" ) );

  optind = 1;
  ret = switch_manager_main( argc, argv );

  assert_true( ret == EXIT_FAILURE );

  teardown();
}


static void
test_switch_manager_main_help_and_exit() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "switch_manager",
      ( char * ) ( uintptr_t ) "-x",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_printf2, 1 );
  will_return( mock_get_current_dir_name, strdup( "/tmp" ) );
  will_return_void( mock_init_trema );
  will_return( mock_get_trema_home, strdup( "/tmp" ) );

  expect_value( mock_exit, status, EXIT_SUCCESS );
  will_return_void( mock_exit );

  optind = 1;
  ret = switch_manager_main( argc, argv );

  assert_true( ret == EXIT_FAILURE );

  teardown();
}

static void
test_switch_manager_main_get_current_dir_name_failed() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "switch_manager",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_get_current_dir_name, NULL );

  optind = 1;
  ret = switch_manager_main( argc, argv );

  assert_true( ret == EXIT_FAILURE );

  teardown();
}


static void
test_switch_manager_main_secure_channel_listen_start_failed() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "switch_manager",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_get_current_dir_name, strdup( "/tmp" ) );
  will_return_void( mock_init_trema );
  will_return( mock_get_trema_home, strdup( "/tmp" ) );
  will_return( mock_access, 0 );

  will_return_void( mock_set_fd_set_callback );
  will_return_void( mock_set_check_fd_isset_callback );

  will_return( mock_secure_channel_listen_start, false );

  optind = 1;
  ret = switch_manager_main( argc, argv );

  assert_true( ret == EXIT_FAILURE );

  teardown();
}


/*************************************************************************
 * Test functions for secure_channel_listener.c
 *************************************************************************/

static void
test_secure_channel_listen_start_succeeded() {
  setup();

  int listen_fd = 1;

  listener_info.listen_fd = -1;

  expect_value( mock_socket, domain, PF_INET );
  expect_value( mock_socket, type, SOCK_STREAM );
  expect_value( mock_socket, protocol, 0 );
  will_return( mock_socket, listen_fd );

  expect_value_count( mock_setsockopt, sockfd, listen_fd, 1 );
  expect_value_count( mock_setsockopt, level, SOL_SOCKET, 1 );
  expect_value_count( mock_setsockopt, optname, SO_REUSEADDR, 1 );
  expect_not_value_count( mock_setsockopt, optval, NULL, 1 );
  expect_value_count( mock_setsockopt, optlen, sizeof( int ), 1 );
  will_return_count( mock_setsockopt, 0, 1 );

  expect_value_count( mock_setsockopt, sockfd, listen_fd, 1 );
  expect_value_count( mock_setsockopt, level, IPPROTO_TCP, 1 );
  expect_value_count( mock_setsockopt, optname, TCP_NODELAY, 1 );
  expect_not_value_count( mock_setsockopt, optval, NULL, 1 );
  expect_value_count( mock_setsockopt, optlen, sizeof( int ), 1 );
  will_return_count( mock_setsockopt, 0, 1 );

  expect_value( mock_bind, sockfd, listen_fd );
  expect_not_value( mock_bind, addr_in, NULL );
  expect_value( mock_bind, addrlen, sizeof( struct sockaddr_in ) );
  expect_value( mock_bind, sin_family32, AF_INET );
  expect_value( mock_bind, sin_port32, ( uint32_t ) htons( OFP_TCP_PORT ) );
  expect_value( mock_bind, addr_in->sin_addr.s_addr, htonl( INADDR_ANY ) );
  will_return( mock_bind, 0 );

  expect_value( mock_listen, sockfd, listen_fd );
  expect_value( mock_listen, backlog, 128 );
  will_return( mock_listen, 0 );

  assert_true( secure_channel_listen_start( &listener_info ) );
  assert_true( listener_info.listen_fd == listen_fd );

  teardown();
}


static void
test_secure_channel_listen_start_socket_failed() {
  setup();

  int listen_fd = 1;

  listener_info.listen_fd = listen_fd;

  expect_value( mock_close, fd, listen_fd );
  will_return_void( mock_close );

  expect_value( mock_socket, domain, PF_INET );
  expect_value( mock_socket, type, SOCK_STREAM );
  expect_value( mock_socket, protocol, 0 );
  will_return( mock_socket, -1 );

  assert_false( secure_channel_listen_start( &listener_info ) );
  assert_true( listener_info.listen_fd == -1 );

  teardown();
}


static void
test_secure_channel_listen_start_setsockopt_bind_failed() {
  setup();

  int listen_fd = 1;

  listener_info.listen_fd = -1;

  expect_value( mock_socket, domain, PF_INET );
  expect_value( mock_socket, type, SOCK_STREAM );
  expect_value( mock_socket, protocol, 0 );
  will_return( mock_socket, listen_fd );

  expect_value_count( mock_setsockopt, sockfd, listen_fd, 1 );
  expect_value_count( mock_setsockopt, level, SOL_SOCKET, 1 );
  expect_value_count( mock_setsockopt, optname, SO_REUSEADDR, 1 );
  expect_not_value_count( mock_setsockopt, optval, NULL, 1 );
  expect_value_count( mock_setsockopt, optlen, sizeof( int ), 1 );
  will_return_count( mock_setsockopt, -1, 1 );

  expect_value_count( mock_setsockopt, sockfd, listen_fd, 1 );
  expect_value_count( mock_setsockopt, level, IPPROTO_TCP, 1 );
  expect_value_count( mock_setsockopt, optname, TCP_NODELAY, 1 );
  expect_not_value_count( mock_setsockopt, optval, NULL, 1 );
  expect_value_count( mock_setsockopt, optlen, sizeof( int ), 1 );
  will_return_count( mock_setsockopt, -1, 1 );

  expect_value( mock_bind, sockfd, listen_fd );
  expect_not_value( mock_bind, addr_in, NULL );
  expect_value( mock_bind, addrlen, sizeof( struct sockaddr_in ) );
  expect_value( mock_bind, sin_family32, AF_INET );
  expect_value( mock_bind, sin_port32, ( uint32_t ) htons( OFP_TCP_PORT ) );
  expect_value( mock_bind, addr_in->sin_addr.s_addr, htonl( INADDR_ANY ) );
  will_return( mock_bind, -1 );

  expect_value( mock_close, fd, listen_fd );
  will_return_void( mock_close );

  assert_false( secure_channel_listen_start( &listener_info ) );
  assert_true( listener_info.listen_fd == -1 );

  teardown();
}


static void
test_secure_channel_listen_start_listen_failed() {
  setup();

  int listen_fd = 1;

  listener_info.listen_fd = -1;

  expect_value( mock_socket, domain, PF_INET );
  expect_value( mock_socket, type, SOCK_STREAM );
  expect_value( mock_socket, protocol, 0 );
  will_return( mock_socket, listen_fd );

  expect_value_count( mock_setsockopt, sockfd, listen_fd, 1 );
  expect_value_count( mock_setsockopt, level, SOL_SOCKET, 1 );
  expect_value_count( mock_setsockopt, optname, SO_REUSEADDR, 1 );
  expect_not_value_count( mock_setsockopt, optval, NULL, 1 );
  expect_value_count( mock_setsockopt, optlen, sizeof( int ), 1 );
  will_return_count( mock_setsockopt, 0, 1 );

  expect_value_count( mock_setsockopt, sockfd, listen_fd, 1 );
  expect_value_count( mock_setsockopt, level, IPPROTO_TCP, 1 );
  expect_value_count( mock_setsockopt, optname, TCP_NODELAY, 1 );
  expect_not_value_count( mock_setsockopt, optval, NULL, 1 );
  expect_value_count( mock_setsockopt, optlen, sizeof( int ), 1 );
  will_return_count( mock_setsockopt, 0, 1 );

  expect_value( mock_bind, sockfd, listen_fd );
  expect_not_value( mock_bind, addr_in, NULL );
  expect_value( mock_bind, addrlen, sizeof( struct sockaddr_in ) );
  expect_value( mock_bind, sin_family32, AF_INET );
  expect_value( mock_bind, sin_port32, ( uint32_t ) htons( OFP_TCP_PORT ) );
  expect_value( mock_bind, addr_in->sin_addr.s_addr, htonl( INADDR_ANY ) );
  will_return( mock_bind, 0 );

  expect_value( mock_listen, sockfd, listen_fd );
  expect_value( mock_listen, backlog, 128 );
  will_return( mock_listen, -1 );

  expect_value( mock_close, fd, listen_fd );
  will_return_void( mock_close );

  assert_false( secure_channel_listen_start( &listener_info ) );
  assert_true( listener_info.listen_fd == -1 );

  teardown();
}


static void
test_secure_channel_accept_parent_succeeded() {
  setup();

  int listen_fd = 0;
  int accept_fd = 1;
  int pid = 1; /* parent */

  listener_info.listen_fd = listen_fd;

  expect_value( mock_accept, sockfd, listen_fd );
  expect_not_value( mock_accept, addr, NULL );
  expect_not_value( mock_accept, addrlen, NULL );
  expect_value( mock_accept, *addrlen, sizeof( struct sockaddr_in ) );
  will_return( mock_accept, accept_fd );

  will_return( mock_fork, pid );

  expect_value( mock_close, fd, accept_fd );
  will_return_void( mock_close );

  secure_channel_accept( &listener_info );
}


static void
test_secure_channel_accept_accept_failed() {
  int listen_fd = 0;

  listener_info.listen_fd = listen_fd;

  expect_value( mock_accept, sockfd, listen_fd );
  expect_not_value( mock_accept, addr, NULL );
  expect_not_value( mock_accept, addrlen, NULL );
  expect_value( mock_accept, *addrlen, sizeof( struct sockaddr_in ) );
  will_return( mock_accept, -1 );

  secure_channel_accept( &listener_info );

  teardown();
}


static void
test_secure_channel_accept_fork_failed() {
  setup();

  int listen_fd = 0;
  int accept_fd = 3;

  listener_info.listen_fd = listen_fd;

  expect_value( mock_accept, sockfd, listen_fd );
  expect_not_value( mock_accept, addr, NULL );
  expect_not_value( mock_accept, addrlen, NULL );
  expect_value( mock_accept, *addrlen, sizeof( struct sockaddr_in ) );
  will_return( mock_accept, accept_fd );

  will_return( mock_fork, -1 );

  expect_value( mock_close, fd, accept_fd );
  will_return_void( mock_close );

  secure_channel_accept( &listener_info );

  teardown();
}


static void
test_secure_channel_accept_child_succeeded() {
  setup();

  int listen_fd = 0;
  int accept_fd = 3;
  int pid = 0; /* child */

  listener_info.switch_manager = "switch_manager";
  listener_info.switch_manager_argc = 0;
  listener_info.listen_fd = listen_fd;

  expect_value( mock_accept, sockfd, listen_fd );
  expect_not_value( mock_accept, addr, NULL );
  expect_not_value( mock_accept, addrlen, NULL );
  expect_value( mock_accept, *addrlen, sizeof( struct sockaddr_in ) );
  will_return( mock_accept, accept_fd );

  will_return( mock_fork, pid );

  expect_string( mock_open, pathname, "/dev/null" );
  expect_value( mock_open, flags, O_RDONLY );
  will_return( mock_open, 1 );

  expect_value( mock_dup2, oldfd, 1 );
  expect_value( mock_dup2, newfd, 0 );
  will_return( mock_dup2, 0 );

  expect_value( mock_close, fd, 1 );
  will_return_void( mock_close );

  expect_string( mock_open, pathname, "/dev/null" );
  expect_value( mock_open, flags, O_WRONLY );
  will_return( mock_open, 2 );

  expect_value( mock_dup2, oldfd, 2 );
  expect_value( mock_dup2, newfd, 1 );
  will_return( mock_dup2, 1 );

  expect_value( mock_close, fd, 2 );
  will_return_void( mock_close );

  expect_string( mock_open, pathname, "/dev/null" );
  expect_value( mock_open, flags, O_WRONLY );
  will_return( mock_open, 3 );

  expect_value( mock_dup2, oldfd, 3 );
  expect_value( mock_dup2, newfd, 2 );
  will_return( mock_dup2, 2 );

  expect_value( mock_close, fd, 3 );
  will_return_void( mock_close );

  expect_string( mock_execvp, file, listener_info.switch_manager );
  expect_not_value( mock_execvp, argv, NULL );
  expect_not_value( mock_execvp, argv[ 0 ], NULL );
  expect_not_value( mock_execvp, argv[ 1 ], NULL );
  will_return( mock_execvp, 0 );

  expect_value( mock_exit, status, EXIT_FAILURE );
  will_return_void( mock_exit );

  secure_channel_accept( &listener_info );

  teardown();
}


static void
test_secure_channel_accept_child_and_args_succeeded() {
  setup();

  int listen_fd = 0;
  int accept_fd = 3;
  int pid = 0; /* child */
  static char const *argv[] = { "a", "b" };

  listener_info.switch_manager = "switch_manager";
  listener_info.switch_manager_argc = 2;
  listener_info.switch_manager_argv = ( char ** ) ( uintptr_t ) argv;
  listener_info.listen_fd = listen_fd;

  expect_value( mock_accept, sockfd, listen_fd );
  expect_not_value( mock_accept, addr, NULL );
  expect_not_value( mock_accept, addrlen, NULL );
  expect_value( mock_accept, *addrlen, sizeof( struct sockaddr_in ) );
  will_return( mock_accept, accept_fd );

  will_return( mock_fork, pid );

  expect_string( mock_open, pathname, "/dev/null" );
  expect_value( mock_open, flags, O_RDONLY );
  will_return( mock_open, 1 );

  expect_value( mock_dup2, oldfd, 1 );
  expect_value( mock_dup2, newfd, 0 );
  will_return( mock_dup2, 0 );

  expect_value( mock_close, fd, 1 );
  will_return_void( mock_close );

  expect_string( mock_open, pathname, "/dev/null" );
  expect_value( mock_open, flags, O_WRONLY );
  will_return( mock_open, 2 );

  expect_value( mock_dup2, oldfd, 2 );
  expect_value( mock_dup2, newfd, 1 );
  will_return( mock_dup2, 1 );

  expect_value( mock_close, fd, 2 );
  will_return_void( mock_close );

  expect_string( mock_open, pathname, "/dev/null" );
  expect_value( mock_open, flags, O_WRONLY );
  will_return( mock_open, 3 );

  expect_value( mock_dup2, oldfd, 3 );
  expect_value( mock_dup2, newfd, 2 );
  will_return( mock_dup2, 2 );

  expect_value( mock_close, fd, 3 );
  will_return_void( mock_close );

  expect_string( mock_execvp, file, listener_info.switch_manager );
  expect_not_value( mock_execvp, argv, NULL );
  expect_not_value( mock_execvp, argv[ 0 ], NULL );
  expect_not_value( mock_execvp, argv[ 1 ], NULL );
  will_return( mock_execvp, 0 );

  expect_value( mock_exit, status, EXIT_FAILURE );
  will_return_void( mock_exit );

  secure_channel_accept( &listener_info );

  teardown();
}


/*************************************************************************
 * Run tests.
 *************************************************************************/

int
main() {
  const UnitTest tests[] = {
    /* Test functions for switch_manager.c */
    unit_test( test_usage ),
    unit_test( test_handle_sigchld_succeeded ),
    unit_test( test_wait_child_succeeded ),
    unit_test( test_wait_child_wait3_failed ),
    unit_test( test_wait_child_wait3_zero_return ),
    unit_test( test_wait_child_wait3_exit ),
    unit_test( test_wait_child_wait3_coredump ),
    unit_test( test_wait_child_wait3_signaled ),
    unit_test( test_secure_channel_fd_set_succeeded ),
    unit_test( test_secure_channel_fd_set_failed ),
    unit_test( test_secure_channel_fd_isset_succeeded ),
    unit_test( test_secure_channel_fd_isset_failed ),
    unit_test( test_absolute_path_absolute ),
    unit_test( test_absolute_path_access_failed ),
    unit_test( test_absolute_path_relative ),
    unit_test( test_switch_manager_main_succeeded ),
    unit_test( test_switch_manager_main_and_port_option_succeeded ),
    unit_test( test_switch_manager_main_and_port_option_failed ),
    unit_test( test_switch_manager_main_help_and_exit ),
    unit_test( test_switch_manager_main_get_current_dir_name_failed ),
    unit_test( test_switch_manager_main_secure_channel_listen_start_failed ),
    /* Test functions for secure_channel_listener.c */
    unit_test( test_secure_channel_listen_start_succeeded ),
    unit_test( test_secure_channel_listen_start_socket_failed ),
    unit_test( test_secure_channel_listen_start_setsockopt_bind_failed ),
    unit_test( test_secure_channel_listen_start_listen_failed ),
    unit_test( test_secure_channel_accept_parent_succeeded ),
    unit_test( test_secure_channel_accept_accept_failed ),
    unit_test( test_secure_channel_accept_fork_failed ),
    unit_test( test_secure_channel_accept_child_succeeded ),
    unit_test( test_secure_channel_accept_child_and_args_succeeded ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
