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


#include <stdlib.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "log.h"
#include "trema_wrapper.h"


#ifdef test_malloc
#undef test_malloc
#endif
static void *
test_malloc( size_t size ) {
  return _test_malloc( size, __FILE__, __LINE__ );
}


#ifdef test_calloc
#undef test_calloc
#endif
static void *
test_calloc( size_t nmemb, size_t size ) {
  return _test_calloc( nmemb, size, __FILE__, __LINE__ );
}


#ifdef test_free
#undef test_free
#endif
static void
test_free( void *ptr ) {
  _test_free( ptr, __FILE__, __LINE__ );
}


void
setup_leak_detector() {
  trema_malloc = test_malloc;
  trema_calloc = test_calloc;
  trema_free = test_free;
}


void
teardown_leak_detector() {
  trema_malloc = malloc;
  trema_calloc = calloc;
  trema_free = free;
}


/********************************************************************************
 * Stub/unstub logger
 ********************************************************************************/

static void ( *original_critical )( const char *format, ... );
static void ( *original_error )( const char *format, ... );
static void ( *original_warn )( const char *format, ... );
static void ( *original_notice )( const char *format, ... );
static void ( *original_info )( const char *format, ... );
static void ( *original_debug )( const char *format, ... );


static void
log_stub( const char *format, ... ) {
  UNUSED( format );
}


void
stub_logger( void ) {
  original_critical = critical;
  original_error = error;
  original_warn = warn;
  original_notice = notice;
  original_info = info;
  original_debug = debug;

  critical = log_stub;
  error = log_stub;
  warn = log_stub;
  notice = log_stub;
  info = log_stub;
  debug = log_stub;
}


void
unstub_logger( void ) {
  critical = original_critical;
  error = original_error;
  warn = original_warn;
  notice = original_notice;
  info = original_info;
  debug = original_debug;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
