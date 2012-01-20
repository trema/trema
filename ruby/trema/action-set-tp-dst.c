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
VALUE cActionSetTpDst;


/*
 * An action to modify the destination TCP or UDP port of a packet.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionSetTpDst.new( :tp_dst => 5555 )
 *
 *   @param [Hash] options
 *     the options to create this action class instance with.
 *   @option options [Number] :tp_dst
 *     the destination TCP or UDP port number. Any numeric 16-bit value.
 *
 *   @raise [ArgumentError] if tp_dst argument is not supplied.
 *   @raise [ArgumentError] if tp_dst is not an unsigned 16-bit integer.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionSetTpDst]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_tp_dst_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "10", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE tp_dst;
    if ( ( tp_dst = rb_hash_aref( options, ID2SYM( rb_intern( "tp_dst" ) ) ) ) != Qnil ) {
      if ( rb_funcall( tp_dst, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Destination TCP or UDP port must be an unsigned 16-bit integer" );
      }
      rb_iv_set( self, "@tp_dst", tp_dst );
    }
    else {
      rb_raise( rb_eArgError, "Destination TCP or UDP port must be a mandatory option" );
    }
  }
  return self;
}


/*
 * The destination TCP or UDP port number.
 *
 * @return [Number] the value of tp_dst.
 */
static VALUE
action_get_tp_dst( VALUE self ) {
  return rb_iv_get( self, "@tp_dst" );
}


/*
 * Appends its actions(tp_dst) to the list of actions.
 *
 * @return [ActionSetTpDst] self
 */
static VALUE
action_set_tp_dst_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_set_tp_dst( actions, ( uint16_t ) NUM2UINT( action_get_tp_dst( self ) ) );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_set_tp_dst_inspect( VALUE self ) {
  char str[ 64 ];
  sprintf( str, "#<%s tp_port=%u>", rb_obj_classname( self ),
    ( uint16_t ) NUM2UINT( action_get_tp_dst( self ) ) );
  return rb_str_new2( str );
}


void
Init_action_set_tp_dst() {
  cActionSetTpDst = rb_define_class_under( mTrema, "ActionSetTpDst", rb_cObject );
  rb_define_method( cActionSetTpDst, "initialize", action_set_tp_dst_init, -1 );
  rb_define_method( cActionSetTpDst, "tp_dst", action_get_tp_dst, 0 );
  rb_define_method( cActionSetTpDst, "append", action_set_tp_dst_append, 1 );
  rb_define_method( cActionSetTpDst, "inspect", action_set_tp_dst_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
