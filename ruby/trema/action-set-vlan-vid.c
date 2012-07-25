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
VALUE cActionSetVlanVid;


/*
 * An action to modify the VLAN id of a packet. The VLAN id is 16-bits long but 
 * the actual VID(VLAN Identifier) of the IEEE 802.1Q frame is 12-bits.
 *
 * @overload initialize(vlan_vid)
 *
 *   @example
 *     ActionSetVlanVid.new( vlan_vid )
 *
 *   @param [Integer] vlan_vid
 *     the VLAN id to set to. Only the lower 12-bits are used.
 *
 *   @raise [ArgumentError] if vlan_vid argument is not supplied.
 *   @raise [TypeError] if vlan_vid is not an Integer.
 *   @raise [RangeError] if vlan_vid not within 1 and 4096 inclusive.
 *
 *   @return [ActionSetVlanVid]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_vlan_vid_init( VALUE self, VALUE vlan_vid ) {
  if ( !rb_obj_is_kind_of( vlan_vid, rb_cInteger ) ) {
    rb_raise( rb_eTypeError, "VLAN id argument must be an Integer" );
  }
  uint16_t vvid = ( uint16_t ) NUM2UINT( vlan_vid );
  if ( !vvid || vvid > 4096 ||
    ( rb_funcall( vlan_vid, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) ) {
    rb_raise( rb_eRangeError, "Valid VLAN id values between 1 to 4096 inclusive" );
  }
  rb_iv_set( self, "@value", vlan_vid );
  return self;
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
  append_action_set_vlan_vid( actions, ( uint16_t ) NUM2UINT( rb_iv_get( self, "@value" ) ) );
  return self;
}


void
Init_action_set_vlan_vid() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionSetVlanVid = rb_define_class_under( mTrema, "ActionSetVlanVid", cAction );
  rb_define_method( cActionSetVlanVid, "initialize", action_set_vlan_vid_init, 1 );
  rb_define_method( cActionSetVlanVid, "append", action_set_vlan_vid_append, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
