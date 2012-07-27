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
VALUE cActionSetDlDst;


/*
 * An action to modify the destination Ethernet address of a packet.
 *
 * @overload initialize(mac_address)
 *
 *   @example
 *     ActionSetDlDst.new("11:22:33:44:55:66")
 *     ActionSetDlDst.new(0x112233445566)
 *
 *   @param [String,Integer] mac_address
 *     the Ethernet address to create this action with.
 *
 *   @raise [ArgumentError] if invalid format is detected.
 *   @raise [TypeError] if supplied argument is not a String or Integer.
 *
 *   @return [ActionSetDlDst]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_dl_dst_init( VALUE self, VALUE mac_address ) {
  VALUE mac = rb_funcall( rb_path2class( "Trema::Mac" ), rb_intern( "new" ), 1, mac_address );
  rb_iv_set( self, "@value", mac );
  return self;
}


void
Init_action_set_dl_dst() {
  rb_require( "trema/action" );
  rb_require( "trema/mac" );
  VALUE cAction = action_base_class();
  cActionSetDlDst = rb_define_class_under( mTrema, "ActionSetDlDst", cAction );
  rb_define_method( cActionSetDlDst, "initialize", action_set_dl_dst_init, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
