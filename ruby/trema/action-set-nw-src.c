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
#include "action-common.h"


extern VALUE mTrema;
VALUE cActionSetNwSrc;


/*
 * An action to modify the IPv4 source address of a packet.
 *
 * @overload initialize(nw_src)
 *
 * @param [IP] nw_src
 *   the source IPv4 address encapsulated as an {IP} object.
 *
 * @raise [ArgumentError] if nw_src argument is not supplied.
 * @raise [ArgumentError] if nw_src argument is not an {IP} object instance.
 *
 * @return [ActionSetNwSrc]
 *   an object that encapsulates this action.
 */
static VALUE
action_set_nw_src_init( VALUE self, VALUE nw_src ) {
  if ( rb_obj_is_instance_of( nw_src, rb_eval_string( "Trema::IP" ) ) == Qfalse ) {
    rb_raise( rb_eArgError, "nw src address should be an IP object" );
    return self;
  }
  rb_iv_set( self, "@nw_src", nw_src );
  return self;
}


/*
 * The source IPv4 address as an {IP} object.
 *
 * @return [IP] the value of attribute nw_src.
 */
static VALUE
action_get_nw_src( VALUE self ) {
  return rb_iv_get( self, "@nw_src" );
}


/*
 * Appends its action(set_nw_src) to the list of actions.
 *
 * @return [ActionSetNwSrc] self
 */
static VALUE
action_set_nw_src_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;

  Data_Get_Struct( action_ptr, openflow_actions, actions );

  append_action_set_nw_src( actions, nw_addr_to_i( action_get_nw_src( self ) ) );

  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_set_nw_src_inspect( VALUE self ) {
  char str[ 64 ];

  sprintf( str, "#<%s nw_src=%s>", rb_obj_classname( self ), RSTRING_PTR( nw_addr_to_s( action_get_nw_src( self ) ) ) );
  return rb_str_new2( str );
}


/*
 * The numeric representation of IPv4 source address.
 *
 * @return [Number] the value of IPv4 source address converted to an integer.
 */
static VALUE
action_set_nw_src_to_i( VALUE self ) {
  return rb_funcall( action_get_nw_src( self ), rb_intern( "to_i" ), 0 );
}


void
Init_action_set_nw_src() {
  rb_require( "ipaddr" );
  cActionSetNwSrc = rb_define_class_under( mTrema, "ActionSetNwSrc", rb_cObject );
  rb_define_method( cActionSetNwSrc, "initialize", action_set_nw_src_init, 1 );
  rb_define_method( cActionSetNwSrc, "nw_src", action_get_nw_src, 0 );
  rb_define_method( cActionSetNwSrc, "append", action_set_nw_src_append, 1 );
  rb_define_method( cActionSetNwSrc, "inspect", action_set_nw_src_inspect, 0 );
  rb_define_method( cActionSetNwSrc, "to_i", action_set_nw_src_to_i, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
