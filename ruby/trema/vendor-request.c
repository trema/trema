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


#define VENDOR_ID 0xccddeeff


extern VALUE mTrema;
VALUE cVendorRequest;


static VALUE
vendor_request_alloc( VALUE klass ) {
  buffer *vendor_request, *data;

  data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );
  vendor_request = create_vendor( get_transaction_id( ), VENDOR_ID, data );

  return Data_Wrap_Struct( klass, NULL, free_buffer, vendor_request );
}


static VALUE
vendor_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *vendor_request;
  uint8_t *buf;
  VALUE xid_r, vendor_r, data_r;
  uint32_t xid;
  uint32_t vendor;
  int32_t i;
  uint16_t data_length;

  Data_Get_Struct( self, buffer, vendor_request );
  data_length = ( uint16_t ) ( vendor_request->length - sizeof ( struct ofp_vendor_header ) );

  if ( rb_scan_args( argc, argv, "03", &xid_r, &vendor_r, &data_r ) == 3 ) {
    xid = NUM2UINT( xid_r );
    vendor = NUM2UINT( vendor_r );
    if ( TYPE( data_r ) == T_ARRAY ) {
      if ( data_length > RARRAY( data_r )->len ) {
        buf = ( uint8_t * ) ( ( char * ) vendor_request->data + sizeof ( struct ofp_vendor_header ) );
        memset( buf, 0, data_length );
        for ( i = 0; i < RARRAY( data_r )->len; i++ ) {
          buf[i] = ( uint8_t ) FIX2INT( RARRAY( data_r )->ptr[i] );
        }
      }
    }
  } else {
    xid = get_transaction_id( );
    vendor = VENDOR_ID;
  }
  ( ( struct ofp_header * ) ( vendor_request->data ) )->xid = htonl( xid );
  ( ( struct ofp_vendor_header * ) ( vendor_request->data ) )->vendor = htonl( vendor );
  return self;
}


static VALUE
vendor_request_transaction_id( VALUE self ) {
  buffer *vendor_request;
  uint32_t xid;

  Data_Get_Struct( self, buffer, vendor_request );
  xid = ntohl( ( ( struct ofp_header * ) ( vendor_request->data ) )->xid );
  return UINT2NUM( xid );
}


static VALUE
vendor_request_vendor( VALUE self ) {
  buffer *vendor_request;
  uint32_t vendor;

  Data_Get_Struct( self, buffer, vendor_request );
  vendor = ntohl( ( ( struct ofp_vendor_header * ) ( vendor_request->data ) )->vendor );
  return UINT2NUM( vendor );
}


static VALUE
vendor_request_data( VALUE self ) {
  VALUE data_arr;
  buffer *vendor_request;
  uint8_t *buf;
  uint32_t i;
  uint16_t data_length;

  Data_Get_Struct( self, buffer, vendor_request );
  data_length = ( uint16_t ) ( vendor_request->length - sizeof ( struct ofp_vendor_header ) );

  if ( data_length > 0 ) {
    data_arr = rb_ary_new2( data_length );
    buf = ( uint8_t * ) ( ( char * ) vendor_request->data + sizeof ( struct ofp_vendor_header ) );
    for ( i = 0; i < data_length; i++ ) {
      rb_ary_push( data_arr, INT2FIX( buf[i] ) );
    }
    return data_arr;
  }
  return Qnil;
}


void
Init_vendor_request( ) {
  cVendorRequest = rb_define_class_under( mTrema, "VendorRequest", rb_cObject );
  rb_define_alloc_func( cVendorRequest, vendor_request_alloc );
  rb_define_method( cVendorRequest, "initialize", vendor_request_init, -1 );
  rb_define_method( cVendorRequest, "transaction_id", vendor_request_transaction_id, 0 );
  rb_define_method( cVendorRequest, "vendor", vendor_request_vendor, 0 );
  rb_define_method( cVendorRequest, "data", vendor_request_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
