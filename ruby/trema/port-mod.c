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
#include "action-common.h"


extern VALUE mTrema;
VALUE cPortMod;


static VALUE
port_mod_alloc( VALUE kclass ) {
  buffer *port_mod;
  static const uint8_t hw_addr[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
  uint32_t config = OFPPC_PORT_DOWN;
  uint32_t mask = 1;
  uint32_t advertise = 0;

  port_mod = create_port_mod( 1, 1, hw_addr, config, mask, advertise );
  return Data_Wrap_Struct( kclass, NULL, free_buffer, port_mod );
}


/*
 * A {PortMod} instance a request object to perform operations on a physical port.
 *
 * @overload initialize(options={})
 *   @example
 *     PortMod.new(
 *       :port_no => 1,
 *       :hw_addr => "11:22:33:44:55:66",
 *       :config => 1,
 *       :mask => 1,
 *       :advertise => 0
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @param [Number] :port_no
 *     an index into datapath's ports list.
 *
 *   @param [String,Number,Trema::Mac] :hw_addr
 *     the hardware address of a port.
 *     Unique for each port. Obtained from +OFPT_FEATURES_REPLY+ message.
 *     Can be supplied as a string, number or as a Mac object.
 *
 *   @param [Number] :config
 *     a bitmap that can be set to configure a port.
 *
 *   @param [Number] :mask
 *     set the bits of the +config+ flag to change.
 *
 *   @param [Number] :advertise
 *     bitmap of +ofp_port_features+ set to zero to prevent
 *     any changes. Or can be copied from +OFPT_FEATURES_REPLY+ message.
 *
 *   @return [PortMod]
 *     an object that encapsulates the +OFPT_PORT_MOD+ OpenFlow message.
 */
static VALUE
port_mod_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "01", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    buffer *port_mod;
    Data_Get_Struct( self, buffer, port_mod );
    VALUE mac;
    VALUE hw_addr;
    uint8_t *ptr;
    uint8_t haddr[ OFP_ETH_ALEN ];

    if ( ( hw_addr = rb_hash_aref( options, ID2SYM( rb_intern( "hw_addr" ) ) ) ) != Qnil ) {
      mac = hw_addr;
      if ( rb_obj_is_kind_of( hw_addr, rb_cString ) == Qtrue ||
        rb_obj_is_kind_of( hw_addr, rb_cInteger ) == Qtrue ) {
        mac = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, hw_addr );
      }
      else if ( rb_obj_is_instance_of( hw_addr, rb_eval_string( "Trema::Mac" ) ) == Qfalse ) {
        rb_raise( rb_eArgError, "hw_addr must be a string or an integer or Mac object" );
      }
      ptr = ( uint8_t * ) dl_addr_to_a( mac, haddr );
      rb_iv_set( self, "@hw_addr", mac );
    }
    VALUE port_no;
    if ( ( port_no = rb_hash_aref( options, ID2SYM( rb_intern( "port_no" ) ) ) ) != Qnil ) {
      if ( rb_funcall( port_no, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Port number must be an unsigned 16-bit integer" );
      }
      rb_iv_set( self, "@port_no", port_no );
    }
    VALUE config;
    if ( ( config = rb_hash_aref( options, ID2SYM( rb_intern( "config" ) ) ) ) != Qnil ) {
      rb_iv_set( self, "@config", config );
    }
    VALUE  mask;
    if ( ( mask = rb_hash_aref( options, ID2SYM( rb_intern( "mask" ) ) ) ) != Qnil ) {
      rb_iv_set( self, "@mask", mask );
    }
    VALUE advertise;
    if ( ( advertise = rb_hash_aref( options, ID2SYM( rb_intern( "advertise" ) ) ) ) != Qnil ) {
      rb_iv_set( self, "@advertise", advertise );
    }
    uint32_t xid = get_transaction_id();
    rb_iv_set( self, "@transaction_id", UINT2NUM( xid ) );

    ( ( struct ofp_header * ) ( port_mod->data ) )->xid = htonl( xid );
    ( ( struct ofp_port_mod * ) ( port_mod->data ) )->port_no = htons( ( uint16_t ) NUM2UINT( port_no ) );
    memcpy( ( ( struct ofp_port_mod * ) ( port_mod->data ) )->hw_addr, ptr, OFP_ETH_ALEN );
    ( ( struct ofp_port_mod * ) ( port_mod->data ) )->config = htonl( ( uint32_t ) NUM2UINT( config ) );
    ( ( struct ofp_port_mod * ) ( port_mod->data ) )->mask = htonl( ( uint32_t ) NUM2UINT( mask ) );
    ( ( struct ofp_port_mod * ) ( port_mod->data ) )->advertise = htonl( ( uint32_t ) NUM2UINT( advertise ) );
  }
  else {
    rb_raise( rb_eArgError, "Port no, hw_addr, config, mask, advertise are mandatory options" );
  }
  return self;
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
port_mod_transaction_id( VALUE self ) {
  return rb_iv_get( self, "@transaction_id" );
}


/*
 * Port number and hardware address as a pair identify a port.
 *
 * @return [Number] the value of port_no.
 */
static VALUE
port_mod_port_no( VALUE self ) {
  return rb_iv_get( self, "@port_no" );
}


/*
 * Ethernet address converted and stored as a {Trema::Mac} object.
 *
 * @return [Mac] the value of hw_addr.
 */
static VALUE
port_mod_hw_addr( VALUE self ) {
  return rb_iv_get( self, "@hw_addr" );
}


/*
 * A port can be administratively brought down, disable flooding or packet
 * forwarding or any other options as per +ofp_port_config+. flags.
 *
 * @return [Number] the value of config.
 */
static VALUE
port_mod_config( VALUE self ) {
  return rb_iv_get( self, "@config" );
}


/*
 * Set the bitmap as per +config+ attribute.
 *
 * @return [Number] the value of mask.
 */
static VALUE
port_mod_mask( VALUE self ) {
  return rb_iv_get( self, "@mask" );
}


/*
 * Set to zero to prevent any changes.
 *
 * @return [Number] the value of advertise.
 */
static VALUE
port_mod_advertise( VALUE self ) {
  return rb_iv_get( self, "@advertise" );
}


/*
 * Document-class: Trema::PortMod
 */
void
Init_port_mod() {
  mTrema = rb_eval_string( "Trema" );
  cPortMod = rb_define_class_under( mTrema, "PortMod", rb_cObject );
  rb_define_alloc_func( cPortMod, port_mod_alloc );
  rb_define_method( cPortMod, "initialize", port_mod_init, -1 );
  rb_define_method( cPortMod, "transaction_id", port_mod_transaction_id, 0 );
  rb_alias( cPortMod, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cPortMod, "port_no", port_mod_port_no, 0 );
  rb_define_method( cPortMod, "hw_addr", port_mod_hw_addr, 0 );
  rb_define_method( cPortMod, "config", port_mod_config, 0 );
  rb_define_method( cPortMod, "mask", port_mod_mask, 0 );
  rb_define_method( cPortMod, "advertise", port_mod_advertise, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
