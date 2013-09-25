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


#include "checks.h"
#include "log.h"
#include "default-logger.h"


extern VALUE mTrema;
VALUE mDefaultLogger;


static VALUE
do_log( void ( *log_function )( const char *format, ... ), int argc, VALUE *argv ) {
  VALUE message = rb_f_sprintf( argc, argv );
  log_function( RSTRING_PTR( message ) );
  return message;
}


/*
 * @overload critical(format ...)
 *   Outputs a message representing that "the system is completely
 *   unusable" to log file.
 *
 *   @example
 *     critical "Trema blue screen. Memory dump = %s", memory
 *
 *   @return [String] the string resulting from applying format to any
 *     additional arguments.
 */
static VALUE
default_logger_critical( int argc, VALUE *argv, VALUE self ) {
  UNUSED( self );
  return( do_log( critical, argc, argv ) );
}


/*
 * @overload error(format ...)
 *   Outputs a message representing that "something went wrong" to log
 *   file.
 *
 *   @example
 *     error "Failed to accept %s", app_socket
 *
 *   @return [String] the string resulting from applying format to any
 *     additional arguments.
 */
static VALUE
default_logger_error( int argc, VALUE *argv, VALUE self ) {
  UNUSED( self );
  return( do_log( error, argc, argv ) );
}


/*
 * @overload warn(format ...)
 *   Outputs a message representing that "something in the system was
 *   not as expected" to log file.
 *
 *   @example
 *     warn "%s: trema is already initialized", app_name
 *
 *   @return [String] the string resulting from applying format to any
 *     additional arguments.
 */
static VALUE
default_logger_warn( int argc, VALUE *argv, VALUE self ) {
  UNUSED( self );
  return( do_log( warn, argc, argv ) );
}


/*
 * @overload notice(format ...)
 *   Outputs a message representing that "normal but significant
 *   condition occurred" to log file.
 *
 *   @example
 *     notice "The switch %s disconnected its secure channel connection", datapath_id
 *
 *   @return [String] the string resulting from applying format to any
 *     additional arguments.
 */
static VALUE
default_logger_notice( int argc, VALUE *argv, VALUE self ) {
  UNUSED( self );
  return( do_log( notice, argc, argv ) );
}


/*
 * @overload info(format ...)
 *   Outputs an informational massage to log file.
 *
 *   @example
 *     info "Hello world from %s!", datapath_id
 *
 *   @return [String] the string resulting from applying format to any
 *     additional arguments.
 */
static VALUE
default_logger_info( int argc, VALUE *argv, VALUE self ) {
  UNUSED( self );
  return( do_log( info, argc, argv ) );
}


/*
 * @overload debug(format ...)
 *   Outputs a debug-level massage to log file.
 *
 *   @example
 *     debug "Setting a packet_in handler: %s", method
 *
 *   @return [String] the string resulting from applying format to any
 *     additional arguments.
 */
static VALUE
default_logger_debug( int argc, VALUE *argv, VALUE self ) {
  UNUSED( self );
  return( do_log( debug, argc, argv ) );
}


void
Init_default_logger() {
  mTrema = rb_define_module( "Trema" );
  mDefaultLogger = rb_define_module_under( mTrema, "DefaultLogger" );

  rb_define_method( mDefaultLogger, "critical", default_logger_critical, -1 );
  rb_define_method( mDefaultLogger, "error", default_logger_error, -1 );
  rb_define_method( mDefaultLogger, "warn", default_logger_warn, -1 );
  rb_define_method( mDefaultLogger, "notice", default_logger_notice, -1 );
  rb_define_method( mDefaultLogger, "info", default_logger_info, -1 );
  rb_define_method( mDefaultLogger, "debug", default_logger_debug, -1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
