/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
VALUE cBuffer;


static VALUE
buffer_alloc( VALUE klass ) {
  buffer *buf = alloc_buffer();
  return Data_Wrap_Struct( klass, NULL, free_buffer, buf );
}


static VALUE
buffer_length( VALUE self ) {
  buffer *buf;
  Data_Get_Struct( self, buffer, buf );
  return INT2FIX( buf->length );
}


static VALUE
buffer_to_a( VALUE self ) {
  buffer *buf;
  Data_Get_Struct( self, buffer, buf );
  if ( buf->length == 0 ) {
    return Qnil;
  }

  VALUE new = rb_ary_new2( ( long ) buf->length );
  uint8_t *data = buf->data;
  size_t i;
  for ( i = 0; i < buf->length; i++ ) {
      rb_ary_push( new, INT2FIX( data[ i ] ) );
  }
  return new;
}


static VALUE
buffer_to_s( VALUE self ) {
  buffer *buf;
  Data_Get_Struct( self, buffer, buf );
  if ( buf->length == 0 ) {
    return Qnil;
  }
  return rb_str_new( ( char * ) buf->data, ( long int ) buf->length );
}


static VALUE
buffer_init( int argc, VALUE *argv, VALUE klass ) {
  buffer *buf;
  Data_Get_Struct( klass, buffer, buf );
  VALUE args;
  rb_scan_args( argc, argv, "0*", &args );
  int i;
  for ( i = 0; i < RARRAY_LEN( args ); i++ ) {
    VALUE data = RARRAY_PTR( args )[ i ];
    Check_Type( data, T_STRING );
    uint16_t length = ( u_int16_t ) RSTRING_LEN( data );
    void *p = append_back_buffer( buf, length );
    memcpy( p, RSTRING_PTR( data ), length );
  }
  return klass;
}


void
Init_buffer() {
  cBuffer = rb_define_class_under( mTrema, "Buffer", rb_cObject );
  rb_define_alloc_func( cBuffer, buffer_alloc );
  rb_define_method( cBuffer, "initialize", buffer_init, -1 );
  rb_define_method( cBuffer, "length", buffer_length, 0 );
  rb_define_method( cBuffer, "size", buffer_length, 0 );
  rb_define_method( cBuffer, "to_a", buffer_to_a, 0 );
  rb_define_method( cBuffer, "to_s", buffer_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
