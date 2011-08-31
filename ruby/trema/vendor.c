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
VALUE cVendor;


static VALUE
vendor_init( VALUE self, VALUE attribute ) {
  rb_iv_set( self, "@attribute", attribute );
  return self;
}


static VALUE
vendor_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


static VALUE
vendor_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


static VALUE
vendor_vendor( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "vendor" ) ) );
}


static VALUE
vendor_buffer( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "buffer" ) ) );
}


void
Init_vendor( ) {
  cVendor = rb_define_class_under( mTrema, "Vendor", rb_cObject );
  rb_define_method( cVendor, "initialize", vendor_init, 1 );
  rb_define_method( cVendor, "datapath_id", vendor_datapath_id, 0 );
  rb_define_method( cVendor, "transaction_id", vendor_transaction_id, 0 );
  rb_define_method( cVendor, "vendor", vendor_vendor, 0 );
  rb_define_method( cVendor, "buffer", vendor_buffer, 0 );
}


void
handle_vendor(
        uint64_t datapath_id,
        uint32_t transaction_id,
        uint32_t vendor,
        buffer *body,
        void *user_data
        ) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "vendor" ) ) == Qfalse ) {
    return;
  }
  VALUE attributes = rb_hash_new();

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "vendor" ) ), UINT2NUM( vendor ) );

  if ( body->length ) {
    rb_hash_aset( attributes, ID2SYM( rb_intern( "buffer" ) ), Data_Wrap_Struct( cVendor, NULL, free_buffer, body ) );
  }
  VALUE vendor_r = rb_funcall( cVendor, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "vendor" ), 1, vendor_r );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
