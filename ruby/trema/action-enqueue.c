/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cActionEnqueue;


/*
 * Enqueues the packet on the specified queue attached to a port. When a queue 
 * is configured the user can associate a flow with this action to forward a 
 * packet through the specific queue in that port.
 * 
 * @overload initialize(port, queue_id)
 * 
 * @param [Number] port
 *   the port the queue is attached to.
 * 
 * @param [Number] queue_id
 *   the configured queue.  Currently only minimum rate queues provided.
 * 
 * @return [ActionEnqueue] self
 *   an object that encapsulates the action:enqueue
 */
static VALUE
action_enqueue_init( VALUE self, VALUE port, VALUE queue_id ) {
  rb_iv_set( self, "@port", port );
  rb_iv_set( self, "@queue_id", queue_id );
  return self;
}


/*
 * The port the queue is attached to.
 * 
 * @return [Number] the value of attribute port.
 */
static VALUE
action_enqueue_get_port( VALUE self ) {
  return rb_iv_get( self, "@port" );
}


/*
 * The configured queue.
 * 
 * @return [Number] the value of attribute queue_id.
 */
static VALUE
action_enqueue_get_queue_id( VALUE self ) {
  return rb_iv_get( self, "@queue_id" );
}

/*
 * Appends the enqueue action to the list of actions.
 * 
 * @return [ActionEnqueue] self
 */
static VALUE
action_enqueue_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  uint32_t queue_id = ( uint32_t ) NUM2UINT( action_enqueue_get_queue_id( self ) );
  uint16_t port = ( uint16_t ) NUM2UINT( action_enqueue_get_port( self ) );

  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_enqueue( actions, port, queue_id );

  return self;
}


/*
 * A string representation of {ActionEnqueue}'s attributes.
 * 
 * @return [String] 
 */
static VALUE
action_enqueue_to_s( VALUE self ) {
  char str[ 64 ];

  uint32_t queue_id = ( uint32_t ) NUM2UINT( action_enqueue_get_queue_id( self ) );
  uint16_t port = ( uint16_t ) NUM2UINT( action_enqueue_get_port( self ) );
  sprintf( str, "#<%s> port = %u, queue_id = %u", rb_obj_classname( self ), port,
          queue_id );
  return rb_str_new2( str );
}


void
Init_action_enqueue( ) {
  cActionEnqueue = rb_define_class_under( mTrema, "ActionEnqueue", rb_cObject );
  rb_define_method( cActionEnqueue, "initialize", action_enqueue_init, 2 );
  rb_define_method( cActionEnqueue, "port", action_enqueue_get_port, 0 );
  rb_define_method( cActionEnqueue, "queue_id", action_enqueue_get_queue_id, 0 );
  rb_define_method( cActionEnqueue, "append", action_enqueue_append, 1 );
  rb_define_method( cActionEnqueue, "to_s", action_enqueue_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
