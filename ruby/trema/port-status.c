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
#include "port.h"


extern VALUE mTrema;
VALUE cPortStatus;


/*
 * An object that is implemented to wrap the details of the +OFPT_PORT_STATUS+
 * asynchronous message. This message is sent when a state transition on any
 * physical port is detected.
 *
 * @overload initialize(options={})
 *   @example
 *     PortStatus.new(
 *       :datapath_id => 2748,
 *       :transaction_id => 0,
 *       :reason => 2,
 *       :phy_port => Port
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :datapath_id
 *     message originator identifier.
 *
 *   @option options [Number] :transaction_id
 *     unsolicited message transaction_id is zero.
 *
 *   @option options [Number] :reason
 *     the reason why this message was sent.
 *
 *   @option options [Port] :phy_port
 *     a {Port} object describing the properties of the port.
 *
 *   @return [PortStatus]
 *     an object that encapsulates the +OFPT_PORT_STATUS+ OpenFlow message.
 */
static VALUE
port_status_init( VALUE self, VALUE options ) {
  rb_iv_set( self, "@attribute", options );
  return self;
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
port_status_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


/*
 * For this asynchronous message the transaction_id is set to zero.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
port_status_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


/*
 * The reason value specifies an addition, deletion or modification to a port.
 *
 * @return [Number] the value of reason.
 */
static VALUE
port_status_reason( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "reason" ) ) );
}


/*
 * Port detailed description, state.
 *
 * @return [Port] the value of phy_port.
 */
static VALUE
port_status_phy_port( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "phy_port" ) ) );
}


void
Init_port_status() {
  cPortStatus = rb_define_class_under( mTrema, "PortStatus", rb_cObject );
  rb_define_method( cPortStatus, "initialize", port_status_init, 1 );
  rb_define_method( cPortStatus, "datapath_id", port_status_datapath_id, 0 );
  rb_define_method( cPortStatus, "transaction_id", port_status_transaction_id, 0 );
  rb_define_method( cPortStatus, "reason", port_status_reason, 0 );
  rb_define_method( cPortStatus, "phy_port", port_status_phy_port, 0 );
}

/*
 * Handler called when +OFPT_PORT_STATUS+ message is received.
 */
void
handle_port_status(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint8_t reason,
  struct ofp_phy_port phy_port,
  void *user_data
) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "port_status" ) ) == Qfalse ) {
    return;
  }
  VALUE attributes = rb_hash_new();

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "reason" ) ), UINT2NUM( reason ) );

  rb_hash_aset( attributes, ID2SYM( rb_intern( "phy_port" ) ), port_from( &phy_port ) );

  VALUE port_status = rb_funcall( cPortStatus, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "port_status" ), 1, port_status );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
