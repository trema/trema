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
VALUE cHello;


static VALUE
hello_alloc( VALUE klass ) {
  buffer *hello = create_hello( 0 );
  return Data_Wrap_Struct( klass, NULL, free_buffer, hello );
}


/*
 * Creates a Hello OpenFlow message.
 *
 * @overload initialize()
 *   @example
 *     Hello.new
 *
 * @overload initialize(transaction_id)
 *   @example
 *     Hello.new( 123 )
 *   @param [Integer] transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *
 * @overload initialize(options)
 *   @example
 *     Hello.new( :xid => 123 )
 *     Hello.new( :transaction_id => 123 )
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :xid
 *   @option options [Number] :transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *
 * @raise [ArgumentError] if transaction ID is not an unsigned 32-bit integer.
 * @raise [TypeError] if argument is not a Integer or a Hash.
 * @return [Hello]
 */
static VALUE
hello_init( int argc, VALUE *argv, VALUE self ) {
  buffer *hello = NULL;
  Data_Get_Struct( self, buffer, hello );
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
      VALUE tmp = Qnil;
      VALUE xid_ruby = Qnil;

      tmp = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
      if ( tmp != Qnil ) {
        xid_ruby = tmp;
      }
      tmp = rb_hash_aref( options, ID2SYM( rb_intern( "xid" ) ) );
      if ( tmp != Qnil ) {
        xid_ruby = tmp;
      }

      if ( xid_ruby != Qnil ) {
        validate_xid( xid_ruby );
        xid = ( uint32_t ) NUM2UINT( xid_ruby );
      }
      else {
        xid = get_transaction_id();
      }
    }
  }

  ( ( struct ofp_header * ) ( hello->data ) )->xid = htonl( xid );

  return self;
}


/*
 * An unsigned 32bit integer number associated with this
 * message. Replies use the same id as was in the request to
 * facilitate pairing.
 *
 * @return [Number] the value of transaction ID.
 */
static VALUE
hello_transaction_id( VALUE self ) {
  buffer *hello;
  Data_Get_Struct( self, buffer, hello );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( hello->data ) )->xid );
  return UINT2NUM( xid );
}


void
Init_hello() {
  cHello = rb_define_class_under( mTrema, "Hello", rb_cObject );
  rb_define_alloc_func( cHello, hello_alloc );
  rb_define_method( cHello, "initialize", hello_init, -1 );
  rb_define_method( cHello, "transaction_id", hello_transaction_id, 0 );
  rb_alias( cHello, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
