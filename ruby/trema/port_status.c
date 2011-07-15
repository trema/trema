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
VALUE cPortStatus;

static VALUE
port_status_init( VALUE self, VALUE attribute ) {
  rb_iv_set( self, "@attribute", attribute );
  return self;
}

static VALUE
port_status_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}

static VALUE
port_status_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}

static VALUE
port_status_reason( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "reason" ) ) );
}

static VALUE
port_status_phy_port( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "phy_port" ) ) );
}

void
Init_port_status( ) {
  cPortStatus = rb_define_class_under( mTrema, "PortStatus", rb_cObject );
  rb_define_method( cPortStatus, "initialize", port_status_init, 1 );
  rb_define_method( cPortStatus, "datapath_id", port_status_datapath_id, 0 );
  rb_define_method( cPortStatus, "transaction_id", port_status_transaction_id, 0 );
  rb_define_method( cPortStatus, "reason", port_status_reason, 0 );
  rb_define_method( cPortStatus, "phy_port", port_status_phy_port, 0 );
}

void
handle_port_status(
        uint64_t datapath_id,
        uint32_t transaction_id,
        uint8_t reason,
        struct ofp_phy_port phy_port,
        void *controller
        ) {
  VALUE attributes = rb_hash_new( );
  VALUE PhyPort, r_phy_port;
  VALUE hw_addr;

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "reason" ) ), UINT2NUM( reason ) );

  hw_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( phy_port.hw_addr ) ) );

  PhyPort = rb_eval_string( "Struct.new(:port_no, :hw_addr, :name, :config, :state, :curr, :advertised, :supported, :peer)" );
  r_phy_port = rb_funcall( PhyPort, rb_intern( "new" ), 9,
          UINT2NUM( phy_port.port_no ),
          hw_addr,
          rb_str_new2( phy_port.name ),
          UINT2NUM( phy_port.config ),
          UINT2NUM( phy_port.state ),
          UINT2NUM( phy_port.curr ),
          UINT2NUM( phy_port.advertised ),
          UINT2NUM( phy_port.supported ),
          UINT2NUM( phy_port.peer ) );

  rb_hash_aset( attributes, ID2SYM( rb_intern( "phy_port" ) ), r_phy_port );

  VALUE port_status = rb_funcall( cPortStatus, rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "port_status" ), 1, port_status );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

