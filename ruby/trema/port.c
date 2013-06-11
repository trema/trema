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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cPort;


/*
 * Creates a {Port} instance that encapsulates the properties of a physical port.
 * The newly-created instance is initialized from an options hash.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     Port.new(
 *       :number => 1,
 *       :hw_addr => Mac.new( "4e:1e:9a:7a:44:be" ),
 *       :name => "trema0-0",
 *       :config => 0,
 *       :state => 0,
 *       :curr => 192,
 *       :advertised => 0,
 *       :supported => 0,
 *       :peer => 0
 *     )
 *
 *   @param [Hash] options the options hash.
 *
 *   @option options [Number] :number
 *     the port's unique number.
 *
 *   @option options [Mac] :hw_addr
 *     the port's Ethernet address expressed as a {Mac} object.
 *
 *   @option options [String] :name
 *     the port's human readable defined name.
 *
 *   @option options [Number] :config
 *     the port's configuration as a 32-bit bitmap.
 *
 *   @option options [Number] :state
 *     the port's state as a 32-bit bitmap.
 *
 *   @option options [Number] :curr
 *      the port's current features as a 32-bit bitmap.
 *
 *   @option options [Number] :advertised
 *     the port's advertised features as a 32-bit bitmap.
 *
 *   @option options [Number] :supported
 *     the port's supported features as a 32-bit bitmap.
 *
 *   @option options [Number] :peer
 *     the features advertised by the peer connected to the port as a 32-bit bitmap.
 *
 *   @return [Port]
 *     an object that encapsulates the properties of a physical port.
 */
static VALUE
port_init( VALUE self, VALUE options ) {
  VALUE number = rb_hash_aref( options, ID2SYM( rb_intern( "number" ) ) );
  rb_iv_set( self, "@number", number );

  VALUE hw_addr = rb_hash_aref( options, ID2SYM( rb_intern( "hw_addr" ) ) );
  rb_iv_set( self, "@hw_addr", hw_addr );

  VALUE name = rb_hash_aref( options, ID2SYM( rb_intern( "name" ) ) );
  rb_iv_set( self, "@name", name );

  VALUE config = rb_hash_aref( options, ID2SYM( rb_intern( "config" ) ) );
  rb_iv_set( self, "@config", config );

  VALUE state = rb_hash_aref( options, ID2SYM( rb_intern( "state" ) ) );
  rb_iv_set( self, "@state", state );

  VALUE curr = rb_hash_aref( options, ID2SYM( rb_intern( "curr" ) ) );
  rb_iv_set( self, "@curr", curr );

  VALUE advertised = rb_hash_aref( options, ID2SYM( rb_intern( "advertised" ) ) );
  rb_iv_set( self, "@advertised", advertised );

  VALUE supported = rb_hash_aref( options, ID2SYM( rb_intern( "supported" ) ) );
  rb_iv_set( self, "@supported", supported );

  VALUE peer = rb_hash_aref( options, ID2SYM( rb_intern( "peer" ) ) );
  rb_iv_set( self, "@peer", peer );
  return self;
}


/*
 * The port's unique number.
 *
 * @return [Number] the value of number.
 */
static VALUE
port_number( VALUE self ) {
  return rb_iv_get( self, "@number" );
}


/*
 * The port's Ethernet address expressed as a {Mac} object.
 *
 * @return [Mac] the value of hw_addr.
 */
static VALUE
port_hw_addr( VALUE self ) {
  return rb_iv_get( self, "@hw_addr" );
}


/*
 * The port's human readable defined name.
 *
 * @return [String] the value of name.
 */
static VALUE
port_name( VALUE self ) {
  return rb_iv_get( self, "@name" );
}


/*
 * The port's configuration as a 32-bit bitmap.
 *
 * @return [Number] the value of config.
 */
static VALUE
port_config( VALUE self ) {
  return rb_iv_get( self, "@config" );
}


/*
 * The port's state as a 32-bit bitmap.
 *
 * @return [Number] the value of state.
 */
static VALUE
port_state( VALUE self ) {
  return rb_iv_get( self, "@state" );
}


/*
 * The port's current features as a 32-bit bitmap.
 *
 * @return [Number] the value of curr.
 */
static VALUE
port_curr( VALUE self ) {
  return rb_iv_get( self, "@curr" );
}


/*
 * The port's advertised features as a 32-bit bitmap.
 *
 * @return [Number] the value of advertised.
 */
static VALUE
port_advertised( VALUE self ) {
  return rb_iv_get( self, "@advertised" );
}


/*
 * The port's supported features as a 32-bit bitmap.
 *
 * @return [Number] the value of supported.
 */
static VALUE
port_supported( VALUE self ) {
  return rb_iv_get( self, "@supported" );
}


/*
 * The features advertised by the peer connected to the port as a 32-bit bitmap.
 *
 * @return [Number] the value of peer.
 */
static VALUE
port_peer( VALUE self ) {
  return rb_iv_get( self, "@peer" );
}


/*
 * Tests if the port is up.
 *
 * @return [Boolean] true if port is up otherwise false.
 */
static VALUE
port_up( VALUE self ) {
  uint32_t config = ( uint16_t ) NUM2UINT( rb_iv_get( self, "@config" ) );
  if ( ( config & OFPPC_PORT_DOWN ) == OFPPC_PORT_DOWN ) {
    return Qfalse;
  }
  return Qtrue;
}


/*
 * Tests if the port is down.
 *
 * @return [Boolean] true if port is down otherwise false.
 */
static VALUE
port_down( VALUE self ) {
  return port_up( self ) == Qfalse ? Qtrue : Qfalse;
}


/*
 * Tests if the link is up.
 *
 * @return [Boolean] true if link is up otherwise false.
 */
static VALUE
link_up( VALUE self ) {
  uint32_t state = ( uint16_t ) NUM2UINT( rb_iv_get( self, "@state" ) );
  if ( ( state & OFPPS_LINK_DOWN ) == OFPPS_LINK_DOWN ) {
    return Qfalse;
  }
  return Qtrue;
}


/*
 * Tests if the link is down.
 *
 * @return [Boolean] true if link is down otherwise false.
 */
static VALUE
link_down( VALUE self ) {
  return link_up( self ) == Qfalse ? Qtrue : Qfalse;
}


/*
 * Tests if the link and port are both up.
 *
 * @return [Boolean] true if link is up otherwise false.
 */
static VALUE
port_and_link_up( VALUE self ) {
  if ( port_up( self ) == Qtrue && link_up( self ) == Qtrue ) {
    return Qtrue;
  }
  return Qfalse;
}


/*
 * Tests if the link or port is down.
 *
 * @return [Boolean] true if link is up otherwise false.
 */
static VALUE
port_or_link_down( VALUE self ) {
  return port_and_link_up( self ) == Qfalse ? Qtrue : Qfalse;
}


/*
 * Compares two ports by substracting their unique numbers.
 *
 * @return [Number] the result of the substraction. Zero ports are equal.
 */
static VALUE
port_compare( VALUE self, VALUE other ) {
  uint16_t a = ( uint16_t ) NUM2UINT( rb_iv_get( self, "@number" ) );
  uint16_t b = ( uint16_t ) NUM2UINT( rb_iv_get( other, "@number" ) );
  return UINT2NUM( ( uint16_t ) ( a - b ) );
}


/*
 * Document-class: Trema::Port
 */
void
Init_port() {
  mTrema = rb_eval_string( "Trema" );
  cPort = rb_define_class_under( mTrema, "Port", rb_cObject );

  rb_define_const( cPort, "OFPPC_PORT_DOWN", INT2NUM( OFPPC_PORT_DOWN ) );
  rb_define_const( cPort, "OFPPC_NO_STP", INT2NUM( OFPPC_NO_STP ) );
  rb_define_const( cPort, "OFPPC_NO_RECV", INT2NUM( OFPPC_NO_RECV ) );
  rb_define_const( cPort, "OFPPC_NO_RECV_STP", INT2NUM( OFPPC_NO_RECV_STP ) );
  rb_define_const( cPort, "OFPPC_NO_FLOOD", INT2NUM( OFPPC_NO_FLOOD ) );
  rb_define_const( cPort, "OFPPC_NO_FWD", INT2NUM( OFPPC_NO_FWD ) );
  rb_define_const( cPort, "OFPPC_NO_PACKET_IN", INT2NUM( OFPPC_NO_PACKET_IN ) );
  rb_define_const( cPort, "OFPPS_LINK_DOWN", INT2NUM( OFPPS_LINK_DOWN ) );
  rb_define_const( cPort, "OFPPS_STP_LISTEN", INT2NUM( OFPPS_STP_LISTEN ) );
  rb_define_const( cPort, "OFPPS_STP_LEARN", INT2NUM( OFPPS_STP_LEARN ) );
  rb_define_const( cPort, "OFPPS_STP_FORWARD", INT2NUM( OFPPS_STP_FORWARD ) );
  rb_define_const( cPort, "OFPPS_STP_BLOCK", INT2NUM( OFPPS_STP_BLOCK ) );
  rb_define_const( cPort, "OFPPS_STP_MASK", INT2NUM( OFPPS_STP_MASK ) );
  rb_define_const( cPort, "OFPPF_10MB_HD", INT2NUM( OFPPF_10MB_HD ) );
  rb_define_const( cPort, "OFPPF_10MB_FD", INT2NUM( OFPPF_10MB_FD ) );
  rb_define_const( cPort, "OFPPF_100MB_HD", INT2NUM( OFPPF_100MB_HD ) );
  rb_define_const( cPort, "OFPPF_100MB_FD", INT2NUM( OFPPF_100MB_FD ) );
  rb_define_const( cPort, "OFPPF_1GB_HD", INT2NUM( OFPPF_1GB_HD ) );
  rb_define_const( cPort, "OFPPF_1GB_FD", INT2NUM( OFPPF_1GB_FD ) );
  rb_define_const( cPort, "OFPPF_10GB_FD", INT2NUM( OFPPF_10GB_FD ) );
  rb_define_const( cPort, "OFPPF_COPPER", INT2NUM( OFPPF_COPPER ) );
  rb_define_const( cPort, "OFPPF_FIBER", INT2NUM( OFPPF_FIBER ) );
  rb_define_const( cPort, "OFPPF_AUTONEG", INT2NUM( OFPPF_AUTONEG ) );
  rb_define_const( cPort, "OFPPF_PAUSE", INT2NUM( OFPPF_PAUSE ) );
  rb_define_const( cPort, "OFPPF_PAUSE_ASYM", INT2NUM( OFPPF_PAUSE_ASYM ) );

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
  rb_define_method( cPort, "port_up?", port_up, 0 );
  rb_define_method( cPort, "port_down?", port_down, 0 );
  rb_define_method( cPort, "link_up?", link_up, 0 );
  rb_define_method( cPort, "link_down?", link_down, 0 );
  rb_define_method( cPort, "up?", port_and_link_up, 0 );
  rb_define_method( cPort, "down?", port_or_link_down, 0 );
  rb_define_method( cPort, "<=>", port_compare, 1 );
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
