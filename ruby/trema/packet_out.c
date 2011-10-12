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
#include "packet_in.h"

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


#include "packet_shared.h"


static VALUE
packet_out_set_arp_sha( VALUE self, VALUE value ) {
  uint64_to_mac( NUM2ULL( value ), get_packet_shared_info( self )->arp_sha );
  return self;
}


static VALUE
packet_out_set_arp_spa( VALUE self, VALUE value ) {
  get_packet_shared_info( self )->arp_spa = ( uint32_t )NUM2UINT( value );
  return self;
}


static VALUE
packet_out_set_arp_tha( VALUE self, VALUE value ) {
  uint64_to_mac( NUM2ULL( value ), get_packet_shared_info( self )->arp_tha );
  return self;
}


static VALUE
packet_out_set_arp_tpa( VALUE self, VALUE value ) {
  get_packet_shared_info( self )->arp_tpa = ( uint32_t )NUM2UINT( value );
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

  /* typedef struct { */
  /*   uint64_t datapath_id; */
  /*   uint32_t transaction_id; */
  /*   uint32_t buffer_id; */
  /*   uint16_t total_len; */
  /*   uint16_t in_port; */
  /*   uint8_t reason; */
  /*   const buffer *data; */
  /*   void *user_data; */
  /* } packet_in; */

  packet_info *packet_info_src = ( packet_info * ) packet_src->data->user_data;

  memcpy( packet_dest, packet_info_src, sizeof( packet_out ) );
  memset( &packet_dest->info.l2_header, 0, sizeof( packet_info ) - offsetof( packet_info, l2_header ) );

  // Copy packet data into a form that we can use later for
  // constructing an out-packet for TCP or UDP.

  return obj;
}


static int
write_arp_length( packet_info *message ) {
  UNUSED( message );

  // Does not handle more complex packets yet...
  return sizeof( struct ether_header ) + sizeof( struct arp_header );
}


static int
write_arp( packet_info *message, void* buffer ) {
  // Does not handle more complex packets yet...
  void* ptr = buffer;
  
  struct ether_header *ether_header = ptr;
  memcpy( ether_header->ether_shost, message->eth_macsa, ETH_ADDRLEN );
  memcpy( ether_header->ether_dhost, message->eth_macda, ETH_ADDRLEN );
  ether_header->ether_type = htons( message->eth_type );

  ptr = ( void * ) ( ether_header + 1 );
  
  /* // vlan tag */
  /* if ( message->eth_type == ETH_ETHTYPE_TPID ) { */

  // Ethernet header
  arp_header_t *arp_header = ptr;
  arp_header->ar_hrd = htons( message->arp_ar_hrd );
  arp_header->ar_pro = htons( message->arp_ar_pro );
  arp_header->ar_hln = message->arp_ar_hln;
  arp_header->ar_pln = message->arp_ar_pln;
  arp_header->ar_op = htons( message->arp_ar_op );
  memcpy( arp_header->sha, message->arp_sha, ETH_ADDRLEN );
  arp_header->sip = htonl( message->arp_spa );
  memcpy( arp_header->tha, message->arp_tha, ETH_ADDRLEN );
  arp_header->tip = htonl( message->arp_tpa );

  ptr = ( void * ) ( arp_header + 1 );

  return ( int ) ( ( char * )ptr - ( char * )buffer );
}


static VALUE
packet_out_to_s( VALUE self ) {
  packet_info *info = get_packet_shared_info( self );

  if ( ! ( info->format & NW_ARP ) ) {
    // Add proper error handling.
    return Qnil;
  }

  // Use temporary size, fixme...
  char buffer[2048];
  int length = write_arp( info, ( void * )buffer );

  if ( length < ETH_MINIMUM_LENGTH ) {
    memset( buffer + length, 0, ( size_t )( ETH_MINIMUM_LENGTH - length ) );
    length = ETH_MINIMUM_LENGTH;
  }

  return rb_str_new( buffer, length );
}


void
Init_packet_out() {
  rb_require( "trema/ip" );
  rb_require( "trema/mac" );
  cPacketOut = rb_define_class_under( mTrema, "PacketOut", rb_cObject );
  rb_define_alloc_func( cPacketOut, packet_out_alloc );

  PACKET_SHARED_DEFINE_METHODS( cPacketOut );

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
