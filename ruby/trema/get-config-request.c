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
VALUE cGetConfigRequest;


static VALUE
get_config_request_alloc( VALUE klass ) {
  buffer *get_config_request = create_get_config_request( get_transaction_id() );
  return Data_Wrap_Struct( klass, NULL, free_buffer, get_config_request );
}


/*
 * Creates a {GetConfigRequest} instance to query configuration parameters 
 * from the switch.
 *
 * @overload initialize(options={})
 *   @example
 *     GetConfigRequest.new
 *     GetConfigRequest.new( :transaction_id => 123 )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     An unsigned 32-bit integer auto-generated if not supplied.
 *
 *   @raise [ArgumentError] if transaction id is not an unsigned 32-bit integer.
 *   @raise [TypeError] if options is not a hash.
 *
 *   @return [GetConfigRequest]
 *     an object that encapsulates the +OFPT_GET_CONFIG+ OpenFlow message.
 */
static VALUE
get_config_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *get_config_request;
  Data_Get_Struct( self, buffer, get_config_request );
  uint32_t xid = get_transaction_id( );
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
  ( ( struct ofp_header * ) ( get_config_request->data ) )->xid = htonl( xid );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
get_config_request_transaction_id( VALUE self ) {
  buffer *get_config_request;
  Data_Get_Struct( self, buffer, get_config_request );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( get_config_request->data ) )->xid );
  return UINT2NUM( xid );
}


void
Init_get_config_request() {
  cGetConfigRequest = rb_define_class_under( mTrema, "GetConfigRequest", rb_cObject );
  rb_define_alloc_func( cGetConfigRequest, get_config_request_alloc );
  rb_define_method( cGetConfigRequest, "initialize", get_config_request_init, -1 );
  rb_define_method( cGetConfigRequest, "transaction_id", get_config_request_transaction_id, 0 );
  rb_alias( cGetConfigRequest, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
