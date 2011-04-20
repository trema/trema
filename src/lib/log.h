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


#ifndef LOG_H
#define LOG_H


#include <syslog.h>
#include "bool.h"


bool init_log( const char *ident, bool run_as_daemon );
bool set_logging_level( const char *level );
int get_logging_level( void );
void critical( const char *format, ... );
void error( const char *format, ... );
void warn( const char *format, ... );
void notice( const char *format, ... );
void info( const char *format, ... );
void debug( const char *format, ... );


#endif // LOG_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
