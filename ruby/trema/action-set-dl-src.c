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
VALUE cActionSetDlSrc;


/*
 * An action to modify the source Ethernet address of a packet.
 *
 * @overload initialize(dl_src)
 *
 * @param [Mac] dl_src
 *   a source Ethernet address encapsulated as a {Mac} object.
 *
 * @raise [ArgumentError] if dl_src argument is not supplied.
 * @raise [ArgumentError] if dl_src argument is not a {Mac} object instance.
 *
 * @return [ActionSetDlSrc] self
 *   an object that encapsulates this action.
 */
static VALUE
action_set_dl_src_init( VALUE self, VALUE dl_src ) {
  if ( rb_obj_is_instance_of( dl_src, rb_eval_string( "Trema::Mac" ) ) == Qfalse ) {
    rb_raise( rb_eArgError, "dl src address should be a Mac object" );
    return self;
  }
  rb_iv_set( self, "@dl_src", dl_src );
  return self;
}


/*
 * A source Ethernet address encapsulated as a {Mac} object.
 *
 * @return [Mac] the value of attribute dl_src.
 */
static VALUE
action_get_dl_src( VALUE self ) {
  return rb_iv_get( self, "@dl_src" );
}


/*
 * Appends its action(set_dl_src) to the list of actions.
 *
 * @return [ActionSetDlSrc] self
 */
static VALUE
action_set_dl_src_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );

  uint8_t dl_src[ OFP_ETH_ALEN ];
  uint8_t *ptr;
  ptr = ( uint8_t* ) dl_addr_short( action_get_dl_src( self ), dl_src );
  append_action_set_dl_src( actions, ptr );
  return self;
}


/*
 * (see ActionEnqueue#to_s)
 */
static VALUE
action_set_dl_src_to_s( VALUE self ) {
  VALUE mac_obj = action_get_dl_src( self );
  
  VALUE dl_src_str = rb_funcall( mac_obj, rb_intern( "to_s" ), 0 );
  char str[ 64 ];
  sprintf( str, "#<%s> dl_src = %s", rb_obj_classname( self ), RSTRING_PTR( dl_src_str ) );
  return rb_str_new2( str );
}


void
Init_action_set_dl_src() {
  cActionSetDlSrc = rb_define_class_under( mTrema, "ActionSetDlSrc", rb_cObject );
  rb_define_method( cActionSetDlSrc, "initialize", action_set_dl_src_init, 1 );
  rb_define_method( cActionSetDlSrc, "dl_src", action_get_dl_src, 0 );
  rb_define_method( cActionSetDlSrc, "append", action_set_dl_src_append, 1 );
  rb_define_method( cActionSetDlSrc, "to_s", action_set_dl_src_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
