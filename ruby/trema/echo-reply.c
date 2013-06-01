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


#include "echo.h"
#include "ruby.h"
#include "trema.h"
#include "trema-ruby-utils.h"


extern VALUE mTrema;
VALUE cEchoReply;


static VALUE
echo_reply_alloc( VALUE klass ) {
  buffer *body = alloc_buffer();
  buffer *echo_reply = create_echo_reply( 0, body );
  free_buffer( body );
  return Data_Wrap_Struct( klass, NULL, free_buffer, echo_reply );
}


#if 0
/*
 * Creates a EchoReply OpenFlow message. This message can be used to
 * measure the bandwidth of a controller/switch connection as well as
 * to verify its liveness.
 *
 * @overload initialize()
 *   @example
 *     EchoReply.new
 *
 * @overload initialize(transaction_id)
 *   @example
 *     EchoReply.new( 123 )
 *   @param [Integer] transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *
 * @overload initialize(options)
 *   @example
 *     EchoReply.new(
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
 * @return [EchoReply]
 */
VALUE
echo_init( int argc, VALUE *argv, VALUE self ) {}
#endif


#if 0
/*
 * Transaction ids, message sequence numbers matching requests to
 * replies.
 *
 * @return [Number] the value of transaction id.
 */
VALUE
echo_transaction_id( VALUE self ) {}
#endif


#if 0
/*
 * An arbitrary length user data payload.
 *
 * @return [String] a user data payload is set.
 * @return [nil] a user data payload is not set.
 */
VALUE
echo_user_data( VALUE self ) {}
#endif


void
Init_echo_reply() {
  mTrema = rb_define_module( "Trema" );
  cEchoReply = rb_define_class_under( mTrema, "EchoReply", rb_cObject );
  rb_define_alloc_func( cEchoReply, echo_reply_alloc );
  rb_define_method( cEchoReply, "initialize", echo_init, -1 );
  rb_define_method( cEchoReply, "transaction_id", echo_transaction_id, 0 );
  rb_alias( cEchoReply, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cEchoReply, "user_data", echo_user_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
