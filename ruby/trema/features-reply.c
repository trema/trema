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


#include <string.h>
#include "trema.h"
#include "ruby.h"
#include "port.h"


extern VALUE mTrema;
VALUE cFeaturesReply;


static VALUE
features_reply_alloc( VALUE klass ) {
  buffer *features_reply = create_features_reply( 0, 0, 0, 0, 0, 0, NULL );
  return Data_Wrap_Struct( klass, NULL, free_buffer, features_reply );
}


/*
 * Creates a FeaturesReply message. A user would not explicitly
 * instantiate a {FeaturesReply} object but would be created while
 * parsing the +OFPT_FEATURES_REPLY+ message.
 *
 * @overload initialize(options)
 *   @example
 *     FeaturesReply.new(
 *       :datapath_id => 0xabc,
 *       :transaction_id => 1,
 *       :n_buffers => 256,
 *       :n_tables => 1,
 *       :capabilities => 135,
 *       :actions => 2048,
 *       :ports => [ port1, port2, ... ]
 *     )
 *   @param [Hash] options
 *     the options to create a message with.
 *   @option options [Number] :datapath_id
 *     datapath unique id. Subsequent commands directed to switch should
 *     embed this id.
 *   @option options [Number] :transaction_id
 *     a positive number lower layers match this to ensure message integrity.
 *   @option options [Number] :n_buffers
 *     maximum number of packets that can be buffered at once.
 *   @option options [Number] :n_tables
 *     number of supported tables, number could vary according to
 *     switch's implementation.
 *   @option options [Number] :capabilities
 *     supported capabilities expressed as a 32-bit bitmap. Ability of a switch
 *     to respond or perform a certain function for example flow statistics,
 *     IP address lookup in APR packets.
 *   @option options [Number] :actions
 *     supported actions expressed as a 32-bit bitmap.
 *   @option options [Port] :ports
 *     an array of {Port} objects detailing port description and function.
 *   @return [FeaturesReply]
 */
static VALUE
features_reply_init( VALUE self, VALUE options ) {
  buffer *buf = NULL;
  Data_Get_Struct( self, buffer, buf );
  struct ofp_switch_features *features_reply = buf->data;
  VALUE tmp = Qnil;

  tmp = rb_hash_aref( options, ID2SYM( rb_intern( "datapath_id" ) ) );
  if ( tmp == Qnil ) {
    rb_raise( rb_eArgError, ":datapath_id is a mandatory option" );
  }
  features_reply->datapath_id = htonll( NUM2ULL( tmp ) );

  tmp = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
  if ( tmp == Qnil ) {
    tmp = rb_hash_aref( options, ID2SYM( rb_intern( "xid" ) ) );
    if ( tmp == Qnil ) {
      rb_raise( rb_eArgError, ":transaction_id is a mandatory option" );
    }
  }
  features_reply->header.xid = htonl( ( uint32_t ) NUM2UINT( tmp ) );

  features_reply->n_buffers = 0;
  tmp = rb_hash_aref( options, ID2SYM( rb_intern( "n_buffers" ) ) );
  if ( tmp != Qnil ) {
    features_reply->n_buffers = htonl( ( uint32_t ) NUM2UINT( tmp ) );
  }

  features_reply->n_tables = 1;
  tmp = rb_hash_aref( options, ID2SYM( rb_intern( "n_tables" ) ) );
  if ( tmp != Qnil ) {
    features_reply->n_tables = ( uint8_t ) NUM2UINT( tmp );
  }

  features_reply->capabilities = 0;
  tmp = rb_hash_aref( options, ID2SYM( rb_intern( "capabilities" ) ) );
  if ( tmp != Qnil ) {
    features_reply->capabilities = htonl( ( uint32_t ) NUM2UINT( tmp ) );
  }

  features_reply->actions = htonl( 1 << OFPAT_OUTPUT );
  tmp = rb_hash_aref( options, ID2SYM( rb_intern( "actions" ) ) );
  if ( tmp != Qnil ) {
    features_reply->actions = htonl( ( uint32_t ) NUM2UINT( tmp ) );
  }

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
  VALUE xid = Qnil;

  xid = rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
  if ( xid != Qnil ) {
    return xid;
  }
  xid = rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "xid" ) ) );
  if ( xid != Qnil ) {
    return xid;
  }

  return Qnil;
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
 * An array of {Port} objects detailing port description and function.
 *
 * @return [Array<Port>] the value of ports.
 */
static VALUE
features_reply_ports( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "ports" ) ) );
}


/*
 * An array of {Port} objects detailing physical port description and function.
 *
 * @return [Array<Port>] the value of ports.
 */
static VALUE
features_reply_physical_ports( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "physical_ports" ) ) );
}


/*
 * Document-class: Trema::FeaturesReply
 */
void
Init_features_reply() {
  mTrema = rb_eval_string( "Trema" );
  cFeaturesReply = rb_define_class_under( mTrema, "FeaturesReply", rb_cObject );
  rb_define_alloc_func( cFeaturesReply, features_reply_alloc );
  rb_define_method( cFeaturesReply, "initialize", features_reply_init, 1 );
  rb_define_method( cFeaturesReply, "datapath_id", features_reply_datapath_id, 0 );
  rb_define_method( cFeaturesReply, "transaction_id", features_reply_transaction_id, 0 );
  rb_alias( cFeaturesReply, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cFeaturesReply, "n_buffers", features_reply_n_buffers, 0 );
  rb_define_method( cFeaturesReply, "n_tables", features_reply_n_tables, 0 );
  rb_define_method( cFeaturesReply, "capabilities", features_reply_capabilities, 0 );
  rb_define_method( cFeaturesReply, "actions", features_reply_actions, 0 );
  rb_define_method( cFeaturesReply, "ports", features_reply_ports, 0 );
  rb_define_method( cFeaturesReply, "physical_ports", features_reply_physical_ports, 0 );
}


void
handle_switch_ready( uint64_t datapath_id, void *controller ) {
  if ( rb_respond_to( ( VALUE ) controller, rb_intern( "switch_ready" ) ) == Qtrue ) {
    rb_funcall( ( VALUE ) controller, rb_intern( "switch_ready" ), 1, ULL2NUM( datapath_id ) );
  }
}


/*
 * Extract and map {Port} to +ofp_phy_port+ structure.
 */
static VALUE
ports_from( const list_element *c_ports ) {
  VALUE ports = rb_ary_new();

  if ( c_ports == NULL ) {
    return ports;
  }

  list_element *port_head = xmalloc( sizeof( list_element ) );
  memcpy( port_head, c_ports, sizeof( list_element ) );
  list_element *port = NULL;
  for ( port = port_head; port != NULL; port = port->next ) {
    rb_ary_push( ports, port_from( ( struct ofp_phy_port * ) port->data ) );
  }
  xfree( port_head );
  return ports;
}


static void
append_physical_port( void *port, void *physical_ports ) {
  if ( ( ( struct ofp_phy_port * ) port )->port_no <= OFPP_MAX ) {
    append_to_tail( ( list_element ** ) physical_ports, port );
  }
}


void
handle_features_reply(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint32_t n_buffers,
  uint8_t n_tables,
  uint32_t capabilities,
  uint32_t actions,
  const list_element *ports,
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
  rb_hash_aset( attributes, ID2SYM( rb_intern( "ports" ) ), ports_from( ports ) );

  list_element *physical_ports = xmalloc( sizeof( list_element ) );
  create_list( &physical_ports );
  if ( ports != NULL ) {
    // use memcpy() to walk over the const qualifier
    list_element *tmp_ports = xmalloc( sizeof( list_element ) );
    memcpy( tmp_ports, ports, sizeof( list_element ) );
    iterate_list( tmp_ports, append_physical_port, &physical_ports );
    xfree( tmp_ports );
  }
  rb_hash_aset( attributes, ID2SYM( rb_intern( "physical_ports" ) ), ports_from( physical_ports ) );
  xfree( physical_ports );

  VALUE features_reply = rb_funcall( cFeaturesReply, rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "features_reply" ), 2, ULL2NUM( datapath_id ), features_reply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
