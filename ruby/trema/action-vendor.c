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
VALUE cActionVendor;


/*
 * An action to set vendor specific extensions. 
 *
 * @overload initialize(vendor)
 *
 *   @example
 *     ActionVendor.new( 0x00004cff )
 *
 *   @param [Integer] vendor
 *     the vendor id this action refers to.
 *
 *   @raise [ArgumentError] if vendor argument is not supplied.
 *   @raise [ArgumentError] if vendor is not an unsigned 32-bit Integer.
 *   @raise [TypeError] if vendor id is not an Integer.
 *
 *   @return [ActionVendor] 
 *     an object that encapsulates this action.
 */
static VALUE
action_vendor_init( VALUE self, VALUE vendor ) {
  if ( !rb_obj_is_kind_of( vendor, rb_cInteger ) ) {
    rb_raise( rb_eTypeError, "Vendor id must be an unsigned 32-bit integer" );
  }
  if ( rb_funcall( vendor, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
    rb_raise( rb_eArgError, "Vendor id must be an unsigned 32-bit integer" );
  }
  rb_iv_set( self, "@value", vendor );
  return self;
}


/*
 * @private
 */
static VALUE
action_vendor_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_vendor( actions, ( uint32_t ) NUM2UINT( rb_iv_get( self, "@value" ) ), NULL );
  return self;
}


void
Init_action_vendor() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionVendor = rb_define_class_under( mTrema, "ActionVendor", cAction );
  rb_define_method( cActionVendor, "initialize", action_vendor_init, 1 );
  rb_define_method( cActionVendor, "append", action_vendor_append, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
