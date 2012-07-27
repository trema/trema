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
VALUE cActionSetNwSrc;


/*
 * An action to modify the IPv4 source address of a packet.
 *
 * @overload initialize(ip_address)
 *
 *   @example
 *     ActionSetNwSrc.new("192.168.1.1")
 *
 *   @param [String] ip_address
 *     the IPv4 address to create this action with.
 *
 *   @return [ActionSetNwSrc]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_nw_src_init( VALUE self, VALUE ip_address ) {
  VALUE ip = rb_funcall( rb_path2class( "IPAddr" ), rb_intern( "new" ), 1, ip_address );
  rb_iv_set( self, "@value", ip );
  return self;
}


void
Init_action_set_nw_src() {
  rb_require( "ipaddr" );
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionSetNwSrc = rb_define_class_under( mTrema, "ActionSetNwSrc", cAction );
  rb_define_method( cActionSetNwSrc, "initialize", action_set_nw_src_init, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
