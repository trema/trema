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
VALUE cOpenflowError;


static VALUE
openflow_error_init( VALUE self, VALUE attribute ) {
  rb_iv_set( self, "@attribute", attribute );
  return self;
}


static VALUE
openflow_error_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


static VALUE
openflow_error_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


static VALUE
openflow_error_type( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "type" ) ) );
}


static VALUE
openflow_error_code( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "code" ) ) );
}


static VALUE
openflow_error_data( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "data" ) ) );
}


void
Init_openflow_error( ) {
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
  VALUE attributes = rb_hash_new( );

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "type" ) ), UINT2NUM( type ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "code" ) ), UINT2NUM( code ) );

  switch ( type ) {
    case OFPET_HELLO_FAILED:
    {
      const char *msg;
      if ( body != NULL ) {
        if ( body->length ) {
          msg = ( char * ) body->data;
          rb_hash_aset( attributes, ID2SYM( rb_intern( "data" ) ), rb_str_new2( msg ) );
        }
      }
    }
      break;
    case OFPET_BAD_REQUEST:
    case OFPET_BAD_ACTION:
    case OFPET_FLOW_MOD_FAILED:
    case OFPET_PORT_MOD_FAILED:
    case OFPET_QUEUE_OP_FAILED:
    {
      uint32_t i;
      if ( body != NULL )
        if ( body->length ) {
          VALUE data_arr = rb_ary_new2( ( int32_t ) body->length );
          uint8_t *buf = ( uint8_t* ) body->data;
          for ( i = 0; i < body->length; i++ ) {
            rb_ary_push( data_arr, INT2FIX( buf[i] ) );
          }
          rb_hash_aset( attributes, ID2SYM( rb_intern( "data" ) ), data_arr );
        }
    }
      break;
    default:
      critical( "Unhandled error type ( type = %u ),", type );
      break;
  }
  VALUE openflow_error = rb_funcall( cOpenflowError, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "openflow_error" ), 1, openflow_error );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

