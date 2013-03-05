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


#include <openflow.h>
#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cSetConfig;


static VALUE
set_config_alloc( VALUE klass ) {
  buffer *set_config = create_set_config( get_transaction_id(), OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );
  return Data_Wrap_Struct( klass, NULL, free_buffer, set_config );
}


/*
 * A {SetConfig} object instance represents a set of attributes which allow
 * tuning the behavior of the openflow protocol in some way.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     SetConfig.new
 *     SetConfig.new(
 *       :flags => OFPC_FRAG_DROP,
 *       :miss_send_len => 256
 *     )
 *     SetConfig.new(
 *       :flags => OFPC_FRAG_DROP,
 *       :miss_send_len => 256,
 *       :transaction_id => 123
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :flags
 *     flags defaults to 0 (no special handling for IP fragments) if not specified.
 *
 *   @option options [Number] :miss_send_len
 *     miss_send_len defaults to 128 bytes if not specified.
 *
 *   @option options [Number] :transaction_id
 *     transaction_id auto-generated if not specified.
 *
 *   @raise [ArgumentError] if transaction id is not an unsigned 32-bit integer.
 *
 *   @return [SetConfig]
 *     an object that encapsulates the +OFPT_SET_CONFIG+ OpenFlow message.
 */
static VALUE
set_config_init( int argc, VALUE *argv, VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );

  uint32_t xid = get_transaction_id();
  uint16_t flags =  OFPC_FRAG_NORMAL;
  uint16_t miss_send_len = OFP_DEFAULT_MISS_SEND_LEN;
  VALUE options;

  if ( rb_scan_args( argc, argv, "01", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE flags_ruby;
    if ( ( flags_ruby = rb_hash_aref( options, ID2SYM( rb_intern( "flags" ) ) ) ) != Qnil ) {
      flags = ( uint16_t ) NUM2UINT( flags_ruby );
    }
    VALUE miss_send_len_ruby;
    if ( ( miss_send_len_ruby = rb_hash_aref( options, ID2SYM( rb_intern( "miss_send_len" ) ) ) ) != Qnil ) {
      miss_send_len = ( uint16_t ) NUM2UINT( miss_send_len_ruby );
    }
    VALUE xid_ruby;
    if ( ( xid_ruby = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) ) ) != Qnil ) {
      if ( rb_funcall( xid_ruby, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Transaction ID must be an unsigned 32-bit integer" );
      }
      xid = ( uint32_t ) NUM2UINT( xid_ruby );
    }
  }
  ( ( struct ofp_header * ) ( set_config->data ) )->xid = htonl( xid );
  ( ( struct ofp_switch_config * ) ( set_config->data ) )->flags = htons( flags );
  ( ( struct ofp_switch_config * ) ( set_config->data ) )->miss_send_len = htons( miss_send_len );
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
set_config_transaction_id( VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( set_config->data ) )->xid );
  return UINT2NUM( xid );
}


/*
 * A 2-bit value that can be set to indicate no special handling, drop or reassemble
 * IP fragments.
 *
 * @return [Number] the value of flags.
 */
static VALUE
set_config_flags( VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );
  uint16_t flags = ntohs( ( ( struct ofp_switch_config * ) ( set_config->data ) )->flags );
  return UINT2NUM( flags );
}


/*
 * The maximum number of bytes to send on flow table miss or flow destined to controller.
 *
 * @return [Number] the value of miss_send_len.
 */
static VALUE
set_config_miss_send_len( VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );
  uint16_t miss_send_len = ntohs( ( ( struct ofp_switch_config * ) ( set_config->data ) )->miss_send_len );
  return UINT2NUM( miss_send_len );
}


/*
 * Document-class: Trema::SetConfig
 */
void
Init_set_config() {
  mTrema = rb_eval_string( "Trema" );
  cSetConfig = rb_define_class_under( mTrema, "SetConfig", rb_cObject );
  rb_define_alloc_func( cSetConfig, set_config_alloc );
  rb_define_const( cSetConfig, "OFPC_FRAG_NORMAL", INT2NUM( OFPC_FRAG_NORMAL ) );
  rb_define_const( cSetConfig, "OFPC_FRAG_DROP", INT2NUM( OFPC_FRAG_DROP ) );
  rb_define_const( cSetConfig, "OFPC_FRAG_REASM", INT2NUM( OFPC_FRAG_REASM ) );
  rb_define_const( cSetConfig, "OFPC_FRAG_MASK", INT2NUM( OFPC_FRAG_MASK ) );

  rb_define_method( cSetConfig, "initialize", set_config_init, -1 );
  rb_define_method( cSetConfig, "transaction_id", set_config_transaction_id, 0 );
  rb_alias( cSetConfig, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cSetConfig, "flags", set_config_flags, 0 );
  rb_define_method( cSetConfig, "miss_send_len", set_config_miss_send_len, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
