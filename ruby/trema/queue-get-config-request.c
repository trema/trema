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
 * @overload initialize(transaction_id=nil, port=nil)
 *
 * @param [Number] transaction_id
 *   a positive number, if not given transaction_id is auto-generated.
 *
 * @param [Number] port
 *   a port number to query (defaults to 1)
 *
 * @raise [ArgumentError] if transaction_id is negative.
 *
 * @return [QueueGetConfigRequest]
 *   an object that encapsulates the +OFPT_GET_CONFIG_REQUEST+ openflow message.
 */
static VALUE
queue_get_config_request_init( int argc, VALUE *argv, VALUE self ) {
  buffer *queue_get_config_request;
  Data_Get_Struct( self, buffer, queue_get_config_request );
  VALUE xid_ruby;
  VALUE port_ruby;
  uint32_t xid;
  uint16_t port;

  if ( rb_scan_args( argc, argv, "02", &xid_ruby, &port_ruby ) == 2 ) {
    if ( NUM2INT( xid_ruby ) < 0 ) {
      rb_raise( rb_eArgError, "Transaction ID must be >= 0" );
    }
    xid = ( uint32_t ) NUM2UINT( xid_ruby );
    port = ( uint16_t ) NUM2UINT( port_ruby );
  }
  else {
    xid = get_transaction_id();
    port = 1;
  }
  ( ( struct ofp_header * ) ( queue_get_config_request->data ) )->xid = htonl( xid );
  ( ( struct ofp_queue_get_config_request * ) ( queue_get_config_request->data ) )->port = htons( port );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of attribute transaction id.
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
 * @return [Number] the value of attribute port.
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
  cQueueGetConfigRequest = rb_define_class_under( mTrema, "QueueGetConfigRequest", rb_cObject );
  rb_define_alloc_func( cQueueGetConfigRequest, queue_get_config_request_alloc );
  rb_define_method( cQueueGetConfigRequest, "initialize", queue_get_config_request_init, -1 );
  rb_define_method( cQueueGetConfigRequest, "transaction_id", queue_get_config_request_transaction_id, 0 );
  rb_define_method( cQueueGetConfigRequest, "port", queue_get_config_request_port, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
