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
#include "openflow.h"
#include "packet_in.h"
#include "trema.h"


VALUE mTrema;
VALUE cController;


static VALUE
controller_send_message( VALUE self, VALUE message, VALUE datapath_id ) {
  buffer *buf;
  Data_Get_Struct( message, buffer, buf );
  send_openflow_message( NUM2ULL( datapath_id ), buf );
  return self;
}


/*
 * call-seq:
 *   send_flow_mod_add(datapath_id, options={})
 *
 * Sends a flow_mod message to add a flow into the datapath.
 *
 *  # packet_in handler
 *  def packet_in message
 *    send_flow_mod_add(
 *      datapath_id,
 *      :match => Match.from(message),
 *      :buffer_id => message.buffer_id,
 *      :actions => ActionOutput.new( OFPP_FLOOD )
 *    )
 *  end
 *
 * Options:
 *
 * <code>match</code>::
 *   A {Match} object describing the fields of the
 *   flow. <em>(default=all fields are wildcarded)</em>
 *
 * <code>buffer_id</code>::
 *   The buffer ID assigned by the datapath of a buffered packet to
 *   apply the flow to. If 0xffffffff, no buffered packet is to be
 *   applied the flow actions. <em>(default=0xffffffff)</em>
 * 
 * <code>actions</code>::
 *   The sequence of actions specifying the actions to perform on the
 *   flow's packets. <em>(default=[])</em>
 */
static VALUE
controller_send_flow_mod_add( int argc, VALUE *argv, VALUE self ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;

  rb_scan_args( argc, argv, "11", &datapath_id, &options );

  struct ofp_match *match = malloc( sizeof( struct ofp_match ) );
  uint32_t buffer_id;
  openflow_actions *actions = create_actions();

  VALUE rmatch = Qnil;
  VALUE rbuffer_id = Qnil;
  VALUE raction = Qnil;

  if ( options != Qnil ) {
    rmatch = rb_hash_aref( options, ID2SYM( rb_intern( "match" ) ) );
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

  if ( rbuffer_id == Qnil ) {
    buffer_id = UINT32_MAX;
  }
  else {
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
    60,
    0,
    UINT16_MAX,
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
controller_send_packet_out( VALUE self, VALUE datapath_id, VALUE buffer_id, VALUE in_port, VALUE action, VALUE data ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, rb_funcall( action, rb_intern( "port" ), 0 ), UINT16_MAX );

  buffer *cbuffer;
  Data_Get_Struct( data, buffer, cbuffer );

  buffer *packet_out = create_packet_out(
    get_transaction_id(),
    NUM2ULONG( buffer_id ),
    NUM2INT( in_port ),
    actions,
    cbuffer
  );
  send_openflow_message( NUM2ULL( datapath_id ), packet_out );
  free_buffer( packet_out );

  delete_actions( actions );
  
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

  rb_funcall( self, rb_intern( "start" ), 0 );

  rb_funcall( self, rb_intern( "start_trema" ), 0 );

  return self;
}


static VALUE
controller_stop( VALUE self ) {
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
 * Logging methods.
 ********************************************************************************/

/*
 * call-seq:
 *   critical(format ...)
 *
 * Outputs a message representing that "the system is completely
 * unusable" to log file.
 *
 * @example
 *   critical "Trema blue screen. Memory dump = %s", memory
 *
 * @return [String] the string resulting from applying format to any
 *   additional arguments.
 */
static VALUE
controller_critical( int argc, VALUE *argv, VALUE self ) {
  VALUE message = rb_f_sprintf( argc, argv );
  critical( STR2CSTR( message ) );
  return message;
}


/*
 * call-seq:
 *   error(format ...)
 *
 * Outputs a message representing that "something went wrong" to log
 * file.
 *
 * @example
 *   error "Failed to accept %s", app_socket
 *
 * @return [String] the string resulting from applying format to any
 *   additional arguments.
 */
static VALUE
controller_error( int argc, VALUE *argv, VALUE self ) {
  VALUE message = rb_f_sprintf( argc, argv );
  error( STR2CSTR( message ) );
  return message;
}


/*
 * call-seq:
 *   warn(format ...)
 *
 * Outputs a message representing that "something in the system was
 * not as expected" to log file.
 *
 * @example
 *   warn "%s: trema is already initialized", app_name
 *
 * @return [String] the string resulting from applying format to any
 *   additional arguments.
 */
static VALUE
controller_warn( int argc, VALUE *argv, VALUE self ) {
  VALUE message = rb_f_sprintf( argc, argv );
  warn( STR2CSTR( message ) );
  return message;
}


/*
 * call-seq:
 *   notice(format ...)
 *
 * Outputs a message representing that "normal but significant
 * condition occurred" to log file.
 *
 * @example
 *   notice "The switch %s disconnected its secure channel connection", datapath_id
 *
 * @return [String] the string resulting from applying format to any
 *   additional arguments.
 */
static VALUE
controller_notice( int argc, VALUE *argv, VALUE self ) {
  VALUE message = rb_f_sprintf( argc, argv );
  notice( STR2CSTR( message ) );
  return message;
}


/*
 * call-seq:
 *   info(format ...)
 *
 * Outputs an informational massage to log file.
 *
 * @example
 *   info "Hello world from %s!", datapath_id
 *
 * @return [String] the string resulting from applying format to any
 *   additional arguments.
 */
static VALUE
controller_info( int argc, VALUE *argv, VALUE self ) {
  VALUE message = rb_f_sprintf( argc, argv );
  info( STR2CSTR( message ) );
  return message;
}


/*
 * call-seq:
 *   debug(format ...)
 *
 * Outputs a debug-level massage to log file.
 *
 * @example
 *   debug "Setting a packet_in handler: %s", method
 *
 * @return [String] the string resulting from applying format to any
 *   additional arguments.
 */
static VALUE
controller_debug( int argc, VALUE *argv, VALUE self ) {
  VALUE message = rb_f_sprintf( argc, argv );
  debug( STR2CSTR( message ) );
  return message;
}


/********************************************************************************
 * Init Controller module.
 ********************************************************************************/

void
Init_controller() {
  mTrema = rb_define_module( "Trema" );

  rb_require( "trema/controller" );

  cController = rb_eval_string( "Trema::Controller" );
  rb_define_const( cController, "OFPP_FLOOD", INT2NUM( OFPP_FLOOD ) );

  rb_define_method( cController, "send_message", controller_send_message, 2 );
  rb_define_method( cController, "send_flow_mod_add", controller_send_flow_mod_add, -1 );
  rb_define_method( cController, "send_packet_out", controller_send_packet_out, 5 );

  rb_define_method( cController, "run!", controller_run, 0 );
  rb_define_method( cController, "stop", controller_stop, 0 );

  // Handlers
  rb_define_method( cController, "start", controller_start, 0 );
  rb_define_method( cController, "switch_ready", controller_switch_ready, 1 );
  rb_define_method( cController, "features_reply", controller_features_reply, 1 );
  rb_define_method( cController, "packet_in", controller_packet_in, 1 );

  // Logging
  rb_define_method( cController, "critical", controller_critical, -1 );
  rb_define_method( cController, "error", controller_error, -1 );
  rb_define_method( cController, "warn", controller_warn, -1 );
  rb_define_method( cController, "notice", controller_notice, -1 );
  rb_define_method( cController, "info", controller_info, -1 );
  rb_define_method( cController, "debug", controller_debug, -1 );

  // Private
  rb_define_private_method( cController, "start_trema", controller_start_trema, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
