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
#include "action-common.h"


extern VALUE mTrema;
VALUE cActionSetDlDst;


/*
 * An action to modify the destination Ethernet address of a packet.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionSetDlDst.new( :dl_dst => Mac.new( "11:22:33:44:55:66" )
 *     ActionSetDlDst.new( :dl_dst => Mac.new( 0x112233445566 )
 *
 *   @param [Hash] options
 *     the options hash to create this action class instance with.
 *
 *   @option options [Mac] :dl_dst
 *     a destination Ethernet address encapsulated as a {Mac} object.
 *
 *   @raise [ArgumentError] if dl_dst argument is not supplied.
 *   @raise [TypeError] if dl_dst argument is not a {Mac} object instance.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionSetDlDst] self
 *     an object that encapsulates this action.
 */
static VALUE
action_set_dl_dst_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "10", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE dl_dst;
    if ( ( dl_dst = rb_hash_aref( options, ID2SYM( rb_intern( "dl_dst" ) ) ) ) != Qnil ) {
      if ( rb_obj_is_instance_of( dl_dst, rb_eval_string( "Trema::Mac" ) ) == Qfalse ) {
        rb_raise( rb_eTypeError, "dl dst address should be a Mac object" );
      }
      rb_iv_set( self, "@dl_dst", dl_dst );
    }
    else {
      rb_raise( rb_eArgError, "dl dst address is a mandatory option" );
    }
  }
  return self;
}


/*
 * A destination Ethernet address encapsulated as a {Mac} object.
 *
 * @return [Mac] the value of dl_dst.
 */
static VALUE
action_get_dl_dst( VALUE self ) {
  return rb_iv_get( self, "@dl_dst" );
}


/*
 * Appends its action(set_dl_dst) to the list of actions.
 *
 * @return [ActionSetDlDst] self
 */
static VALUE
action_set_dl_dst_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  
  uint8_t dl_dst[ OFP_ETH_ALEN ];
  uint8_t *ptr;
  ptr = ( uint8_t* ) dl_addr_short( action_get_dl_dst( self ), dl_dst );
  append_action_set_dl_dst( actions, ptr );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_set_dl_dst_inspect( VALUE self ) {
  VALUE mac_obj = action_get_dl_dst( self );
  VALUE dl_dst_str = rb_funcall( mac_obj, rb_intern( "to_s" ), 0 );
  
  char str[ 64 ];
  sprintf( str, "#<%s dl_dst=%s>", rb_obj_classname( self ), RSTRING_PTR( dl_dst_str ) );
  return rb_str_new2( str );
}


void
Init_action_set_dl_dst() {
  cActionSetDlDst = rb_define_class_under( mTrema, "ActionSetDlDst", rb_cObject );
  rb_define_method( cActionSetDlDst, "initialize", action_set_dl_dst_init, -1 );
  rb_define_method( cActionSetDlDst, "dl_dst", action_get_dl_dst, 0 );
  rb_define_method( cActionSetDlDst, "append", action_set_dl_dst_append, 1 );
  rb_define_method( cActionSetDlDst, "inspect", action_set_dl_dst_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
