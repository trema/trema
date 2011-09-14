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


/**
 * @file
 *
 * @brief Logging Function Implementation
 *
 * File containing various Logging related functions. 
 * @code
 * //Open the log file
 * init_log( "log.file", "log_directory", 0 );
 * // Setting last argument as 0 would make the logger write all log messages to terminal.
 *
 * // Log a message with Critical priority
 * critical( "This is a critical level log message with ID: %d\n", 0 );
 * // Log a message with Notice priority
 * notice( "This is a notice level log message with ID: %d\n", 3 );
 *
 * // Read the current logging level
 * int log_level = get_logging_level();
 *
 * // Close the log file
 * finalize_log();
 * @endcode
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


/**
 * Structure for defining Log Level priorities
 */
typedef struct {
  const char *name;
  const int value;
} priority;


static FILE *fd = NULL;
static int level = -1;
static bool daemonized = false;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


/** 
 * Definition of array containing all available Priority levels.
 */
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


/**
 * Convert Log Priority Level into Priority Name.
 * @param level Integer value of level
 * @return char* String representing the name of the Level
 */
static char *
priority_name_from( int level ) {
  assert( level >= LOG_CRITICAL && level <= LOG_DEBUG );
  const char *name = priorities[ level ][ 0 ].name;
  assert( name != NULL );
  return xstrdup( name );
}


static const size_t max_message_length = 1024;

/**
 * Main logging routine. Writes log message to the log file.
 * @param priority Priority of the message
 * @param format Pointer to the specifier string defining format of variable argument list
 * @param ap Variable argument list
 * @return None
 */
static void
log_file( int priority, const char *format, va_list ap ) {
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


/**
 * Logging to Output Stream (terminal).
 * This is wrapped around by the do_log function.
 * @param format Specifier string for format of the variable arguments
 * @param ap variable argument list
 * @return None
 * @see do_log
 */
static void
log_stdout( const char *format, va_list ap ) {
  char format_newline[ strlen( format ) + 2 ];
  sprintf( format_newline, "%s\n", format );
  va_list new_ap;
  va_copy( new_ap, ap );
  trema_vprintf( format_newline, new_ap );
}


/**
 * Open the Log file.
 * @param ident Name of the log file
 * @param log_directory Path of the directory in which Log file would be created
 * @return FILE* Object to the file opened, if successful, else NULL is returned
 */
static FILE*
open_log( const char *ident, const char *log_directory ) {
  char pathname[ PATH_MAX ];
  sprintf( pathname, "%s/%s.log", log_directory, ident );
  return fopen( pathname, "w" );
}


/**
 * Initializing the Logger. Creates the log file to which logging would be done.
 * @param ident Name of the log file, used as an identifier
 * @param log_directory Name of the directory in which file with name ident would be created
 * @param run_as_daemon Boolean variable defining if logging should be reported to terminal as well
 * @return bool True always
 */
bool
init_log( const char *ident, const char *log_directory, bool run_as_daemon ) {
  pthread_mutex_lock( &mutex );

  level = LOG_INFO;
  daemonized = run_as_daemon;
  fd = open_log( ident, log_directory );

  pthread_mutex_unlock( &mutex );

  return true;
}


/**
 * Close the log file.
 * @param None
 * @return bool True always
 */
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


/**
 * Converts a string into its lower case equivalent.
 * @param string String to convert into lower case
 * @return char* String converted into lower case
 */
static char *
lower( const char *string ) {
  char *new_string = xstrdup( string );
  for ( int i = 0; new_string[ i ] != '\0'; ++i ) {
    new_string[ i ] = ( char ) tolower( new_string[ i ] );
  }
  return new_string;
}


/**
 * Get integer Priority value from the Priority Name.
 * @param name String containing name of the Priority Level
 * @return int Integer representation of the priority level
 */
static int
priority_value_from( const char *name ) {
  assert( name != NULL );

  int level_value = -1;
  char *name_lower = lower( name );

  for ( int i = 0; i <= LOG_DEBUG; i++ ) {
    for ( priority *p = priorities[ i ]; p->name != NULL; p++ ) {
      if ( strcmp( p->name, name_lower ) == 0 ) {
        level_value = p->value;
        break;
      }
    }
  }
  xfree( name_lower );
  return level_value;
}


/**
 * Check if the logger has been started or not. Applicable for interfaces which need to know loggers
 * state before any configuration or logging has to be performed.
 * @param None
 * @return bool True if the Log file is open/active, else False
 */
static bool
started() {
  if ( fd == NULL ) {
    return false;
  }
  else {
    return true;
  }
}


/**
 * Check the state of the Logger. If the logger has not been initialized, it aborts.
 * @param None
 * @return None
 */
static void
check_initialized() {
  if ( !started() ) {
    // We can't call die() here because die() calls critical() internally.
    trema_abort();
  }
}


/**
 * Set the logging level, over-riding any which has been previously set.
 * @param name Name of the logging level to be set
 * @return bool True if the logging level was successfully updated, else False
 */
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


/**
 * Gets the logging level which is currently set.
 * @param None
 * @return int Logging level currently prevalent
 */
static int
_get_logging_level() {
  check_initialized();

  char *level_string = getenv( "LOGGING_LEVEL" );
  if ( level_string != NULL ) {
    set_logging_level( level_string );
  }

  assert( level >= LOG_CRITICAL && level <= LOG_DEBUG );
  return level;
}
int ( *get_logging_level )( void ) = _get_logging_level;


/**
 * Main logging routine which writes the log messages to the file as well as the standard output.
 * @param priority Priority level of the log
 * @param format Specifier string for the variable argument list
 * @param ap Variable argument list
 * @return None
 */
static void
do_log( int priority, const char *format, va_list ap ) {
  assert( started() );

  log_file( priority, format, ap );
  if ( !daemonized ) {
    log_stdout( format, ap );
  }
}


/**
 * Macro for log writer. This acts as external visible logging routine. Invokes the internal do_log function.
 * @param _priority Priority Level
 * @param _format Specifier string containing format string and variable argument list
 * @return None
 */
#define DO_LOG( _priority, _format )                \
  do {                                              \
    if ( _format == NULL ) {                        \
      die( "Log message must not be NULL" );        \
    }                                               \
    if ( get_logging_level() >= _priority ) {       \
      pthread_mutex_lock( &mutex );                 \
      va_list _args;                                \
      va_start( _args, _format );                   \
      do_log( _priority, _format, _args );          \
      va_end( _args );                              \
      pthread_mutex_unlock( &mutex );               \
    }                                               \
  } while ( 0 )


/**
 * Internal function for writing a critical priority level log message.
 * @param format Variable list specifying format and their arguments
 * @return None. 
 */
static void
_critical( const char *format, ... ) {
  DO_LOG( LOG_CRITICAL, format );
}
/**
 * Log a critical priority level message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
void ( *critical )( const char *format, ... ) = _critical;


/**
 * Internal function for writing a Error priority level log message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
static void
_error( const char *format, ... ) {
  DO_LOG( LOG_ERROR, format );
}
/**
 * Log a Error (err) priority level message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
void ( *error )( const char *format, ... ) = _error;


/**
 * Internal function for writing a Warning priority level log message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
static void
_warn( const char *format, ... ) {
  DO_LOG( LOG_WARN, format );
}
/**
 * Log a Warning (warn) priority level message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
void ( *warn )( const char *format, ... ) = _warn;


/**
 * Internal function for writing a Notice priority level log message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
static void
_notice( const char *format, ... ) {
  DO_LOG( LOG_NOTICE, format );
}
/**
 * Log a Notification (notice) priority level message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
void ( *notice )( const char *format, ... ) = _notice;


/**
 * Internal function for writing a Information priority level log message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
static void
_info( const char *format, ... ) {
  DO_LOG( LOG_INFO, format );
}
/**
 * Log a Information (info) priority level message. 
 * @param format Variable list specifying format and their arguments
 * @return None
 */
void ( *info )( const char *format, ... ) = _info;


/**
 * Internal function for writing a Debug priority level log message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
static void
_debug( const char *format, ... ) {
  DO_LOG( LOG_DEBUG, format );
}
/**
 * Log a Debug (dbg) priority level message.
 * @param format Variable list specifying format and their arguments
 * @return None
 */
void ( *debug )( const char *format, ... ) = _debug;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
