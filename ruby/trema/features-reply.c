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


#include <string.h>
#include "trema.h"
#include "ruby.h"
#include "port.h"


extern VALUE mTrema;
VALUE cFeaturesReply;


/* 
 * A user would not explicitly instantiate a {FeaturesReply} object but would be 
 * created while parsing the +OFPT_FEATURES_REPLY+ message.
 *
 * @overload initialize(options={})
 *   @example
 *     FeaturesReply.new( 
 *       :datapath_id => 0xabc,
 *       :transaction_id => 1,
 *       :n_buffers => 256,
 *       :n_tables => 1,
 *       :capabilities => 135,
 *       :actions => 2048,
 *       :port => [ Trema::Port ]
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :datapath_id
 *     datapath unique id. Subsequent commands directed to switch should 
 *     embed this id.
 *
 *   @option options [Number] :transaction_id
 *     a positive number lower layers match this to ensure message integrity.
 *
 *   @option options [Number] :n_buffers
 *     maximum number of packets that can be buffered at once.
 *
 *   @option options [Number] :n_tables
 *     number of supported tables, number could vary according to 
 *     switch's implementation.
 *
 *   @option options [Number] :capabilities
 *     supported capabilities expressed as a 32-bit bitmap. Ability of a switch 
 *     to respond or perform a certain function for example flow statistics, 
 *     IP address lookup in APR packets.
 *
 *   @option options [Number] :actions
 *     supported actions expressed as a 32-bit bitmap.
 *
 *   @option options [Port] :port
 *     an array of {Port} objects detailing physical port description and function.
 *
 *   @return [FeaturesReply]
 *     an object that encapsulates the +OFPT_FEATURES_REPLY+ OpenFlow message.
 */
static VALUE
features_reply_init( VALUE self, VALUE options ) {
  rb_iv_set( self, "@attribute", options );
  return self;
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id
 */
static VALUE
features_reply_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction id.
 */
static VALUE
features_reply_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


/*
 * Maximum number of packets that can be buffered at once.
 *
 * @return [Number] the value of n_buffers.
 */
static VALUE
features_reply_n_buffers( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "n_buffers" ) ) );
}


/*
 * Number of supported tables.
 *
 * @return [Number] the value of n_tables.
 */
static VALUE
features_reply_n_tables( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "n_tables" ) ) );
}


/*
 * Supported capabilities expressed as a 32-bit bitmap.
 *
 * @return [Number] the value of capabilities.
 */
static VALUE
features_reply_capabilities( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "capabilities" ) ) );
}


/*
 * Supported actions expressed as a 32-bit bitmap.
 *
 * @return [Number] the value of actions.
 */
static VALUE
features_reply_actions( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "actions" ) ) );
}


/*
 * An array of {Port} objects detailing physical port description and function.
 *
 * @return [Array<Port>] the value of ports.
 */
static VALUE
features_reply_ports( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "ports" ) ) );
}


void
Init_features_reply() {
  cFeaturesReply = rb_define_class_under( mTrema, "FeaturesReply", rb_cObject );
  rb_define_method( cFeaturesReply, "initialize", features_reply_init, 1 );
  rb_define_method( cFeaturesReply, "datapath_id", features_reply_datapath_id, 0 );
  rb_define_method( cFeaturesReply, "transaction_id", features_reply_transaction_id, 0 );
  rb_define_method( cFeaturesReply, "n_buffers", features_reply_n_buffers, 0 );
  rb_define_method( cFeaturesReply, "n_tables", features_reply_n_tables, 0 );
  rb_define_method( cFeaturesReply, "capabilities", features_reply_capabilities, 0 );
  rb_define_method( cFeaturesReply, "actions", features_reply_actions, 0 );
  rb_define_method( cFeaturesReply, "ports", features_reply_ports, 0 );
}


/*
 * Extract and map {Port} to +ofp_phy_port+ structure.
 */
static VALUE
ports_from( const list_element *phy_ports ) {
  VALUE ports = rb_ary_new();
  
  list_element *port_head = xmalloc( sizeof( list_element ) );
  memcpy( port_head, phy_ports, sizeof( list_element ) );
  list_element *port = NULL;
  for ( port = port_head; port != NULL; port = port->next ) {
    rb_ary_push( ports, port_from( ( struct ofp_phy_port * ) port->data ) );
  }
  xfree( port_head );
  return ports;
}


void
handle_switch_ready( uint64_t datapath_id, void *controller ) {
  if ( rb_respond_to( ( VALUE ) controller, rb_intern( "switch_ready" ) ) == Qtrue ) {
    rb_funcall( ( VALUE ) controller, rb_intern( "switch_ready" ), 1, ULL2NUM( datapath_id ) );
  }
}

/*
 * The handler that is called when an +OFPT_FEATURES_REPLY+ message is received.
 */
void
handle_features_reply(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t n_buffers,
  uint8_t n_tables,
  uint32_t capabilities,
  uint32_t actions,
  const list_element *phy_ports,
  void *controller
) {
  if ( rb_respond_to( ( VALUE ) controller, rb_intern( "features_reply" ) ) == Qfalse ) {
    return;
  }

  VALUE attributes = rb_hash_new();
  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "n_buffers" ) ), UINT2NUM( n_buffers ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "n_tables" ) ), UINT2NUM( n_tables ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "capabilities" ) ), UINT2NUM( capabilities ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "actions" ) ), UINT2NUM( actions ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "ports" ) ), ports_from( phy_ports ) );

  VALUE features_reply = rb_funcall( cFeaturesReply, rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "features_reply" ), 1, features_reply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
