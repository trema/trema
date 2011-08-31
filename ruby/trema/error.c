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
VALUE cError;


/*
 * @overload initialize(transaction_id=nil, type=OFPET_HELLO_FAILED, code=OFPHFC_INCOMPATIBLE, user_data=nil)
 *
 * @param [Number] transaction_id
 *   a positive number, not recently attached to any previous pending commands to
 *   guarantee message integrity auto-generated if not specified.
 *
 * @param [Number] type
 *   a command or action that failed. Defaults to +OFPET_HELLO_FAILED+ if 
 *   not specified.
 *
 * @param [Number] code
 *   the reason of the failed type error. Defaults to +OFPHFC_INCOMPATIBLE+ if 
 *   not specified.
 *
 * @param [String] user_data
 *   a more user friendly explanation of the error. Defaults to nil if not 
 *   specified.
 *
 * @example Instantiate with type and code
 *   Error.new(OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE)
 *
 * @example Instantiate with transaction_id, type and code.
 *   Error.new(1234, OFPET_BAD_ACTION, OFPBAC_BAD_VENDOR)
 *
 * @example Instantiate with transaction_id, type, code, user_data
 *   Error.new(6789, OFPET_FLOW_MOD_FAILED, OFPFMFC_BAD_EMERG_TIMEOUT, "this is a test") 
 *
 * @raise [ArgumentError] if transaction id is negative.
 * @raise [ArgumentError] if user data is not a string.
 *
 * @return [Error] 
 *   an object that encapsulates the +OFPT_ERROR+ openflow message.
 */
static VALUE
error_new( int argc, VALUE *argv, VALUE klass ) {
  buffer *data = NULL;
  uint32_t xid = get_transaction_id();
  VALUE xid_r;
  VALUE user_data;
  VALUE type_r;
  VALUE code_r;
  uint16_t type;
  uint16_t code;

  switch ( argc ) {
    case 2:
      // type, code specified.
      rb_scan_args( argc, argv, "02", &type_r, &code_r );
      type = ( uint16_t ) NUM2UINT( type_r );
      code = ( uint16_t ) NUM2UINT( code_r );
      break;
    case 3:
      // transaction id, type, code specified.
      rb_scan_args( argc, argv, "03", &xid_r, &type_r, &code_r );
      if ( NUM2INT( xid_r ) < 0 ) {
        rb_raise( rb_eArgError, "Transaction ID must be >= 0" );
      }
      xid = ( uint32_t ) NUM2UINT( xid_r );
      type = ( uint16_t ) NUM2UINT( type_r );
      code = ( uint16_t ) NUM2UINT( code_r );
      break;
    case 4:
      rb_scan_args( argc, argv, "04", &xid_r, &type_r, &code_r, &user_data );
      if ( NUM2INT( xid_r ) < 0 ) {
        rb_raise( rb_eArgError, "Transaction ID must be >= 0" );
      }
      if ( rb_obj_is_kind_of( user_data, rb_cString ) == Qfalse ) {
        rb_raise( rb_eArgError, "User data must be a string" );
      }
      xid = ( uint32_t ) NUM2UINT( xid_r );
      type = ( uint16_t ) NUM2UINT( type_r );
      code = ( uint16_t ) NUM2UINT( code_r );
      uint16_t length = ( u_int16_t ) RSTRING_LEN( user_data );
      data = alloc_buffer_with_length( length );
      void *p = append_back_buffer( data, length );
      memcpy( p, RSTRING_PTR( user_data ), length );
      break;
    default:
      type = OFPET_HELLO_FAILED;
      code = OFPHFC_INCOMPATIBLE;
      break;
  }
  buffer *error = create_error( xid, type, code, data );
  return Data_Wrap_Struct( klass, NULL, free_buffer, error );
}


static struct ofp_error_msg *
get_error( VALUE self ) {
  buffer *error;
  Data_Get_Struct( self, buffer, error );
  return ( struct ofp_error_msg * ) error->data;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of attribute transaction id.
 */
static VALUE
error_transaction_id( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );
  uint32_t xid = ntohl( error->header.xid );
  return UINT2NUM( xid );
}


/*
 * An optional user data payload field, possibly detailed explanation of the error.
 *
 * @return [String] user data payload is set.
 * @return [nil] user data payload is not set.
 */
static VALUE
error_user_data( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );
  long length = ( long ) ( ntohs( error->header.length ) - sizeof( struct ofp_error_msg ) );
  if ( length > 0 ) {
    return rb_str_new( ( char * ) error->data, length );
  }
  else {
    return Qnil;
  }
}


/*
 * Indicates the command or action that failed.
 *
 * @return [Number] the value of attribute error type.
 */
static VALUE
error_type( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );
  return UINT2NUM( ntohs( error->type ) );
}


/*
 * Reason of the failed type error.
 *
 * @return [Number] the value of attribute error code.
 */
static VALUE
error_code( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );
  return UINT2NUM( ntohs( error->code ) );
}


void
Init_error() {
  cError = rb_define_class_under( mTrema, "Error", rb_cObject );
  rb_define_singleton_method( cError, "new", error_new, -1 );
  rb_define_const( cError, "OFPET_HELLO_FAILED", INT2NUM( OFPET_HELLO_FAILED ) );
  rb_define_const( cError, "OFPET_BAD_REQUEST", INT2NUM( OFPET_BAD_REQUEST ) );
  rb_define_const( cError, "OFPET_BAD_ACTION", INT2NUM( OFPET_BAD_ACTION ) );
  rb_define_const( cError, "OFPET_FLOW_MOD_FAILED", INT2NUM( OFPET_FLOW_MOD_FAILED ) );
  rb_define_const( cError, "OFPET_PORT_MOD_FAILED", INT2NUM( OFPET_PORT_MOD_FAILED ) );
  rb_define_const( cError, "OFPET_QUEUE_OP_FAILED", INT2NUM( OFPET_QUEUE_OP_FAILED ) );

  rb_define_const( cError, "OFPHFC_INCOMPATIBLE", INT2NUM( OFPHFC_INCOMPATIBLE ) );
  rb_define_const( cError, "OFPHFC_EPERM", INT2NUM( OFPHFC_EPERM ) );

  rb_define_const( cError, "OFPBRC_BAD_VERSION", INT2NUM( OFPBRC_BAD_VERSION ) );
  rb_define_const( cError, "OFPBRC_BAD_TYPE", INT2NUM( OFPBRC_BAD_TYPE ) );
  rb_define_const( cError, "OFPBRC_BAD_STAT", INT2NUM( OFPBRC_BAD_STAT ) );
  rb_define_const( cError, "OFPBRC_BAD_VENDOR", INT2NUM( OFPBRC_BAD_VENDOR ) );
  rb_define_const( cError, "OFPBRC_BAD_SUBTYPE", INT2NUM( OFPBRC_BAD_SUBTYPE ) );
  rb_define_const( cError, "OFPBRC_EPERM", INT2NUM( OFPBRC_EPERM ) );
  rb_define_const( cError, "OFPBRC_BAD_LEN", INT2NUM( OFPBRC_BAD_LEN ) );
  rb_define_const( cError, "OFPBRC_BUFFER_EMPTY", INT2NUM( OFPBRC_BUFFER_EMPTY ) );
  rb_define_const( cError, "OFPBRC_BUFFER_UNKNOWN", INT2NUM( OFPBRC_BUFFER_UNKNOWN ) );

  rb_define_const( cError, "OFPBAC_BAD_TYPE", INT2NUM( OFPBAC_BAD_TYPE ) );
  rb_define_const( cError, "OFPBAC_BAD_LEN", INT2NUM( OFPBAC_BAD_LEN ) );
  rb_define_const( cError, "OFPBAC_BAD_VENDOR", INT2NUM( OFPBAC_BAD_VENDOR ) );
  rb_define_const( cError, "OFPBAC_BAD_VENDOR_TYPE", INT2NUM( OFPBAC_BAD_VENDOR_TYPE ) );
  rb_define_const( cError, "OFPBAC_BAD_OUT_PORT", INT2NUM( OFPBAC_BAD_OUT_PORT ) );
  rb_define_const( cError, "OFPBAC_BAD_ARGUMENT", INT2NUM( OFPBAC_BAD_ARGUMENT ) );
  rb_define_const( cError, "OFPBAC_EPERM", INT2NUM( OFPBAC_EPERM ) );
  rb_define_const( cError, "OFPBAC_TOO_MANY", INT2NUM( OFPBAC_TOO_MANY ) );
  rb_define_const( cError, "OFPBAC_BAD_QUEUE", INT2NUM( OFPBAC_BAD_QUEUE ) );

  rb_define_const( cError, "OFPFMFC_ALL_TABLES_FULL", INT2NUM( OFPFMFC_ALL_TABLES_FULL ) );
  rb_define_const( cError, "OFPFMFC_OVERLAP", INT2NUM( OFPFMFC_OVERLAP ) );
  rb_define_const( cError, "OFPFMFC_EPERM", INT2NUM( OFPFMFC_EPERM ) );
  rb_define_const( cError, "OFPFMFC_BAD_EMERG_TIMEOUT", INT2NUM( OFPFMFC_BAD_EMERG_TIMEOUT ) );
  rb_define_const( cError, "OFPFMFC_BAD_COMMAND", INT2NUM( OFPFMFC_BAD_COMMAND ) );
  rb_define_const( cError, "OFPFMFC_UNSUPPORTED", INT2NUM( OFPFMFC_UNSUPPORTED ) );

  rb_define_const( cError, "OFPPMFC_BAD_PORT", INT2NUM( OFPPMFC_BAD_PORT ) );
  rb_define_const( cError, "OFPPMFC_BAD_HW_ADDR", INT2NUM( OFPPMFC_BAD_HW_ADDR ) );
  
  rb_define_const( cError, "OFPQOFC_BAD_PORT", INT2NUM(OFPQOFC_BAD_PORT));
  rb_define_const( cError, "OFPQOFC_BAD_QUEUE", INT2NUM(OFPQOFC_BAD_QUEUE));
  rb_define_const( cError, "OFPQOFC_EPERM", INT2NUM(OFPQOFC_EPERM));

  rb_define_method( cError, "transaction_id", error_transaction_id, 0 );
  rb_define_method( cError, "user_data", error_user_data, 0 );
  rb_define_method( cError, "error_type", error_type, 0 );
  rb_define_method( cError, "code", error_code, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
