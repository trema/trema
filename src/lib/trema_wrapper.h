/*
 * Wraps *printf family.
 *
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


#ifndef TREMA_WRAPPER_H
#define TREMA_WRAPPER_H


#include <sqlite3.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


extern int ( *trema_fprintf )( FILE *stream, const char *format, ... );
extern int ( *trema_vprintf )( const char *format, va_list ap );
extern int ( *trema_vasprintf )( char **strp, const char *fmt, va_list ap );

extern void * ( *trema_malloc )( size_t size );
extern void * ( *trema_calloc )( size_t nmemb, size_t size );
extern void ( *trema_free )( void *ptr );

extern void ( *trema_abort )( void );

extern int ( *trema_unlink )( const char *pathname );

extern pid_t ( *trema_getpid )( void );

extern void ( *trema_openlog )( const char *ident, int option, int facility );
extern void ( *trema_closelog )( void );
extern void ( *trema_vsyslog )( int priority, const char *format, va_list ap );

extern int ( *trema_sqlite3_open )( const char *filename, sqlite3 **ppDb );
extern int ( *trema_sqlite3_close )( sqlite3 * );
extern int ( *trema_sqlite3_exec )( sqlite3 *, const char *sql, int ( *callback )( void *, int, char **, char ** ), void *, char **errmsg );
extern int ( *trema_sqlite3_changes )( sqlite3 * );
extern void ( *trema_sqlite3_free )( void * );
extern const char * ( *trema_sqlite3_errmsg )( sqlite3 * );


#endif // TREMA_WRAPPER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
