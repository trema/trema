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
VALUE cGetConfigReply;


/*
 * Handles the response to +OFPT_GET_CONFIG_REQUEST+ message. The user would not
 * explicitly instantiate a {GetConfigReply} object but would be created while
 * parsing the +OFPT_GET_CONFIG_REPLY+ message. The {GetConfigReply} object is
 * an object whose attributes represent the return values of the message.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     GetConfigReply.new(
 *       :datapath_id => 2748
 *       :transaction_id => 1,
 *       :flags => 0
 *       :miss_send_len => 65535
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :datapath_id
 *     message originator identifier.
 *
 *   @option options [Number] :transaction_id
 *     the saved transaction_id from +OFPT_GET_CONFIG_REQUEST+ message.
 *
 *   @option options [Number] :flags
 *     indicates how IP fragments are treated.
 *
 *   @option options [Number] :miss_send_len
 *     the maximum number of bytes to send on a flow table miss or
 *     flow destined to controller.
 *
 *   @return [GetConfigReply]
 *     an object that encapsulates the +OFPT_GET_CONFIG_REPLY+ OpenFlow message.
 */
static VALUE
get_config_reply_init( VALUE self, VALUE options ) {
  rb_iv_set( self, "@attribute", options );
  return self;
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
get_config_reply_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
get_config_reply_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


/*
 * Flags indicate how IP fragments should be treated (no special handling,
 * dropped or reassembled).
 *
 * @return [Number] the value of flags.
 */
static VALUE
get_config_reply_flags( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "flags" ) ) );
}


/*
 * The maximum number of bytes to send on flow table miss or flow destined to controller.
 *
 * @return [Number] the value of miss_send_len.
 */
static VALUE
get_config_reply_miss_send_len( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "miss_send_len" ) ) );
}


void
Init_get_config_reply() {
  mTrema = rb_define_module( "Trema" );
  cGetConfigReply = rb_define_class_under( mTrema, "GetConfigReply", rb_cObject );
  rb_define_method( cGetConfigReply, "initialize", get_config_reply_init, 1 );
  rb_define_method( cGetConfigReply, "datapath_id", get_config_reply_datapath_id, 0 );
  rb_define_method( cGetConfigReply, "transaction_id", get_config_reply_transaction_id, 0 );
  rb_define_method( cGetConfigReply, "flags", get_config_reply_flags, 0 );
  rb_define_method( cGetConfigReply, "miss_send_len", get_config_reply_miss_send_len, 0 );
}


/*
 * Handler called when +OFPT_GET_CONFIG_REPLY+ message is received.
 */
void
handle_get_config_reply(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t flags,
  uint16_t miss_send_len,
  void *user_data
) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "get_config_reply" ) ) == Qfalse ) {
    return;
  }
  VALUE attributes = rb_hash_new();

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "flags" ) ), UINT2NUM( flags ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "miss_send_len" ) ), UINT2NUM( miss_send_len ) );

  VALUE get_config_reply = rb_funcall( cGetConfigReply, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "get_config_reply" ), 2, ULL2NUM( datapath_id ), get_config_reply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
