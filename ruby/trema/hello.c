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


#include "trema-ruby-utils.h"


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
 *     An unsigned 32-bit integer number associated with this message.
 *
 * @overload initialize(options)
 *   @example
 *     Hello.new( :xid => 123 )
 *     Hello.new( :transaction_id => 123 )
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :xid an alias to transaction_id.
 *   @option options [Number] :transaction_id
 *     An unsigned 32-bit integer number associated with this message.
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
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    set_xid( hello, get_transaction_id() );
  }
  else {
    if ( options == Qnil ) {
      set_xid( hello, get_transaction_id() );
    }
    else if ( rb_obj_is_kind_of( options, rb_cInteger ) == Qtrue ) {
      validate_xid( options );
      set_xid( hello, ( uint32_t ) NUM2UINT( options ) );
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
        set_xid( hello, ( uint32_t ) NUM2UINT( xid ) );
      }
      else {
        set_xid( hello, get_transaction_id() );
      }
    }
  }

  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction ID.
 */
static VALUE
hello_transaction_id( VALUE self ) {
  return get_xid( self );
}


void
Init_hello() {
  mTrema = rb_define_module( "Trema" );
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
