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
VALUE cActionSetNwDst;


/*
 * An action to modify the IPv4 destination address of a packet.
 *
 * @overload initialize(nw_dst)
 *
 * @param [IP] nw_dst
 *   a destination IPv4 address encapsulated as an {IP} object.
 *
 * @raise [ArgumentError] if nw_dst argument is not supplied.
 * @raise [ArgumentError] if nw_dst argument is not an {IP} object instance.
 *
 * @return [ActionSetNwDst]
 *   an object that encapsulates this action.
 */
static VALUE
action_set_nw_dst_init( VALUE self, VALUE nw_dst ) {
  if ( rb_obj_is_instance_of( nw_dst, rb_eval_string( "Trema::IP" ) ) == Qfalse ) {
    rb_raise( rb_eArgError, "nw dst address should be an IP object" );
    return self;
  }
  rb_iv_set( self, "@nw_dst", nw_dst );
  return self;
}


/*
 * The destination IPv4 address as an {IP} object.
 *
 * @return [IP] the value of attribute nw_dst.
 */
static VALUE
action_get_nw_dst( VALUE self ) {
  return rb_iv_get( self, "@nw_dst" );
}


/*
 * Appends its action(nw_dst) to the list of actions.
 *
 * @return [ActionSetNwDst] self
 */
static VALUE
action_set_nw_dst_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_set_nw_dst( actions, nw_addr_to_i( action_get_nw_dst( self ) ) );
  return self;
}


/*
 * (see ActionEnqueue#to_s)
 */
static VALUE
action_set_nw_dst_to_s( VALUE self ) {
  char str[ 64 ];
  sprintf( str, "#<%s> nw_dst = %s", rb_obj_classname( self ), RSTRING_PTR( nw_addr_to_s( action_get_nw_dst( self ) ) ) );
  return rb_str_new2( str );
}


/*
 * The numeric representation of IPv4 destination address.
 *
 * @return [Number] the value of IPv4 destination address converted to an integer.
 */
static VALUE
action_set_nw_dst_to_i( VALUE self ) {
  return rb_funcall( action_get_nw_dst( self ), rb_intern( "to_i" ), 0 );
}


void
Init_action_set_nw_dst() {
	rb_require( "trema/ip" );
  cActionSetNwDst = rb_define_class_under( mTrema, "ActionSetNwDst", rb_cObject );
  rb_define_method( cActionSetNwDst, "initialize", action_set_nw_dst_init, 1 );
  rb_define_method( cActionSetNwDst, "nw_dst", action_get_nw_dst, 0 );
  rb_define_method( cActionSetNwDst, "append", action_set_nw_dst_append, 1 );
  rb_define_method( cActionSetNwDst, "to_s", action_set_nw_dst_to_s, 0 );
  rb_define_method( cActionSetNwDst, "to_i", action_set_nw_dst_to_i, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
