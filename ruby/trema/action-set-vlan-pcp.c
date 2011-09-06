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
VALUE cActionSetVlanPcp;


/*
 * An action to modify the VLAN priority of a packet. Valid values are between
 * (0) lowest and (7) highest. Priority bits can be used to prioritize different
 * classes of traffic.
 *
 * @overload initialize(vlan_pcp)
 *
 * @param [Number] vlan_pcp
 *   the VLAN priority to set to.
 *
 * @raise [ArgumentError] if vlan_pcp argument is not supplied.
 *
 * @return [ActionSetVlanPcp]
 *   an object that encapsulates this action.
 */
static VALUE
action_set_vlan_pcp_init( VALUE self, VALUE vlan_pcp ) {
  rb_iv_set( self, "@vlan_pcp", vlan_pcp );
  return self;
}


/*
 * The VLAN priority value.
 *
 * @return [Number] the value of attribute vlan_pcp.
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
 * (see ActionEnqueue#to_s)
 */
static VALUE
action_set_vlan_pcp_to_s( VALUE self ) {
	char str[ 64 ];
  uint8_t vlan_pcp = ( uint8_t ) NUM2UINT( action_get_vlan_pcp( self ) );
	sprintf(str, "#<%s> vlan_pcp = %u", rb_obj_classname( self ), vlan_pcp );
	return rb_str_new2( str );
}


void
Init_action_set_vlan_pcp() {
  cActionSetVlanPcp = rb_define_class_under( mTrema, "ActionSetVlanPcp", rb_cObject );
  rb_define_method( cActionSetVlanPcp, "initialize", action_set_vlan_pcp_init, 1 );
  rb_define_method( cActionSetVlanPcp, "vlan_pcp", action_get_vlan_pcp, 0 );
  rb_define_method( cActionSetVlanPcp, "append", action_set_vlan_pcp_append, 1 );
  rb_define_method( cActionSetVlanPcp, "to_s", action_set_vlan_pcp_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
