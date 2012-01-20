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


extern VALUE mTrema;
VALUE cActionVendor;


/*
 * An action to set vendor specific extensions. 
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionVendor.new( :vendor => VENDOR_ID )
 *   @param [Hash] options
 *     the options hash to create this action class instance with.
 *
 *   @option options [Number] vendor
 *     the vendor id this action refers to.
 *
 *   @raise [ArgumentError] if vendor argument is not supplied.
 *   @raise [ArgumentError] if vendor is not an unsigned 32-bit integer.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionVendor] 
 *     an object that encapsulates this action.
 */
static VALUE
action_vendor_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "10", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE vendor;
    if ( ( vendor = rb_hash_aref( options, ID2SYM( rb_intern( "vendor" ) ) ) ) != Qnil ) {
      if ( rb_funcall( vendor, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Vendor id must be an unsigned 32-bit integer" );
      }
      rb_iv_set( self, "@vendor", vendor );
    }
  }
  return self;
}


/*
 * The vendor id of this action.
 *
 * @return [Number] the value of vendor.
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
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_vendor_inspect( VALUE self ) {
  uint32_t vendor = ( uint32_t ) NUM2UINT( action_get_vendor( self ) );
  char str[ 64 ];
  sprintf( str, "#<%s vendor=%u>", rb_obj_classname( self ), vendor );
  return rb_str_new2( str );
}


void
Init_action_vendor() {
  cActionVendor = rb_define_class_under( mTrema, "ActionVendor", rb_cObject );
  rb_define_method( cActionVendor, "initialize", action_vendor_init, -1 );
  rb_define_method( cActionVendor, "vendor", action_get_vendor, 0 );
  rb_define_method( cActionVendor, "append", action_vendor_append, 1 );
  rb_define_method( cActionVendor, "inspect", action_vendor_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
