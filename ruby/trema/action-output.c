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
VALUE cActionOutput;


/*
 *  call-seq:
 *    ao = ActionOutput.new( 1 ) specify only port number.
 *    ao = ActionOutput.new( 1, 32 ) specify port and maximum length to send to 
 *    controller.
 * 
 */
static VALUE
action_output_init( int argc, VALUE *argv, VALUE self ) {
  VALUE port;
  VALUE max_len = Qnil;
  
  if ( !argc ) {
    rb_raise( rb_eArgError, "Port is a mandatory option." );
  }
  rb_scan_args( argc, argv, "11", &port, &max_len );
  rb_iv_set( self, "@port", port );
  if ( max_len == Qnil ) {
    max_len = UINT2NUM( UINT16_MAX );
  } 
  rb_iv_set( self, "@max_len", max_len );
  return self;
}


static VALUE
action_output_port( VALUE self ) {
  return rb_iv_get( self, "@port" );
}


static VALUE
action_output_max_len( VALUE self ) {
  return rb_iv_get( self, "@max_len" );
}


static VALUE
action_output_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  uint16_t port = ( uint16_t ) NUM2UINT( action_output_port( self ) );
  uint16_t max_len = ( uint16_t ) NUM2UINT( action_output_max_len( self ) );

  Data_Get_Struct( action_ptr, openflow_actions, actions );

  append_action_output( actions, port, max_len );
  return self;
}


static VALUE
action_output_to_s( VALUE self ) {
  char str[ 64 ];
  uint16_t port = ( uint16_t ) NUM2UINT( action_output_port( self ) );
  uint16_t max_len = ( uint16_t ) NUM2UINT( action_output_max_len( self ) );

  sprintf( str, "#<%s> port = %u, max_len = %u", rb_obj_classname( self ), port, max_len );
  return rb_str_new2( str );
}


void
Init_action_output( ) {
  cActionOutput = rb_define_class_under( mTrema, "ActionOutput", rb_cObject );
  rb_define_method( cActionOutput, "initialize", action_output_init, -1 );
  rb_define_method( cActionOutput, "port", action_output_port, 0 );
  rb_define_method( cActionOutput, "max_len", action_output_max_len, 0 );
  rb_define_method( cActionOutput, "append", action_output_append, 1 );
  rb_define_method( cActionOutput, "to_s", action_output_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
