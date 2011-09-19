/*
 * A simple logging library.
 *
 * Some good logging guidelines can be found here:
 *  http://watchitlater.com/blog/2009/12/logging-guidelines/
 *
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
 * @brief Loggger Implementation
 *
 * Logger Implementation routines and variables. Defines the various Logging priority
 * level and functions declarations used for Logging information.
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
 
#ifndef LOG_H
#define LOG_H


#include "bool.h"


/**
 * Enumerator defining Logging Priority Levels.
 */
enum {
  LOG_CRITICAL,
  LOG_ERROR,
  LOG_WARN,
  LOG_NOTICE,
  LOG_INFO,
  LOG_DEBUG,
};


bool init_log( const char *ident, const char *log_directory, bool run_as_daemon );
bool finalize_log( void );

bool set_logging_level( const char *level );
extern int ( *get_logging_level )( void );

extern void ( *critical )( const char *format, ... );
extern void ( *error )( const char *format, ... );
extern void ( *warn )( const char *format, ... );
extern void ( *notice )( const char *format, ... );
extern void ( *info )( const char *format, ... );
extern void ( *debug )( const char *format, ... );


#endif // LOG_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
