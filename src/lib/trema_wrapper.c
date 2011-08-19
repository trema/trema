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
 * @brief Wrappers for basic library functions
 *
 * @code
 * // For printing to stream or screen within the trema code, call trema_* series
 * // of wrappers. e.g. trema_fprintf, trema_vsprintf, trema_vasprintf
 * trema_fprintf( stdout, "A test message: %s\n", "TEST" );
 * // Or, allocation of memory, call trema_* series of wrappers
 * trema_malloc( 10 );
 * @endcode
 *
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


int ( *trema_fprintf )( FILE *stream, const char *format, ... ) = fprintf;
int ( *trema_vprintf )( const char *format, va_list ap ) = vprintf;
int ( *trema_vasprintf )( char **strp, const char *fmt, va_list ap ) = vasprintf;

void * ( *trema_malloc )( size_t size ) = malloc;
void * ( *trema_calloc )( size_t nmemb, size_t size ) = calloc;
void ( *trema_free )( void *ptr ) = free;

void ( *trema_abort )( void ) = abort;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
