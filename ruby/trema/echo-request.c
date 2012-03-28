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
#include "trema.h"
#include "trema_ruby_utils.h"


extern VALUE mTrema;
VALUE cEchoRequest;


static VALUE
echo_request_alloc( VALUE klass ) {
  buffer *body = alloc_buffer();
  buffer *echo_request = create_echo_request( 0, body );
  free_buffer( body );
  return Data_Wrap_Struct( klass, NULL, free_buffer, echo_request );
}


/*
 * Creates a EchoRequest OpenFlow message. This message can be used to
 * measure the bandwidth of a controller/switch connection as well as
 * to verify its liveness.
 *
 * @overload initialize()
 *   @example
 *     EchoRequest.new
 *
 * @overload initialize(transaction_id)
 *   @example
 *     EchoRequest.new( 123 )
 *   @param [Integer] transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *
 * @overload initialize(options)
 *   @example
 *     EchoRequest.new(
 *       :transaction_id => transaction_id,
 *       :user_data => "Thu Aug 25 13:09:00 +0900 2011"
 *     )
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :xid
 *   @option options [Number] :transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *   @option options [String] :user_data
 *     the user data field specified as a String may be a message timestamp to check latency,
 *     various lengths to measure bandwidth or zero-size(nil) to verify liveness between
 *     the switch and controller.
 *
 * @raise [ArgumentError] if transaction ID is not an unsigned 32-bit integer.
 * @raise [ArgumentError] if user data is not a string.
 * @raise [TypeError] if argument is not a hash.
 * @return [EchoRequest]
 */
static VALUE
echo_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *echo_request = NULL;
  Data_Get_Struct( self, buffer, echo_request );
  uint32_t xid;
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    xid = get_transaction_id();
  }
  else {
    if ( options == Qnil ) {
      xid = get_transaction_id();
    }
    else if ( rb_obj_is_kind_of( options, rb_cInteger ) == Qtrue ) {
      validate_xid( options );
      xid = ( uint32_t ) NUM2UINT( options );
    }
    else {
      Check_Type( options, T_HASH );
      VALUE xid_tmp = Qnil;
      VALUE xid_ruby = Qnil;
      xid_tmp = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
      if ( xid_tmp != Qnil ) {
        xid_ruby = xid_tmp;
      }
      xid_tmp = rb_hash_aref( options, ID2SYM( rb_intern( "xid" ) ) );
      if ( xid_tmp != Qnil ) {
        xid_ruby = xid_tmp;
      }
      if ( xid_ruby != Qnil ) {
        validate_xid( xid_ruby );
        xid = ( uint32_t ) NUM2UINT( xid_ruby );
      }
      else {
        xid = get_transaction_id();
      }

      VALUE user_data = rb_hash_aref( options, ID2SYM( rb_intern( "user_data" ) ) );
      if ( user_data != Qnil ) {
        Check_Type( user_data, T_STRING );
        uint16_t length = ( u_int16_t ) RSTRING_LEN( user_data );
        append_back_buffer( echo_request, length );
        ( ( struct ofp_header * ) ( echo_request->data ) )->length = htons( ( uint16_t ) ( sizeof( struct ofp_header ) + length ) );
        memcpy( ( char * ) echo_request->data + sizeof( struct ofp_header ), RSTRING_PTR( user_data ), length );
      }
    }
  }

  ( ( struct ofp_header * ) ( echo_request->data ) )->xid = htonl( xid );

  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to
 * replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
echo_request_transaction_id( VALUE self ) {
  return get_xid( self );
}


/*
 * An arbitrary length user data payload.
 *
 * @return [String] a user data payload is set.
 * @return [nil] a user data payload is not set.
 */
static VALUE
echo_request_user_data( VALUE self ) {
  buffer *echo_request;
  Data_Get_Struct( self, buffer, echo_request );
  if ( echo_request->length > sizeof( struct ofp_header ) ) {
    return rb_str_new( ( char * ) echo_request->data + sizeof( struct ofp_header ),
                       ( long ) ( echo_request->length - sizeof( struct ofp_header ) ) );
  }
  return Qnil;
}


void
Init_echo_request() {
  cEchoRequest = rb_define_class_under( mTrema, "EchoRequest", rb_cObject );
  rb_define_alloc_func( cEchoRequest, echo_request_alloc );
  rb_define_method( cEchoRequest, "initialize", echo_request_init, -1 );
  rb_define_method( cEchoRequest, "transaction_id", echo_request_transaction_id, 0 );
  rb_alias( cEchoRequest, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cEchoRequest, "user_data", echo_request_user_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
