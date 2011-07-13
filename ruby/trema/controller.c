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


#include "buffer.h"
#include "controller.h"
#include "features_reply.h"
#include "logger.h"
#include "openflow.h"
#include "packet_in.h"
#include "trema.h"


extern VALUE mTrema;
VALUE cController;


static void
handle_timer_event( void *self ) {
  rb_funcall( ( VALUE ) self, rb_intern( "handle_timer_event" ), 0 );
}


static VALUE
controller_send_message( VALUE self, VALUE message, VALUE datapath_id ) {
  buffer *buf;
  Data_Get_Struct( message, buffer, buf );
  send_openflow_message( NUM2ULL( datapath_id ), buf );
  return self;
}


/*
 * @overload send_flow_mod_add(datapath_id, options={})
 *   Sends a flow_mod message to add a flow into the datapath.
 *
 *   @example
 *     # packet_in handler
 *     def packet_in message
 *       send_flow_mod_add(
 *         message.datapath_id,
 *         :match => Match.from(message),
 *         :buffer_id => message.buffer_id,
 *         :actions => ActionOutput.new(OFPP_FLOOD)
 *       )
 *     end
 *
 *
 *   @param [Integer] datapath_id
 *     the datapath to which a message is sent.
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *
 *   @option options [Match] :match (nil)
 *     A {Match} object describing the fields of the flow.
 *
 *   @option options [Integer] :idle_timeout (0)
 *     The idle time in seconds before discarding.
 *
 *   @option options [Integer] :hard_timeout (0)
 *     The maximum time before discarding in seconds.
 *
 *   @option options [Integer] :priority (0xffff)
 *     The priority level of the flow entry.
 *
 *   @option options [Integer] :buffer_id (0xffffffff)
 *     The buffer ID assigned by the datapath of a buffered packet to
 *     apply the flow to. If 0xffffffff, no buffered packet is to be
 *     applied the flow actions.
 *
 *   @option options [Array] :actions (nil)
 *     The sequence of actions specifying the actions to perform on
 *     the flow's packets.
 */
static VALUE
controller_send_flow_mod_add( int argc, VALUE *argv, VALUE self ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;

  rb_scan_args( argc, argv, "11", &datapath_id, &options );

  struct ofp_match *match = malloc( sizeof( struct ofp_match ) );
  uint16_t idle_timeout = 0;
  uint16_t hard_timeout = 0;
  uint16_t priority = UINT16_MAX;
  uint32_t buffer_id = UINT32_MAX;
  openflow_actions *actions = create_actions();

  VALUE rmatch = Qnil;
  VALUE ridle_timeout = Qnil;
  VALUE rhard_timeout = Qnil;
  VALUE rpriority = Qnil;
  VALUE rbuffer_id = Qnil;
  VALUE raction = Qnil;

  if ( options != Qnil ) {
    rmatch = rb_hash_aref( options, ID2SYM( rb_intern( "match" ) ) );
    ridle_timeout = rb_hash_aref( options, ID2SYM( rb_intern( "idle_timeout" ) ) );
    rhard_timeout = rb_hash_aref( options, ID2SYM( rb_intern( "hard_timeout" ) ) );
    rpriority = rb_hash_aref( options, ID2SYM( rb_intern( "priority" ) ) );
    rbuffer_id = rb_hash_aref( options, ID2SYM( rb_intern( "buffer_id" ) ) );
    raction = rb_hash_aref( options, ID2SYM( rb_intern( "actions" ) ) );
  }

  if ( rmatch != Qnil ) {
    Data_Get_Struct( rmatch, struct ofp_match, match );
  }
  else {
    memset( match, 0, sizeof( struct ofp_match ) );
    match->wildcards = OFPFW_ALL;
  }

  if ( ridle_timeout != Qnil ) {
    idle_timeout = NUM2UINT( ridle_timeout );
  }

  if ( rhard_timeout != Qnil ) {
    hard_timeout = NUM2UINT( rhard_timeout );
  }

  if ( rpriority != Qnil ) {
    priority = NUM2UINT( priority );
  }

  if ( rbuffer_id != Qnil ) {
    buffer_id = NUM2ULONG( rbuffer_id );
  }

  if ( raction != Qnil ) {
    append_action_output( actions, rb_funcall( raction, rb_intern( "port" ), 0 ), UINT16_MAX );
  }

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    *match,
    get_cookie(),
    OFPFC_ADD,
    idle_timeout,
    hard_timeout,
    priority,
    buffer_id,
    OFPP_NONE,
    0,
    actions
  );
  send_openflow_message( NUM2ULL( datapath_id ), flow_mod );

  free_buffer( flow_mod );
  delete_actions( actions );

  return self;
}


static VALUE
controller_send_packet_out( int argc, VALUE *argv, VALUE self ) {
  VALUE first_arg = Qnil;
  VALUE second_arg = Qnil;

  rb_scan_args( argc, argv, "11", &first_arg, &second_arg );

  if ( rb_funcall( first_arg, rb_intern( "is_a?" ), 1, cPacketIn ) ) {
    packet_in *cpacket_in;
    Data_Get_Struct( first_arg, packet_in, cpacket_in );

    VALUE raction = second_arg;
    openflow_actions *actions = create_actions();
    if ( raction != Qnil ) {
      append_action_output( actions, rb_funcall( raction, rb_intern( "port" ), 0 ), UINT16_MAX );
    }

    buffer *packet_out = create_packet_out(
      get_transaction_id(),
      cpacket_in->buffer_id,
      cpacket_in->in_port,
      actions,
      cpacket_in->buffer_id == UINT32_MAX ? cpacket_in->data : NULL
    );

    send_openflow_message( cpacket_in->datapath_id, packet_out );
    free_buffer( packet_out );

    delete_actions( actions );
  }
  else {
    uint32_t buffer_id;

    VALUE datapath_id = first_arg;
    VALUE options = second_arg;

    VALUE rbuffer_id = Qnil;
    VALUE rin_port = Qnil;
    VALUE raction = Qnil;
    VALUE rdata = Qnil;

    if ( options != Qnil ) {
      rbuffer_id = rb_hash_aref( options, ID2SYM( rb_intern( "buffer_id" ) ) );
      rin_port = rb_hash_aref( options, ID2SYM( rb_intern( "in_port" ) ) );
      raction = rb_hash_aref( options, ID2SYM( rb_intern( "actions" ) ) );
      rdata = rb_hash_aref( options, ID2SYM( rb_intern( "data" ) ) );
    }

    if ( rbuffer_id == Qnil ) {
      buffer_id = UINT32_MAX;
    }
    else {
      buffer_id = NUM2ULONG( rbuffer_id );
    }

    openflow_actions *actions = create_actions();
    if ( raction != Qnil ) {
      append_action_output( actions, rb_funcall( raction, rb_intern( "port" ), 0 ), UINT16_MAX );
    }

    buffer *packet_out;
    if ( rdata == Qnil ) {
      packet_out = create_packet_out(
        get_transaction_id(),
        buffer_id,
        NUM2INT( rin_port ),
        actions,
        NULL
      );
    }
    else {
      buffer *cbuffer;
      Data_Get_Struct( rdata, buffer, cbuffer );

      packet_out = create_packet_out(
        get_transaction_id(),
        buffer_id,
        NUM2INT( rin_port ),
        actions,
        cbuffer
      );
    }

    send_openflow_message( NUM2ULL( datapath_id ), packet_out );
    free_buffer( packet_out );

    delete_actions( actions );
  }
  return self;
}


/*
 * call-seq:
 *   run()  => self
 *
 * Starts this controller. Usually you do not need to invoke
 * explicitly, because this is called implicitly by "trema run"
 * command.
 */
static VALUE
controller_run( VALUE self ) {
  setenv( "TREMA_HOME", STR2CSTR( rb_funcall( mTrema, rb_intern( "home" ), 0 ) ), 1 );

  VALUE name = rb_funcall( self, rb_intern( "name" ), 0 );
  rb_gv_set( "$PROGRAM_NAME", name );

  int argc = 3;
  char **argv = malloc( sizeof( char * ) * ( argc + 1 ) );
  argv[ 0 ] = STR2CSTR( name );
  argv[ 1 ] = "--name";
  argv[ 2 ] = STR2CSTR( name );
  argv[ 3 ] = NULL;
  init_trema( &argc, &argv );

  set_switch_ready_handler( handle_switch_ready, ( void * ) self );
  set_features_reply_handler( handle_features_reply, ( void * ) self );
  set_packet_in_handler( handle_packet_in, ( void * ) self );

  struct itimerspec interval;
  interval.it_interval.tv_sec = 1;
  interval.it_interval.tv_nsec = 0;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, handle_timer_event, ( void * ) self );

  rb_funcall( self, rb_intern( "start" ), 0 );

  rb_funcall( self, rb_intern( "start_trema" ), 0 );

  return self;
}


static VALUE
controller_shutdown( VALUE self ) {
  stop_trema();
  return self;
}


static void
thread_pass( void *user_data ) {
  rb_funcall( rb_cThread, rb_intern( "pass" ), 0 );
}


static VALUE
controller_start_trema( VALUE self ) {
  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 1000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, thread_pass, NULL );

  start_trema();

  return self;
}


/********************************************************************************
 * Handlers.
 ********************************************************************************/

// Override me if necessary.
static VALUE
controller_start( VALUE self ) {
  return self;
}


// Override me if necessary.
static VALUE
controller_switch_ready( VALUE self, VALUE datapath_id ) {
  return self;
}


// Override me if necessary.
static VALUE
controller_features_reply( VALUE self, VALUE message ) {
  return self;
}


/*
 * call-seq:
 *   packet_in(message)
 *
 * Handle the reception of a {PacketIn} message.
 */
static VALUE
controller_packet_in( VALUE self, VALUE packet_in ) {
  return self;
}


/********************************************************************************
 * Init Controller module.
 ********************************************************************************/

void
Init_controller() {
  rb_require( "trema/app" );
  VALUE cApp = rb_eval_string( "Trema::App" );
  cController = rb_define_class_under( mTrema, "Controller", cApp );
  rb_include_module( cController, mLogger );

  rb_define_const( cController, "OFPP_FLOOD", INT2NUM( OFPP_FLOOD ) );

  rb_define_method( cController, "send_message", controller_send_message, 2 );
  rb_define_method( cController, "send_flow_mod_add", controller_send_flow_mod_add, -1 );
  rb_define_method( cController, "send_packet_out", controller_send_packet_out, -1 );

  rb_define_method( cController, "run!", controller_run, 0 );
  rb_define_method( cController, "shutdown!", controller_shutdown, 0 );

  // Handlers
  rb_define_method( cController, "start", controller_start, 0 );
  rb_define_method( cController, "switch_ready", controller_switch_ready, 1 );
  rb_define_method( cController, "features_reply", controller_features_reply, 1 );
  rb_define_method( cController, "packet_in", controller_packet_in, 1 );

  // Private
  rb_define_private_method( cController, "start_trema", controller_start_trema, 0 );

  rb_require( "trema/controller" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
