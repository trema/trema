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
VALUE cActionSetTpSrc;


/*
 * An action to modify the source TCP or UDP port of a packet.
 *
 * @overload initialize(tp_src)
 *
 *   @example
 *     ActionSetTpSrc.new( 5555 )
 *
 *   @param [Integer] :tp_src
 *     the source TCP or UDP port number. Any numeric 16-bit value.
 *
 *   @raise [ArgumentError] if tp_src argument is not supplied.
 *   @raise [ArgumentError] if tp_src is not an unsigned 16-bit Integer.
 *   @raise [TypeError] if tp_src is not an Integer.
 *
 *   @return [ActionSetTpSrc]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_tp_src_init( VALUE self, VALUE tp_src ) {
  if ( !rb_obj_is_kind_of( tp_src, rb_cInteger ) ) {
    rb_raise( rb_eTypeError, "Source TCP or UDP port must be an unsigned 16-bit integer" );
  }
  if ( rb_funcall( tp_src, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
    rb_raise( rb_eArgError, "Source TCP or UDP port must be an unsigned 16-bit integer" );
  }
  rb_iv_set( self, "@value", tp_src );
  return self;
}


void
Init_action_set_tp_src() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionSetTpSrc = rb_define_class_under( mTrema, "ActionSetTpSrc", cAction );
  rb_define_method( cActionSetTpSrc, "initialize", action_set_tp_src_init, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
