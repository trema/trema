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
#include <linux/limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bool.h"
#include "checks.h"
#include "log.h"
#include "trema_string.h"
#include "wrapper.h"


static FILE *fd = NULL;
static int level = LOG_INFO;
static const int level_min = LOG_CRIT;
static const int level_max = LOG_DEBUG;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


typedef struct priority {
  const char *name;
  const int value;
} priority;


static priority priority_list[][ 5 ] = {
  {
    { .name = "critical", .value = LOG_CRIT },
    { .name = "CRITICAL", .value = LOG_CRIT },
    { .name = "CRIT", .value = LOG_CRIT },
    { .name = "crit", .value = LOG_CRIT },
    { .name = NULL }
  },
  
  {
    { .name = "error", .value = LOG_ERR },
    { .name = "ERROR", .value = LOG_ERR },
    { .name = "ERR", .value = LOG_ERR },
    { .name = "err", .value = LOG_ERR },
    { .name = NULL }
  },

  {
    { .name = "warning", .value = LOG_WARNING },
    { .name = "WARNING", .value = LOG_WARNING },
    { .name = "WARN", .value = LOG_WARNING },
    { .name = "warn", .value = LOG_WARNING },
    { .name = NULL }
  },

  {
    { .name = "notice", .value = LOG_NOTICE },
    { .name = "NOTICE", .value = LOG_NOTICE },
    { .name = NULL }
  },

  {
    { .name = "info", .value = LOG_INFO },
    { .name = "INFORMATION", .value = LOG_INFO },
    { .name = "information", .value = LOG_INFO },
    { .name = "INFO", .value = LOG_INFO },
    { .name = NULL }
  },

  {
    { .name = "debug", .value = LOG_DEBUG },
    { .name = "DEBUG", .value = LOG_DEBUG },
    { .name = "DBG", .value = LOG_DEBUG },
    { .name = "dbg", .value = LOG_DEBUG },
    { .name = NULL }
  }
};


static void ( *do_log )( int priority, const char *format, va_list ap ) = NULL;


static void
level_string_from( int level, char *string ) {
  assert( level >= level_min && level <= level_max );
  assert( string != NULL );

  const char *name = priority_list[ level ][ 0 ].name;
  strncpy( string, name, strlen( name ) + 1 );
}


static void
log_file( int priority, const char *format, va_list ap ) {
  UNUSED( priority );

  time_t tm = time( NULL );
  char date_str[ 256 ];
  asctime_r( localtime( &tm ), date_str );
  date_str[ strlen( date_str ) - 1 ] = '\0';

  char logging_level[ 128 ];
  level_string_from( priority, logging_level );

  char message[ 1024 ];
  vsprintf( message, format, ap );
  trema_fprintf( fd, "%s [%s] %s\n", date_str, logging_level, message );
}


static void
log_stdout( int priority, const char *format, va_list ap ) {
  log_file( priority, format, ap );

  char format_newline[ strlen( format ) + 1 ];
  sprintf( format_newline, "%s\n", format );
  trema_vprintf( format_newline, ap );
}


bool
init_log( const char *ident, const char *log_directory, bool run_as_daemon ) {
  pthread_mutex_lock( &mutex );

  level = LOG_INFO;

  if ( run_as_daemon ) {
    do_log = log_file;
  }
  else {
    do_log = log_stdout;
  }
  char pathname[ PATH_MAX ];
  sprintf( pathname, "%s/%s.log", log_directory, ident );
  fd = fopen( pathname, "w" );

  char *level_string = getenv( "LOGGING_LEVEL" );
  if ( level_string != NULL ) {
    set_logging_level( level_string );
  }

  pthread_mutex_unlock( &mutex );

  return true;
}


static int
level_value_from( const char *name ) {
  assert( name != NULL );

  int i;
  for ( i = 0; i <= LOG_DEBUG; i++ ) {
    priority *p;
    for ( p = priority_list[ i ]; p->name != NULL; p++ ) {
      if ( strcmp( p->name, name ) == 0 ) {
        return p->value;
      }
    }
  }
  return -1;
}


bool
set_logging_level( const char *name ) {
  int newLevel = level_value_from( name );
  if ( newLevel == -1 ) {
    die( "Invalid logging level: %s", name );
  }
  pthread_mutex_lock( &mutex );
  level = newLevel;
  pthread_mutex_unlock( &mutex );

  return true;
}


int
get_logging_level( void ) {
  return level;
}


bool
logging_started( void ) {
  return do_log != NULL ? true : false;
}


#define DO_LOG( _priority, _format )                \
  do {                                              \
    assert( do_log != NULL );                       \
    if ( _format == NULL ) {                        \
      die( "Log message should not be NULL" );      \
    }                                               \
    if ( level >= _priority ) {                     \
      pthread_mutex_lock( &mutex );                 \
      va_list _args;                                \
      va_start( _args, _format );                   \
      ( *do_log )( _priority, _format, _args );     \
      va_end( _args );                              \
      pthread_mutex_unlock( &mutex );               \
    }                                               \
  } while ( 0 )


void
critical( const char *format, ... ) {
  DO_LOG( LOG_CRIT, format );
}


void
error( const char *format, ... ) {
  DO_LOG( LOG_ERR, format );
}


void
warn( const char *format, ... ) {
  DO_LOG( LOG_WARNING, format );
}


void
notice( const char *format, ... ) {
  DO_LOG( LOG_NOTICE, format );
}


void
info( const char *format, ... ) {
  DO_LOG( LOG_INFO, format );
}


void
debug( const char *format, ... ) {
  DO_LOG( LOG_DEBUG, format );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
