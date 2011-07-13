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
VALUE cActionSetVlanVid;

static VALUE
action_set_vlan_vid_init( VALUE self, VALUE vlan_vid ) {
  rb_iv_set( self, "@vlan_vid", vlan_vid );
  return self;
}

static VALUE
action_get_vlan_vid( VALUE self ) {
  return NUM2UINT( rb_iv_get( self, "@vlan_vid" ) );
}

static VALUE
action_set_vlan_vid_append( VALUE self, VALUE action_ptr ) {
  openflow_actions *actions;

  Data_Get_Struct( action_ptr, openflow_actions, actions );
  append_action_set_vlan_vid( actions, action_get_vlan_vid( self ) );

  return self;
}

void
Init_action_set_vlan_vid( ) {
  cActionSetVlanVid = rb_define_class_under( mTrema, "ActionSetVlanVid", rb_cObject );
  rb_define_method( cActionSetVlanVid, "initialize", action_set_vlan_vid_init, 1 );
  rb_define_method( cActionSetVlanVid, "vlan_vid", action_get_vlan_vid, 0 );
  rb_define_method( cActionSetVlanVid, "append", action_set_vlan_vid_append, 1 );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
