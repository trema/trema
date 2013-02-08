/*
 * A common header file for Trema unit tests.
 * All *_test.c in unittests/ must include this.
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


#ifndef CMOCKERY_TREMA_H
#define CMOCKERY_TREMA_H


/********************************************************************************
 * Load google's cmockery and its prerequisites.
 ********************************************************************************/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "cmockery.h"


/********************************************************************************
 * Setup/teardown dynamic allocators with leak detection
 ********************************************************************************/

void setup_leak_detector( void );
void teardown_leak_detector( void );

void stub_logger( void );
void unstub_logger( void );


/********************************************************************************
 * Relax cmockery's restrictions to avoid warnings.
 * See also: http://bit.ly/iw9ZbB
 ********************************************************************************/

#ifdef expect_string_count
#undef expect_string_count
#endif
#define expect_string_count( function, parameter, string, count )            \
  _expect_string( #function, #parameter, __FILE__, __LINE__, string, count )


#ifdef expect_assert_failure
#undef expect_assert_failure
#endif
#define expect_assert_failure( function_call )                                                  \
  {                                                                                             \
    const char *expression = ( const char * ) ( uintptr_t ) setjmp( global_expect_assert_env ); \
    global_expecting_assert = 1;                                                                \
    if ( expression ) {                                                                         \
      print_message( "Expected assertion %s occurred\n", expression );                          \
      global_expecting_assert = 0;                                                              \
    }                                                                                           \
    else {                                                                                      \
      function_call;                                                                            \
      global_expecting_assert = 0;                                                              \
      print_error( "Expected assert in %s\n", #function_call );                                 \
      _fail( __FILE__, __LINE__ );                                                              \
    }                                                                                           \
  }


#ifdef cast_to_largest_integral_type
#undef cast_to_largest_integral_type
#endif
#define cast_to_largest_integral_type( value ) \
    ( ( LargestIntegralType ) ( ( uintptr_t ) ( value ) ) )


#endif /* CMOCKERY_TREMA_H */


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
