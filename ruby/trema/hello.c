/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
VALUE cHello;


static VALUE
hello_alloc( VALUE klass ) {
  buffer *hello = create_hello( 0 );
  return Data_Wrap_Struct( klass, NULL, free_buffer, hello );
}


/*
 * @overload initialize(transaction_id=nil)
 *   Creates a {Hello} object by specifying its transaction id. If
 *   transaction_id is not specified, an auto-generated transaction_id
 *   is set.
 * 
 *   @raise [ArgumentError] if transaction id is negative.
 * 
 *   @return [Hello] an object that encapsulates the OFPT_HELLO openflow message.
 */
static VALUE
hello_init( int argc, VALUE *argv, VALUE self ) {
  buffer *hello;
  Data_Get_Struct( self, buffer, hello );

  VALUE xid_ruby;
  uint32_t xid;
  if ( rb_scan_args( argc, argv, "01", &xid_ruby ) == 0 ) {
    xid = get_transaction_id();
  }
  else {
    if ( rb_funcall( xid_ruby, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
      rb_raise( rb_eArgError, "Transaction ID must be an unsigned 32bit integer" );
    }
    xid = ( uint32_t ) NUM2UINT( xid_ruby );
  }
  ( ( struct ofp_header * ) ( hello->data ) )->xid = htonl( xid );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of attribute transaction id.
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
