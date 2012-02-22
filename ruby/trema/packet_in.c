/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


#include <string.h>
#include "buffer.h"
#include "ruby.h"
#include "trema.h"


extern VALUE mTrema;
VALUE cPacketIn;
VALUE mPacketInARP;
VALUE mPacketInIPv4;
VALUE mPacketInICMPv4;
VALUE mPacketInIGMP;
VALUE mPacketInTCP;
VALUE mPacketInUDP;


#define PACKET_IN_RETURN_MAC( packet_member ) {                         \
  VALUE ret = ULL2NUM( mac_to_uint64( get_packet_in_info( self )->packet_member ) ); \
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret ); }

#define PACKET_IN_RETURN_IP( packet_member ) {                          \
  VALUE ret = ULONG2NUM( get_packet_in_info( self )->packet_member );   \
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, ret ); }


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
 * The ethernet type.
 *
 * @return [integer] eth_type The ehternet type.
 */
static VALUE
packet_in_eth_type( VALUE self ) {
  return UINT2NUM( get_packet_in_info( self )->eth_type );
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
  return UINT2NUM( ( unsigned int ) get_packet_in_info( self )->arp_ar_op );
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
 * Is an ICMPv4 packet?
 *
 * @return [bool] icmpv4? Is an ICMPv4 packet?
 */
static VALUE
packet_in_is_icmpv4( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ICMPV4 ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The ICMPv4 message type.
 *
 * @return [Integer] icmpv4_type The ICMPv4 message type.
 */
static VALUE
packet_in_icmpv4_type( VALUE self ) {
  return get_packet_in_info( self )->icmpv4_type;
}


/*
 * The ICMPv4 message code.
 *
 * @return [Integer] icmpv4_code The ICMPv4 message code.
 */
static VALUE
packet_in_icmpv4_code( VALUE self ) {
  return get_packet_in_info( self )->icmpv4_code;
}


/*
 * The ICMPv4 message checksum.
 *
 * @return [Integer] icmpv4_checksum The ICMPv4 message checksum.
 */
static VALUE
packet_in_icmpv4_checksum( VALUE self ) {
  return UINT2NUM( get_packet_in_info( self )->icmpv4_checksum );
}


/*
 * The identifier of ICMPv4 echo.
 *
 * @return [Integer] icmpv4_id The identifier of ICMPv4 echo.
 */
static VALUE
packet_in_icmpv4_id( VALUE self ) {
  return UINT2NUM( get_packet_in_info( self )->icmpv4_id );
}


/*
 * The sequence number of ICMPv4 echo.
 *
 * @return [Integer] icmpv4_id The sequence number of ICMPv4 echo.
 */
static VALUE
packet_in_icmpv4_seq( VALUE self ) {
  return UINT2NUM( get_packet_in_info( self )->icmpv4_seq );
}


/*
 * The gateway address of ICMPv4 redicect.
 *
 * @return [Trema::IP] icmp_gateway The gateway address of ICMPv4 redicect.
 */
static VALUE
packet_in_icmpv4_gateway( VALUE self ) {
  PACKET_IN_RETURN_IP( icmpv4_gateway );
}


/*
 * Is an IGMP packet?
 *
 * @return [bool] igmp? Is an IGMP packet?
 */
static VALUE
packet_in_is_igmp( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_IGMP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an IGMP membership query packet?
 *
 * @return [bool] igmp_membership_query? Is an IGMP membership query packet?
 */
static VALUE
packet_in_is_igmp_membership_query( VALUE self ) {
  if ( packet_type_igmp_membership_query( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an IGMP v1 membership report packet?
 *
 * @return [bool] igmp_v1_membership_report? Is an IGMP v1 membership report packet?
 */
static VALUE
packet_in_is_igmp_v1_membership_report( VALUE self ) {
  if ( packet_type_igmp_v1_membership_report( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an IGMP v2 membership report packet?
 *
 * @return [bool] igmp_v2_membership_report? Is an IGMP v2 membership report packet?
 */
static VALUE
packet_in_is_igmp_v2_membership_report( VALUE self ) {
  if ( packet_type_igmp_v2_membership_report( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an IGMP v2 leave group packet?
 *
 * @return [bool] igmp_v2_leave_group? Is an IGMP v2 leave group packet?
 */
static VALUE
packet_in_is_igmp_v2_leave_group( VALUE self ) {
  if ( packet_type_igmp_v2_leave_group( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an IGMP v3 membership report packet?
 *
 * @return [bool] igmp_v3_membership_report? Is an IGMP v3 membership report packet?
 */
static VALUE
packet_in_is_igmp_v3_membership_report( VALUE self ) {
  if ( packet_type_igmp_v3_membership_report( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The IGMP message type.
 *
 * @return [Integer] igmp_type IGMP type.
 */
static VALUE
packet_in_igmp_type( VALUE self ) {
  return UINT2NUM( get_packet_in_info( self )->igmp_type );
}


/*
 * The IGMP group address.
 *
 * @return [Trema::IP] igmp_group an IGMP group address.
 */
static VALUE
packet_in_igmp_group( VALUE self ) {
  PACKET_IN_RETURN_IP( igmp_group );
}


/*
 * Is a TCP packet?
 *
 * @return [bool] tcp? Is a TCP packet?
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
 * @return [Integer] tcp_src_port TCP port.
 */
static VALUE
packet_in_tcp_src_port( VALUE self ) {
  return ULONG2NUM( get_packet_in_info( self )->tcp_src_port );
}


/*
 * The TCP destination port.
 *
 * @return [Integer] tcp_dst_port TCP port.
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
  return rb_str_new( cpacket->l4_payload, ( long ) cpacket->l4_payload_length );
}


/*
 * The UDP source port.
 *
 * @return [Integer] udp_src_port UDP port.
 */
static VALUE
packet_in_udp_src_port( VALUE self ) {
  return ULONG2NUM( get_packet_in_info( self )->udp_src_port );
}


/*
 * The UDP destination port.
 *
 * @return [Integer] udp_dst_port UDP port.
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
  rb_define_method( cPacketIn, "eth_type", packet_in_eth_type, 0 );

  rb_define_method( cPacketIn, "arp?", packet_in_is_arp, 0 );
  rb_define_method( cPacketIn, "ipv4?", packet_in_is_ipv4, 0 );
  rb_define_method( cPacketIn, "icmpv4?", packet_in_is_icmpv4, 0 );
  rb_define_method( cPacketIn, "igmp?", packet_in_is_igmp, 0 );
  rb_define_method( cPacketIn, "tcp?", packet_in_is_tcp, 0 );
  rb_define_method( cPacketIn, "udp?", packet_in_is_udp, 0 );

  mPacketInARP = rb_define_module_under( mTrema, "PacketInARP" );
  rb_define_method( mPacketInARP, "arp_oper", packet_in_arp_oper, 0 );
  rb_define_method( mPacketInARP, "arp_sha", packet_in_arp_sha, 0 );
  rb_define_method( mPacketInARP, "arp_spa", packet_in_arp_spa, 0 );
  rb_define_method( mPacketInARP, "arp_tha", packet_in_arp_tha, 0 );
  rb_define_method( mPacketInARP, "arp_tpa", packet_in_arp_tpa, 0 );

  mPacketInIPv4 = rb_define_module_under( mTrema, "PacketInIPv4" );
  rb_define_method( mPacketInIPv4, "ipv4_saddr", packet_in_ipv4_saddr, 0 );
  rb_define_method( mPacketInIPv4, "ipv4_daddr", packet_in_ipv4_daddr, 0 );

  mPacketInICMPv4 = rb_define_module_under( mTrema, "PacketInICMPv4" );
  rb_define_method( mPacketInICMPv4, "icmpv4_type", packet_in_icmpv4_type, 0 );
  rb_define_method( mPacketInICMPv4, "icmpv4_code", packet_in_icmpv4_code, 0 );
  rb_define_method( mPacketInICMPv4, "icmpv4_checksum", packet_in_icmpv4_checksum, 0 );
  rb_define_method( mPacketInICMPv4, "icmpv4_id", packet_in_icmpv4_id, 0 );
  rb_define_method( mPacketInICMPv4, "icmpv4_seq", packet_in_icmpv4_seq, 0 );
  rb_define_method( mPacketInICMPv4, "icmpv4_group", packet_in_icmpv4_gateway, 0 );  

  mPacketInIGMP = rb_define_module_under( mTrema, "PacketInIGMP" );
  rb_define_method( mPacketInIGMP, "igmp_type", packet_in_igmp_type, 0 );
  rb_define_method( mPacketInIGMP, "igmp_group", packet_in_igmp_group, 0 );
  rb_define_method( mPacketInIGMP, "igmp_membership_query?", packet_in_is_igmp_membership_query, 0 );
  rb_define_method( mPacketInIGMP, "igmp_v1_membership_report?", packet_in_is_igmp_v1_membership_report, 0 );
  rb_define_method( mPacketInIGMP, "igmp_v2_membership_report?", packet_in_is_igmp_v2_membership_report, 0 );
  rb_define_method( mPacketInIGMP, "igmp_v2_leave_group?", packet_in_is_igmp_v2_leave_group, 0 );
  rb_define_method( mPacketInIGMP, "igmp_v3_membership_report?", packet_in_is_igmp_v3_membership_report, 0 );

  mPacketInTCP = rb_define_module_under( mTrema, "PacketInTCP" );
  rb_define_method( mPacketInTCP, "tcp_src_port", packet_in_tcp_src_port, 0 );
  rb_define_method( mPacketInTCP, "tcp_dst_port", packet_in_tcp_dst_port, 0 );

  mPacketInUDP = rb_define_module_under( mTrema, "PacketInUDP" );
  rb_define_method( mPacketInUDP, "udp_payload", packet_in_udp_payload, 0 );
  rb_define_method( mPacketInUDP, "udp_src_port", packet_in_udp_src_port, 0 );
  rb_define_method( mPacketInUDP, "udp_dst_port", packet_in_udp_dst_port, 0 );
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
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, mPacketInARP );
  }

  if ( ( info->format & NW_IPV4 ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, mPacketInIPv4 );
  }

  if ( ( info->format & NW_ICMPV4 ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, mPacketInICMPv4 );
  }

  if ( ( info->format & NW_IGMP ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, mPacketInIGMP );
  }

  if ( ( info->format & TP_TCP ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, mPacketInTCP );
  }

  if ( ( info->format & TP_UDP ) ) {
    rb_funcall( cPacketIn, rb_intern( "include" ), 1, mPacketInUDP );
  }

  rb_funcall( controller, rb_intern( "packet_in" ), 2, ULL2NUM( datapath_id ), r_message );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
