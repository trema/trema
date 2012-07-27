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
VALUE cActionSetNwTos;


/*
 * An action to modify the IP ToS/DSCP field of a packet.
 *
 * @overload initialize(nw_tos)
 *
 *   @example
 *     ActionSetNwTos.new(32)
 *
 *   @param [Integer] :nw_tos
 *     the ToS/DSCP field to set to.
 *
 *   @raise [ArgumentError]
 *     if nw_tos argument is not supplied or is not an unsigned 8-bit Integer.
 *   @raise [TypeError] if supplied argument is not an Integer.
 *
 *   @return [ActionSetNwTos]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_nw_tos_init( VALUE self, VALUE nw_tos ) {
  if ( !rb_obj_is_kind_of( nw_tos, rb_cInteger ) ) {
    rb_raise( rb_eTypeError, "Nw tos must be an unsigned 8-bit integer" );
  }
  if ( rb_funcall( nw_tos, rb_intern( "unsigned_8bit?" ), 0 ) == Qfalse ) {
    rb_raise( rb_eArgError, "Nw tos must be an unsigned 8-bit integer" );
  }
  rb_iv_set( self, "@value", nw_tos );
  return self;
}


void
Init_action_set_nw_tos() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionSetNwTos = rb_define_class_under( mTrema, "ActionSetNwTos", cAction );
  rb_define_method( cActionSetNwTos, "initialize", action_set_nw_tos_init, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
