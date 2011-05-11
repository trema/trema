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


extern VALUE mTrema;
VALUE cController;


static VALUE name;


static VALUE
controller_init( VALUE self ) {
  // Do nothing.
  return self;
}


static VALUE
controller_send_message( VALUE self, VALUE message, VALUE datapath_id ) {
  buffer *buf;
  Data_Get_Struct( message, buffer, buf );
  send_openflow_message( NUM2ULL( datapath_id ), buf );
  return self;
}


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
    raction = rb_hash_aref( options, ID2SYM( rb_intern( "action" ) ) );
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

  free( match );
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


static VALUE
controller_run( VALUE self ) {
  name = rb_funcall( self, rb_intern( "name" ), 0 );
  rb_gv_set( "$0", name );

  int argc = 3;
  char **argv = malloc( sizeof( char * ) * ( argc + 1 ) );
  argv[ 0 ] = STR2CSTR( name );
  argv[ 1 ] = "--name";
  argv[ 2 ] = STR2CSTR( name );
  argv[ 3 ] = NULL;

  setenv( "TREMA_HOME", STR2CSTR( rb_funcall( mTrema, rb_intern( "home" ), 0 ) ), 1 );

  init_trema( &argc, &argv );

  set_switch_ready_handler( handle_switch_ready, ( void * ) self );
  set_features_reply_handler( handle_features_reply, ( void * ) self );
  set_packet_in_handler( handle_packet_in, ( void * ) self );

  rb_funcall( self, rb_intern( "start" ), 0 );
  start_trema();
  return self;
}


static VALUE
controller_stop( VALUE self ) {
  stop_trema();
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


// Override me if necessary.
static VALUE
controller_packet_in( VALUE self, VALUE packet_in ) {
  return self;
}


/********************************************************************************
 * Logging methods.
 ********************************************************************************/

static VALUE
controller_critical( int argc, VALUE *argv, VALUE self ) {
  critical( STR2CSTR( rb_f_sprintf( argc, argv ) ) );
  return self;
}


static VALUE
controller_error( int argc, VALUE *argv, VALUE self ) {
  error( STR2CSTR( rb_f_sprintf( argc, argv ) ) );
  return self;
}


static VALUE
controller_warn( int argc, VALUE *argv, VALUE self ) {
  warn( STR2CSTR( rb_f_sprintf( argc, argv ) ) );
  return self;
}


static VALUE
controller_notice( int argc, VALUE *argv, VALUE self ) {
  notice( STR2CSTR( rb_f_sprintf( argc, argv ) ) );
  return self;
}


static VALUE
controller_info( int argc, VALUE *argv, VALUE self ) {
  info( STR2CSTR( rb_f_sprintf( argc, argv ) ) );
  return self;
}


static VALUE
controller_debug( int argc, VALUE *argv, VALUE self ) {
  debug( STR2CSTR( rb_f_sprintf( argc, argv ) ) );
  return self;
}


/********************************************************************************
 * Init Controller module.
 ********************************************************************************/

void
Init_controller() {
  rb_require( "trema/controller" );

  cController = rb_define_class_under( mTrema, "Controller", rb_cObject );

  rb_define_const( cController, "OFPP_FLOOD", INT2NUM( OFPP_FLOOD ) );

  rb_define_method( cController, "initialize", controller_init, 0 );
  rb_define_method( cController, "send_message", controller_send_message, 2 );
  rb_define_method( cController, "send_flow_mod_add", controller_send_flow_mod_add, -1 );
  rb_define_method( cController, "send_packet_out", controller_send_packet_out, 5 );

  rb_define_method( cController, "run", controller_run, 0 );
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
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
