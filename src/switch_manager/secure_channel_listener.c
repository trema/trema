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


#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"
#include "secure_channel_listener.h"
#include "switch_manager.h"
#include "switch_option.h"


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
int mock_open( const char *pathname, int flags );

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


static char **
make_switch_daemon_args( struct listener_info *listener_info, struct sockaddr_in *addr, int accept_fd ) {
  const int argc = SWITCH_MANAGER_DEFAULT_ARGC
      + listener_info->switch_daemon_argc
      + ( int ) list_length_of( listener_info->vendor_service_name_list )
      + ( int ) list_length_of( listener_info->packetin_service_name_list )
      + ( int ) list_length_of( listener_info->portstatus_service_name_list )
      + ( int ) list_length_of( listener_info->state_service_name_list )
      + 1;
  char **argv = xcalloc( ( size_t ) argc, sizeof( char * ) );
  char *command_name = xasprintf( "%s%s:%u", SWITCH_MANAGER_COMMAND_PREFIX,
                                  inet_ntoa( addr->sin_addr ),
                                   ntohs( addr->sin_port ) );
  char *service_name = xasprintf( "%s%s%s:%u", SWITCH_MANAGER_NAME_OPTION,
                                  SWITCH_MANAGER_PREFIX,
                                  inet_ntoa( addr->sin_addr ),
                                  ntohs( addr->sin_port ) );
  char *socket_opt = xasprintf( "%s%d", SWITCH_MANAGER_SOCKET_OPTION,
                                accept_fd );
  char *daemonize_opt = xstrdup( SWITCH_MANAGER_DAEMONIZE_OPTION );
  char *notify_opt = xasprintf( "%s%s", SWITCH_MANAGER_STATE_PREFIX,
                                get_trema_name() );

  int i = 0;
  argv[ i++ ] = command_name;
  argv[ i++ ] = service_name;
  argv[ i++ ] = socket_opt;
  argv[ i++ ] = daemonize_opt;
  argv[ i++ ] = notify_opt;
  int j;
  for ( j = 0; j < listener_info->switch_daemon_argc; i++, j++ ) {
    argv[ i ] = xstrdup( listener_info->switch_daemon_argv[ j ] );
  }

  for ( list_element *e = listener_info->vendor_service_name_list; e != NULL; e = e->next, ++i ) {
    argv[ i ] = xasprintf( VENDOR_PREFIX "%s",  e->data );
  }
  for ( list_element *e = listener_info->packetin_service_name_list; e != NULL; e = e->next, ++i ) {
    argv[ i ] = xasprintf( PACKET_IN_PREFIX "%s",  e->data );
  }
  for ( list_element *e = listener_info->portstatus_service_name_list; e != NULL; e = e->next, ++i ) {
    argv[ i ] = xasprintf( PORTSTATUS_PREFIX "%s",  e->data );
  }
  for ( list_element *e = listener_info->state_service_name_list; e != NULL; e = e->next, ++i ) {
    argv[ i ] = xasprintf( STATE_PREFIX "%s",  e->data );
  }

  return argv;
}


static void
free_switch_daemon_args( char **argv ) {
  int i;
  for ( i = 0; argv[ i ] != NULL; i++ ) {
    xfree( argv[ i ] );
  }
  xfree( argv );
}


static const int ACCEPT_FD = 3;


void
secure_channel_accept( int fd, void *data ) {
  struct listener_info *listener_info = data;
  struct sockaddr_in addr;
  socklen_t addr_len;
  int accept_fd;
  int pid;

  addr_len = sizeof( struct sockaddr_in );
  accept_fd = accept( fd, ( struct sockaddr * ) &addr, &addr_len );
  if ( accept_fd < 0 ) {
    // TODO: close listener socket
    error( "Failed to accept from switch. :%s.", strerror( errno ) );
    return;
  }
  pid = fork();
  if ( pid < 0 ) {
    error( "Failed to fork. %s.", strerror( errno ) );
    close( accept_fd );
    return;
  }
  if ( pid == 0 ) {
    close( listener_info->listen_fd );
    if ( accept_fd < ACCEPT_FD ) {
      dup2( accept_fd, ACCEPT_FD );
      close( accept_fd );
      accept_fd = ACCEPT_FD;
    }

    char **argv = make_switch_daemon_args( listener_info, &addr, accept_fd );

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

    execvp( listener_info->switch_daemon, argv );
    error( "Failed to execvp: %s(%s) %s %s. %s.",
      argv[ 0 ], listener_info->switch_daemon,
      argv[ 1 ], argv[ 2 ], strerror( errno ) );

    free_switch_daemon_args( argv );

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
