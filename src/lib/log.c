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
#include <ctype.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bool.h"
#include "log.h"
#include "trema_wrapper.h"
#include "wrapper.h"


typedef struct {
  const char *name;
  const logging_level value;
} priority;


static FILE *fd = NULL;
static logging_level level = -1;
static bool daemonized = false;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


static priority priorities[][ 3 ] = {
  {
    { .name = "critical", .value = LOG_CRITICAL },
    { .name = "crit", .value = LOG_CRITICAL },
    { .name = NULL },
  },

  {
    { .name = "error", .value = LOG_ERROR },
    { .name = "err", .value = LOG_ERROR },
    { .name = NULL },
  },

  {
    { .name = "warn", .value = LOG_WARN },
    { .name = "warning", .value = LOG_WARN },
    { .name = NULL },
  },

  {
    { .name = "notice", .value = LOG_NOTICE },
    { .name = NULL },
  },

  {
    { .name = "info", .value = LOG_INFO },
    { .name = "information", .value = LOG_INFO },
    { .name = NULL },
  },

  {
    { .name = "debug", .value = LOG_DEBUG },
    { .name = "dbg", .value = LOG_DEBUG },
    { .name = NULL },
  },
};


static char *
priority_name_from( logging_level level ) {
  const char *name = priorities[ level ][ 0 ].name;
  assert( name != NULL );
  return xstrdup( name );
}


static const size_t max_message_length = 1024;

static void
log_file( logging_level priority, const char *format, va_list ap ) {
  time_t tm = time( NULL );
  char now[ 26 ];
  asctime_r( localtime( &tm ), now );
  now[ 24 ] = '\0'; // chomp

  char *priority_name = priority_name_from( priority );

  char message[ max_message_length ];
  va_list new_ap;
  va_copy( new_ap, ap );
  vsnprintf( message, max_message_length, format, new_ap );

  trema_fprintf( fd, "%s [%s] %s\n", now, priority_name, message );
  fflush( fd );

  xfree( priority_name );
}


static void
log_stdout( const char *format, va_list ap ) {
  char format_newline[ strlen( format ) + 2 ];
  sprintf( format_newline, "%s\n", format );
  va_list new_ap;
  va_copy( new_ap, ap );
  trema_vprintf( format_newline, new_ap );
  fflush( stdout );
}


static FILE*
open_log( const char *ident, const char *log_directory ) {
  assert( ident != NULL );
  assert( log_directory != NULL );

  char pathname[ PATH_MAX ];
  sprintf( pathname, "%s/%s.log", log_directory, ident );
  FILE *log = fopen( pathname, "w" );
  if ( log == NULL ) {
    perror( "fopen" );
    trema_abort();
  }

  return log;
}


bool
init_log( const char *ident, const char *log_directory, bool run_as_daemon ) {
  pthread_mutex_lock( &mutex );

  level = LOG_INFO;
  daemonized = run_as_daemon;
  fd = open_log( ident, log_directory );

  pthread_mutex_unlock( &mutex );

  return true;
}


bool
finalize_log() {
  pthread_mutex_lock( &mutex );

  level = -1;
  if ( fd != NULL ) {
    fclose( fd );
    fd = NULL;
  }

  pthread_mutex_unlock( &mutex );

  return true;
}


static char *
lower( const char *string ) {
  char *new_string = xstrdup( string );
  for ( int i = 0; new_string[ i ] != '\0'; ++i ) {
    new_string[ i ] = ( char ) tolower( new_string[ i ] );
  }
  return new_string;
}


static int
priority_value_from( const char *name ) {
  assert( name != NULL );

  int level_value = -1;
  char *name_lower = lower( name );

  for ( int i = 0; i <= LOG_DEBUG; i++ ) {
    for ( priority *p = priorities[ i ]; p->name != NULL; p++ ) {
      if ( strncmp( p->name, name_lower, 20 ) == 0 ) {
        level_value = p->value;
        break;
      }
    }
  }
  xfree( name_lower );
  return level_value;
}


static bool
started() {
  if ( fd == NULL ) {
    return false;
  }
  else {
    return true;
  }
}


static void
check_initialized() {
  if ( !started() ) {
    // We can't call die() here because die() calls critical() internally.
    trema_abort();
  }
}


bool
set_logging_level( const char *name ) {
  check_initialized();

  int new_level = priority_value_from( name );
  if ( new_level == -1 ) {
    unsetenv( "LOGGING_LEVEL" );; // avoid an infinite loop
    die( "Invalid logging level: %s", name );
  }
  pthread_mutex_lock( &mutex );
  level = new_level;
  pthread_mutex_unlock( &mutex );

  return true;
}


static logging_level
_get_logging_level() {
  check_initialized();

  char *level_string = getenv( "LOGGING_LEVEL" );
  if ( level_string != NULL ) {
    set_logging_level( level_string );
  }

  return level;
}
logging_level ( *get_logging_level )( void ) = _get_logging_level;


static void
do_log( logging_level priority, const char *format, va_list ap ) {
  assert( started() );

  log_file( priority, format, ap );
  if ( !daemonized ) {
    log_stdout( format, ap );
  }
}


#define DO_LOG( _priority, _format )                    \
  do {                                                  \
    if ( _format == NULL ) {                            \
      trema_abort();                                    \
    }                                                   \
    if ( ( int ) get_logging_level() >= _priority ) {   \
      pthread_mutex_lock( &mutex );                     \
      va_list _args;                                    \
      va_start( _args, _format );                       \
      do_log( _priority, _format, _args );              \
      va_end( _args );                                  \
      pthread_mutex_unlock( &mutex );                   \
    }                                                   \
  } while ( 0 )


static void
_critical( const char *format, ... ) {
  DO_LOG( LOG_CRITICAL, format );
}
void ( *critical )( const char *format, ... ) = _critical;


static void
_error( const char *format, ... ) {
  DO_LOG( LOG_ERROR, format );
}
void ( *error )( const char *format, ... ) = _error;


static void
_warn( const char *format, ... ) {
  DO_LOG( LOG_WARN, format );
}
void ( *warn )( const char *format, ... ) = _warn;


static void
_notice( const char *format, ... ) {
  DO_LOG( LOG_NOTICE, format );
}
void ( *notice )( const char *format, ... ) = _notice;


static void
_info( const char *format, ... ) {
  DO_LOG( LOG_INFO, format );
}
void ( *info )( const char *format, ... ) = _info;


static void
_debug( const char *format, ... ) {
  DO_LOG( LOG_DEBUG, format );
}
void ( *debug )( const char *format, ... ) = _debug;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
