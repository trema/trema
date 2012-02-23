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


extern VALUE mTrema;
VALUE cActionEnqueue;


/*
 * Enqueues the packet on the specified queue attached to a port. When a queue
 * is configured the user can associate a flow with this action to forward a
 * packet through the specific queue in that port.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     ActionEnqueue.new( :port => 1, :queue_id => 2 )
 *
 *   @param [Hash] options
 *     the options hash to create this action class instance with.
 *
 *   @option options [Number] :port
 *     the port the queue is attached to.
 *
 *   @option options [Number] :queue_id
 *     the configured queue. Currently only minimum rate queues provided.
 *
 *   @raise [ArgumentError] if both port and queue_id arguments not supplied.
 *   @raise [ArgumentError] if port is not an unsigned 16-bit integer.
 *   @raise [ArgumentError] if queue id is not an unsigned 32-bit integer.
 *   @raise [TypeError] if options is not a Hash.
 *
 *   @return [ActionEnqueue] self
 *     an object that encapsulates this action.
 */
static VALUE
action_enqueue_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( rb_scan_args( argc, argv, "01", &options ) == 1 ) {
    Check_Type( options, T_HASH );
    VALUE port;
    if ( ( port = rb_hash_aref( options, ID2SYM( rb_intern( "port" ) ) ) ) != Qnil ) {
      if ( rb_funcall( port, rb_intern( "unsigned_16bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Port must be an unsigned 16-bit integer" );
      }
      rb_iv_set( self, "@port", port );
    }
    else {
      rb_raise( rb_eArgError, "Port is a mandatory option" );
    }
    VALUE queue_id;
    if ( ( queue_id = rb_hash_aref( options, ID2SYM( rb_intern( "queue_id" ) ) ) ) != Qnil ) {
      if ( rb_funcall( queue_id, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
        rb_raise( rb_eArgError, "Queue id must be an unsigned 32-bit integer" );
      }
      rb_iv_set( self, "@queue_id", queue_id );
    }
    else {
      rb_raise( rb_eArgError, "Queue id is a mandatory option" );
    }
  }
  else {
    rb_raise( rb_eArgError, "Port, queue id are mandatory options" );
  }
  return self;
}


/*
 * The port the queue is attached to.
 *
 * @return [Number] the value of port.
 */
static VALUE
action_enqueue_get_port( VALUE self ) {
  return rb_iv_get( self, "@port" );
}


/*
 * The configured queue.
 *
 * @return [Number] the value of queue_id.
 */
static VALUE
action_enqueue_get_queue_id( VALUE self ) {
  return rb_iv_get( self, "@queue_id" );
}

/*
 * Appends its action(enqueue) to the list of actions.
 *
 * @return [ActionEnqueue] self
 */
static VALUE
action_enqueue_append( VALUE self, VALUE action_ptr ) {
  uint32_t queue_id = ( uint32_t ) NUM2UINT( action_enqueue_get_queue_id( self ) );
  uint16_t port = ( uint16_t ) NUM2UINT( action_enqueue_get_port( self ) );
  
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_enqueue( actions, port, queue_id );
  return self;
}


/*
 * A text representation of its attributes.
 *
 * @return [String] 
 */
static VALUE
action_enqueue_inspect( VALUE self ) {
  uint32_t queue_id = ( uint32_t ) NUM2UINT( action_enqueue_get_queue_id( self ) );
  uint16_t port = ( uint16_t ) NUM2UINT( action_enqueue_get_port( self ) );
  
  char str[ 64 ];
  sprintf( str, "#<%s port=%u,queue_id=%u>", rb_obj_classname( self ), port,
          queue_id );
  return rb_str_new2( str );
}


void
Init_action_enqueue() {
  cActionEnqueue = rb_define_class_under( mTrema, "ActionEnqueue", rb_cObject );
  rb_define_method( cActionEnqueue, "initialize", action_enqueue_init, -1 );
  rb_define_method( cActionEnqueue, "port", action_enqueue_get_port, 0 );
  rb_define_method( cActionEnqueue, "queue_id", action_enqueue_get_queue_id, 0 );
  rb_define_method( cActionEnqueue, "append", action_enqueue_append, 1 );
  rb_define_method( cActionEnqueue, "inspect", action_enqueue_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
