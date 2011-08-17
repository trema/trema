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
 * @overload Error.new( )
 *   Create instance with no arguments.
 *   Create an {Error} object with auto-generated transaction id,
 *   error type set to {OFPET_HELLO_FAILED} and error code set to OFPHFC_INCOMPATIBLE
 *   with no user data payload.
 *
 * @overload Error.new( type, code )
 *   Create instance by specifying its error type and code.
 *   @example
 *     error = Error.new(OFPET_BAD_REQUEST, ERROR_CODES[ OFPET_BAD_REQUEST ][ 1 ])
 *
 * @overload Error.new( transaction_id, type, code )
 *   Create instance by specifying its transaction id, its error type and code.
 *   @example
 *     error = Error.new( 1234, OFPET_BAD_ACTION, ERROR_CODES[ OFPET_BAD_ACTION ][ 2 ] )
 *
 * @overload Error.new( transaction_id, type, code, user_data )
 *   Create instance by specifying all its arguments.
 *   @example
 *     error = Error.new( 6789, OFPET_FLOW_MOD_FAILED, ERROR_CODES[ OFPET_FLOW_MOD_FAILED ][ 3 ], "this is a test" ) 
 *
 * @return [Error] an object that encapsulates the OFPT_ERROR Openflow message.
 */
static VALUE
error_new( int argc, VALUE *argv, VALUE klass ) {
  buffer *error;
  buffer *data = NULL;

  VALUE xid_r, user_data;
  VALUE type_r, code_r;
  uint32_t xid;
  uint16_t type, code;

  xid = get_transaction_id( );
  switch ( argc ) {
    case 2:
      /* 
       * type, code specified.
       */
      rb_scan_args( argc, argv, "02", &type_r, &code_r );
      type = ( uint16_t ) NUM2UINT( type_r );
      code = ( uint16_t ) NUM2UINT( code_r );
      break;
    case 3:
      /*
       * transaction id, type, code specified.
       */
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
  error = create_error( xid, type, code, data );
  return Data_Wrap_Struct( klass, NULL, free_buffer, error );
}


static struct ofp_error_msg *
get_error( VALUE self ) {
  buffer *error;

  Data_Get_Struct( self, buffer, error );
  return (struct ofp_error_msg *) error->data;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 * @return [Number] the value of attribute transaction id.
 */
static VALUE
error_transaction_id( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );

  uint32_t xid = ntohl( error->header.xid );
  return UINT2NUM( xid );
}


/*
 * An optional user data payload field, possibly detailed description of the error.
 * @return [String] a user data payload is set.
 * @return [nil] a user data payload is not set.
 */
static VALUE
error_user_data( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );

  if ( ntohs( error->header.length ) > sizeof ( struct ofp_error_msg ) ) {
    return rb_str_new2( ( char * ) error->data );
  } else {
    return Qnil;
  }
}


/*
 * A type uniquely identifies the error.
 * @return [Number] the value of attribute error type.
 */
static VALUE
error_type( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );

  return UINT2NUM( ntohs( error->type ) );
}


/*
 * Associated with the type, the code further signifies the error.
 * @return [Number] the value of attribute error code.
 */
static VALUE
error_code( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );

  return UINT2NUM( ntohs( error->code ) );
}


void
Init_error( ) {
  cError = rb_define_class_under( mTrema, "Error", rb_cObject );
  rb_define_singleton_method( cError, "new", error_new, -1 );
  rb_define_const( cError, "OFPET_HELLO_FAILED", INT2NUM( OFPET_HELLO_FAILED ) );
  rb_define_const( cError, "OFPET_BAD_REQUEST", INT2NUM( OFPET_BAD_REQUEST ) );
  rb_define_const( cError, "OFPET_BAD_ACTION", INT2NUM( OFPET_BAD_ACTION ) );
  rb_define_const( cError, "OFPET_FLOW_MOD_FAILED", INT2NUM( OFPET_FLOW_MOD_FAILED ) );
  rb_define_const( cError, "OFPET_PORT_MOD_FAILED", INT2NUM( OFPET_PORT_MOD_FAILED ) );
  rb_define_const( cError, "OFPET_QUEUE_OP_FAILED", INT2NUM( OFPET_QUEUE_OP_FAILED ) );
  VALUE error_code_hash = rb_hash_new( );
  rb_hash_aset( error_code_hash, INT2NUM( OFPET_HELLO_FAILED ), rb_ary_new3( 2,
    INT2NUM( OFPHFC_INCOMPATIBLE ),
    INT2NUM( OFPHFC_EPERM ) ) );

  rb_hash_aset( error_code_hash, INT2NUM( OFPET_BAD_REQUEST ), rb_ary_new3( 9,
    INT2NUM( OFPBRC_BAD_VERSION ),
    INT2NUM( OFPBRC_BAD_TYPE ),
    INT2NUM( OFPBRC_BAD_STAT ),
    INT2NUM( OFPBRC_BAD_VENDOR ),
    INT2NUM( OFPBRC_BAD_SUBTYPE ),
    INT2NUM( OFPBRC_EPERM ),
    INT2NUM( OFPBRC_BAD_LEN ),
    INT2NUM( OFPBRC_BUFFER_EMPTY ),
    INT2NUM( OFPBRC_BUFFER_UNKNOWN ) ) );

  rb_hash_aset( error_code_hash, INT2NUM( OFPET_BAD_ACTION ), rb_ary_new3( 9,
    INT2NUM( OFPBAC_BAD_TYPE ),
    INT2NUM( OFPBAC_BAD_LEN ),
    INT2NUM( OFPBAC_BAD_VENDOR ),
    INT2NUM( OFPBAC_BAD_VENDOR_TYPE ),
    INT2NUM( OFPBAC_BAD_OUT_PORT ),
    INT2NUM( OFPBAC_BAD_ARGUMENT ),
    INT2NUM( OFPBAC_EPERM ),
    INT2NUM( OFPBAC_TOO_MANY ),
    INT2NUM( OFPBAC_BAD_QUEUE ) ) );

  rb_hash_aset( error_code_hash, INT2NUM( OFPET_FLOW_MOD_FAILED ), rb_ary_new3( 6,
    INT2NUM( OFPFMFC_ALL_TABLES_FULL ),
    INT2NUM( OFPFMFC_OVERLAP ),
    INT2NUM( OFPFMFC_EPERM ),
    INT2NUM( OFPFMFC_BAD_EMERG_TIMEOUT ),
    INT2NUM( OFPFMFC_BAD_COMMAND ),
    INT2NUM( OFPFMFC_UNSUPPORTED ) ) );

  rb_hash_aset( error_code_hash, INT2NUM( OFPET_PORT_MOD_FAILED ), rb_ary_new3( 2,
    INT2NUM( OFPPMFC_BAD_PORT ),
    INT2NUM( OFPPMFC_BAD_HW_ADDR ) ) );

  rb_hash_aset( error_code_hash, INT2NUM( OFPET_QUEUE_OP_FAILED ), rb_ary_new3( 3,
    INT2NUM( OFPQOFC_BAD_PORT ),
    INT2NUM( OFPQOFC_BAD_QUEUE ),
    INT2NUM( OFPQOFC_EPERM ) ) );
  /*
   * error codes stored as an array of numeric values accessed from a hash 
   * whose key is the error type.
   *
   * @return [ERROR_CODES{type => Array<codes>}] the mapping of error types to codes.
   */  
  rb_define_const( cError, "ERROR_CODES", error_code_hash );
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
