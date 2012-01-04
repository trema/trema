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
#include "ruby.h"
#include "trema.h"


extern VALUE mTrema;
VALUE cPacketIn;
VALUE cPacketInARP;
VALUE cPacketInIPv4;
VALUE cPacketInTCP;
VALUE cPacketInUDP;


#define PACKET_IN_RETURN_MAC( packet_member )                         \
  VALUE ret = ULL2NUM( mac_to_uint64( get_packet_in_info( self )->packet_member ) ); \
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );

#define PACKET_IN_RETURN_IP( packet_member )                          \
  VALUE ret = ULONG2NUM( get_packet_in_info( self )->packet_member );   \
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, ret );


#if 0
/*
 * @overload initialize()
 *   Allocates and wraps a {PacketIn} object to store the details of 
 *   the +OFPT_PACKET_IN+ message.
 *
 * @return [PacketIn] an object that encapsulates the +OPFT_PACKET_IN+ OpenFlow message. 
 */
static VALUE
packet_in_init( VALUE kclass ) {
  packet_in *_packet_in = xmalloc( sizeof( packet_in ) );
  return Data_Wrap_Struct( klass, 0, xfree, _packet_in );
}
#endif


static VALUE
packet_in_alloc( VALUE klass ) {
  packet_in *_packet_in = xmalloc( sizeof( packet_in ) );
  return Data_Wrap_Struct( klass, 0, xfree, _packet_in );
}


static packet_in *
get_packet_in( VALUE self ) {
  packet_in *cpacket;
  Data_Get_Struct( self, packet_in, cpacket );
  return cpacket;
}


static packet_info *
get_packet_in_info( VALUE self ) {
  packet_in *cpacket;
  Data_Get_Struct( self, packet_in, cpacket );
  return ( packet_info * ) cpacket->data->user_data;
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
packet_in_datapath_id( VALUE self ) {
  return ULL2NUM( get_packet_in( self )->datapath_id );
}


/*
 * For this asynchronous message the transaction_id is set to zero.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
packet_in_transaction_id( VALUE self ) {
  return ULONG2NUM( get_packet_in( self )->transaction_id );
}


/*
 * Buffer id value signifies if the entire frame (packet is not buffered) or 
 * portion of it (packet is buffered) is included in the data field of 
 * this +OFPT_PACKET_IN+ message.
 *
 * @return [Number] the value of buffer id.
 */
static VALUE
packet_in_buffer_id( VALUE self ) {
  return ULONG2NUM( get_packet_in( self )->buffer_id );
}


/*
 * A buffer_id value either than +UINT32_MAX+ marks the packet_in as buffered.
 *
 * @return [true] if packet_in is buffered.
 * @return [false] if packet_in is not buffered.
 */
static VALUE
packet_in_is_buffered( VALUE self ) {
  if ( get_packet_in( self )->buffer_id == UINT32_MAX ) {
    return Qfalse;
  }
  else {
    return Qtrue;
  }
}


/*
 * The port the frame was received.
 *
 * @return [Number] the value of in_port.
 */
static VALUE
packet_in_in_port( VALUE self ) {
  return INT2NUM( get_packet_in( self )->in_port );
}


/*
 * The full length of the received frame.
 *
 * @return [Number] the value of total_len.
 */
static VALUE
packet_in_total_len( VALUE self ) {
  return INT2NUM( get_packet_in( self )->total_len );
}


/*
 * A String that holds the entire or portion of the received frame.
 * Length of data, total_len - 20 bytes.
 *
 * @return [String] the value of data.
 */
static VALUE
packet_in_data( VALUE self ) {
  const buffer *buf = get_packet_in( self )->data;
  return rb_str_new( buf->data, ( long ) buf->length );
}


/*
 * The reason why the +OFPT_PACKET_IN+ message was sent.
 *
 * @return [Number] the value of reason.
 */
static VALUE
packet_in_reason( VALUE self ) {
  return INT2NUM( get_packet_in( self )->reason );
}


/*
 * The MAC source address.
 *
 * @return [Trema::Mac] macsa MAC source address.
 */
static VALUE
packet_in_macsa( VALUE self ) {
  PACKET_IN_RETURN_MAC( eth_macsa );
}


/*
 * The MAC destination address.
 *
 * @return [Trema::Mac] macda MAC destination address.
 */
static VALUE
packet_in_macda( VALUE self ) {
  PACKET_IN_RETURN_MAC( eth_macda );
}


/*
 * Is an ARP packet?
 *
 * @return [bool] arp? Is an ARP packet?
 */
static VALUE
packet_in_is_arp( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ARP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The ARP operation code.
 *
 * @return [integer] arp_oper Operation code.
 */
static VALUE
packet_in_arp_oper( VALUE self ) {
  return get_packet_in_info( self )->arp_ar_op;
}


/*
 * The ARP source hardware address.
 *
 * @return [Trema::Mac] arp_sha MAC hardware address.
 */
static VALUE
packet_in_arp_sha( VALUE self ) {
  PACKET_IN_RETURN_MAC( arp_sha );
}


/*
 * The ARP source protocol address.
 *
 * @return [Trema::IP] arp_spa IP protocol address.
 */
static VALUE
packet_in_arp_spa( VALUE self ) {
  PACKET_IN_RETURN_IP( arp_spa );
}


/*
 * The ARP target hardware address.
 *
 * @return [Trema::Mac] arp_tha MAC hardware address.
 */
static VALUE
packet_in_arp_tha( VALUE self ) {
  PACKET_IN_RETURN_MAC( arp_tha );
}


/*
 * The ARP target protocol address.
 *
 * @return [Trema::IP] arp_tpa IP protocol address.
 */
static VALUE
packet_in_arp_tpa( VALUE self ) {
  PACKET_IN_RETURN_IP( arp_tpa );
}


/*
 * Is an IPV4 packet?
 *
 * @return [bool] ipv4? Is an IPV4 packet?
 */
static VALUE
packet_in_is_ipv4( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_IPV4 ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The IPV4 source protocol address.
 *
 * @return [Trema::IP] ipv4_saddr IP protocol address.
 */
static VALUE
packet_in_ipv4_saddr( VALUE self ) {
  PACKET_IN_RETURN_IP( ipv4_saddr );
}


/*
 * The IPV4 destination protocol address.
 *
 * @return [Trema::IP] ipv4_daddr IP protocol address.
 */
static VALUE
packet_in_ipv4_daddr( VALUE self ) {
  PACKET_IN_RETURN_IP( ipv4_daddr );
}


/*
 * Is an TCP packet?
 *
 * @return [bool] tcp? Is an TCP packet?
 */
static VALUE
packet_in_is_tcp( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & TP_TCP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The TCP source port.
 *
 * @return [Trema::IP] tcp_src_port TCP port.
 */
static VALUE
packet_in_tcp_src_port( VALUE self ) {
  return ULONG2NUM( get_packet_in_info( self )->tcp_src_port );
}


/*
 * The TCP destination port.
 *
 * @return [Trema::IP] tcp_dst_port TCP port.
 */
static VALUE
packet_in_tcp_dst_port( VALUE self ) {
  return ULONG2NUM( get_packet_in_info( self )->tcp_dst_port );
}


/*
 * Is an UDP packet?
 *
 * @return [bool] udp? Is an UDP packet?
 */
static VALUE
packet_in_is_udp( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & TP_UDP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * A String that holds the UDP payload.
 * Length of data, total_len - 20 bytes.
 *
 * @return [String] the value of data.
 */
static VALUE
packet_in_udp_payload( VALUE self ) {
  packet_info *cpacket = get_packet_in_info( self );
  return rb_str_new( cpacket->l4_payload, ( long ) cpacket->udp_len );
}


/*
 * The UDP source port.
 *
 * @return [Trema::IP] udp_src_port UDP port.
 */
static VALUE
packet_in_udp_src_port( VALUE self ) {
  return ULONG2NUM( get_packet_in_info( self )->udp_src_port );
}


/*
 * The UDP destination port.
 *
 * @return [Trema::IP] udp_dst_port UDP port.
 */
static VALUE
packet_in_udp_dst_port( VALUE self ) {
  return ULONG2NUM( get_packet_in_info( self )->udp_dst_port );
}


void
Init_packet_in() {
  rb_require( "trema/ip" );
  rb_require( "trema/mac" );
  cPacketIn = rb_define_class_under( mTrema, "PacketIn", rb_cObject );
  rb_define_alloc_func( cPacketIn, packet_in_alloc );
#if 0
  /*
   * Do not remove this is to fake yard to create a constructor for 
   * PacketIn object.
   */
  rb_define_method( cPacketIn, "initialize", packet_in_init, 0 );
#endif  
  rb_define_method( cPacketIn, "datapath_id", packet_in_datapath_id, 0 );
  rb_define_method( cPacketIn, "transaction_id", packet_in_transaction_id, 0 );  
  rb_define_method( cPacketIn, "buffer_id", packet_in_buffer_id, 0 );
  rb_define_method( cPacketIn, "buffered?", packet_in_is_buffered, 0 );
  rb_define_method( cPacketIn, "in_port", packet_in_in_port, 0 );
  rb_define_method( cPacketIn, "total_len", packet_in_total_len, 0 );
  rb_define_method( cPacketIn, "reason", packet_in_reason, 0 );
  rb_define_method( cPacketIn, "data", packet_in_data, 0 );

  rb_define_method( cPacketIn, "macsa", packet_in_macsa, 0 );
  rb_define_method( cPacketIn, "macda", packet_in_macda, 0 );

  rb_define_method( cPacketIn, "arp?", packet_in_is_arp, 0 );
  rb_define_method( cPacketIn, "ipv4?", packet_in_is_ipv4, 0 );
  rb_define_method( cPacketIn, "tcp?", packet_in_is_tcp, 0 );
  rb_define_method( cPacketIn, "udp?", packet_in_is_udp, 0 );

  cPacketInARP = rb_define_module_under( mTrema, "PacketInARP" );
  rb_define_method( cPacketInARP, "arp_oper", packet_in_arp_oper, 0 );
  rb_define_method( cPacketInARP, "arp_sha", packet_in_arp_sha, 0 );
  rb_define_method( cPacketInARP, "arp_spa", packet_in_arp_spa, 0 );
  rb_define_method( cPacketInARP, "arp_tha", packet_in_arp_tha, 0 );
  rb_define_method( cPacketInARP, "arp_tpa", packet_in_arp_tpa, 0 );

  cPacketInIPv4 = rb_define_module_under( mTrema, "PacketInIPv4" );
  rb_define_method( cPacketInIPv4, "ipv4_saddr", packet_in_ipv4_saddr, 0 );
  rb_define_method( cPacketInIPv4, "ipv4_daddr", packet_in_ipv4_daddr, 0 );

  cPacketInTCP = rb_define_module_under( mTrema, "PacketInTCP" );
  rb_define_method( cPacketInTCP, "tcp_src_port", packet_in_tcp_src_port, 0 );
  rb_define_method( cPacketInTCP, "tcp_dst_port", packet_in_tcp_dst_port, 0 );

  cPacketInUDP = rb_define_module_under( mTrema, "PacketInUDP" );
  rb_define_method( cPacketInUDP, "udp_payload", packet_in_udp_payload, 0 );
  rb_define_method( cPacketInUDP, "udp_src_port", packet_in_udp_src_port, 0 );
  rb_define_method( cPacketInUDP, "udp_dst_port", packet_in_udp_dst_port, 0 );
}


/*
 * Handler called when +OFPT_PACKET_IN+ message is received.
 */
void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  VALUE controller = ( VALUE ) message.user_data;
  if ( rb_respond_to( controller, rb_intern( "packet_in" ) ) == Qfalse ) {
    return;
  }

  VALUE r_message = rb_funcall( cPacketIn, rb_intern( "new" ), 0 );
  packet_in *tmp = NULL;
  Data_Get_Struct( r_message, packet_in, tmp );
  memcpy( tmp, &message, sizeof( packet_in ) );

  packet_info* info = ( packet_info * ) tmp->data->user_data;

  if ( ( info->format & NW_ARP ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, cPacketInARP );
  }

  if ( ( info->format & NW_IPV4 ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, cPacketInIPv4 );
  }

  if ( ( info->format & TP_TCP ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, cPacketInTCP );
  }

  if ( ( info->format & TP_UDP ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, cPacketInUDP );
  }

  rb_funcall( controller, rb_intern( "packet_in" ), 2, ULL2NUM( datapath_id ), r_message );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
