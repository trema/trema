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
VALUE cActionVendor;


/*
 * An action to set vendor specific extensions. 
 * 
 * @overload initialize(vendor_id)
 * 
 * @param [Number] vendor
 *   the vendor id this action refers to.
 * 
 * @return [ActionVendor]
 *   An object that encapsulates this action.
 */
static VALUE
action_vendor_init( VALUE self, VALUE vendor ) {
  rb_iv_set( self, "@vendor", vendor );
  return self;
}

/*
 * The vendor id of this action.
 * 
 * @return [Number] the value of attribute vendor.
 */
static VALUE
action_get_vendor( VALUE self ) {
  return rb_iv_get( self, "@vendor" );
}


/*
 * Appends its action(vendor) to the list of actions.
 * 
 * @return [ActionVendor] self
 */
static VALUE
action_vendor_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  uint32_t vendor = ( uint32_t ) NUM2UINT( action_get_vendor( self ) );

  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_vendor( actions, vendor, NULL );

  return self;
}


/*
 * (see ActionEnqueue#to_s)
 */
static VALUE
action_vendor_to_s( VALUE self ) {
  char str[ 64 ];
  uint32_t vendor = ( uint32_t ) NUM2UINT( action_get_vendor( self ) );

  sprintf( str, "#<%s> vendor = %u", rb_obj_classname( self ), vendor );
  return rb_str_new2( str );
}


void
Init_action_vendor( ) {
  cActionVendor = rb_define_class_under( mTrema, "ActionVendor", rb_cObject );
  rb_define_method( cActionVendor, "initialize", action_vendor_init, 1 );
  rb_define_method( cActionVendor, "vendor", action_get_vendor, 0 );
  rb_define_method( cActionVendor, "append", action_vendor_append, 1 );
  rb_define_method( cActionVendor, "to_s", action_vendor_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
