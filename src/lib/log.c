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
#include "bool.h"
#include "checks.h"
#include "log.h"
#include "trema_string.h"
#include "wrapper.h"


static void log_stdout( int priority, const char *format, va_list ap );

static int level = LOG_INFO;
static const int level_min = LOG_CRIT;
static const int level_max = LOG_DEBUG;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static char ident[ PATH_MAX ];


typedef struct priority {
  const char *name;
  const int value;
} priority;


static priority priority_list[] = {
  { .name = "CRITICAL", .value = LOG_CRIT },
  { .name = "critical", .value = LOG_CRIT },
  { .name = "CRIT", .value = LOG_CRIT },
  { .name = "crit", .value = LOG_CRIT },

  { .name = "ERROR", .value = LOG_ERR },
  { .name = "error", .value = LOG_ERR },
  { .name = "ERR", .value = LOG_ERR },
  { .name = "err", .value = LOG_ERR },

  { .name = "WARNING", .value = LOG_WARNING },
  { .name = "warning", .value = LOG_WARNING },
  { .name = "WARN", .value = LOG_WARNING },
  { .name = "warn", .value = LOG_WARNING },

  { .name = "NOTICE", .value = LOG_NOTICE },
  { .name = "notice", .value = LOG_NOTICE },

  { .name = "INFORMATION", .value = LOG_INFO },
  { .name = "information", .value = LOG_INFO },
  { .name = "INFO", .value = LOG_INFO },
  { .name = "info", .value = LOG_INFO },

  { .name = "DEBUG", .value = LOG_DEBUG },
  { .name = "debug", .value = LOG_DEBUG },
  { .name = "DBG", .value = LOG_DEBUG },
  { .name = "dbg", .value = LOG_DEBUG },
  { .name = NULL }
};


static void ( *do_log )( int priority, const char *format, va_list ap ) = NULL;


static int
logging_level_from( const char *name ) {
  assert( name != NULL );

  priority *p;
  for ( p = priority_list; p->name != NULL; p++ ) {
    if ( strcmp( p->name, name ) == 0 ) {
      return p->value;
    }
  }
  return -1;
}


bool
init_log( const char *custom_ident, bool run_as_daemon ) {
  pthread_mutex_lock( &mutex );
  level = LOG_INFO;

  if ( run_as_daemon ) {
    do_log = trema_vsyslog;
    // we need to copy custom_ident since it might be freed.
    strncpy( ident, custom_ident, sizeof( ident ) );
    ident[ sizeof( ident ) - 1 ] = '\0';
    openlog( ident, LOG_PID, LOG_DAEMON );
  }
  else {
    do_log = log_stdout;
  }
  char *level_string = getenv( "LOGGING_LEVEL" );
  if ( level_string != NULL ) {
    set_logging_level( level_string );
  }

  pthread_mutex_unlock( &mutex );

  return true;
}


bool
set_logging_level( const char *name ) {
  int newLevel = logging_level_from( name );
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


static void
log_stdout( int priority, const char *format, va_list ap ) {
  UNUSED( priority );

  char format_newline[ strlen( format ) + 1 ];
  sprintf( format_newline, "%s\n", format );
  trema_vprintf( format_newline, ap );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
