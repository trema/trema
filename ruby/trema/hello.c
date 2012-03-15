/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
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
 * @overload initialize(options={})
 *
 *   @example
 *     Hello.new
 *     Hello.new( :transaction_id => 123 )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *
 *   @raise [ArgumentError] if transaction ID is not an unsigned 32-bit integer.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [Hello]
 *     an object that encapsulates the +OPFT_HELLO+ OpenFlow message.
 */
static VALUE
hello_init( int argc, VALUE *argv, VALUE self ) {
  buffer *hello;
  Data_Get_Struct( self, buffer, hello );
  uint32_t xid = get_transaction_id();
  VALUE options;

  if ( rb_scan_args( argc, argv, "01", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE xid_ruby;
    if ( ( xid_ruby = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) ) ) != Qnil ) {
      if ( rb_funcall( xid_ruby, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Transaction ID must be an unsigned 32-bit integer" );
      }
      xid = ( uint32_t ) NUM2UINT( xid_ruby );
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
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
