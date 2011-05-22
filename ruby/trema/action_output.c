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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cActionOutput;


static VALUE
action_output_init( VALUE self, VALUE port ) {
  rb_iv_set( self, "@port", port );
  return self;
}


static VALUE
action_output_port( VALUE self ) {
  return NUM2INT( rb_iv_get( self, "@port" ) );
}


void
Init_action_output() {
  cActionOutput = rb_define_class_under( mTrema, "ActionOutput", rb_cObject );
  rb_define_method( cActionOutput, "initialize", action_output_init, 1 );
  rb_define_method( cActionOutput, "port", action_output_port, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
