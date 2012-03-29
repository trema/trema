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


#include "ruby.h"
#include "trema-ruby-utils.h"
#include "trema.h"


extern VALUE mTrema;
VALUE cVendorRequest;


static VALUE
vendor_request_alloc( VALUE klass ) {
  buffer *vendor_request = create_vendor( 0, 0, NULL );
  return Data_Wrap_Struct( klass, NULL, free_buffer, vendor_request );
}


/*
 * Creates a VendorReqeust OpenFlow message. This message can be used
 * to facilitate sending of vendor experimental messages.
 *
 * @overload initialize
 *   @example
 *     VendorRequest.new
 *
 * @overload initialize(options)
 *   @example
 *     VendorRequest.new(
 *       :vendor => 0x3000,
 *       :data => "deadbeef".unpack( "C*" ),
 *       :transaction_id => 123
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :xid
 *   @option options [Number] :transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *   @option options [Number] :vendor
 *     The assigned vendor id.
 *   @option options [Array] :data
 *     Vendor specific data.
 *
 * @raise [ArgumentError] if transaction ID is not an unsigned 32-bit integer.
 * @raise [ArgumentError] if user data is not an array of bytes.
 * @raise [TypeError] if options is not a hash.
 * @return [VendorRequest]
 */
static VALUE
vendor_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *vendor_request = NULL;
  Data_Get_Struct( self, buffer, vendor_request );
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    set_xid( vendor_request, get_transaction_id() );
  }
  else {
    if ( options == Qnil ) {
      set_xid( vendor_request, get_transaction_id() );
    }
    else {
      if ( rb_scan_args( argc, argv, "01", &options ) == 1 ) {
        Check_Type( options, T_HASH );
        VALUE tmp = Qnil;
        VALUE xid = Qnil;

        tmp = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
        if ( tmp != Qnil ) {
          xid = tmp;
        }
        tmp = rb_hash_aref( options, ID2SYM( rb_intern( "xid" ) ) );
        if ( tmp != Qnil ) {
          xid = tmp;
        }

        if ( xid != Qnil ) {
          validate_xid( xid );
          set_xid( vendor_request, ( uint32_t ) NUM2UINT( xid ) );
        }
        else {
          set_xid( vendor_request, get_transaction_id() );
        }

        tmp = rb_hash_aref( options, ID2SYM( rb_intern( "vendor" ) ) );
        if ( tmp != Qnil ) {
          ( ( struct ofp_vendor_header * ) ( vendor_request->data ) )->vendor = htonl( ( uint32_t ) NUM2UINT( tmp ) );
        }

        tmp = rb_hash_aref( options, ID2SYM( rb_intern( "data" ) ) );
        if ( tmp != Qnil ) {
          Check_Type( tmp, T_ARRAY );
          uint16_t length = ( uint16_t ) RARRAY_LEN( tmp );
          append_back_buffer( vendor_request, length );
          set_length( vendor_request, length );
          uint8_t *data = ( uint8_t * ) ( ( char * ) vendor_request->data + sizeof( struct ofp_vendor_header ) );
          int i;
          for ( i = 0; i < length; i++ ) {
            data[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( tmp )[ i ] );
          }
        }
      }
    }
  }

  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
vendor_request_transaction_id( VALUE self ) {
  return get_xid( self );
}


/*
 * A 32-bit value that uniquely identifies the vendor.
 *
 * @return [Number] the value of vendor id.
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
  buffer *vendor_request;
  Data_Get_Struct( self, buffer, vendor_request );
  uint16_t length = get_length( vendor_request );

  if ( length > 0 ) {
    VALUE data_array = rb_ary_new2( length );
    uint8_t *data = ( uint8_t * ) ( ( char * ) vendor_request->data + sizeof( struct ofp_vendor_header ) );
    int i;
    for ( i = 0; i < length; i++ ) {
      rb_ary_push( data_array, INT2FIX( data[ i ] ) );
    }
    return data_array;
  }
  return Qnil;
}


void
Init_vendor_request() {
  cVendorRequest = rb_define_class_under( mTrema, "VendorRequest", rb_cObject );
  rb_define_alloc_func( cVendorRequest, vendor_request_alloc );
  rb_define_method( cVendorRequest, "initialize", vendor_request_init, -1 );
  rb_define_method( cVendorRequest, "transaction_id", vendor_request_transaction_id, 0 );
  rb_alias( cVendorRequest, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cVendorRequest, "vendor", vendor_request_vendor, 0 );
  rb_define_method( cVendorRequest, "data", vendor_request_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
