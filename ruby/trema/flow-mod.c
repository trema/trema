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
#include "trema.h"
#include "trema-ruby-utils.h"


extern VALUE mTrema;
VALUE cFlowMod;


static VALUE
flow_mod_alloc( VALUE klass ) {
  struct ofp_match match;
  buffer *flow_mod = create_flow_mod(
    0,
    match,
    0,
    0,
    0,
    0,
    UINT16_MAX,
    UINT32_MAX,
    OFPP_NONE,
    0,
    NULL
  );
  return Data_Wrap_Struct( klass, NULL, free_buffer, flow_mod );
}


static VALUE
flow_mod_init( int argc, VALUE *argv, VALUE self ) {
  buffer *flow_mod = NULL;
  Data_Get_Struct( self, buffer, flow_mod );
  VALUE options = Qnil;

  if ( rb_scan_args( argc, argv, "01", &options ) == 0 ) {
    set_xid( flow_mod, get_transaction_id() );
  }
  else {
    if ( options == Qnil ) {
      set_xid( flow_mod, get_transaction_id() );
    }
    else if ( rb_obj_is_kind_of( options, rb_cInteger ) == Qtrue ) {
      validate_xid( options );
      set_xid( flow_mod, ( uint32_t ) NUM2UINT( options ) );
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

      tmp = rb_hash_aref( options, ID2SYM( rb_intern( "idle_timeout" ) ) );
      if ( tmp != Qnil ) {
        ( ( struct ofp_flow_mod * ) ( flow_mod->data ) )->idle_timeout = ( uint16_t ) NUM2UINT( tmp );
      }

      if ( xid != Qnil ) {
        validate_xid( xid );
        set_xid( flow_mod, ( uint32_t ) NUM2UINT( xid ) );
      }
      else {
        set_xid( flow_mod, get_transaction_id() );
      }
    }
  }

  return self;
}


static VALUE
flow_mod_transaction_id( VALUE self ) {
  return get_xid( self );
}


static VALUE
flow_mod_command( VALUE self ) {
  buffer *flow_mod;
  Data_Get_Struct( self, buffer, flow_mod );
  uint32_t command = ntohl( ( ( struct ofp_flow_mod * ) ( flow_mod->data ) )->command );
  return UINT2NUM( command );
}


static VALUE
flow_mod_idle_timeout( VALUE self ) {
  buffer *flow_mod;
  Data_Get_Struct( self, buffer, flow_mod );
  uint32_t idle_timeout = ntohl( ( ( struct ofp_flow_mod * ) ( flow_mod->data ) )->idle_timeout );
  return UINT2NUM( idle_timeout );
}


static VALUE
flow_mod_hard_timeout( VALUE self ) {
  buffer *flow_mod;
  Data_Get_Struct( self, buffer, flow_mod );
  uint32_t hard_timeout = ntohl( ( ( struct ofp_flow_mod * ) ( flow_mod->data ) )->hard_timeout );
  return UINT2NUM( hard_timeout );
}


void
Init_flow_mod() {
  mTrema = rb_define_module( "Trema" );
  cFlowMod = rb_define_class_under( mTrema, "FlowMod", rb_cObject );
  rb_define_alloc_func( cFlowMod, flow_mod_alloc );
  rb_define_method( cFlowMod, "initialize", flow_mod_init, -1 );
  rb_define_method( cFlowMod, "transaction_id", flow_mod_transaction_id, 0 );
  rb_define_method( cFlowMod, "command", flow_mod_command, 0 );
  rb_define_method( cFlowMod, "idle_timeout", flow_mod_idle_timeout, 0 );
  rb_define_method( cFlowMod, "hard_timeout", flow_mod_hard_timeout, 0 );
  rb_alias( cFlowMod, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
