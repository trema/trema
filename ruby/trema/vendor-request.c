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
  buffer *data = alloc_buffer_with_length( 16 );
  append_back_buffer( data, 16 );
  memset( data->data, 'a', 16 );
  buffer *vendor_request = create_vendor( get_transaction_id( ), VENDOR_ID, data );
  free_buffer( data );

  return Data_Wrap_Struct( klass, NULL, free_buffer, vendor_request );
}


/*
 * Creates a {VendorRequest} instance to facilitate sending of vendor 
 * experimental messages.
 *
 * @overload initialize(transaction_id=nil, vendor_id = nil, vendor_data=nil)
 *
 * @example
 *   vendor_data = "test".unpack( "C*" ) => [ 116, 101, 115, 116 ]
 *   vendor = Vendor.new( 1234, 0x3000, vendor_data )
 *
 * @param [Number] transaction_id
 *   Auto-generated transaction_id if not specified.
 *
 * @param [Number] vendor_id
 *   The assigned vendor id defaults to 0xccddeeff if not specified.
 *
 * @param [Array] vendor_data
 *   Fixed 16 bytes of data if not specified. User can set upto 16 bytes of any
 *   vendor specific data.
 *
 * @raise [ArgumentError] if transaction id is negative.
 * @raise [ArgumentError] if user data is not an array of bytes.
 *
 * @return [VendorRequest] an object that encapsulates the +OFPT_VENDOR+ openFlow message.
 */
static VALUE
vendor_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *vendor_request;
  uint8_t *buf;
  VALUE xid_r;
  VALUE vendor_r;
  VALUE data_r;
  uint32_t xid;
  uint32_t vendor;
  int32_t i;

  Data_Get_Struct( self, buffer, vendor_request );
  uint16_t data_length = ( uint16_t ) ( vendor_request->length - sizeof( struct ofp_vendor_header ) );

  if ( rb_scan_args( argc, argv, "03", &xid_r, &vendor_r, &data_r ) == 3 ) {
    if ( NUM2INT( xid_r ) < 0 ) {
      rb_raise( rb_eArgError, "Transaction ID must be >= 0" );
    }
    xid = ( uint32_t ) NUM2UINT( xid_r );
    vendor = ( uint32_t ) NUM2UINT( vendor_r );
    if ( TYPE( data_r ) == T_ARRAY ) {
      buf = ( uint8_t * ) ( ( char * ) vendor_request->data + sizeof( struct ofp_vendor_header ) );
      memset( buf, 0, data_length );
      for ( i = 0; i < data_length && i < RARRAY( data_r )->len; i++ ) {
        buf[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( data_r )[ i ] );
      }
    }
    else {
      rb_raise( rb_eArgError, "User data must be an array of bytes" );
    }
  }
  else {
    xid = get_transaction_id( );
    vendor = VENDOR_ID;
  }
  ( ( struct ofp_header * ) ( vendor_request->data ) )->xid = htonl( xid );
  ( ( struct ofp_vendor_header * ) ( vendor_request->data ) )->vendor = htonl( vendor );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of attribute transaction id.
 */
static VALUE
vendor_request_transaction_id( VALUE self ) {
  buffer *vendor_request;
  Data_Get_Struct( self, buffer, vendor_request );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( vendor_request->data ) )->xid );
  return UINT2NUM( xid );
}


/*
 * A 32-bit value that uniquely identifies the vendor.
 *
 * @return [Number] the value of attribute vendor id.
 */
static VALUE
vendor_request_vendor( VALUE self ) {
  buffer *vendor_request;
  Data_Get_Struct( self, buffer, vendor_request );
  uint32_t vendor = ntohl( ( ( struct ofp_vendor_header * ) ( vendor_request->data ) )->vendor );
  return UINT2NUM( vendor );
}


/*
 * Vendor specific data payload.
 *
 * @return [Array] an array of data payload bytes.
 * @return [nil] vendor specific data not found.
 */
static VALUE
vendor_request_data( VALUE self ) {
  VALUE data_arr;
  buffer *vendor_request;
  uint8_t *buf;
  uint32_t i;

  Data_Get_Struct( self, buffer, vendor_request );
  uint16_t data_length = ( uint16_t ) ( vendor_request->length - sizeof ( struct ofp_vendor_header ) );

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
Init_vendor_request() {
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
