/*
 * A simple logging library.
 *
 * Some good logging guidelines can be found here:
 *  http://watchitlater.com/blog/2009/12/logging-guidelines/
 *
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


/**
 * @file
 *
 * @brief Versatile support for logging messages with different levels
 * of importance.
 */


#ifndef LOG_H
#define LOG_H


#include <syslog.h>
#include "bool.h"


#define LOGGING_LEVEL_STR_LENGTH 12


typedef enum {
  LOGGING_TYPE_FILE = 0x1,
  LOGGING_TYPE_SYSLOG = 0x2,
  LOGGING_TYPE_STDOUT = 0x4,
} logging_type;


bool init_log( const char *ident, const char *log_directory, logging_type type );
void restart_log( const char *new_ident );
void rename_log( const char *new_ident );
bool finalize_log( void );

bool set_logging_level( const char *level );
bool valid_logging_level( const char *level );
bool set_syslog_facility( const char *facility );

extern int ( *get_logging_level )( void );

extern void ( *critical )( const char *format, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
extern void ( *error )( const char *format, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
extern void ( *warn )( const char *format, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
extern void ( *notice )( const char *format, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
extern void ( *info )( const char *format, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
extern void ( *debug )( const char *format, ... ) __attribute__( ( format( printf, 1, 2 ) ) );


#endif // LOG_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
