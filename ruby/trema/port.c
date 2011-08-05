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
VALUE cPort;


static VALUE
port_init( VALUE self, VALUE attributes ) {
  VALUE number = rb_hash_aref( attributes, ID2SYM( rb_intern( "number" ) ) );
  rb_iv_set( self, "@number", number );

  VALUE hw_addr = rb_hash_aref( attributes, ID2SYM( rb_intern( "hw_addr" ) ) );
  rb_iv_set( self, "@hw_addr", hw_addr );

  VALUE name = rb_hash_aref( attributes, ID2SYM( rb_intern( "name" ) ) );
  rb_iv_set( self, "@name", name );
  
  VALUE config = rb_hash_aref( attributes, ID2SYM( rb_intern( "config" ) ) );
  rb_iv_set( self, "@config", config );

  VALUE state = rb_hash_aref( attributes, ID2SYM( rb_intern( "state" ) ) );
  rb_iv_set( self, "@state", state );

  VALUE curr = rb_hash_aref( attributes, ID2SYM( rb_intern( "curr" ) ) );
  rb_iv_set( self, "@curr", curr );

  VALUE advertised = rb_hash_aref( attributes, ID2SYM( rb_intern( "advertised" ) ) );
  rb_iv_set( self, "@advertised", advertised );

  VALUE supported = rb_hash_aref( attributes, ID2SYM( rb_intern( "supported" ) ) );
  rb_iv_set( self, "@supported", supported );

  VALUE peer = rb_hash_aref( attributes, ID2SYM( rb_intern( "peer" ) ) );
  rb_iv_set( self, "@peer", peer );
  
  return self;
}


static VALUE
port_number( VALUE self ) {
  return rb_iv_get( self, "@number" );
}


static VALUE
port_hw_addr( VALUE self ) {
  return rb_iv_get( self, "@hw_addr" );
}


static VALUE
port_name( VALUE self ) {
  return rb_iv_get( self, "@name" );
}


static VALUE
port_config( VALUE self ) {
  return rb_iv_get( self, "@config" );
}


static VALUE
port_state( VALUE self ) {
  return rb_iv_get( self, "@state" );
}


static VALUE
port_curr( VALUE self ) {
  return rb_iv_get( self, "@curr" );
}


static VALUE
port_advertised( VALUE self ) {
  return rb_iv_get( self, "@advertised" );
}


static VALUE 
port_supported( VALUE self ) {
  return rb_iv_get( self, "@supported" );
}


static VALUE
port_peer( VALUE self ) {
  return rb_iv_get( self, "@peer" );
}


void
Init_port() {
  cPort = rb_define_class_under( mTrema, "Port", rb_cObject );
  rb_define_method( cPort, "initialize", port_init, 1 );
  rb_define_method( cPort, "number", port_number, 0 );
  rb_define_method( cPort, "hw_addr", port_hw_addr, 0 );
  rb_define_method( cPort, "name", port_name, 0 );
  rb_define_method( cPort, "config", port_config, 0 );
  rb_define_method( cPort, "state", port_state, 0 );
  rb_define_method( cPort, "curr", port_curr, 0 );
  rb_define_method( cPort, "advertised", port_advertised, 0 );
  rb_define_method( cPort, "supported", port_supported, 0 );
  rb_define_method( cPort, "peer", port_peer, 0 );
}


VALUE
port_from( const struct ofp_phy_port *phy_port ) {
  VALUE attributes = rb_hash_new();
  rb_hash_aset( attributes, ID2SYM( rb_intern( "number" ) ), UINT2NUM( phy_port->port_no ) );
  VALUE hw_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( phy_port->hw_addr ) ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "hw_addr" ) ), hw_addr );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "name" ) ), rb_str_new2( phy_port->name ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "config" ) ), UINT2NUM( phy_port->config ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "state" ) ), UINT2NUM( phy_port->state ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "curr" ) ), UINT2NUM( phy_port->curr ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "advertised" ) ), UINT2NUM( phy_port->advertised ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "supported" ) ), UINT2NUM( phy_port->supported ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "peer" ) ), UINT2NUM( phy_port->peer ) );
  return rb_funcall( cPort, rb_intern( "new" ), 1, attributes );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
