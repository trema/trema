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
VALUE cBarrierRequest;


static VALUE
barrier_request_alloc( VALUE klass ) {
  buffer *barrier_request = create_barrier_request( get_transaction_id( ) );

  return Data_Wrap_Struct( klass, NULL, free_buffer, barrier_request );
}


static VALUE
barrier_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *barrier_request;

  Data_Get_Struct( self, buffer, barrier_request );
  VALUE xid_ruby;
  uint32_t xid;

  if ( rb_scan_args( argc, argv, "01", &xid_ruby ) == 1 ) {
    xid = ( uint32_t ) NUM2UINT( xid_ruby );
  } else {
    xid = get_transaction_id( );
  }
  ( ( struct ofp_header * ) ( barrier_request->data ) )->xid = htonl( xid );
  return self;
}


static VALUE
barrier_request_transaction_id( VALUE self ) {
  buffer *barrier_request;

  Data_Get_Struct( self, buffer, barrier_request );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( barrier_request->data ) )->xid );
  return UINT2NUM( xid );
}


void
Init_barrier_request( ) {
  cBarrierRequest = rb_define_class_under( mTrema, "BarrierRequest", rb_cObject );
  rb_define_alloc_func( cBarrierRequest, barrier_request_alloc );
  rb_define_method( cBarrierRequest, "initialize", barrier_request_init, -1 );
  rb_define_method( cBarrierRequest, "transaction_id", barrier_request_transaction_id, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
