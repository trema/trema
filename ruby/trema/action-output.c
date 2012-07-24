/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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
#include "action-common.h"


extern VALUE mTrema;
VALUE cActionOutput;
static const char *attrs[] = {
  "@port",
  "@max_len"
};


/*
 * An action to output a packet to a port.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionOutput.new( 1 )
 *     ActionOutput.new( :port => 1, :max_len => 256 )
 *     ActionOutput.new( :port => 1 )
 *     ActionOutput.new( :port => 1, :max_len => 256 )
 *
 *   @param [Hash] options
 *     the options hash to create this action class instance with.
 *
 *   @option options [Number] :port
 *     port number an index into switch's physical port list. There are also 
 *     fake output ports. For example a port number set to +OFPP_FLOOD+ would 
 *     output packets to all physical ports except input port and ports 
 *     disabled by STP.
 *
 *   @option options [Number] :max_len
 *     the maximum number of bytes from a packet to send to controller when port
 *     is set to +OFPP_CONTROLLER+. A zero length means no bytes of the packet
 *     should be sent. It defaults to 64K.
 *
 *   @raise [ArgumentError] if port is not an unsigned 16-bit integer.
 *   @raise [ArgumentError] if max_len is not an unsigned 16-bit integer.
 *
 *   @return [ActionOutput] self
 *     an object that encapsulates this action.
 */
static VALUE
action_output_init( VALUE self, VALUE options ) {
  VALUE fields = rb_ary_new();
  rb_ary_push( fields, rb_str_new2( attrs[ 0 ] + 1 ) );
  rb_ary_push( fields, rb_str_new2( attrs[ 1 ] + 1 ) );
  rb_call_super( 1, &fields );
  if ( rb_obj_is_kind_of( options, rb_cHash ) ) {
    VALUE port;
    if ( ( port = rb_hash_aref( options, ID2SYM( rb_intern( attrs[ 0 ] + 1 ) ) ) ) != Qnil ) {
      if ( rb_funcall( port, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Port must be an unsigned 16-bit integer" );
      }
      rb_iv_set( self, attrs[ 0 ], port );
    }
    else {
      rb_raise( rb_eArgError, "Port is a mandatory option" );
    }
    VALUE max_len;
    if ( ( max_len = rb_hash_aref( options, ID2SYM( rb_intern( attrs[ 1 ] + 1 ) ) ) ) != Qnil ) {
      if ( rb_funcall( max_len, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Maximum length must be an unsigned 16-bit integer" );
      }
    }
    else {
      max_len = UINT2NUM( UINT16_MAX );
    }
    rb_iv_set( self, attrs[ 1 ], max_len );
  }
  else if ( rb_obj_is_kind_of( options, rb_cInteger ) ) {
    if ( rb_funcall( options, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
      rb_raise( rb_eArgError, "Port must be an unsigned 16-bit integer" );
    }
    rb_iv_set( self, attrs[ 0 ], options );
    rb_iv_set( self, attrs[ 1 ], UINT2NUM( UINT16_MAX ) );
  }
  else {
    rb_raise( rb_eArgError, "Invalid option" );
  }
  return self;
}


/*
 * The index into switch's physical port list.
 *
 * @return [Number] the value of port.
 */
static VALUE
action_output_port( VALUE self ) {
  return rb_iv_get( self, attrs[ 0 ] );
}


/*
 * The maximum number of bytes from a packet to send to controller when port
 * is set to +OFPP_CONTROLLER+.
 *
 * @return [Number] the value of max_len.
 */
static VALUE
action_output_max_len( VALUE self ) {
  return rb_iv_get( self, attrs[ 1 ] );
}


/*
 * Appends its action(output to port) to the list of actions.
 *
 * @return [ActionOutput] self
 */
static VALUE
action_output_append( VALUE self, VALUE action_ptr ) {
  uint16_t port = ( uint16_t ) NUM2UINT( action_output_port( self ) );
  uint16_t max_len = ( uint16_t ) NUM2UINT( action_output_max_len( self ) );

  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );

  append_action_output( actions, port, max_len );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_output_inspect( VALUE self ) {
  uint16_t port = ( uint16_t ) NUM2UINT( action_output_port( self ) );
  uint16_t max_len = ( uint16_t ) NUM2UINT( action_output_max_len( self ) );

  char str[ 64 ];
  sprintf( str, "#<%s %s=%u,%s=%u>", rb_obj_classname( self ), attrs[ 0 ] + 1, port, attrs[ 1 ] + 1, max_len );
  return rb_str_new2( str );
}


void
Init_action_output() {
  rb_require( "trema/action" );
  VALUE cAction = action_base_class();
  cActionOutput = rb_define_class_under( mTrema, "ActionOutput", cAction );
  rb_define_method( cActionOutput, "initialize", action_output_init, 1 );
  rb_define_method( cActionOutput, "append", action_output_append, 1 );
  rb_define_method( cActionOutput, "inspect", action_output_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
