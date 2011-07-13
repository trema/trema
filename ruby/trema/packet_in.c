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
#include "buffer.h"
#include "rbuffer.h"
#include "ruby.h"
#include "trema.h"


extern VALUE mTrema;
VALUE cPacketIn;


static VALUE
packet_in_alloc( VALUE klass ) {
  packet_in *_packet_in = malloc( sizeof( packet_in ) );
  return Data_Wrap_Struct( klass, 0, free, _packet_in );
}


static VALUE
packet_in_datapath_id( VALUE self ) {
  packet_in *cpacket;

  Data_Get_Struct( self, packet_in, cpacket );

  return ULL2NUM( cpacket->datapath_id );
}


static VALUE
packet_in_buffer_id( VALUE self ) {
  packet_in *cpacket_in;
  Data_Get_Struct( self, packet_in, cpacket_in );
  return ULONG2NUM( cpacket_in->buffer_id );
}


static VALUE
packet_in_is_buffered( VALUE self ) {
  packet_in *cpacket_in;
  Data_Get_Struct( self, packet_in, cpacket_in );
  if ( cpacket_in->buffer_id == UINT32_MAX ) {
    return Qfalse;
  }
  else {
    return Qtrue;
  }
}


static VALUE
packet_in_in_port( VALUE self ) {
  packet_in *cpacket_in;
  Data_Get_Struct( self, packet_in, cpacket_in );
  return INT2NUM( cpacket_in->in_port );
}


static VALUE
packet_in_data( VALUE self ) {
  packet_in *from;
  buffer *to;

  VALUE rbuffer = rb_funcall( cBuffer, rb_intern( "new" ), 0 );
  Data_Get_Struct( rbuffer, buffer, to );
  Data_Get_Struct( self, packet_in, from );
  memcpy( to, from->data, sizeof( buffer ) );

  return rbuffer;
}


static VALUE
packet_in_macsa( VALUE self ) {
  packet_in *cpacket_in;
  Data_Get_Struct( self, packet_in, cpacket_in );
  VALUE macsa = ULL2NUM( mac_to_uint64( packet_info( cpacket_in->data )->l2_data.eth->macsa ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, macsa );
}


static VALUE
packet_in_macda( VALUE self ) {
  packet_in *cpacket_in;
  Data_Get_Struct( self, packet_in, cpacket_in );
  VALUE macda = ULL2NUM( mac_to_uint64( packet_info( cpacket_in->data )->l2_data.eth->macda ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, macda );
}


void
Init_packet_in() {
  rb_require( "trema/mac" );

  cPacketIn = rb_define_class_under( mTrema, "PacketIn", rb_cObject );
  rb_define_alloc_func( cPacketIn, packet_in_alloc );
  rb_define_method( cPacketIn, "datapath_id", packet_in_datapath_id, 0 );
  rb_define_method( cPacketIn, "buffer_id", packet_in_buffer_id, 0 );
  rb_define_method( cPacketIn, "buffered?", packet_in_is_buffered, 0 );
  rb_define_method( cPacketIn, "in_port", packet_in_in_port, 0 );
  rb_define_method( cPacketIn, "data", packet_in_data, 0 );
  rb_define_method( cPacketIn, "macsa", packet_in_macsa, 0 );
  rb_define_method( cPacketIn, "macda", packet_in_macda, 0 );
}


void
handle_packet_in( packet_in message ) {
  packet_in *tmp;

  VALUE r_message = rb_funcall( cPacketIn, rb_intern( "new" ), 0 );
  Data_Get_Struct( r_message, packet_in, tmp );
  memcpy( tmp, &message, sizeof( packet_in ) );

  VALUE controller = ( VALUE ) message.user_data;
  rb_funcall( controller, rb_intern( "packet_in" ), 2, ULL2NUM( message.datapath_id ), r_message );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

