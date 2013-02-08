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


#include <sqlite3.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>


int ( *trema_fprintf )( FILE *stream, const char *format, ... ) = fprintf;
int ( *trema_vprintf )( const char *format, va_list ap ) = vprintf;
int ( *trema_vasprintf )( char **strp, const char *fmt, va_list ap ) = vasprintf;

void * ( *trema_malloc )( size_t size ) = malloc;
void * ( *trema_calloc )( size_t nmemb, size_t size ) = calloc;
void ( *trema_free )( void *ptr ) = free;

void ( *trema_abort )( void ) = abort;

int ( *trema_unlink )( const char *pathname ) = unlink;

pid_t ( *trema_getpid )( void ) = getpid;

void ( *trema_openlog )( const char *ident, int option, int facility ) = openlog;
void ( *trema_closelog )( void ) = closelog;
void ( *trema_vsyslog )( int priority, const char *format, va_list ap ) = vsyslog;

int ( *trema_sqlite3_open )( const char *filename, sqlite3 **ppDb ) = sqlite3_open;
int ( *trema_sqlite3_close )( sqlite3 * ) = sqlite3_close;
int ( *trema_sqlite3_exec )( sqlite3 *, const char *sql, int ( *callback )( void *, int, char **, char ** ), void *, char **errmsg ) = sqlite3_exec;
int ( *trema_sqlite3_changes )( sqlite3 * ) = sqlite3_changes;
void ( *trema_sqlite3_free )( void * ) = sqlite3_free;
const char * ( *trema_sqlite3_errmsg )( sqlite3 * ) = sqlite3_errmsg;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
