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

static VALUE
action_enqueue_init( VALUE self, VALUE port, VALUE queue_id ) {
  rb_iv_set( self, "@port", port );
  rb_iv_set( self, "@queue_id", queue_id );
  return self;
}

static VALUE
action_get_port( VALUE self ) {
  return NUM2UINT( rb_iv_get( self, "@port" ) );
}

static VALUE
action_get_queue_id( VALUE self ) {
  return NUM2UINT( rb_iv_get( self, "@queue_id" ) );
}

static VALUE
action_enqueue_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;

  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_enqueue( actions, (uint16_t) action_get_port( self ), (uint16_t) action_get_queue_id( self ) );

  return self;
}

void
Init_action_enqueue( ) {
  cActionEnqueue = rb_define_class_under( mTrema, "ActionEnqueue", rb_cObject );
  rb_define_method( cActionEnqueue, "initialize", action_enqueue_init, 2 );
  rb_define_method( cActionEnqueue, "port", action_get_port, 0 );
  rb_define_method( cActionEnqueue, "queue_id", action_get_queue_id, 0 );
  rb_define_method( cActionEnqueue, "append", action_enqueue_append, 1 );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
