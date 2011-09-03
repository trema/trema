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
VALUE cActionSetVlanVid;


/*
 * An action to modify the VLAN id of a packet. The VLAN id is 16-bits long but 
 * the actual VID(VLAN Identifier) of the IEEE 802.1Q frame is 12-bits.
 *
 * @overload initialize(vlan_vid)
 *
 * @param [Number] vlan_vid
 *   the VLAN id to set to. Only the lower 12-bits are used.
 *
 * @raise [ArgumentError] if vlan_vid argument is not supplied.
 *
 * @return [ActionSetVlanVid]
 *   an object that encapsulates this action.
 */
static VALUE
action_set_vlan_vid_init( VALUE self, VALUE vlan_vid ) {
  rb_iv_set( self, "@vlan_vid", vlan_vid );
  return self;
}


/*
 * The VLAN id value.
 *
 * @return [Number] the value of attribute vlan_vid.
 */
static VALUE
action_get_vlan_vid( VALUE self ) {
  return rb_iv_get( self, "@vlan_vid" );
}


/*
 * Appends its action(vlan_vid) to the list of actions.
 *
 * @return [ActionSetVlanVid] self
 */
static VALUE
action_set_vlan_vid_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_set_vlan_vid( actions, ( uint16_t ) NUM2UINT( action_get_vlan_vid( self ) ) );
  return self;
}


/*
 * (see ActionEnqueue#to_s)
 */
static VALUE
action_set_vlan_vid_to_s( VALUE self ) {
  char str[ 64 ];
  uint16_t vlan_vid = ( uint16_t ) NUM2UINT( action_get_vlan_vid( self ) );
  sprintf( str, "#<%s> vlan_vid = %u", rb_obj_classname( self ), vlan_vid );
  return rb_str_new2( str );
}


void
Init_action_set_vlan_vid() {
  cActionSetVlanVid = rb_define_class_under( mTrema, "ActionSetVlanVid", rb_cObject );
  rb_define_method( cActionSetVlanVid, "initialize", action_set_vlan_vid_init, 1 );
  rb_define_method( cActionSetVlanVid, "vlan_vid", action_get_vlan_vid, 0 );
  rb_define_method( cActionSetVlanVid, "append", action_set_vlan_vid_append, 1 );
  rb_define_method( cActionSetVlanVid, "to_s", action_set_vlan_vid_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
