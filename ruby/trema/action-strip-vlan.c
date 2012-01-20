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
VALUE cActionStripVlan;


/*
 * Strips the VLAN tag of a packet.
 *
 * @overload initialize()
 *
 * @return [ActionStripVlan] an object that encapsulates this action.
 */
static VALUE
action_strip_vlan_init( VALUE self ) {
  return self;
}


/*
 * Appends its action(strip VLAN tag) to the list of actions.
 *
 * @return [ActionStripVlan] self
 */
static VALUE
action_strip_vlan_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;
  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_strip_vlan( actions );
  return self;
}


/*
 * (see ActionEnqueue#inspect)
 */
static VALUE
action_strip_vlan_inspect( VALUE self ) {
  char str[ 64 ];
  sprintf( str, "#<%s>", rb_obj_classname( self ) );
  return rb_str_new2( str );
}


void
Init_action_strip_vlan() {
  cActionStripVlan = rb_define_class_under( mTrema, "ActionStripVlan", rb_cObject );
  rb_define_method( cActionStripVlan, "initialize", action_strip_vlan_init, 0 );
  rb_define_method( cActionStripVlan, "append", action_strip_vlan_append, 1 );
  rb_define_method( cActionStripVlan, "inspect", action_strip_vlan_inspect, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
