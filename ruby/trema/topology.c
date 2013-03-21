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
#include "topology.h"

#include "trema.h"

#include <stdio.h>
#include <stdlib.h>

extern VALUE mTrema;
VALUE mTopology;

typedef struct topology_callback {
  VALUE self;
  VALUE block;
} topology_callback;

static VALUE topology_subscribe_topology( VALUE self );
static VALUE topology_enable_topology_discovery( VALUE self );


/*
 * init_libtopology(service_name)
 *   Initialize topology client.
 *
 *   This method is intended to be called inside Controller#start.
 *
 *   @param [String] service_name
 *     name of the topology service to use.
 */
static VALUE
topology_init_libtopology( VALUE self, VALUE service_name ) {
  init_libtopology( StringValuePtr( service_name ) );
  rb_iv_set( self, "@is_libtopology_initialized", Qtrue );
  return self;
}


static void
maybe_init_libtopology( VALUE self ) {
  ID is_init = rb_intern( "@is_libtopology_initialized" );
  if ( rb_ivar_defined( self, is_init ) == Qfalse
      || rb_ivar_get( self, is_init ) == Qfalse ) {
    info( "libtopology was not initialized. Default initializing." );
    init_libtopology( "topology" );
    rb_iv_set( self, "@is_libtopology_initialized", Qtrue );

    if ( rb_funcall( self, rb_intern( "topology_handler_implemented?" ),
                     0 ) == Qtrue ) {
      topology_subscribe_topology( self );
    }

    if ( rb_respond_to( ( VALUE ) self,
                        rb_intern( "link_status_updated" ) ) == Qtrue ) {
      topology_enable_topology_discovery( self );
    }
  }
}


/*
 * finalize_libtopology(service_name)
 *   Finalize topology client.
 *
 *   This method is intended to be called inside Controller#shutdown!.
 */
static VALUE
topology_finalize_libtopology( VALUE self ) {
  finalize_libtopology();
  rb_iv_set( self, "@is_libtopology_initialized", Qfalse );
  return self;
}

static VALUE
switch_status_to_hash( const topology_switch_status* sw_status ) {
  VALUE sw = rb_hash_new();
  rb_hash_aset( sw, ID2SYM( rb_intern( "dpid" ) ), ULL2NUM( sw_status->dpid ) );

  if ( sw_status->status == TD_SWITCH_UP ) {
    rb_hash_aset( sw, ID2SYM( rb_intern( "up" ) ), Qtrue );
  }
  else {
    rb_hash_aset( sw, ID2SYM( rb_intern( "up" ) ), Qfalse );
  }
  return sw;
}


static void
handle_switch_status_updated( void* self, const topology_switch_status* sw_status ) {
  rb_iv_set( ( VALUE ) self, "@map_up_to_date", Qfalse );

  if ( rb_respond_to( ( VALUE ) self,
                      rb_intern( "_switch_status_updated" ) ) == Qtrue ) {
    VALUE sw = switch_status_to_hash( sw_status );
    rb_funcall( ( VALUE ) self, rb_intern( "_switch_status_updated" ), 1, sw );
  }
}


static VALUE
port_status_to_hash( const topology_port_status* port_status ) {
  VALUE port = rb_hash_new();
  rb_hash_aset( port, ID2SYM( rb_intern( "dpid" ) ),
                ULL2NUM( port_status->dpid ) );

  rb_hash_aset( port, ID2SYM( rb_intern( "portno" ) ),
                INT2FIX( (int)port_status->port_no ) );
  rb_hash_aset( port, ID2SYM( rb_intern( "name" ) ),
                rb_str_new2( port_status->name ) );
  char macaddr[ ] = "FF:FF:FF:FF:FF:FF";
  const uint8_t* mac = port_status->mac;
  snprintf( macaddr, sizeof( macaddr ),
            "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", mac[ 0 ], mac[ 1 ],
            mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
  rb_hash_aset( port, ID2SYM( rb_intern( "mac" ) ), rb_str_new2( macaddr ) );

  if ( port_status->external == TD_PORT_EXTERNAL ) {
    rb_hash_aset( port, ID2SYM( rb_intern( "external" ) ), Qtrue );
  }
  else {
    rb_hash_aset( port, ID2SYM( rb_intern( "external" ) ), Qfalse );
  }

  if ( port_status->status == TD_PORT_UP ) {
    rb_hash_aset( port, ID2SYM( rb_intern( "up" ) ), Qtrue );
  }
  else {
    rb_hash_aset( port, ID2SYM( rb_intern( "up" ) ), Qfalse );
  }
  return port;
}

static void
handle_port_status_updated( void* self, const topology_port_status* port_status ) {
  rb_iv_set( ( VALUE ) self, "@map_up_to_date", Qfalse );

  if ( rb_respond_to( ( VALUE ) self,
                      rb_intern( "_port_status_updated" ) ) == Qtrue ) {
    VALUE port = port_status_to_hash( port_status );
    rb_funcall( ( VALUE ) self, rb_intern( "_port_status_updated" ), 1, port );
  }
}


static VALUE
link_status_to_hash( const topology_link_status* link_status ) {
  VALUE link = rb_hash_new();
  rb_hash_aset( link, ID2SYM( rb_intern( "from_dpid" ) ),
                ULL2NUM( link_status->from_dpid ) );
  rb_hash_aset( link, ID2SYM( rb_intern( "from_portno" ) ),
                INT2FIX( (int)link_status->from_portno ) );
  rb_hash_aset( link, ID2SYM( rb_intern( "to_dpid" ) ),
                ULL2NUM( link_status->to_dpid ) );
  rb_hash_aset( link, ID2SYM( rb_intern( "to_portno" ) ),
                INT2FIX( (int)link_status->to_portno ) );

  if ( link_status->status != TD_LINK_DOWN ) {
    rb_hash_aset( link, ID2SYM( rb_intern( "up" ) ), Qtrue );
  }
  else {
    rb_hash_aset( link, ID2SYM( rb_intern( "up" ) ), Qfalse );
  }
  if ( link_status->status == TD_LINK_UNSTABLE ) {
    rb_hash_aset( link, ID2SYM( rb_intern( "unstable" ) ), Qtrue );
  }
  else {
    rb_hash_aset( link, ID2SYM( rb_intern( "unstable" ) ), Qfalse );
  }
  return link;
}


static void
handle_link_status_updated( void* self, const topology_link_status* link_status ) {
  rb_iv_set( ( VALUE ) self, "@map_up_to_date", Qfalse );

  if ( rb_respond_to( ( VALUE ) self,
                      rb_intern( "_link_status_updated" ) ) == Qtrue ) {
    VALUE link = link_status_to_hash( link_status );
    rb_funcall( ( VALUE ) self, rb_intern( "_link_status_updated" ), 1, link );
  }
}


static void
handle_subscribed_reply( void* tcb, topology_response *res ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  switch ( res->status ) {
    case TD_RESPONSE_OK:
    case TD_RESPONSE_ALREADY_SUBSCRIBED:
      if ( res->status == TD_RESPONSE_ALREADY_SUBSCRIBED ) {
        warn( "Already subscribed to topology service." );
      }

      rb_iv_set( ( VALUE ) self, "@is_subscribed", Qtrue );

      if ( block != Qnil ) {
        rb_funcall( block, rb_intern( "call" ), 0 );
      }
      else if ( rb_respond_to( ( VALUE ) self,
                               rb_intern( "subscribe_topology_reply" ) )
                == Qtrue ) {
        rb_funcall( ( VALUE ) self, rb_intern( "subscribe_topology_reply" ),
                    0 );
      }
      break;

    default:
      warn( "%s: Abnormal subscribed reply: %#x", __func__,
            ( unsigned int ) res->status );
  }
  xfree( cb );
}


static void
handle_unsubscribed_reply( void* tcb, topology_response *res ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  if ( res->status == TD_RESPONSE_NO_SUCH_SUBSCRIBER ) {
    warn( "Already unsubscribed from topology Service." );
  }
  else if ( res->status != TD_RESPONSE_OK ) {
    warn( "%s: Abnormal unsubscribed reply: %#x", __func__,
          ( unsigned int ) res->status );
  }
  rb_iv_set( self, "@is_subscribed", Qfalse );

  if ( block != Qnil ) {
    rb_funcall( block, rb_intern( "call" ), 0 );
  }
  else if ( rb_respond_to( ( VALUE ) self,
                           rb_intern( "unsubscribe_topology_reply" ) )
            == Qtrue ) {
    rb_funcall( ( VALUE ) self, rb_intern( "unsubscribe_topology_reply" ), 0 );
  }
  xfree( cb );
}


/**
 * @!group Topology update event subscription control
 * Manually subscribe to topology to get topology update events.
 * @note This will be automatically called by #start if a Topology update event handler is defined.
 * @raise [RuntimeError]if sending request to topology daemon failed.
 */
static VALUE
topology_subscribe_topology( VALUE self ) {
  maybe_init_libtopology( self );
  add_callback_switch_status_updated( handle_switch_status_updated,
                                      ( void * ) self );
  add_callback_port_status_updated( handle_port_status_updated,
                                    ( void * ) self );
  add_callback_link_status_updated( handle_link_status_updated,
                                    ( void * ) self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }
  bool succ = subscribe_topology( handle_subscribed_reply, ( void * ) cb );

  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send subscribe_topology." );
    return Qfalse ;
  }
}


/**
 * @!group Topology update event subscription control
 * Unsubscribe from topology.
 *
 * @raise [RuntimeError]if sending request to topology daemon failed.
 *
 */
static VALUE
topology_unsubscribe_topology( VALUE self ) {
  maybe_init_libtopology( self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }
  bool succ = unsubscribe_topology( handle_unsubscribed_reply, ( void * ) cb );

  add_callback_switch_status_updated( NULL, NULL );
  add_callback_port_status_updated( NULL, NULL );
  add_callback_link_status_updated( NULL, NULL );

  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send unsubscribe_topology." );
    return Qfalse ;
  }
}


static void
handle_enable_topology_discovery_reply( void* tcb, const topology_response *res ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  if ( res->status != TD_RESPONSE_OK ) {
    warn( "%s: Abnormal reply: %#x", __func__, ( unsigned int ) res->status );
    xfree( cb );
    return;
  }
  if ( block != Qnil ) {
    rb_funcall( block, rb_intern( "call" ), 0 );
  }
  else if ( rb_respond_to( ( VALUE ) self,
                           rb_intern( "enable_topology_discovery_reply" ) )
            == Qtrue ) {
    rb_funcall( ( VALUE ) self, rb_intern( "enable_topology_discovery_reply" ),
                0 );
  }
  xfree( cb );
}


/**
 *  @!group Discovery control
 *
 * Enable topology discovery.
 *
 * @raise [RuntimeError]if sending request to topology daemon failed.
 *
 */
static VALUE
topology_enable_topology_discovery( VALUE self ) {
  maybe_init_libtopology( self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }

  bool succ = enable_topology_discovery( handle_enable_topology_discovery_reply,
                                         ( void* ) cb );
  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send enable_topology." );
    return Qfalse ;
  }
}


static void
handle_disable_topology_discovery_reply( void* tcb, const topology_response *res ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  if ( res->status != TD_RESPONSE_OK ) {
    warn( "%s: Abnormal reply: %#x", __func__, ( unsigned int ) res->status );
    xfree( cb );
    return;
  }
  if ( block != Qnil ) {
    rb_funcall( block, rb_intern( "call" ), 0 );
  }
  else if ( rb_respond_to( ( VALUE ) self,
                           rb_intern( "disable_topology_discovery_reply" ) )
            == Qtrue ) {
    rb_funcall( ( VALUE ) self, rb_intern( "disable_topology_discovery_reply" ),
                0 );
  }
  xfree( cb );
}


/**
 *  @!group Discovery control
 *
 * Disable topology discovery.
 *
 * @raise [RuntimeError]if sending request to topology daemon failed.
 *
 */
static VALUE
topology_disable_topology_discovery( VALUE self ) {
  maybe_init_libtopology( self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }

  bool succ = disable_topology_discovery(
      handle_disable_topology_discovery_reply, ( void* ) cb );
  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send disable_topology." );
    return Qfalse ;
  }
}


static void
handle_get_all_link_status_callback( void *tcb, size_t number, const topology_link_status *link_status ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  if ( block != Qnil ) {
    VALUE links = rb_ary_new2( ( long ) number );
    for ( size_t i = 0; i < number; ++i ) {
      VALUE link = link_status_to_hash( &link_status[ i ] );
      rb_ary_push( links, link );
    }
    rb_funcall( block, rb_intern( "call" ), 1, links );
  }
  else if ( rb_respond_to( ( VALUE ) self,
                           rb_intern( "all_link_status" ) ) == Qtrue ) {
    VALUE links = rb_ary_new2( ( long ) number );
    for ( size_t i = 0; i < number; ++i ) {
      VALUE link = link_status_to_hash( &link_status[ i ] );
      rb_ary_push( links, link );
    }
    rb_funcall( ( VALUE ) self, rb_intern( "all_link_status" ), 1, links );
  }
  xfree( cb );
}


/**
 *  @!group Get all status request/reply
 *
 * Request Topology service to send all link status it holds.
 * Results will be returned as callback to Block given,
 * or as a call to all_link_status_reply handler if no Block was given.
 *
 * @yieldparam link_stats [Array<Hash>]  Array of Hash including current status.
 *
 * @raise [RuntimeError]if sending request to topology daemon failed.
 *
 * @see #link_status_updated Each Hash instance included in the array is equivalent to link_status_updated argument Hash.
 * @see #all_link_status
 */
static VALUE
topology_send_all_link_status( VALUE self ) {
  maybe_init_libtopology( self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }
  bool succ = get_all_link_status( handle_get_all_link_status_callback,
                                   ( void* ) cb );
  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send get_all_link_status." );
    return Qfalse ;
  }
}


static void
handle_get_all_port_status_callback( void *tcb, size_t number, const topology_port_status *port_status ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  if ( block != Qnil ) {
    VALUE ports = rb_ary_new2( ( long ) number );
    for ( size_t i = 0; i < number; ++i ) {
      VALUE port = port_status_to_hash( &port_status[ i ] );
      rb_ary_push( ports, port );
    }
    rb_funcall( block, rb_intern( "call" ), 1, ports );
  }
  else if ( rb_respond_to( ( VALUE ) self,
                           rb_intern( "all_port_status" ) ) == Qtrue ) {
    VALUE ports = rb_ary_new2( ( long ) number );
    for ( size_t i = 0; i < number; ++i ) {
      VALUE port = port_status_to_hash( &port_status[ i ] );
      rb_ary_push( ports, port );
    }
    rb_funcall( ( VALUE ) self, rb_intern( "all_port_status" ), 1, ports );
  }
  xfree( cb );
}


/**
 *  @!group Get all status request/reply
 *
 * Request Topology service to send all port status it holds.
 * Results will be returned as callback to Block given,
 * or as a call to all_port_status_reply handler if no Block was given.
 *
 * @yieldparam port_stats [Array<Hash>]  Array of Hash including current status.
 *
 * @raise [RuntimeError]if sending request to topology daemon failed.
 *
 * @see #port_status_updated Each Hash instance included in the array is equivalent to port_status_updated argument Hash.
 * @see #all_port_status
 */
static VALUE
topology_send_all_port_status( VALUE self ) {
  maybe_init_libtopology( self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }
  bool succ = get_all_port_status( handle_get_all_port_status_callback,
                                   ( void* ) cb );
  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send get_all_port_status." );
    return Qfalse ;
  }
}


static void
handle_get_all_switch_status_callback( void *tcb, size_t number, const topology_switch_status *sw_status ) {
  topology_callback* cb = tcb;
  VALUE self = cb->self;
  VALUE block = cb->block;

  if ( block != Qnil ) {
    VALUE switches = rb_ary_new2( ( long ) number );
    for ( size_t i = 0; i < number; ++i ) {
      VALUE sw = switch_status_to_hash( &sw_status[ i ] );
      rb_ary_push( switches, sw );
    }
    rb_funcall( block, rb_intern( "call" ), 1, switches );
  }
  else if ( rb_respond_to( ( VALUE ) self,
                           rb_intern( "all_switch_status" ) ) == Qtrue ) {
    VALUE switches = rb_ary_new2( ( long ) number );
    for ( size_t i = 0; i < number; ++i ) {
      VALUE sw = switch_status_to_hash( &sw_status[ i ] );
      rb_ary_push( switches, sw );
    }
    rb_funcall( ( VALUE ) self, rb_intern( "all_switch_status" ), 1, switches );
  }
  xfree( cb );
}


/**
 *  @!group Get all status request/reply
 *
 * @!method send_all_switch_status_request { |switches| ... } | nil
 * Request Topology service to send all switch status it holds.
 * Results will be returned as callback to Block given,
 * or as a call to all_switch_status_reply handler if no Block was given.
 *
 * @yieldparam switche_stats [Array<Hash>]  Array of Hash including current status.
 *
 * @raise [RuntimeError]if sending request to topology daemon failed.
 *
 * @see #switch_status_updated Each Hash instance included in the array is equivalent to switch_status_updated argument Hash.
 * @see #all_switch_status
 */
static VALUE
topology_send_all_switch_status( VALUE self ) {
  maybe_init_libtopology( self );

  topology_callback* cb = xcalloc( 1, sizeof( topology_callback ) );
  cb->self = self;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  else {
    cb->block = Qnil;
  }
  bool succ = get_all_switch_status( handle_get_all_switch_status_callback,
                                     ( void* ) cb );
  if ( succ ) {
    return Qtrue ;
  }
  else {
    xfree( cb );
    rb_raise( rb_eRuntimeError, "Failed to send get_all_switch_status." );
    return Qfalse ;
  }
}


/*
 * Document-module: Trema::Topology
 */
void
Init_topology( void ) {
  mTopology = rb_define_module_under( mTrema, "Topology" );

  rb_define_protected_method( mTopology, "init_libtopology",
                              topology_init_libtopology, 1 );
  rb_define_protected_method( mTopology, "finalize_libtopology",
                              topology_finalize_libtopology, 0 );

  rb_define_protected_method( mTopology, "send_subscribe_topology",
                              topology_subscribe_topology, 0 );
  rb_define_protected_method( mTopology, "send_unsubscribe_topology",
                              topology_unsubscribe_topology, 0 );

  rb_define_protected_method( mTopology, "send_enable_topology_discovery",
                              topology_enable_topology_discovery, 0 );
  rb_define_protected_method( mTopology, "send_disable_topology_discovery",
                              topology_disable_topology_discovery, 0 );

  rb_define_method( mTopology, "get_all_link_status",
                    topology_send_all_link_status, 0 );
  rb_define_method( mTopology, "get_all_port_status",
                    topology_send_all_port_status, 0 );
  rb_define_method( mTopology, "get_all_switch_status",
                    topology_send_all_switch_status, 0 );

  rb_require( "trema/topology" );
}

