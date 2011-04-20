/*
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


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "checks.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#ifdef die
#undef die
#endif
#define die mock_die
void mock_die( const char *format, ... );

#ifdef chdir
#undef chdir
#endif // chdir
#define chdir mock_chdir
extern int mock_chdir( const char *path );

#ifdef fork
#undef fork
#endif // fork
#define fork mock_fork
extern pid_t mock_fork( void );

#ifdef exit
#undef exit
#endif // exit
#define exit mock_exit
extern void mock_exit( int status );

#ifdef setsid
#undef setsid
#endif // setsid
#define setsid mock_setsid
extern pid_t mock_setsid( void );

#ifdef umask
#undef umask
#endif // umask
#define umask mock_umask
extern mode_t mock_umask( mode_t mask );

#ifdef open
#undef open
#endif // open
#define open mock_open
extern int mock_open( const char *pathname, int flags, mode_t mode );

#ifdef close
#undef close
#endif // close
#define close mock_close
extern int mock_close( int fd );

#ifdef lockf
#undef lockf
#endif // lockf
#define lockf mock_lockf
extern int mock_lockf( int fd, int cmd, off_t len );

#ifdef getpid
#undef getpid
#endif // getpid
#define getpid mock_getpid
extern pid_t mock_getpid( void );

#ifdef write
#undef write
#endif // write
#define write mock_write
extern ssize_t mock_write( int fd, const void *buf, size_t count );

#ifdef unlink
#undef unlink
#endif // unlink
#define unlink mock_unlink
extern int mock_unlink( const char *pathname );

#endif // UNIT_TESTING


void
daemonize( const char *home ) {
  assert( home != NULL );

  char buf[ 256 ];

  if ( chdir( home ) < 0 ) {
    die( "Could not cd to %s: %s.", home, strerror_r( errno, buf, sizeof( buf ) ) );
  }

  pid_t pid = fork();
  if ( pid < 0 ) {
    die( "fork() failed: %s.", strerror_r( errno, buf, sizeof( buf ) ) );
  }

  if ( pid > 0 ) {
    exit( EXIT_SUCCESS );
  }

  pid_t sid = setsid();
  if ( sid < 0 ) {
    die( "setsid() failed: %s.", strerror_r( errno, buf, sizeof( buf ) ) );
  }

  umask( 0 );

  close( STDIN_FILENO );
  close( STDOUT_FILENO );
  close( STDERR_FILENO );
}


void
write_pid( const char *directory, const char *name ) {
  assert( directory != NULL );
  assert( name != NULL );

  char path[ PATH_MAX ];
  snprintf( path, PATH_MAX, "%s/%s.pid", directory, name );
  path[ PATH_MAX - 1 ] = '\0';

  int pidFileHandle = open( path, O_RDWR | O_CREAT, 0600 );
  if ( pidFileHandle == -1 ) {
    die( "Could not create a PID file: %s", path );
  }

  if ( lockf( pidFileHandle, F_TLOCK, 0 ) == -1 ) {
    die( "Could not acquire a lock on a PID file: %s", path );
  }

  char pid[ 10 ];
  snprintf( pid, sizeof( pid ),"%d\n", getpid() );
  pid[ sizeof( pid ) - 1 ] = '\0';
  write( pidFileHandle, pid, strlen( pid ) );
}


void
unlink_pid( const char *directory, const char *name ) {
  assert( directory != NULL );
  assert( name != NULL );

  char path[ PATH_MAX ];
  snprintf( path, PATH_MAX, "%s/%s.pid", directory, name );
  path[ PATH_MAX - 1 ] = '\0';

  int ret = unlink( path );
  if ( ret < 0 ) {
    die( "Could not remove a PID file: %s", path );
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
