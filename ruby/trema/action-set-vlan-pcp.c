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
VALUE cActionSetVlanPcp;



/*
 * An action to modify the VLAN priority of a packet. Valid values are between
 * (0) lowest and (7) highest. Priority bits can be used to prioritize different
 * classes of traffic.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionSetVlanPcp.new( :vlan_pcp => 7 )
 *
 *   @param [Hash] options
 *     the options to create this action class instance with.
 *
 *   @option options [Number] :vlan_pcp
 *     the VLAN priority to set to.
 *
 *   @raise [ArgumentError] if vlan_pcp argument is not supplied.
 *   @raise [RangeError] if vlan_pcp is not within 0 and 7 inclusive.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionSetVlanPcp]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_vlan_pcp_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "10", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE vlan_pcp;
    if ( ( vlan_pcp = rb_hash_aref( options, ID2SYM( rb_intern( "vlan_pcp" ) ) ) ) != Qnil ) {
      uint8_t vpcp = ( uint8_t ) NUM2UINT( vlan_pcp );
      if ( vpcp & ~7 ) {
        rb_raise( rb_eRangeError, "Valid VLAN priority values are 0 to 7 inclusive" );
      }
      rb_iv_set( self, "@vlan_pcp", vlan_pcp );
    }
    else {
      rb_raise( rb_eArgError, "VLAN priority is a mandatory option" );
    }
  }
  return self;
}


/*
 * The VLAN priority value.
 *
 * @return [Number] the value of vlan_pcp.
 */
static VALUE
action_get_vlan_pcp( VALUE self ) {
  return rb_iv_get( self, "@vlan_pcp" );
}


/*
 * Appends its action(vlan_pcp) to the list of actions.
 *
 * @return [ActionSetVlanPcp] self
 */
static VALUE
action_set_vlan_pcp_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  uint8_t vlan_pcp = ( uint8_t ) NUM2UINT( action_get_vlan_pcp( self ) );
  append_action_set_vlan_pcp( actions, vlan_pcp );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_set_vlan_pcp_inspect( VALUE self ) {
  char str[ 64 ];
  uint8_t vlan_pcp = ( uint8_t ) NUM2UINT( action_get_vlan_pcp( self ) );
  sprintf(str, "#<%s vlan_pcp=%u>", rb_obj_classname( self ), vlan_pcp );
  return rb_str_new2( str );
}


void
Init_action_set_vlan_pcp() {
  cActionSetVlanPcp = rb_define_class_under( mTrema, "ActionSetVlanPcp", rb_cObject );
  rb_define_method( cActionSetVlanPcp, "initialize", action_set_vlan_pcp_init, -1 );
  rb_define_method( cActionSetVlanPcp, "vlan_pcp", action_get_vlan_pcp, 0 );
  rb_define_method( cActionSetVlanPcp, "append", action_set_vlan_pcp_append, 1 );
  rb_define_method( cActionSetVlanPcp, "inspect", action_set_vlan_pcp_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
