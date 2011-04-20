/*
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


#ifndef UNITTEST_H
#define UNITTEST_H


#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/select.h>
#include "cmockery.h"


#define ARRAY_SIZE( name ) ( sizeof( name ) / sizeof( name[ 0 ] ) )

#define will_return_void( f ) will_return( f, 0 )

#define FD_ISZERO( s )                                   \
  ( {                                                    \
      fd_set zero_set;                                   \
      FD_ZERO( &zero_set );                              \
      memcmp( ( s ), &zero_set, sizeof( fd_set ) ) == 0; \
    }                                                    \
  )

/*
 * Relax cmockery's restrictions of expect_string() to avoid warnings.
 * See also: http://stackoverflow.com/questions/4610892/how-do-i-tell-gcc-to-relax-its-restrictions-on-typecasting-when-calling-a-c-funct
 */
#ifdef expect_string_count
#undef expect_string_count
#endif
#define expect_string_count( function, parameter, string, count )	\
  _expect_string( #function, #parameter, __FILE__, __LINE__, string, count )


#endif /* UNITTEST_H */
