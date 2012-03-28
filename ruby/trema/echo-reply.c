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
VALUE cEchoReply;


static VALUE
echo_reply_alloc( VALUE klass ) {
  buffer *body = alloc_buffer();
  buffer *echo_reply = create_echo_reply( 0, body );
  free_buffer( body );
  return Data_Wrap_Struct( klass, NULL, free_buffer, echo_reply );
}


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
static VALUE
echo_reply_init( int argc, VALUE *argv, VALUE self ) {
  buffer *echo_reply = NULL;
  Data_Get_Struct( self, buffer, echo_reply );
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    set_xid( echo_reply, get_transaction_id() );
  }
  else {
    if ( options == Qnil ) {
      set_xid( echo_reply, get_transaction_id() );
    }
    else if ( rb_obj_is_kind_of( options, rb_cInteger ) == Qtrue ) {
      validate_xid( options );
      set_xid( echo_reply, ( uint32_t ) NUM2UINT( options ) );
    }
    else {
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
        set_xid( echo_reply, ( uint32_t ) NUM2UINT( xid ) );
      }
      else {
        set_xid( echo_reply, get_transaction_id() );
      }

      VALUE user_data = rb_hash_aref( options, ID2SYM( rb_intern( "user_data" ) ) );
      if ( user_data != Qnil ) {
        Check_Type( user_data, T_STRING );
        uint16_t length = ( u_int16_t ) RSTRING_LEN( user_data );
        append_back_buffer( echo_reply, length );
        set_length( echo_reply, length );
        memcpy( ( char * ) echo_reply->data + sizeof( struct ofp_header ), RSTRING_PTR( user_data ), length );
      }
    }
  }

  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to
 * replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
echo_reply_transaction_id( VALUE self ) {
  return get_xid( self );
}


/*
 * An arbitrary length user data payload.
 *
 * @return [String] a user data payload is set.
 * @return [nil] a user data payload is not set.
 */
static VALUE
echo_reply_user_data( VALUE self ) {
  buffer *echo_reply;
  Data_Get_Struct( self, buffer, echo_reply );
  if ( echo_reply->length > sizeof( struct ofp_header ) ) {
    return rb_str_new( ( char * ) echo_reply->data + sizeof( struct ofp_header ),
                       ( long ) ( echo_reply->length - sizeof( struct ofp_header ) ) );
  }
  return Qnil;
}


void
Init_echo_reply() {
  cEchoReply = rb_define_class_under( mTrema, "EchoReply", rb_cObject );
  rb_define_alloc_func( cEchoReply, echo_reply_alloc );
  rb_define_method( cEchoReply, "initialize", echo_reply_init, -1 );
  rb_define_method( cEchoReply, "transaction_id", echo_reply_transaction_id, 0 );
  rb_alias( cEchoReply, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cEchoReply, "user_data", echo_reply_user_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
