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
#include <net/ethernet.h>
#include <netinet/ip.h>
#include "arp.h"
#include "buffer.h"
#include "ether.h"
#include "ruby.h"
#include "trema.h"
#include "packet_out.h"
#include "packet_shared.h"

extern VALUE mTrema;
VALUE cPacketOut;


typedef struct {
  // void *data; Payload data...
  packet_info info;
} packet_out;


static VALUE
packet_out_alloc( VALUE klass ) {
  packet_out *_packet_out = xmalloc( sizeof( packet_out ) );
  return Data_Wrap_Struct( klass, 0, xfree, _packet_out );
}


static packet_out *
get_packet_out( VALUE self ) {
  packet_out *cpacket;
  Data_Get_Struct( self, packet_out, cpacket );
  return cpacket;
}


static packet_info *
get_packet_shared_info( VALUE self ) {
  packet_out *cpacket;
  Data_Get_Struct( self, packet_out, cpacket );
  return &cpacket->info;
}


/*
 * The MAC source address.
 *
 * @return [Trema::Mac] macsa MAC source address.
 */
static VALUE
packet_out_macsa( VALUE self ) {
  PACKET_SHARED_RETURN_MAC( eth_macsa );
}


/*
 * The MAC destination address.
 *
 * @return [Trema::Mac] macda MAC destination address.
 */
static VALUE
packet_out_macda( VALUE self ) {
  PACKET_SHARED_RETURN_MAC( eth_macda );
}


/*
 * Is an ARP packet?
 *
 * @return [bool] arp? Is an ARP packet?
 */
static VALUE
packet_out_is_arp( VALUE self ) {
  if ( ( get_packet_shared_info( self )->format & NW_ARP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The ARP source hardware address.
 *
 * @return [Trema::Mac] arp_sha MAC hardware address.
 */
static VALUE
packet_out_arp_sha( VALUE self ) {
  PACKET_SHARED_RETURN_MAC( arp_sha );
}


/*
 * The ARP source protocol address.
 *
 * @return [Trema::IP] arp_spa IP protocol address.
 */
static VALUE
packet_out_arp_spa( VALUE self ) {
  PACKET_SHARED_RETURN_IP( arp_spa );
}


/*
 * The ARP target hardware address.
 *
 * @return [Trema::Mac] arp_tha MAC hardware address.
 */
static VALUE
packet_out_arp_tha( VALUE self ) {
  PACKET_SHARED_RETURN_MAC(arp_tha);
}


/*
 * The ARP target protocol address.
 *
 * @return [Trema::IP] arp_tpa IP protocol address.
 */
static VALUE
packet_out_arp_tpa( VALUE self ) {
  PACKET_SHARED_RETURN_IP( arp_tpa );
}


static VALUE
packet_out_set_macsa( VALUE self, VALUE value ) {
  PACKET_SHARED_SET_MAC( eth_macsa );
  return self;
}


static VALUE
packet_out_set_macda( VALUE self, VALUE value ) {
  PACKET_SHARED_SET_MAC( eth_macda );
  return self;
}


static VALUE
packet_out_set_arp_sha( VALUE self, VALUE value ) {
  PACKET_SHARED_SET_MAC( arp_sha );
  return self;
}


static VALUE
packet_out_set_arp_spa( VALUE self, VALUE value ) {
  PACKET_SHARED_SET_IP( arp_spa );
  return self;
}


static VALUE
packet_out_set_arp_tha( VALUE self, VALUE value ) {
  PACKET_SHARED_SET_MAC( arp_tha );
  return self;
}


static VALUE
packet_out_set_arp_tpa( VALUE self, VALUE value ) {
  PACKET_SHARED_SET_IP( arp_tpa );
  return self;
}


static VALUE
packet_out_from( VALUE self, VALUE message ) {
  VALUE obj;
  packet_in *packet_src;
  packet_out *packet_dest;

  // Validate the input message type.

  Data_Get_Struct( message, packet_in, packet_src );
  obj = rb_funcall( self, rb_intern( "new" ), 0 );
  Data_Get_Struct( obj, packet_out, packet_dest );

  packet_info *packet_info_src = ( packet_info * ) packet_src->data->user_data;

  memcpy( packet_dest, packet_info_src, sizeof( packet_out ) );
  memset( &packet_dest->info.l2_header, 0, sizeof( packet_info ) - offsetof( packet_info, l2_header ) );

  // Copy packet data into a form that we can use later for
  // constructing an out-packet for TCP or UDP.

  return obj;
}


static VALUE
packet_out_to_s( VALUE self ) {
  packet_info *info = get_packet_shared_info( self );

  // Use temporary size, fixme...
  char buffer[2048];
  int length = write_packet( info, buffer, 2048 );

  if ( length == -1 ) {
    return Qnil;
  }

  if ( length > 2048 ) {
    // Ehm... better handling of too large packets...
    return Qnil;
  }

  return rb_str_new( buffer, length );
}


void
Init_packet_out() {
  rb_require( "trema/ip" );
  rb_require( "trema/mac" );
  cPacketOut = rb_define_class_under( mTrema, "PacketOut", rb_cObject );
  rb_define_alloc_func( cPacketOut, packet_out_alloc );

  rb_define_method( cPacketOut, "macsa", packet_out_macsa, 0 );
  rb_define_method( cPacketOut, "macda", packet_out_macda, 0 );
  rb_define_method( cPacketOut, "macsa=", packet_out_set_macsa, 1 );
  rb_define_method( cPacketOut, "macda=", packet_out_set_macda, 1 );

  rb_define_method( cPacketOut, "arp?", packet_out_is_arp, 0 );
  rb_define_method( cPacketOut, "arp_sha", packet_out_arp_sha, 0 );
  rb_define_method( cPacketOut, "arp_spa", packet_out_arp_spa, 0 );
  rb_define_method( cPacketOut, "arp_tha", packet_out_arp_tha, 0 );
  rb_define_method( cPacketOut, "arp_tpa", packet_out_arp_tpa, 0 );
  rb_define_method( cPacketOut, "arp_sha=", packet_out_set_arp_sha, 1 );
  rb_define_method( cPacketOut, "arp_spa=", packet_out_set_arp_spa, 1 );
  rb_define_method( cPacketOut, "arp_tha=", packet_out_set_arp_tha, 1 );
  rb_define_method( cPacketOut, "arp_tpa=", packet_out_set_arp_tpa, 1 );

  rb_define_singleton_method( cPacketOut, "from", packet_out_from, 1 );
  rb_define_method( cPacketOut, "to_s", packet_out_to_s, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
