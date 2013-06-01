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


VALUE
echo_init( int argc, VALUE *argv, VALUE self ) {
  buffer *echo = NULL;
  Data_Get_Struct( self, buffer, echo );
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    set_xid( echo, get_transaction_id() );
  }
  else {
    if ( options == Qnil ) {
      set_xid( echo, get_transaction_id() );
    }
    else if ( rb_obj_is_kind_of( options, rb_cInteger ) == Qtrue ) {
      validate_xid( options );
      set_xid( echo, ( uint32_t ) NUM2UINT( options ) );
    }
    else {
      Check_Type( options, T_HASH );
      VALUE tmp = Qnil;
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
        set_xid( echo, ( uint32_t ) NUM2UINT( xid ) );
      }
      else {
        set_xid( echo, get_transaction_id() );
      }

      VALUE user_data = rb_hash_aref( options, ID2SYM( rb_intern( "user_data" ) ) );
      if ( user_data != Qnil ) {
        Check_Type( user_data, T_STRING );
        uint16_t length = ( uint16_t ) RSTRING_LEN( user_data );
        append_back_buffer( echo, length );
        set_length( echo, ( uint16_t ) ( sizeof( struct ofp_header ) + length ) );
        memcpy( ( char * ) echo->data + sizeof( struct ofp_header ), RSTRING_PTR( user_data ), length );
      }
    }
  }

  return self;
}


VALUE
echo_transaction_id( VALUE self ) {
  return get_xid( self );
}


VALUE
echo_user_data( VALUE self ) {
  buffer *echo;
  Data_Get_Struct( self, buffer, echo );
  if ( echo->length > sizeof( struct ofp_header ) ) {
    return rb_str_new( ( char * ) echo->data + sizeof( struct ofp_header ),
                       ( long ) ( echo->length - sizeof( struct ofp_header ) ) );
  }
  return Qnil;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
