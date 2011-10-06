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
VALUE cEchoReply;


static VALUE
echo_reply_alloc( VALUE klass ) {
  buffer *echo_reply = create_echo_reply( get_transaction_id(), NULL );
  return Data_Wrap_Struct( klass, NULL, free_buffer, echo_reply );
}


/*
 * Creates a {EchoReply} instance mainly used for testing
 *
 * @overload initialize(transaction_id=nil)
 *   Creates a {EchoReply} object by specifying its transaction id. If
 *   transaction_id is not specified, an auto-generated transaction_id
 *   is set.
 *
 * @raise [ArgumentError] if transaction id is negative.
 *
 * @return [EchoReply] an object that encapsulates the +OFPT_ECHO_REPLY+ openflow message.
 */
static VALUE
echo_reply_init( int argc, VALUE *argv, VALUE self ) {
  buffer *echo_reply;
  Data_Get_Struct( self, buffer, echo_reply );
  
  uint32_t xid;
  VALUE xid_ruby;
  if ( rb_scan_args( argc, argv, "01", &xid_ruby ) == 1 ) {
    if ( NUM2INT( xid_ruby ) < 0 ) {
      rb_raise( rb_eArgError, "Transaction ID must be >= 0" );
    }
    xid = ( uint32_t ) NUM2UINT( xid_ruby );
  } 
  else {
    xid = get_transaction_id();
  }
  ( ( struct ofp_header * ) ( echo_reply->data ) )->xid = htonl( xid );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of attribute transaction id.
 */
static VALUE
echo_reply_transaction_id( VALUE self ) {
  buffer *echo_reply;
  Data_Get_Struct( self, buffer, echo_reply );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( echo_reply->data ) )->xid );
  return UINT2NUM( xid );
}


void
Init_echo_reply() {
  cEchoReply = rb_define_class_under( mTrema, "EchoReply", rb_cObject );
  rb_define_alloc_func( cEchoReply, echo_reply_alloc );
  rb_define_method( cEchoReply, "initialize", echo_reply_init, -1 );
  rb_define_method( cEchoReply, "transaction_id", echo_reply_transaction_id, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
