/*
 * Author: Kazushi Sugyo
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


#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"
#include "secure_channel_listener.h"
#include "switch_manager.h"


const int LISTEN_SOCK_MAX = 128;
const int SWITCH_MANAGER_DEFAULT_ARGC = 10;

#ifdef UNIT_TESTING
#define static

#ifdef close
#undef close
#endif
#define close mock_close
int mock_close( int fd );

#ifdef socket
#undef socket
#endif
#define socket mock_socket
int mock_socket( int domain, int type, int protocol );

#ifdef bind
#undef bind
#endif
#define bind mock_bind
int mock_bind( int sockfd, const struct sockaddr *addr, socklen_t addrlen );

#ifdef listen
#undef listen
#endif
#define listen mock_listen
int mock_listen( int sockfd, int backlog );

#ifdef accept
#undef accept
#endif
#define accept mock_accept
int mock_accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen );

#ifdef setsockopt
#undef setsockopt
#endif
#define setsockopt mock_setsockopt
int mock_setsockopt( int sockfd, int level, int optname, const void *optval,
  socklen_t optlen );

#ifdef fork
#undef fork
#endif
#define fork mock_fork
pid_t mock_fork( void );

#ifdef dup2
#undef dup2
#endif
#define dup2 mock_dup2
int mock_dup2( int oldfd, int newfd );

#ifdef open
#undef open
#endif
#define open mock_open
int mock_open(const char *pathname, int flags);

#ifdef execvp
#undef execvp
#endif
#define execvp mock_execvp
int mock_execvp( const char *file, char *const argv[] );

#ifdef exit
#undef exit
#endif
#define exit mock_exit
void mock_exit( int status );

#endif // UNIT_TESTING


bool
secure_channel_listen_start( struct listener_info *listener_info ) {
  struct sockaddr_in addr;
  int listen_fd;
  int flag;
  int ret;

  if ( listener_info->listen_fd >= 0 ) {
    close( listener_info->listen_fd );
  }
  listener_info->listen_fd = -1;

  memset( &addr, 0, sizeof( struct sockaddr_in ) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( listener_info->listen_port );
  addr.sin_addr.s_addr = htonl( INADDR_ANY );

  listen_fd = socket( PF_INET, SOCK_STREAM, 0 );
  if ( listen_fd < 0 ) {
    error( "Failed to create socket." );
    return false;
  }
  flag = 1;

  ret = setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof( flag ) );
  if ( ret < 0 ) {
    warn( "Failed to set socket options." );
  }

  ret = setsockopt( listen_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) );
  if ( ret < 0 ) {
    warn( "Failed to set socket options." );
  }

  ret = bind( listen_fd, ( struct sockaddr * ) &addr, sizeof( struct sockaddr_in ) );
  if ( ret < 0 ) {
    error( "%s: Failed to bind socket.: %s(%d)",
      inet_ntoa( addr.sin_addr ), strerror( errno ), errno );
    close( listen_fd );
    return false;
  }

  ret = listen( listen_fd, LISTEN_SOCK_MAX );
  if ( ret < 0 ) {
    error( "Failed to listen." );
    close( listen_fd );
    return false;
  }

  listener_info->listen_fd = listen_fd;


  return true;
}


static const int ACCEPT_FD = 3;


void
secure_channel_accept( struct listener_info *listener_info ) {
  struct sockaddr_in addr;
  socklen_t addr_len;
  int accept_fd;
  int pid;
  uint command_name_len;
  char *command_name;
  uint service_name_len;
  char *service_name;
  uint socket_opt_len;
  char *socket_opt;
  char *daemonize_opt;
  int argc;
  char **argv;
  int i, j;

  addr_len = sizeof( struct sockaddr_in );
  accept_fd = accept( listener_info->listen_fd, ( struct sockaddr * ) &addr, &addr_len );
  if ( accept_fd < 0 ) {
    // TODO: close listener socket
    error( "Failed to accept from switch. :%s.", strerror( errno )  );
    return;
  }
  pid = fork();
  if ( pid < 0 ) {
    error( "Failed to fork. %s.", strerror( errno ) );
    close( accept_fd );
    return;
  }
  if ( pid == 0 ) {
    if ( accept_fd < ACCEPT_FD ) {
      dup2( accept_fd, ACCEPT_FD );
      close( accept_fd );
      accept_fd = ACCEPT_FD;
    }

    argc = SWITCH_MANAGER_DEFAULT_ARGC + listener_info->switch_manager_argc + 1;
    argv = xcalloc( ( size_t ) argc, sizeof( char * ) );

    command_name_len = SWITCH_MANAGER_COMMAND_PREFIX_STR_LEN
                       + SWITCH_MANAGER_ADDR_STR_LEN;
    command_name = xmalloc( command_name_len );
    snprintf( command_name, command_name_len, "%s%s:%u",
      SWITCH_MANAGER_COMMAND_PREFIX,
      inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );

    service_name_len = SWITCH_MANAGER_NAME_OPTION_STR_LEN
                       + SWITCH_MANAGER_PREFIX_STR_LEN
                       + SWITCH_MANAGER_ADDR_STR_LEN;
    service_name = xmalloc( service_name_len );
    snprintf( service_name, service_name_len, "%s%s%s:%u",
              SWITCH_MANAGER_NAME_OPTION,
              SWITCH_MANAGER_PREFIX,
              inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );

    socket_opt_len = SWITCH_MANAGER_SOCKET_OPTION_STR_LEN
                     + SWITCH_MANAGER_SOCKET_STR_LEN;
    socket_opt = xmalloc( socket_opt_len );
    snprintf( socket_opt, socket_opt_len, "%s%d",
              SWITCH_MANAGER_SOCKET_OPTION, accept_fd );

    daemonize_opt = xstrdup( SWITCH_MANAGER_DAEMONIZE_OPTION );

    i = 0;
    argv[ i++ ] = command_name;
    argv[ i++ ] = service_name;
    argv[ i++ ] = socket_opt;
    argv[ i++ ] = daemonize_opt;
    for ( j = 0; j < listener_info->switch_manager_argc; i++, j++ ) {
      argv[ i ] = listener_info->switch_manager_argv[ j ];
    }

    int in_fd = open( "/dev/null", O_RDONLY );
    if ( in_fd != 0 ) {
      dup2( in_fd, 0 );
      close( in_fd );
    }
    int out_fd = open( "/dev/null", O_WRONLY );
    if ( out_fd != 1 ) {
      dup2( out_fd, 1 );
      close( out_fd );
    }
    int err_fd = open( "/dev/null", O_WRONLY );
    if ( err_fd != 2 ) {
      dup2( err_fd, 2 );
      close( err_fd );
    }

    execvp( listener_info->switch_manager, argv );
    error( "Failed to execvp: %s(%s) %s %s. %s.",
      argv[ 0 ], listener_info->switch_manager,
      argv[ 1 ], argv[ 2 ], strerror( errno ) );

    xfree( service_name );
    xfree( command_name );
    xfree( socket_opt );
    xfree( daemonize_opt );
    xfree( argv );

    UNREACHABLE();
  }
  else {
    /* parent */
    close( accept_fd );
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
