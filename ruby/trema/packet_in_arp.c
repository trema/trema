/*
 * Author: Jari Sundell
 *
 * Copyright (C) 2011 axsh Ltd.
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
#include "packet_in.h"
#include "ruby.h"
#include "trema.h"

extern VALUE mTrema;
VALUE cPacketInArp;


static packet_in *
get_packet_in( VALUE self ) {
  packet_in *cpacket;
  Data_Get_Struct( self, packet_in, cpacket );
  return cpacket;
}


static VALUE
packet_in_arp_sha( VALUE self ) {
  packet_in *cpacket_in = get_packet_in( self );
  VALUE value = ULL2NUM( mac_to_uint64( ( ( packet_info * ) cpacket_in->data->user_data )->arp_sha ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, value );
}


static VALUE
packet_in_arp_spa( VALUE self ) {
  packet_in *cpacket_in = get_packet_in( self );
  VALUE value = ULONG2NUM( ( ( packet_info * ) cpacket_in->data->user_data )->arp_spa );
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, value );
}


static VALUE
packet_in_arp_tha( VALUE self ) {
  packet_in *cpacket_in = get_packet_in( self );
  VALUE value = ULL2NUM( mac_to_uint64( ( ( packet_info * ) cpacket_in->data->user_data )->arp_tha ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, value );
}


static VALUE
packet_in_arp_tpa( VALUE self ) {
  packet_in *cpacket_in = get_packet_in( self );
  VALUE value = ULONG2NUM( ( ( packet_info * ) cpacket_in->data->user_data )->arp_tpa );
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, value );
}


static VALUE
match_from( VALUE self, VALUE message ) {
  VALUE obj;
  packet_in *packet_src, *packet_dest;
  packet_info *info = NULL;

  Data_Get_Struct( message, packet_in, packet_src );

  info = ( packet_info * ) packet_src->data->user_data;

  if ( ! ( info->format & NW_ARP ) ) {
    return Qnil;
  }

  obj = rb_funcall( self, rb_intern( "new" ), 0 );
  Data_Get_Struct( obj, packet_in, packet_dest );

  memcpy( packet_dest, packet_src, sizeof( packet_in ) );
  return obj;
}


void
Init_packet_in_arp() {
   rb_require( "trema/ip" );
  cPacketInArp = rb_define_class_under( mTrema, "PacketInArp", cPacketIn );

  rb_define_method( cPacketIn, "arp_sha", packet_in_arp_sha, 0 );
  rb_define_method( cPacketIn, "arp_spa", packet_in_arp_spa, 0 );
  rb_define_method( cPacketIn, "arp_tha", packet_in_arp_tha, 0 );
  rb_define_method( cPacketIn, "arp_tpa", packet_in_arp_tpa, 0 );

  rb_define_singleton_method( cPacketInArp, "from", match_from, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
