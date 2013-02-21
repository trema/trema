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


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "checks.h"
#include "log.h"
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

#ifdef access
#undef access
#endif // access
#define access mock_access
extern int mock_access( const char *pathname, int mode );

#ifdef read
#undef read
#endif // read
#define read mock_read
extern ssize_t mock_read( int fd, void *buf, size_t count );

#ifdef kill
#undef kill
#endif // kill
#define kill mock_kill
extern int mock_kill( pid_t pid, int sig );

#ifdef readlink
#undef readlink
#endif // readlink
#define readlink mock_readlink
extern ssize_t mock_readlink( const char *path, char *buf, size_t bufsiz );

#ifdef basename
#undef basename
#endif // basename
#define basename mock_basename
extern char *mock_basename( char *path );

#ifdef rename
#undef rename
#endif // rename
#define rename mock_rename
extern int mock_rename( const char *oldpath, const char *newpath );

#ifdef warn
#undef warn
#endif // warn
#define warn mock_warn
extern void mock_warn( const char *format, ... );

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


static const int PID_STRING_LENGTH = 10;

void
write_pid( const char *directory, const char *name ) {
  assert( directory != NULL );
  assert( name != NULL );

  char path[ PATH_MAX ];
  snprintf( path, PATH_MAX, "%s/%s.pid", directory, name );
  path[ PATH_MAX - 1 ] = '\0';

  int fd = open( path, O_RDWR | O_CREAT, 0600 );
  if ( fd == -1 ) {
    die( "Could not create a PID file: %s", path );
  }

  if ( lockf( fd, F_TLOCK, 0 ) == -1 ) {
    die( "Could not acquire a lock on a PID file: %s", path );
  }

  char str[ PID_STRING_LENGTH ];
  snprintf( str, sizeof( str ), "%d\n", getpid() );
  str[ sizeof( str ) - 1 ] = '\0';
  ssize_t ret = write( fd, str, strlen( str ) );
  if ( ret == -1 ) {
    die( "Could not write a PID file: %s", path );
  }
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


pid_t
read_pid( const char *directory, const char *name ) {
  assert( directory != NULL );
  assert( name != NULL );

  char path[ PATH_MAX ];
  snprintf( path, PATH_MAX, "%s/%s.pid", directory, name );
  path[ PATH_MAX - 1 ] = '\0';

  if ( access( path, R_OK ) < 0 ) {
    return -1;
  }

  int fd = open( path, O_RDONLY, 0 );
  if ( fd < 0 ) {
    warn( "Failed to open %s ( %s [%d] ).", path, strerror( errno ), errno );
    return -1;
  }
  char str[ PID_STRING_LENGTH ];
  ssize_t length = read( fd, str, sizeof( str ) - 1 );
  if ( length < 0 ) {
    warn( "Failed to read pid from %s ( %s [%d] ).", path, strerror( errno ), errno );
    close( fd );
    return -1;
  }
  str[ length ] = '\0';
  if ( str[ length - 1 ] == '\n' ) {
    str[ length - 1 ] = '\0';
  }
  close( fd );

  char *endp = NULL;
  pid_t pid = ( pid_t ) strtol( str, &endp, 0 );
  if ( *endp != '\0' ) {
    warn( "Invalid pid `%s'.", str );
    return -1;
  }
  if ( pid <= 0 ) {
    warn( "Invalid pid %d.", pid );
    return -1;
  }

  int ret = kill( pid, 0 );
  if ( ret < 0 ) {
    warn( "Failed to check process id %d ( %s [%d] ).", pid, strerror( errno ), errno );
    if ( errno == ESRCH ) {
      unlink( path );
    }
    return -1;
  }

  char proc_path[ 32 ];
  char exe_path[ PATH_MAX ];
  sprintf( proc_path, "/proc/%d/exe", pid );
  ssize_t readsiz = readlink( proc_path, exe_path, sizeof( exe_path ) - 1 );
  if ( readsiz == -1 ) {
    warn( "Failed to check process id %d ( %s [%d] ).", pid, strerror( errno ), errno );
    return -1;
  }
  exe_path[ readsiz ] = '\0';

  char *exe_name = basename( exe_path );
  if ( strcmp( name, exe_name ) == 0 ) {
    return pid;
  }

  // check if -n (service_name) option is used
  sprintf( proc_path, "/proc/%d/cmdline", pid );
  fd = open( proc_path, O_RDONLY, 0 );
  if ( fd < 0 ) {
    warn( "Failed to open %s ( %s [%d] ).", proc_path, strerror( errno ), errno );
    return -1;
  }

  char* buffer = xcalloc( PATH_MAX+2, sizeof( char ) );
  char* write_head = buffer;
  size_t buffer_left = PATH_MAX;
  size_t read_byte = 0;
  while( ( readsiz = read( fd, write_head, buffer_left) ) > 0 ) {
    read_byte += ( size_t ) readsiz;
    write_head += readsiz;
    buffer_left = PATH_MAX - read_byte;
  }
  close( fd );

  // convert cmdline to argv format
  size_t argc = 0;
  char ** argv = xcalloc( 1, sizeof( char * ) );

  char* token = buffer;
  const char* buffer_end = buffer + read_byte;
  while( token <= buffer_end ) {
    size_t token_len = strlen( token );
    if ( token_len > 0 ) {
      ++argc;
      char** new_argv = xmalloc( ( argc + 1 ) * sizeof( char * ) );
      for ( size_t i = 0 ; i < argc - 1 ; ++i ) {
        new_argv[ i ] = argv[ i ];
      }
      xfree( argv );
      argv = new_argv;
      argv[ argc - 1 ] = token;
      argv[ argc ] = NULL;
    } else {
      if ( *(token+1) == '\0' ) {
        // "\0\0", end of cmdline
        break;
      }
    }
    token += token_len + 1;
  }

  const struct option long_options[] = {
    { "name", 1, NULL, 'n' },
    { NULL, 0, NULL, 0 },
  };
  const char short_options[] = "n:";
  int c;
  optind = 0;
  opterr = 0;
  const char* service_name = basename( argv[0] );
  while ( ( c = getopt_long( ( int )argc, argv, short_options, long_options, NULL ) ) != -1 ){
    switch ( c ) {
    case 'n':
      service_name = optarg;
      break;
    }
  }

  if ( strcmp( name, service_name ) == 0 ) {
    xfree( argv );
    xfree( buffer );
    return pid;
  } else {
    warn( "Failed to check process name %s.", name );
    xfree( argv );
    xfree( buffer );
    return -1;
  }
}


void
rename_pid( const char *directory, const char *old, const char *new ) {
  assert( directory != NULL );
  assert( old != NULL );
  assert( new != NULL );

  char old_path[ PATH_MAX ];
  snprintf( old_path, PATH_MAX, "%s/%s.pid", directory, old );
  old_path[ PATH_MAX - 1 ] = '\0';
  char new_path[ PATH_MAX ];
  snprintf( new_path, PATH_MAX, "%s/%s.pid", directory, new );
  new_path[ PATH_MAX - 1 ] = '\0';

  unlink( new_path );
  int ret = rename( old_path, new_path );
  if ( ret < 0 ) {
    die( "Could not rename a PID file from %s to %s.", old_path, new_path );
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
