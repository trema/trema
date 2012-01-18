/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cActionSetTpSrc;


/*
 * An action to modify the source TCP or UDP port of a packet.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionSetTpSrc.new( :tp_src => 5555 )
 *
 *   @param [Hash] options
 *     the options to create this action class instance with.
 *
 *   @option options [Number] :tp_src
 *     the source TCP or UDP port number. Any numeric 16-bit value.
 *
 *   @raise [ArgumentError] if tp_src argument is not supplied.
 *   @raise [ArgumentError] if tp_src is not an unsigned 16-bit integer.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionSetTpSrc]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_tp_src_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "10", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE tp_src;
    if ( ( tp_src = rb_hash_aref( options, ID2SYM( rb_intern( "tp_src" ) ) ) ) != Qnil ) {
      if ( rb_funcall( tp_src, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Source TCP or UDP port must be an unsigned 16-bit integer" );
      }
      rb_iv_set( self, "@tp_src", tp_src );
    }
    else {
      rb_raise( rb_eArgError, "Source TCP or UDP port is a mandatory option" );
    }
  }
  return self;
}


/*
 * The source TCP or UDP port number.
 *
 * @return [Number] the value of tp_src.
 */
static VALUE
action_get_tp_src( VALUE self ) {
  return rb_iv_get( self, "@tp_src" );
}


/*
 * Appends its action(tp_src) to the list of actions.
 *
 * @return [ActionSetTpSrc] self
 */
static VALUE
action_set_tp_src_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  uint16_t tp_src = ( uint16_t ) NUM2UINT( action_get_tp_src( self ) );
  append_action_set_tp_src( actions, tp_src );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_set_tp_src_inspect( VALUE self ) {
  char str[ 64 ];
  uint16_t tp_src = ( uint16_t ) NUM2UINT( action_get_tp_src( self ) );
  sprintf( str, "#<%s tp_port=%u>", rb_obj_classname( self ), tp_src );
  return rb_str_new2( str );
}


void
Init_action_set_tp_src() {
  cActionSetTpSrc = rb_define_class_under( mTrema, "ActionSetTpSrc", rb_cObject );
  rb_define_method( cActionSetTpSrc, "initialize", action_set_tp_src_init, -1 );
  rb_define_method( cActionSetTpSrc, "tp_src", action_get_tp_src, 0 );
  rb_define_method( cActionSetTpSrc, "append", action_set_tp_src_append, 1 );
  rb_define_method( cActionSetTpSrc, "inspect", action_set_tp_src_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
