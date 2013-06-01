/*
 * Copyright (C) 2008-2013 NEC Corporation
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
VALUE cVendor;


static VALUE
vendor_alloc( VALUE klass ) {
  buffer *vendor = create_vendor( 0, 0, NULL );
  return Data_Wrap_Struct( klass, NULL, free_buffer, vendor );
}


/*
 * Creates a Vendor Request message. This message can be used
 * to facilitate sending of vendor-defined arbitrary data.
 *
 * @overload initialize
 *   @example
 *     Vendor.new
 *
 * @overload initialize(options)
 *   @example
 *     Vendor.new(
 *       :vendor => 0x3000,
 *       :data => "deadbeef".unpack( "C*" ),
 *       :transaction_id => 123
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :xid
 *   @option options [Number] :transaction_id
 *     an unsigned 32bit integer number associated with this message.
 *     if not specified, an auto-generated value is set.
 *   @option options [Number] :vendor
 *     the vendor identifier. If MSB is zero low order bytes are IEEE OUI. Otherwise defined by openflow.
 *   @option options [Array] :data
 *     a String that holds vendor's defined arbitrary length data.
 *
 * @raise [ArgumentError] if transaction ID is not an unsigned 32-bit integer.
 * @raise [ArgumentError] if user data is not an array of bytes.
 * @raise [TypeError] if options is not a hash.
 * @return [Vendor]
 */
static VALUE
vendor_init( int argc, VALUE *argv, VALUE self ) {
  buffer *vendor = NULL;
  Data_Get_Struct( self, buffer, vendor );
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    set_xid( vendor, get_transaction_id() );
  }
  else {
    if ( options == Qnil ) {
      set_xid( vendor, get_transaction_id() );
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
          set_xid( vendor, ( uint32_t ) NUM2UINT( xid ) );
        }
        else {
          set_xid( vendor, get_transaction_id() );
        }

        tmp = rb_hash_aref( options, ID2SYM( rb_intern( "vendor" ) ) );
        if ( tmp != Qnil ) {
          ( ( struct ofp_vendor_header * ) ( vendor->data ) )->vendor = htonl( ( uint32_t ) NUM2UINT( tmp ) );
        }

        tmp = rb_hash_aref( options, ID2SYM( rb_intern( "data" ) ) );
        if ( tmp != Qnil ) {
          Check_Type( tmp, T_ARRAY );
          uint16_t length = ( uint16_t ) RARRAY_LEN( tmp );
          append_back_buffer( vendor, length );
          set_length( vendor, ( uint16_t ) ( sizeof( struct ofp_vendor_header ) + length ) );
          uint8_t *data = ( uint8_t * ) ( ( char * ) vendor->data + sizeof( struct ofp_vendor_header ) );
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
vendor_transaction_id( VALUE self ) {
  return get_xid( self );
}


/*
 * A 32-bit value that uniquely identifies the vendor.
 *
 * @return [Number] the value of vendor id.
 */
static VALUE
vendor_vendor( VALUE self ) {
  buffer *vendor_message;
  Data_Get_Struct( self, buffer, vendor_message );
  uint32_t vendor = ntohl( ( ( struct ofp_vendor_header * ) ( vendor_message->data ) )->vendor );
  return UINT2NUM( vendor );
}


/*
 * Vendor specific data payload.
 *
 * @return [Array] an array of data payload bytes.
 * @return [nil] vendor specific data not found.
 */
static VALUE
vendor_data( VALUE self ) {
  buffer *vendor;
  Data_Get_Struct( self, buffer, vendor );
  uint16_t length = get_length( vendor );

  if ( length > 0 ) {
    VALUE data_array = rb_ary_new2( length );
    uint8_t *data = ( uint8_t * ) ( ( char * ) vendor->data + sizeof( struct ofp_vendor_header ) );
    long i;
    for ( i = 0; i < length; i++ ) {
      rb_ary_push( data_array, INT2FIX( data[ i ] ) );
    }
    return data_array;
  }
  return Qnil;
}


/*
 * Document-class: Trema::Vendor
 */
void
Init_vendor() {
  mTrema = rb_eval_string( "Trema" );
  cVendor = rb_define_class_under( mTrema, "Vendor", rb_cObject );
  rb_define_alloc_func( cVendor, vendor_alloc );
  rb_define_method( cVendor, "initialize", vendor_init, -1 );
  rb_define_method( cVendor, "transaction_id", vendor_transaction_id, 0 );
  rb_alias( cVendor, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cVendor, "vendor", vendor_vendor, 0 );
  rb_define_method( cVendor, "data", vendor_data, 0 );
}


void
handle_vendor(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t vendor,
  buffer *data,
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

  if ( data != NULL && data->length > 0 ) {
    VALUE data_array = rb_ary_new2( ( long ) data->length );
    size_t i;
    for ( i = 0; i < data->length; i++ ) {
      rb_ary_push( data_array, INT2FIX( ( ( uint8_t * ) data->data)[ i ] ) );
    }
    rb_hash_aset( attributes, ID2SYM( rb_intern( "data" ) ), data_array );
  }
  VALUE vendor_r = rb_funcall( cVendor, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "vendor" ), 2, ULL2NUM( datapath_id ), vendor_r );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
