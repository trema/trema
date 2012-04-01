/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cBarrierRequest;


static VALUE
barrier_request_alloc( VALUE klass ) {
  buffer *barrier_request = create_barrier_request( get_transaction_id() );
  return Data_Wrap_Struct( klass, NULL, free_buffer, barrier_request );
}


/*
 * A barrier request message could be sent to ensure that an operation
 * completed successfully signaled with the reception of a barrier reply message.
 *
 * @overload initialize(options={})
 *   @example 
 *     BarrierRequest.new
 *     BarrierRequest.new( :transaction_id => 123 )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     an unsigned 32-bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *
 *   @raise [ArgumentError] if transaction_id is not an unsigned 32-bit integer.
 *   @raise [TypeError] if options is not a hash.
 *
 *   @return [BarrierRequest]
 *     an object that encapsulates the +OPFT_BARRIER_REQUEST+ OpenFlow message.
 */
static VALUE
barrier_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *barrier_request;
  Data_Get_Struct( self, buffer, barrier_request );
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
  ( ( struct ofp_header * ) ( barrier_request->data ) )->xid = htonl( xid );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
barrier_request_transaction_id( VALUE self ) {
  buffer *barrier_request;
  Data_Get_Struct( self, buffer, barrier_request );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( barrier_request->data ) )->xid );
  return UINT2NUM( xid );
}


void
Init_barrier_request() {
  cBarrierRequest = rb_define_class_under( mTrema, "BarrierRequest", rb_cObject );
  rb_define_alloc_func( cBarrierRequest, barrier_request_alloc );
  rb_define_method( cBarrierRequest, "initialize", barrier_request_init, -1 );
  rb_define_method( cBarrierRequest, "transaction_id", barrier_request_transaction_id, 0 );
  rb_alias( cBarrierRequest, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
