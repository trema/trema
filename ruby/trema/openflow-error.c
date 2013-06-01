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
VALUE cOpenflowError;


/*
 * The occurence of reported errors/exceptions manifested as an instance - a
 * {OpenflowError} object. The user would not explicitly instantiate
 * a {OpenflowError} but would be created while parsing the +OFPT_ERROR+ message.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     OpenflowError.new(
 *       :datapath_id => 0xabc,
 *       :transaction_id => 123,
 *       :type => OFPET_BAD_REQUEST,
 *       :code => OFPBRC_BAD_SUBTYPE,
 *       :data => data
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :datapath_id
 *     message originator identifier. This idenfier is typed as a 64-bit number
 *     and must be unique in a given domain of application deployment.
 *
 *   @option options [Number] :transaction_id
 *     the transaction_id of the offended message.
 *
 *   @option options [Number] :type
 *     the command or action that failed signifies the kind of error.
 *
 *   @option options [Number] :code
 *     the reason of the failed type error.
 *
 *   @option options [String] :data
 *     variable length data interpreted based on type and code.
 *
 *   @return [OpenflowError]
 *     an object that encapsulates the +OFPT_ERROR+ OpenFlow message.
 */
static VALUE
openflow_error_init( VALUE self, VALUE options ) {
  rb_iv_set( self, "@attribute", options );
  return self;
}


/*
 *  Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
openflow_error_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


/*
 * The transaction_id of the offended message.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
openflow_error_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


/*
 * The command or action that failed.
 *
 * @return [Number] the value of type.
 */
static VALUE
openflow_error_type( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "type" ) ) );
}


/*
 * The reason of the failed type error.
 *
 * @return [Number] the value of code.
 */
static VALUE
openflow_error_code( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "code" ) ) );
}


/*
 * Variable length data interpreted based on type and code.
 *
 * @return [String] if error type is +OFPET_HELLO_FAILED+.
 * @return [Array] an array of bytes of the offending message for any other error type.
 */
static VALUE
openflow_error_data( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "data" ) ) );
}


/*
 * Document-class: Trema::OpenflowError
 */
void
Init_openflow_error() {
  mTrema = rb_eval_string( "Trema" );
  cOpenflowError = rb_define_class_under( mTrema, "OpenflowError", rb_cObject );
  rb_define_method( cOpenflowError, "initialize", openflow_error_init, 1 );
  rb_define_method( cOpenflowError, "datapath_id", openflow_error_datapath_id, 0 );
  rb_define_method( cOpenflowError, "transaction_id", openflow_error_transaction_id, 0 );
  rb_define_method( cOpenflowError, "type", openflow_error_type, 0 );
  rb_define_method( cOpenflowError, "code", openflow_error_code, 0 );
  rb_define_method( cOpenflowError, "data", openflow_error_data, 0 );
}


void
handle_openflow_error(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t type,
  uint16_t code,
  buffer *body,
  void *user_data
) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "openflow_error" ) ) == Qfalse ) {
    return;
  }
  VALUE attributes = rb_hash_new();

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "type" ) ), UINT2NUM( type ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "code" ) ), UINT2NUM( code ) );

  switch ( type ) {
    case OFPET_HELLO_FAILED:
    case OFPET_BAD_REQUEST:
    case OFPET_BAD_ACTION:
    case OFPET_FLOW_MOD_FAILED:
    case OFPET_PORT_MOD_FAILED:
    case OFPET_QUEUE_OP_FAILED:
    {
      if ( body != NULL ) {
        if ( body->length ) {
          rb_hash_aset( attributes, ID2SYM( rb_intern( "data" ) ), rb_str_new( body->data, ( long ) body->length ) );
        }
      }
    }
      break;
    default:
      critical( "Un-handled error type ( type = %u ),", type );
      break;
  }
  VALUE openflow_error = rb_funcall( cOpenflowError, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "openflow_error" ), 2, ULL2NUM( datapath_id ), openflow_error );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
