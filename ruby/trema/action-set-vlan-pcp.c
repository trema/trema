/*
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
VALUE cActionSetVlanPcp;


/*
 * An action to modify the VLAN priority of a packet. Valid values are between
 * (0) lowest and (7) highest. Priority bits can be used to prioritize different
 * classes of traffic.
 *
 * @overload initialize(vlan_pcp)
 *
 *   @example
 *     ActionSetVlanPcp.new( 7 )
 *
 *   @param [Integer] :vlan_pcp
 *     the VLAN priority to set to.
 *
 *   @raise [ArgumentError] if vlan_pcp argument is not supplied.
 *   @raise [RangeError] if vlan_pcp is not within 0 and 7 inclusive.
 *   @raise [TypeError] if vlan_pcp is not an Integer.
 *
 *   @return [ActionSetVlanPcp]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_vlan_pcp_init( VALUE self, VALUE vlan_pcp ) {
  if ( !rb_obj_is_kind_of( vlan_pcp, rb_cInteger ) ) {
    rb_raise( rb_eTypeError, "VLAN priority must be an unsigned 8-bit Integer" );
  }
  uint8_t vpcp = ( uint8_t ) NUM2UINT( vlan_pcp );
  if ( vpcp & ~7 ) {
    rb_raise( rb_eRangeError, "Valid VLAN priority values are 0 to 7 inclusive" );
  }
  rb_iv_set( self, "@value", vlan_pcp );
  return self;
}


/*
 * @private
 */
static VALUE
action_set_vlan_pcp_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  uint8_t vlan_pcp = ( uint8_t ) NUM2UINT( rb_iv_get( self, "@value" ) );
  append_action_set_vlan_pcp( actions, vlan_pcp );
  return self;
}


void
Init_action_set_vlan_pcp() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionSetVlanPcp = rb_define_class_under( mTrema, "ActionSetVlanPcp", cAction );
  rb_define_method( cActionSetVlanPcp, "initialize", action_set_vlan_pcp_init, 1 );
  rb_define_method( cActionSetVlanPcp, "append", action_set_vlan_pcp_append, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
