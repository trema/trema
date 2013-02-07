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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cQueueGetConfigRequest;


static VALUE
queue_get_config_request_alloc( VALUE klass ) {
  uint16_t port = 1;
  buffer *queue_get_config_request = create_queue_get_config_request( get_transaction_id(), port );
  return Data_Wrap_Struct( klass, NULL, free_buffer, queue_get_config_request );
}


/*
 * Request message to retrieve configuration about a queue port setting that
 * quantifies a QoS.
 * Each flow entry contains a queue that a flow is mapped to set constraints to
 * define some restriction like maximum/minimum data rate.
 *
 * @overload initialize(options={})
 *   @example
 *     QueueGetConfigRequest.new
 *     QueueGetConfigRequest.new( :port => 1 )
 *     QueueGetConfigRequest.new( :port => 1, :transaction_id => 123 )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :port
 *     a port number to query (defaults to 1).
 *
 *   @option options [Number] :transaction_id
 *     An unsigned 32-bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *
 *   @raise [ArgumentError] if transaction_id is not an unsigned 32-bit integer.
 *   @raise [ArgumentError] if port is not an unsigned 16-bit integer.
 *   @raise [TypeError] if options is not a hash.
 *
 *   @return [QueueGetConfigRequest]
 *     an object that encapsulates the +OFPT_GET_CONFIG_REQUEST+ OpenFlow message.
 */
static VALUE
queue_get_config_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *queue_get_config_request;
  Data_Get_Struct( self, buffer, queue_get_config_request );
  uint32_t xid = get_transaction_id();
  uint16_t port = 1;

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
    VALUE port_ruby;
    if ( ( port_ruby = rb_hash_aref( options, ID2SYM( rb_intern( "port" ) ) ) ) != Qnil ) {
      if ( rb_funcall( port_ruby, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Port must be an unsigned 16-bit integer" );
      }
      port = ( uint16_t ) NUM2UINT( port_ruby );
    }
  }
  ( ( struct ofp_header * ) ( queue_get_config_request->data ) )->xid = htonl( xid );
  ( ( struct ofp_queue_get_config_request * ) ( queue_get_config_request->data ) )->port = htons( port );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
queue_get_config_request_transaction_id( VALUE self ) {
  buffer *queue_get_config_request;
  Data_Get_Struct( self, buffer, queue_get_config_request );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( queue_get_config_request->data ) )->xid );
  return UINT2NUM( xid );
}


/*
 * The port the queue is attached to.
 *
 * @return [Number] the value of port.
 */
static VALUE
queue_get_config_request_port( VALUE self ) {
  buffer *queue_get_config_request;
  Data_Get_Struct( self, buffer, queue_get_config_request );
  uint16_t port = ntohs( ( ( struct ofp_queue_get_config_request * ) ( queue_get_config_request->data ) )->port );
  return UINT2NUM( port );
}


void
Init_queue_get_config_request() {
  mTrema = rb_define_module( "Trema" );
  cQueueGetConfigRequest = rb_define_class_under( mTrema, "QueueGetConfigRequest", rb_cObject );
  rb_define_alloc_func( cQueueGetConfigRequest, queue_get_config_request_alloc );
  rb_define_method( cQueueGetConfigRequest, "initialize", queue_get_config_request_init, -1 );
  rb_define_method( cQueueGetConfigRequest, "transaction_id", queue_get_config_request_transaction_id, 0 );
  rb_alias( cQueueGetConfigRequest, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cQueueGetConfigRequest, "port", queue_get_config_request_port, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
