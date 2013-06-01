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


#include "ruby.h"
#include "trema-ruby-utils.h"
#include "trema.h"


extern VALUE mTrema;
VALUE cError;


static VALUE
error_alloc( VALUE klass ) {
  buffer *error = create_error( 0, 0, 0, NULL );
  return Data_Wrap_Struct( klass, NULL, free_buffer, error );
}


/*
 * @overload initialize(options)
 *   @example
 *     Error.new(
 *       :type => OFPET_BAD_REQUEST,
 *       :code => OFPBRC_BAD_TYPE,
 *     )
 *     Error.new(
 *       :type => OFPET_BAD_REQUEST,
 *       :code => OFPBRC_BAD_TYPE,
 *       :transcation_id => 123
 *     )
 *     Error.new(
 *       :type => OFPET_BAD_REQUEST,
 *       :code => OFPBRC_BAD_TYPE,
 *       :transcation_id => 123
 *       :data => "Error!!"
 *     )
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :type
 *     a command or action that failed.
 *   @option options [Number] :code
 *     the reason of the failed type error.
 *   @option options [String] :data
 *     a more user friendly explanation of the error. Defaults to nil
 *     if not specified.
 *   @option options [Number] :xid
 *   @option options [Number] :transaction_id
 *     An unsigned 32bit integer number associated with this message.
 *     If not specified, an auto-generated value is set.
 *   @raise [ArgumentError] if transaction ID is not an unsigned 32bit integer.
 *   @raise [ArgumentError] if type and code are not supplied.
 *   @raise [ArgumentError] if user data is not a string.
 *   @raise [TypeError] if options is not a hash.
 *   @return [Error]
 */
static VALUE
error_init( int argc, VALUE *argv, VALUE self ) {
  buffer *error = NULL;
  Data_Get_Struct( self, buffer, error );
  VALUE options;

  if ( rb_scan_args( argc, argv, "01", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE tmp = Qnil;

    tmp = rb_hash_aref( options, ID2SYM( rb_intern( "type" ) ) );
    if ( tmp != Qnil ) {
      ( ( struct ofp_error_msg * ) error->data )->type = htons( ( uint16_t ) NUM2UINT( tmp ) );
    }
    else {
      rb_raise( rb_eArgError, "Type is a mandatory option" );
    }

    tmp = rb_hash_aref( options, ID2SYM( rb_intern( "code" ) ) );
    if ( tmp != Qnil ) {
      ( ( struct ofp_error_msg * ) error->data )->code = htons( ( uint16_t ) NUM2UINT( tmp ) );
    }
    else {
      rb_raise( rb_eArgError, "Code is a mandatory option" );
    }

    VALUE xid = Qnil;
    tmp = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
    if ( tmp != Qnil ) {
      xid = tmp;
    }
    tmp = rb_hash_aref( options, ID2SYM( rb_intern( "xid" ) ) );
    if ( tmp != Qnil ) {
      xid = tmp;
    }
    if ( xid != Qnil ) {
      validate_xid( xid );
      set_xid( error, ( uint32_t ) NUM2UINT( xid ) );
    }
    else {
      set_xid( error, get_transaction_id() );
    }

    VALUE data = rb_hash_aref( options, ID2SYM( rb_intern( "data" ) ) );
    if ( data != Qnil ) {
      Check_Type( data, T_STRING );
      uint16_t length = ( uint16_t ) RSTRING_LEN( data );
      append_back_buffer( error, length );
      ( ( struct ofp_header * ) ( error->data ) )->length = htons( ( uint16_t ) ( offsetof( struct ofp_error_msg, data ) + length ) );
      memcpy( ( char * ) error->data + offsetof( struct ofp_error_msg, data ), RSTRING_PTR( data ), length );
    }
  }
  else {
    rb_raise( rb_eArgError, "Type and code are mandatory options" );
  }

  return self;
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
 * @return [Number] the value of transaction id.
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
error_data( VALUE self ) {
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
 * @return [Number] the value of error type.
 */
static VALUE
error_type( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );
  return UINT2NUM( ntohs( error->type ) );
}


/*
 * Reason of the failed type error.
 *
 * @return [Number] the value of error code.
 */
static VALUE
error_code( VALUE self ) {
  struct ofp_error_msg *error = get_error( self );
  return UINT2NUM( ntohs( error->code ) );
}


/*
 * Document-class: Trema::Error
 */
void
Init_error() {
  mTrema = rb_eval_string( "Trema" );
  rb_define_const( mTrema, "OFPET_HELLO_FAILED", INT2NUM( OFPET_HELLO_FAILED ) );
  rb_define_const( mTrema, "OFPHFC_INCOMPATIBLE", INT2NUM( OFPHFC_INCOMPATIBLE ) );
  rb_define_const( mTrema, "OFPHFC_EPERM", INT2NUM( OFPHFC_EPERM ) );

  rb_define_const( mTrema, "OFPET_BAD_REQUEST", INT2NUM( OFPET_BAD_REQUEST ) );
  rb_define_const( mTrema, "OFPBRC_BAD_VERSION", INT2NUM( OFPBRC_BAD_VERSION ) );
  rb_define_const( mTrema, "OFPBRC_BAD_TYPE", INT2NUM( OFPBRC_BAD_TYPE ) );
  rb_define_const( mTrema, "OFPBRC_BAD_STAT", INT2NUM( OFPBRC_BAD_STAT ) );
  rb_define_const( mTrema, "OFPBRC_BAD_VENDOR", INT2NUM( OFPBRC_BAD_VENDOR ) );
  rb_define_const( mTrema, "OFPBRC_BAD_SUBTYPE", INT2NUM( OFPBRC_BAD_SUBTYPE ) );
  rb_define_const( mTrema, "OFPBRC_EPERM", INT2NUM( OFPBRC_EPERM ) );
  rb_define_const( mTrema, "OFPBRC_BAD_LEN", INT2NUM( OFPBRC_BAD_LEN ) );
  rb_define_const( mTrema, "OFPBRC_BUFFER_EMPTY", INT2NUM( OFPBRC_BUFFER_EMPTY ) );
  rb_define_const( mTrema, "OFPBRC_BUFFER_UNKNOWN", INT2NUM( OFPBRC_BUFFER_UNKNOWN ) );

  rb_define_const( mTrema, "OFPET_BAD_ACTION", INT2NUM( OFPET_BAD_ACTION ) );
  rb_define_const( mTrema, "OFPBAC_BAD_TYPE", INT2NUM( OFPBAC_BAD_TYPE ) );
  rb_define_const( mTrema, "OFPBAC_BAD_LEN", INT2NUM( OFPBAC_BAD_LEN ) );
  rb_define_const( mTrema, "OFPBAC_BAD_VENDOR", INT2NUM( OFPBAC_BAD_VENDOR ) );
  rb_define_const( mTrema, "OFPBAC_BAD_VENDOR_TYPE", INT2NUM( OFPBAC_BAD_VENDOR_TYPE ) );
  rb_define_const( mTrema, "OFPBAC_BAD_OUT_PORT", INT2NUM( OFPBAC_BAD_OUT_PORT ) );
  rb_define_const( mTrema, "OFPBAC_BAD_ARGUMENT", INT2NUM( OFPBAC_BAD_ARGUMENT ) );
  rb_define_const( mTrema, "OFPBAC_EPERM", INT2NUM( OFPBAC_EPERM ) );
  rb_define_const( mTrema, "OFPBAC_TOO_MANY", INT2NUM( OFPBAC_TOO_MANY ) );
  rb_define_const( mTrema, "OFPBAC_BAD_QUEUE", INT2NUM( OFPBAC_BAD_QUEUE ) );

  rb_define_const( mTrema, "OFPET_FLOW_MOD_FAILED", INT2NUM( OFPET_FLOW_MOD_FAILED ) );
  rb_define_const( mTrema, "OFPFMFC_ALL_TABLES_FULL", INT2NUM( OFPFMFC_ALL_TABLES_FULL ) );
  rb_define_const( mTrema, "OFPFMFC_OVERLAP", INT2NUM( OFPFMFC_OVERLAP ) );
  rb_define_const( mTrema, "OFPFMFC_EPERM", INT2NUM( OFPFMFC_EPERM ) );
  rb_define_const( mTrema, "OFPFMFC_BAD_EMERG_TIMEOUT", INT2NUM( OFPFMFC_BAD_EMERG_TIMEOUT ) );
  rb_define_const( mTrema, "OFPFMFC_BAD_COMMAND", INT2NUM( OFPFMFC_BAD_COMMAND ) );
  rb_define_const( mTrema, "OFPFMFC_UNSUPPORTED", INT2NUM( OFPFMFC_UNSUPPORTED ) );

  rb_define_const( mTrema, "OFPET_PORT_MOD_FAILED", INT2NUM( OFPET_PORT_MOD_FAILED ) );
  rb_define_const( mTrema, "OFPPMFC_BAD_PORT", INT2NUM( OFPPMFC_BAD_PORT ) );
  rb_define_const( mTrema, "OFPPMFC_BAD_HW_ADDR", INT2NUM( OFPPMFC_BAD_HW_ADDR ) );

  rb_define_const( mTrema, "OFPET_QUEUE_OP_FAILED", INT2NUM( OFPET_QUEUE_OP_FAILED ) );
  rb_define_const( mTrema, "OFPQOFC_BAD_PORT", INT2NUM( OFPQOFC_BAD_PORT ) );
  rb_define_const( mTrema, "OFPQOFC_BAD_QUEUE", INT2NUM( OFPQOFC_BAD_QUEUE ) );
  rb_define_const( mTrema, "OFPQOFC_EPERM", INT2NUM( OFPQOFC_EPERM ) );

  cError = rb_define_class_under( mTrema, "Error", rb_cObject );

  rb_define_alloc_func( cError, error_alloc );
  rb_define_method( cError, "initialize", error_init, -1 );
  rb_define_method( cError, "transaction_id", error_transaction_id, 0 );
  rb_alias( cError, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cError, "data", error_data, 0 );
  rb_define_method( cError, "error_type", error_type, 0 );
  rb_define_method( cError, "code", error_code, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
