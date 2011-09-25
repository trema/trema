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
VALUE cEchoRequest;


/*
 * An echo request message can be used to measure the bandwidth of a 
 * controller/switch connection as well as to verify its liveness.
 *
 * @overload initialize(transaction_id=nil, user_data=nil)
 *
 * @param [Number] transaction_id
 *   a positive number, not recently attached to any previous pending commands to
 *   guarantee message integrity auto-generated if not specified.
 *
 * @param [String] user_data
 *   the user data field may be a message timestamp to check latency, various
 *   lengths to measure bandwidth or zero-size(nil) to verify liveness between 
 *   the switch and controller.
 *
 * @example Instantiate with transaction_id, user_data
 *   EchoRequest.new(1234, "Thu Aug 25 13:09:00 +0900 2011")
 *
 * @raise [ArgumentError] if transaction id is negative.
 * @raise [ArgumentError] if user data is not a string.
 *
 * @return [EchoRequest]
 *   a {EchoRequest} object that encapsulates the +OPFT_ECHO_REQUEST+ openflow
 *   message.
 */
static VALUE
echo_request_new( int argc, VALUE *argv, VALUE klass ) {
  buffer *echo_request;
  buffer *body = NULL;
  VALUE xid_ruby, user_data;
  uint32_t xid = get_transaction_id();

  if ( argc == 1 ) {
    if ( rb_scan_args( argc, argv, "01", &xid_ruby ) == 1 ) {
      if ( NUM2INT( xid_ruby ) < 0 ) {
        rb_raise( rb_eArgError, "Transaction ID must be >= 0" );
      }
      xid = ( uint32_t ) NUM2UINT( xid_ruby );
    }
  }
  if ( argc == 2 ) {
    if ( rb_scan_args( argc, argv, "02", &xid_ruby, &user_data ) == 2 ) {
      xid = ( uint32_t ) NUM2UINT( xid_ruby );
      if ( rb_obj_is_kind_of( user_data, rb_cString ) == Qfalse ) {
        rb_raise( rb_eArgError, "User data must be a string" );
      }
      uint16_t length = ( u_int16_t ) RSTRING_LEN( user_data );
      body = alloc_buffer_with_length( length );
      void *p = append_back_buffer( body, length );
      memcpy( p, RSTRING_PTR( user_data ), length );
    }
  }
  echo_request = create_echo_request( xid, body );
  if ( body != NULL ) {
    free_buffer( body );
  }
  return Data_Wrap_Struct( klass, NULL, free_buffer, echo_request );
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of attribute transaction id.
 */
static VALUE
echo_request_transaction_id( VALUE self ) {
  buffer *echo_request;
  Data_Get_Struct( self, buffer, echo_request );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( echo_request->data ) )->xid );
  return UINT2NUM( xid );
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
  rb_define_singleton_method( cEchoRequest, "new", echo_request_new, -1 );
  rb_define_method( cEchoRequest, "transaction_id", echo_request_transaction_id, 0 );
  rb_define_method( cEchoRequest, "user_data", echo_request_user_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
