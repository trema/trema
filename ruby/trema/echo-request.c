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


static VALUE
echo_request_new( int argc, VALUE *argv, VALUE klass ) {
  buffer *echo_request;
  buffer *body = NULL;

  VALUE xid_ruby, user_data;
  uint32_t xid;

  xid = get_transaction_id( );
  if ( argc == 1 ) {
    if ( rb_scan_args( argc, argv, "01", &xid_ruby ) == 1 ) {
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
  return Data_Wrap_Struct( klass, NULL, free_buffer, echo_request );
}


static VALUE
echo_request_transaction_id( VALUE self ) {
  buffer *echo_request;
  
  Data_Get_Struct( self, buffer, echo_request );
  uint32_t xid = ntohl( (( struct ofp_header *)(echo_request->data))->xid);
  return UINT2NUM(xid);
}


static VALUE
echo_request_user_data( VALUE self ) {
  buffer *echo_request;

  Data_Get_Struct( self, buffer, echo_request );
  if ( echo_request->length > sizeof ( struct ofp_header ) ) {
    return rb_str_new2( ( char * ) echo_request->data + sizeof ( struct ofp_header ) );
  }
  return Qnil;
}


void
Init_echo_request( ) {
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
