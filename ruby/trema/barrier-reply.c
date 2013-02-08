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
VALUE cBarrierReply;


/*
 * A new instance of {BarrierReply} constructed when +OFPT_BARRIER_REPLY+
 * message received.
 *
 * @overload initialize(datapath_id, transaction_id)
 *
 * @param [Number] datapath_id
 *   a unique name that identifies an OpenVSwitch, the message originator.
 *
 * @param [Number] transaction_id
 *   value copied from the +OFPT_BARRIER_REQUEST+ message.
 *
 * @return [BarrierReply] self an object that encapsulates the +OFPT_BARRIER_REPLY+ message.
 */
static VALUE
barrier_reply_init( VALUE self, VALUE datapath_id, VALUE transaction_id ) {
  rb_iv_set( self, "@datapath_id", datapath_id );
  rb_iv_set( self, "@transaction_id", transaction_id );
  return self;
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
barrier_reply_datapath_id( VALUE self ) {
  return rb_iv_get( self, "@datapath_id" );
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction ID.
 */
static VALUE
barrier_reply_transaction_id( VALUE self ) {
  return rb_iv_get( self, "@transaction_id" );
}


void
Init_barrier_reply() {
  mTrema = rb_define_module( "Trema" );
  cBarrierReply = rb_define_class_under( mTrema, "BarrierReply", rb_cObject );
  rb_define_method( cBarrierReply, "initialize", barrier_reply_init, 2 );
  rb_define_method( cBarrierReply, "datapath_id", barrier_reply_datapath_id, 0 );
  rb_define_method( cBarrierReply, "transaction_id", barrier_reply_transaction_id, 0 );
}


void
handle_barrier_reply( uint64_t datapath_id, uint32_t transaction_id, void *user_data ) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "barrier_reply" ) ) == Qfalse ) {
    return;
  }

  VALUE barrier_reply = rb_funcall( cBarrierReply, rb_intern( "new" ), 2, ULL2NUM( datapath_id ), UINT2NUM( transaction_id ) );
  rb_funcall( controller, rb_intern( "barrier_reply" ), 2, ULL2NUM( datapath_id ), barrier_reply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
