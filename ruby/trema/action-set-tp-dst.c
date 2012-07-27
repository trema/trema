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
VALUE cActionSetTpDst;


/*
 * An action to modify the destination TCP or UDP port of a packet.
 *
 * @overload initialize(tp_dst)
 *
 *   @example
 *     ActionSetTpDst.new( 5555 )
 *
 *   @param [Number] :tp_dst
 *     the destination TCP or UDP port number. Any numeric 16-bit value.
 *
 *   @raise [ArgumentError] if tp_dst argument is not supplied.
 *   @raise [ArgumentError] if tp_dst is not an unsigned 16-bit Integer.
 *   @raise [TypeError] if tp_dst is not an Integer.
 *
 *   @return [ActionSetTpDst]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_tp_dst_init( VALUE self, VALUE tp_dst ) {
  if ( !rb_obj_is_kind_of( tp_dst, rb_cInteger ) ) {
    rb_raise( rb_eTypeError, "Destination TCP or UDP port must be an unsigned 16-bit integer" );
  }
  if ( rb_funcall( tp_dst, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
    rb_raise( rb_eArgError, "Destination TCP or UDP port must be an unsigned 16-bit integer" );
  }
  rb_iv_set( self, "@value", tp_dst );
  return self;
}


void
Init_action_set_tp_dst() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionSetTpDst = rb_define_class_under( mTrema, "ActionSetTpDst", cAction );
  rb_define_method( cActionSetTpDst, "initialize", action_set_tp_dst_init, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
