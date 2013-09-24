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


#include "default-logger.h"
#include "flow-mod.h"
#include "ruby.h"
#include "rubysig.h"
#include "switch.h"

#include "chibach.h" // must be included after ruby.h for undef ruby's xmalloc


VALUE mTrema;
VALUE cSwitch;


static VALUE
switch_send_message( VALUE self, VALUE message ) {
  buffer *buf;
  Data_Get_Struct( message, buffer, buf );
  switch_send_openflow_message( buf );
  return self;
}


static void
handle_controller_connected( void *rbswitch ) {
  if ( rb_respond_to( ( VALUE ) rbswitch, rb_intern( "controller_connected" ) ) ) {
    rb_funcall( ( VALUE ) rbswitch, rb_intern( "controller_connected" ), 0 );
  }
}


static void
handle_hello( uint32_t transaction_id, uint8_t version, void *rbswitch ) {
  if ( rb_respond_to( ( VALUE ) rbswitch, rb_intern( "hello" ) ) ) {
    rb_funcall( ( VALUE ) rbswitch, rb_intern( "hello" ), 2, UINT2NUM( transaction_id ), UINT2NUM( version ) );
  }
}


static void
handle_features_request( uint32_t transaction_id, void *rbswitch ) {
  if ( rb_respond_to( ( VALUE ) rbswitch, rb_intern( "features_request" ) ) ) {
    rb_funcall( ( VALUE ) rbswitch, rb_intern( "features_request" ), 1, UINT2NUM( transaction_id ) );
  }
}


static void
handle_set_config( uint32_t transaction_id, uint16_t flags, uint16_t miss_send_len, void *rbswitch ) {
  if ( rb_respond_to( ( VALUE ) rbswitch, rb_intern( "set_config" ) ) ) {
    rb_funcall( ( VALUE ) rbswitch, rb_intern( "set_config" ), 3, UINT2NUM( transaction_id ), UINT2NUM( flags ), UINT2NUM( miss_send_len ) );
  }
}


static void
handle_flow_mod(
  uint32_t transaction_id,
  struct ofp_match match,
  uint64_t cookie,
  uint16_t command,
  uint16_t idle_timeout,
  uint16_t hard_timeout,
  uint16_t priority,
  uint32_t buffer_id,
  uint16_t out_port,
  uint16_t flags,
  const openflow_actions *actions,
  void *rbswitch
) {
  UNUSED( match );
  UNUSED( cookie );
  UNUSED( priority );
  UNUSED( buffer_id );
  UNUSED( out_port );
  UNUSED( flags );
  UNUSED( actions );
  if ( rb_respond_to( ( VALUE ) rbswitch, rb_intern( "flow_mod" ) ) ) {
    VALUE options = rb_hash_new();
    rb_hash_aset( options, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "command" ) ), UINT2NUM( command ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "idle_timeout" ) ), UINT2NUM( idle_timeout ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "hard_timeout" ) ), UINT2NUM( hard_timeout ) );
    VALUE flow_mod = rb_funcall( cFlowMod, rb_intern( "new" ), 1, options );
    rb_funcall( ( VALUE ) rbswitch, rb_intern( "flow_mod" ), 2, UINT2NUM( transaction_id ), flow_mod );
  }
}


static void
handle_echo_request( uint32_t transaction_id, const buffer *body, void *rbswitch ) {
  if ( rb_respond_to( ( VALUE ) rbswitch, rb_intern( "echo_request" ) ) ) {
    VALUE rbody = rb_str_new( body->data, ( long ) body->length );
    rb_funcall( ( VALUE ) rbswitch, rb_intern( "echo_request" ), 2, UINT2NUM( transaction_id ), rbody );
  }
}


static VALUE
switch_run( VALUE self ) {
  setenv( "CHIBACH_HOME", RSTRING_PTR( rb_funcall( mTrema, rb_intern( "home" ), 0 ) ), 1 );

  VALUE name = rb_funcall( self, rb_intern( "name" ), 0 );
  rb_gv_set( "$PROGRAM_NAME", name );

  int argc = 6;
  char **argv = xmalloc( sizeof( char * ) * ( uint32_t ) ( argc + 1 ) );
  argv[ 0 ] = RSTRING_PTR( name );
  argv[ 1 ] = ( char * ) ( uintptr_t ) "--name";
  argv[ 2 ] = RSTRING_PTR( name );
  argv[ 3 ] = ( char * ) ( uintptr_t ) "--datapath_id";
  argv[ 4 ] = RSTRING_PTR( rb_funcall( rb_iv_get( self, "@dpid" ), rb_intern( "to_hex" ), 0 ) );
  argv[ 5 ] = ( char * ) ( uintptr_t ) "--daemonize";
  argv[ 6 ] = NULL;
  init_chibach( &argc, &argv );
  xfree( argv );

  set_controller_connected_handler( handle_controller_connected, ( void * ) self );
  set_hello_handler( handle_hello, ( void * ) self );
  set_features_request_handler( handle_features_request, ( void * ) self );
  set_set_config_handler( handle_set_config, ( void * ) self );
  set_flow_mod_handler( handle_flow_mod, ( void * ) self );
  set_echo_request_handler( handle_echo_request, ( void * ) self );

  if ( rb_respond_to( self, rb_intern( "start" ) ) ) {
    rb_funcall( self, rb_intern( "start" ), 0 );
  }

  rb_funcall( self, rb_intern( "start_chibach" ), 0 );

  return self;
}


static void
thread_pass( void *user_data ) {
  UNUSED( user_data );
  CHECK_INTS;
  rb_funcall( rb_cThread, rb_intern( "pass" ), 0 );
}


static VALUE
switch_start_chibach( VALUE self ) {
  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 1000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, thread_pass, NULL );

  start_chibach();

  return self;
}


/*
 * Document-class: Trema::Switch
 */
void
Init_switch() {
  mTrema = rb_eval_string( "Trema" );
  cSwitch = rb_define_class_under( mTrema, "Switch", rb_cObject );
  rb_include_module( cSwitch, mDefaultLogger );

  rb_define_method( cSwitch, "run!", switch_run, 0 );
  rb_define_method( cSwitch, "send_message", switch_send_message, 1 );
  rb_define_private_method( cSwitch, "start_chibach", switch_start_chibach, 0 );

  rb_require( "trema/switch" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
