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


#include <string.h>
#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cPacketIn;


static VALUE
packet_in_init( VALUE self, VALUE datapath_id ) {
  rb_iv_set( self, "@datapath_id", datapath_id );
  return self;
}


static VALUE
packet_in_datapath_id( VALUE self ) {
  return rb_iv_get( self, "@datapath_id" );
}


void
Init_packet_in() {
  cPacketIn = rb_define_class_under( mTrema, "PacketIn", rb_cObject );
  rb_define_method( cPacketIn, "initialize", packet_in_init, 1 );
  rb_define_method( cPacketIn, "datapath_id", packet_in_datapath_id, 0 );
}


void
handle_packet_in( packet_in packet_in ) {
  VALUE _packet_in = rb_funcall( cPacketIn, rb_intern( "new" ), 1, ULL2NUM( packet_in.datapath_id ) );
  rb_funcall( ( VALUE ) packet_in.user_data, rb_intern( "packet_in" ), 1, _packet_in );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

