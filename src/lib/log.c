/*
 * Copyright (C) 2008-2012 NEC Corporation
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
#include <syslog.h>
#include <time.h>
#include "bool.h"
#include "log.h"
#include "trema_wrapper.h"
#include "wrapper.h"


typedef struct {
  const char *name;
  const int value;
} priority;

typedef priority facility;


static bool initialized = false;
static FILE *fd = NULL;
static int level = -1;
static int facility_value = -1;
static char ident_string[ PATH_MAX ];
static char log_directory[ PATH_MAX ];
static logging_type output = LOGGING_TYPE_FILE;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


static priority priorities[][ 3 ] = {
  {
    { .name = "critical", .value = LOG_CRIT },
    { .name = "crit", .value = LOG_CRIT },
    { .name = NULL },
  },

  {
    { .name = "error", .value = LOG_ERR },
    { .name = "err", .value = LOG_ERR },
    { .name = NULL },
  },

  {
    { .name = "warn", .value = LOG_WARNING },
    { .name = "warning", .value = LOG_WARNING },
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

static facility facilities[] = {
  { .name = "kern", .value = LOG_KERN },
  { .name = "user", .value = LOG_USER },
  { .name = "mail", .value = LOG_MAIL },
  { .name = "daemon", .value = LOG_DAEMON },
  { .name = "auth", .value = LOG_AUTH },
  { .name = "syslog", .value = LOG_SYSLOG },
  { .name = "lpr", .value = LOG_LPR },
  { .name = "news", .value = LOG_NEWS },
  { .name = "uucp", .value = LOG_UUCP },
  { .name = "cron", .value = LOG_CRON },
  { .name = "authpriv", .value = LOG_AUTHPRIV },
  { .name = "ftp", .value = LOG_FTP },
  { .name = "local0", .value = LOG_LOCAL0 },
  { .name = "local1", .value = LOG_LOCAL1 },
  { .name = "local2", .value = LOG_LOCAL2 },
  { .name = "local3", .value = LOG_LOCAL3 },
  { .name = "local4", .value = LOG_LOCAL4 },
  { .name = "local5", .value = LOG_LOCAL5 },
  { .name = "local6", .value = LOG_LOCAL6 },
  { .name = "local7", .value = LOG_LOCAL7 },
  { .name = NULL },
};


static bool
started() {
  return initialized;
}


static const char *
priority_name_from( int level ) {
  assert( level >= LOG_CRIT && level <= LOG_DEBUG );
  const char *name = priorities[ level - LOG_CRIT ][ 0 ].name;
  assert( name != NULL );
  return name;
}


static const size_t max_message_length = 1024;

static void
log_file( int priority, const char *format, va_list ap ) {
  time_t tm = time( NULL );
  char now[ 26 ];
  asctime_r( localtime( &tm ), now );
  now[ 24 ] = '\0'; // chomp

  const char *priority_name = priority_name_from( priority );

  char message[ max_message_length ];
  va_list new_ap;
  va_copy( new_ap, ap );
  vsnprintf( message, max_message_length, format, new_ap );
  va_end( new_ap );

  trema_fprintf( fd, "%s [%s] %s\n", now, priority_name, message );
  fflush( fd );
}


static void
log_stdout( const char *format, va_list ap ) {
  char format_newline[ strlen( format ) + 2 ];
  sprintf( format_newline, "%s\n", format );
  va_list new_ap;
  va_copy( new_ap, ap );
  trema_vprintf( format_newline, new_ap );
  va_end( new_ap );
  fflush( stdout );
}


static void
log_syslog( int priority, const char *format, va_list ap ) {
  va_list new_ap;
  va_copy( new_ap, ap );
  trema_vsyslog( priority, format, new_ap );
  va_end( new_ap );
}


static void
unset_ident_string() {
  memset( ident_string, '\0', sizeof( ident_string ) );
}


static void
set_ident_string( const char *ident ) {
  assert( ident != NULL );

  unset_ident_string();
  strncpy( ident_string, ident, sizeof( ident_string ) - 1 );
}


static const char *
get_ident_string() {
  return ident_string;
}


static void
unset_log_directory() {
  memset( log_directory, '\0', sizeof( log_directory ) );
}


static void
set_log_directory( const char *directory ) {
  assert( directory != NULL );

  unset_log_directory();
  strncpy( log_directory, directory, sizeof( log_directory ) - 1 );
}


static const char *
get_log_directory() {
  return log_directory;
}


static FILE *
open_log_file( bool append ) {
  assert( strlen( get_log_directory() ) > 0 );
  assert( strlen( get_ident_string() ) > 0 );

  char pathname[ PATH_MAX ];
  sprintf( pathname, "%s/%s.log", get_log_directory(), get_ident_string() );
  FILE *log = fopen( pathname, append ? "a" : "w" );

  if ( log == NULL ) {
    char error_msg[ PATH_MAX + 32 ];
    snprintf( error_msg, PATH_MAX + 32, "open_log: fopen( \"%s\", \"w\" )", pathname );
    perror( error_msg );
    trema_abort();
  }

  return log;
}


static void
open_log_syslog() {
  assert( strlen( get_ident_string() ) > 0 );
  assert( ( facility_value & ~LOG_FACMASK ) == 0 );
  trema_openlog( get_ident_string(), LOG_NDELAY, facility_value );
}


static int
facility_value_from( const char *name ) {
  assert( name != NULL );

  int value = -1;

  for ( int i = 0; facilities[ i ].name != NULL; i++ ) {
    if ( strcasecmp( facilities[ i ].name, name ) == 0 ) {
      value = facilities[ i ].value;
      break;
    }
  }

  return value;
}


/**
 * Sets syslog facility.
 *
 * @param name name of the syslog facility to be set.
 * @return true on success; false otherwise.
 */
bool
set_syslog_facility( const char *name ) {
  assert( name != NULL );

  int new_facility_value = facility_value_from( name );
  if ( new_facility_value == -1 ) {
    fprintf( stderr, "Invalid syslog facility: %s\n", name );
    trema_abort();
  }

  pthread_mutex_lock( &mutex );
  facility_value = new_facility_value;
  if ( ( output & LOGGING_TYPE_SYSLOG ) != 0 && started() ) {
    trema_closelog();
    open_log_syslog();
  }
  pthread_mutex_unlock( &mutex );

  return true;
}


/**
 * Initializes the Logger. This creates a log file to which messages
 * are written.
 *
 * @param ident name of the log file, used as an identifier.
 * @param log_directory the directory in which log file is created.
 * @param type log output type.
 * @return true on success; false otherwise.
 */
bool
init_log( const char *ident, const char *directory, logging_type type ) {
  assert( ident != NULL );
  assert( directory != NULL );

  pthread_mutex_lock( &mutex );

  // set_logging_level() may be called before init_log().
  // level = -1 indicates that logging level is not set yet.
  if ( level < 0 || level > LOG_DEBUG ) {
    level = LOG_INFO;
  }
  char *level_string = getenv( "LOGGING_LEVEL" );
  if ( level_string != NULL ) {
    set_logging_level( level_string );
  }

  // set_syslog_facility() may be called before init_log().
  // facility_value = -1 indicates that facility value is not set yet.
  if ( ( facility_value & ~LOG_FACMASK ) != 0 ) {
    facility_value = LOG_USER;
  }
  char *facility_string = getenv( "LOGGING_FACILITY" );
  if ( facility_string != NULL ) {
    set_syslog_facility( facility_string );
  }

  set_ident_string( ident );
  set_log_directory( directory );
  output = type;
  if ( output & LOGGING_TYPE_FILE ) {
    fd = open_log_file( false );
  }
  if ( output & LOGGING_TYPE_SYSLOG ) {
    open_log_syslog();
  }

  initialized = true;

  pthread_mutex_unlock( &mutex );

  return true;
}


void
restart_log( const char *new_ident ) {
  pthread_mutex_lock( &mutex );

  if ( new_ident != NULL ) {
    set_ident_string( new_ident );
  }

  if ( output & LOGGING_TYPE_FILE ) {
    if ( fd != NULL ) {
      fclose( fd );
    }
    fd = open_log_file( true );
  }
  if ( output & LOGGING_TYPE_SYSLOG ) {
    trema_closelog();
    open_log_syslog();
  }

  pthread_mutex_unlock( &mutex );
}


void
rename_log( const char *new_ident ) {
  assert( new_ident != NULL );

  pthread_mutex_lock( &mutex );

  if ( output & LOGGING_TYPE_FILE ) {
    char old_path[ PATH_MAX ];
    snprintf( old_path, PATH_MAX, "%s/%s.log", get_log_directory(), get_ident_string() );
    old_path[ PATH_MAX - 1 ] = '\0';
    char new_path[ PATH_MAX ];
    set_ident_string( new_ident );
    snprintf( new_path, PATH_MAX, "%s/%s.log", get_log_directory(), get_ident_string() );
    new_path[ PATH_MAX - 1 ] = '\0';

    unlink( new_path );
    int ret = rename( old_path, new_path );
    if ( ret < 0 ) {
      die( "Could not rename a log file from %s to %s.", old_path, new_path );
    }
  }
  if ( output & LOGGING_TYPE_SYSLOG ) {
    trema_closelog();
    set_ident_string( new_ident );
    open_log_syslog();
  }

  pthread_mutex_unlock( &mutex );
}


/**
 * Closes the log file.
 *
 * @return true on success; false otherwise.
 */
bool
finalize_log() {
  pthread_mutex_lock( &mutex );

  level = -1;
  facility_value = -1;

  if ( output & LOGGING_TYPE_FILE ) {
    if ( fd != NULL ) {
      fclose( fd );
      fd = NULL;
    }
  }
  if ( output & LOGGING_TYPE_SYSLOG ) {
    trema_closelog();
  }

  unset_ident_string();
  unset_log_directory();

  initialized = false;

  pthread_mutex_unlock( &mutex );

  return true;
}


static int
priority_value_from( const char *name ) {
  assert( name != NULL );

  int level_value = -1;

  assert( ( LOG_DEBUG - LOG_CRIT + 1 ) == ( sizeof( priorities )/sizeof( priorities[ 0 ] ) ) );
  for ( int i = 0; i <= ( LOG_DEBUG - LOG_CRIT ); i++ ) {
    for ( priority *p = priorities[ i ]; p->name != NULL; p++ ) {
      if ( strcasecmp( p->name, name ) == 0 ) {
        level_value = p->value;
        break;
      }
    }
  }
  return level_value;
}


/**
 * Check if a provided logging level is valid or not.
 *
 * @param name name of the logging level to be checked.
 * @return true if valid; false otherwise.
 */
bool
valid_logging_level( const char *name ) {
  assert( name != NULL );

  int level_value = priority_value_from( name );
  if ( level_value < 0 ) {
    return false;
  }

  return true;
}


/**
 * Sets a new logging level. This overrides the value which has been
 * previously set.
 *
 * @param name name of the logging level to be set.
 * @return true on success; false otherwise.
 */
bool
set_logging_level( const char *name ) {
  int new_level = priority_value_from( name );
  if ( new_level == -1 ) {
    fprintf( stderr, "Invalid logging level: %s\n", name );
    trema_abort();
  }
  pthread_mutex_lock( &mutex );
  level = new_level;
  pthread_mutex_unlock( &mutex );

  return true;
}


static int
_get_logging_level() {
  return level;
}
int ( *get_logging_level )( void ) = _get_logging_level;


static void
do_log( int priority, const char *format, va_list ap ) {
  assert( started() );

  if ( output & LOGGING_TYPE_FILE ) {
    log_file( priority, format, ap );
  }
  if ( output & LOGGING_TYPE_SYSLOG ) {
    log_syslog( priority, format, ap );
  }
  if ( output & LOGGING_TYPE_STDOUT ) {
    log_stdout( format, ap );
  }
}


#define DO_LOG( _priority, _format )                    \
  do {                                                  \
    if ( _format == NULL ) {                            \
      trema_abort();                                    \
    }                                                   \
    if ( get_logging_level() >= _priority ) {           \
      pthread_mutex_lock( &mutex );                     \
      va_list _args;                                    \
      va_start( _args, _format );                       \
      do_log( _priority, _format, _args );              \
      va_end( _args );                                  \
      pthread_mutex_unlock( &mutex );                   \
    }                                                   \
  }                                                     \
  while ( 0 )


static void
_critical( const char *format, ... ) {
  DO_LOG( LOG_CRIT, format );
}
/**
 * Logs an critical message.
 *
 * @param format format string, followed by parameters to insert into the format string (as with printf())
 */
void ( *critical )( const char *format, ... ) = _critical;


static void
_error( const char *format, ... ) {
  DO_LOG( LOG_ERR, format );
}
/**
 * Logs an error message.
 *
 * @param format format string, followed by parameters to insert into the format string (as with printf())
 */
void ( *error )( const char *format, ... ) = _error;


static void
_warn( const char *format, ... ) {
  DO_LOG( LOG_WARNING, format );
}
/**
 * Logs a warning message.
 *
 * @param format format string, followed by parameters to insert into the format string (as with printf())
 */
void ( *warn )( const char *format, ... ) = _warn;


static void
_notice( const char *format, ... ) {
  DO_LOG( LOG_NOTICE, format );
}
/**
 * Logs a notice message.
 *
 * @param format format string, followed by parameters to insert into the format string (as with printf())
 */
void ( *notice )( const char *format, ... ) = _notice;


static void
_info( const char *format, ... ) {
  DO_LOG( LOG_INFO, format );
}
/**
 * Logs an info message.
 *
 * @param format format string, followed by parameters to insert into the format string (as with printf())
 */
void ( *info )( const char *format, ... ) = _info;


static void
_debug( const char *format, ... ) {
  DO_LOG( LOG_DEBUG, format );
}
/**
 * Logs a debug message.
 *
 * @param format format string, followed by parameters to insert into the format string (as with printf())
 */
void ( *debug )( const char *format, ... ) = _debug;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
