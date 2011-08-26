/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cActionSetTpSrc;


/*
 * An action to modify the source TCP or UDP port of a packet.
 * 
 * @overload initialize(tp_src)
 * 
 * @param [Number] tp_src
 *   the source TCP or UDP port number. Any numeric 16-bit value.
 * 
 * @raise [ArgumentError] if tp_src argument is not supplied.
 * 
 * @return [ActionSetTpSrc]
 *   an object that encapsulates this action.
 */
static VALUE
action_set_tp_src_init( VALUE self, VALUE tp_src ) {
  rb_iv_set( self, "@tp_src", tp_src );
  return self;
}


/*
 * The source TCP or UDP port number.
 * 
 * @return [Number] the value of attribute tp_src.
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
  uint16_t tp_src = ( uint16_t ) NUM2UINT( action_get_tp_src( self ) );

  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_set_tp_src( actions, tp_src );

  return self;
}


/*
 * (see ActionEnqueue#to_s)
 */
static VALUE
action_set_tp_src_to_s( VALUE self ) {
  char str[ 64 ];
  uint16_t tp_src = ( uint16_t ) NUM2UINT( action_get_tp_src( self ) );

  sprintf( str, "#<%s> tp_port = %u", rb_obj_classname( self ), tp_src );
  return rb_str_new2( str );
}


void
Init_action_set_tp_src( ) {
  cActionSetTpSrc = rb_define_class_under( mTrema, "ActionSetTpSrc", rb_cObject );
  rb_define_method( cActionSetTpSrc, "initialize", action_set_tp_src_init, 1 );
  rb_define_method( cActionSetTpSrc, "tp_src", action_get_tp_src, 0 );
  rb_define_method( cActionSetTpSrc, "append", action_set_tp_src_append, 1 );
  rb_define_method( cActionSetTpSrc, "to_s", action_set_tp_src_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
