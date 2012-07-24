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
VALUE cActionSetDlSrc;


/*
 * An action to modify the source Ethernet address of a packet.
 *
 * @overload initialize(mac_address)
 *
 *   @example
 *     ActionSetDlSrc.new("11:22:33:44:55:66")
 *     ActionSetDlSrc.new(0x112233445566)
 *
 *   @param [String,Integer] mac_address
 *     the Ethernet address to create this action with.
 *
 *   @raise [ArgumentError] if invalid format is detected.
 *   @raise [TypeError] if supplied argument is not a String or Integer.
 *
 *   @return [ActionSetDlSrc]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_dl_src_init( VALUE self, VALUE mac_address ) {
  VALUE mac = rb_funcall( rb_path2class( "Trema::Mac" ), rb_intern( "new" ), 1, mac_address );
  rb_iv_set( self, "@value", mac );
  return self;
}


/*
 * @private
 */
static VALUE
action_set_dl_src_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );

  uint8_t dl_src[ OFP_ETH_ALEN ];
  uint8_t *ptr;
  ptr = ( uint8_t* ) dl_addr_to_a( rb_iv_get( self, "@value" ), dl_src );
  append_action_set_dl_src( actions, ptr );
  return self;
}


void
Init_action_set_dl_src() {
  rb_require( "trema/action" );
  rb_require( "trema/mac" );
  VALUE cAction = action_base_class();
  cActionSetDlSrc = rb_define_class_under( mTrema, "ActionSetDlSrc", cAction );
  rb_define_method( cActionSetDlSrc, "initialize", action_set_dl_src_init, 1 );
  rb_define_method( cActionSetDlSrc, "append", action_set_dl_src_append, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
