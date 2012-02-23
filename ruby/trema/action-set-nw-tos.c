/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


extern VALUE mTrema;
VALUE cActionSetNwTos;


/*
 * An action to modify the IP ToS/DSCP field of a packet.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionSetNwTos.new( :nw_tos => 1 )
 *
 *   @param [Hash] options
 *     the options hash to create this action with.
 *
 *   @option options [Number] :nw_tos
 *     the ToS/DSCP field to set to.
 *
 *   @raise [ArgumentError] if nw_tos argument is not supplied.
 *   @raise [ArgumentError] if nw_tos is not an unsigned 8-bit integer.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionSetNwTos]
 *     an object that encapsulates this action.
 */
static VALUE
action_set_nw_tos_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "10", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE nw_tos;
    if ( ( nw_tos = rb_hash_aref( options, ID2SYM( rb_intern( "nw_tos" ) ) ) ) != Qnil ) {
      if ( rb_funcall( nw_tos, rb_intern( "unsigned_8bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Nw tos must be an unsigned 8-bit integer" );
      }
      rb_iv_set( self, "@nw_tos", nw_tos );
    }
    else {
      rb_raise( rb_eArgError, "Nw tos is a mandatory option" );
    }
  }
  return self;
}


/*
 * The ToS/DSCP value to set to.
 *
 * @return [Number] the value of nw_tos.
 */
static VALUE
action_get_nw_tos( VALUE self ) {
  return rb_iv_get( self, "@nw_tos" );
}


/*
 * Appends its action(nw_tos) to the list of actions.
 *
 * @return [ActionSetNwTos] self
 */
static VALUE
action_set_nw_tos_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  uint8_t nw_tos = ( uint8_t ) NUM2UINT( action_get_nw_tos( self ) );
  append_action_set_nw_tos( actions, nw_tos );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_set_nw_tos_inspect( VALUE self ) {
  char str[ 64 ];
  uint8_t nw_tos = ( uint8_t ) NUM2UINT( action_get_nw_tos( self ) );
  sprintf( str, "#<%s nw_tos=%u>", rb_obj_classname( self ), nw_tos );
  return rb_str_new2( str );
}


void
Init_action_set_nw_tos() {
  cActionSetNwTos = rb_define_class_under( mTrema, "ActionSetNwTos", rb_cObject );
  rb_define_method( cActionSetNwTos, "initialize", action_set_nw_tos_init, -1 );
  rb_define_method( cActionSetNwTos, "nw_tos", action_get_nw_tos, 0 );
  rb_define_method( cActionSetNwTos, "append", action_set_nw_tos_append, 1 );
  rb_define_method( cActionSetNwTos, "inspect", action_set_nw_tos_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
