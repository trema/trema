/*
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


#define PACKET_IN_RETURN_MAC( packet_member )                                          \
  {                                                                                    \
    VALUE ret = ULL2NUM( mac_to_uint64( get_packet_in_info( self )->packet_member ) ); \
    return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );   \
  }

#define PACKET_IN_RETURN_IP( packet_member )                                        \
  {                                                                                 \
    VALUE ret = ULONG2NUM( get_packet_in_info( self )->packet_member );             \
    return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, ret ); \
  }

#define PACKET_IN_RETURN_NUM( flag, func, packet_member )               \
  {                                                                     \
    if ( get_packet_in_info( self )->format & flag ) {                  \
      return func( get_packet_in_info( self )->packet_member );         \
    } else {                                                            \
      return Qnil;                                                      \
    }                                                                   \
  }


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
  return UINT2NUM( get_packet_in( self )->in_port );
}


/*
 * The full length of the received frame.
 *
 * @return [Number] the value of total_len.
 */
static VALUE
packet_in_total_len( VALUE self ) {
  return UINT2NUM( get_packet_in( self )->total_len );
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
  return UINT2NUM( ( unsigned int ) get_packet_in( self )->reason );
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
 * Is a packet with VLAN tag?
 *
 * @return [bool] vtag? Is a packet with VLAN tag?
 */
static VALUE
packet_in_is_vtag( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & ETH_8021Q ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * The vlan tpid.
 *
 * @return [integer] vlan_tpid The vlan tpid
 */
static VALUE
packet_in_vlan_tpid( VALUE self ) {
  PACKET_IN_RETURN_NUM( ETH_8021Q, UINT2NUM, vlan_tpid );
}


/*
 * The vlan tci.
 *
 * @return [integer] vlan_tci The vlan tci
 */
static VALUE
packet_in_vlan_tci( VALUE self ) {
  PACKET_IN_RETURN_NUM( ETH_8021Q, UINT2NUM, vlan_tci );
}


/*
 * The vlan prio.
 *
 * @return [integer] vlan_prio The vlan prio
 */
static VALUE
packet_in_vlan_prio( VALUE self ) {
  PACKET_IN_RETURN_NUM( ETH_8021Q, UINT2NUM, vlan_prio );
}


/*
 * The vlan cfi.
 *
 * @return [integer] vlan_cfi The vlan cfi
 */
static VALUE
packet_in_vlan_cfi( VALUE self ) {
  PACKET_IN_RETURN_NUM( ETH_8021Q, UINT2NUM, vlan_cfi );
}


/*
 * The vlan vid.
 *
 * @return [integer] vlan_vid The vlan vid
 */
static VALUE
packet_in_vlan_vid( VALUE self ) {
  PACKET_IN_RETURN_NUM( ETH_8021Q, UINT2NUM, vlan_vid );
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
 * Is an ARP request packet?
 *
 * @return [bool] arp_request? Is an ARP request packet?
 */
static VALUE
packet_in_is_arp_request( VALUE self ) {
  if ( packet_type_arp_request( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an ARP reply packet?
 *
 * @return [bool] arp_reply? Is an ARP reply packet?
 */
static VALUE
packet_in_is_arp_reply( VALUE self ) {
  if ( packet_type_arp_reply( get_packet_in( self )->data ) ) {
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
  PACKET_IN_RETURN_NUM( NW_ARP, UINT2NUM, arp_ar_op );
}


/*
 * The ARP source hardware address.
 *
 * @return [Trema::Mac] arp_sha MAC hardware address.
 */
static VALUE
packet_in_arp_sha( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ARP ) ) {
    PACKET_IN_RETURN_MAC( arp_sha );
  }
  else {
    return Qnil;
  }
}


/*
 * The ARP source protocol address.
 *
 * @return [Trema::IP] arp_spa IP protocol address.
 */
static VALUE
packet_in_arp_spa( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ARP ) ) {
    PACKET_IN_RETURN_IP( arp_spa );
  }
  else {
    return Qnil;
  }
}


/*
 * The ARP target hardware address.
 *
 * @return [Trema::Mac] arp_tha MAC hardware address.
 */
static VALUE
packet_in_arp_tha( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ARP ) ) {
    PACKET_IN_RETURN_MAC( arp_tha );
  }
  else {
    return Qnil;
  }
}


/*
 * The ARP target protocol address.
 *
 * @return [Trema::IP] arp_tpa IP protocol address.
 */
static VALUE
packet_in_arp_tpa( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ARP ) ) {
    PACKET_IN_RETURN_IP( arp_tpa );
  }
  else {
    return Qnil;
  }
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
 * The IPv4 version number.
 *
 * @return [Integer] ipv4_version The IPv4 version number.
 */
static VALUE
packet_in_ipv4_version( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_version );
}


/*
 * The IPv4 internet header length.
 *
 * @return [Integer] ipv4_ihl The IPv4 internet header length.
 */
static VALUE
packet_in_ipv4_ihl( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_ihl );
}


/*
 * The IPv4 tos value.
 *
 * @return [Integer] ipv4_tos The IPv4 tos value.
 */
static VALUE
packet_in_ipv4_tos( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_tos );
}


/*
 * The IPv4 total length.
 *
 * @return [Integer] ipv4_tot_len The IPv4 total length.
 */
static VALUE
packet_in_ipv4_tot_len( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_tot_len );
}


/*
 * The IPv4 identifier.
 *
 * @return [Integer] ipv4_id The IPv4 identifier.
 */
static VALUE
packet_in_ipv4_id( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_id );
}


/*
 * The IPv4 fragment offset.
 *
 * @return [Integer] ipv4_frag_off The IPv4 fragment offset.
 */
static VALUE
packet_in_ipv4_frag_off( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_frag_off );
}


/*
 * The IPv4 ttl value.
 *
 * @return [Integer] ipv4_ttl The IPv4 ttl value.
 */
static VALUE
packet_in_ipv4_ttl( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_ttl );
}


/*
 * The IPv4 protocol number.
 *
 * @return [Integer] ipv4_protocol The IPv4 protocol number.
 */
static VALUE
packet_in_ipv4_protocol( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_protocol );
}


/*
 * The IPv4 checksum.
 *
 * @return [Integer] ipv4_checksum The IPv4 checksum.
 */
static VALUE
packet_in_ipv4_checksum( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IPV4, UINT2NUM, ipv4_checksum );
}


/*
 * The IPV4 source protocol address.
 *
 * @return [Trema::IP] ipv4_saddr IP protocol address.
 */
static VALUE
packet_in_ipv4_saddr( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_IPV4 ) ) {
    PACKET_IN_RETURN_IP( ipv4_saddr );
  }
  else {
    return Qnil;
  }
}


/*
 * The IPV4 destination protocol address.
 *
 * @return [Trema::IP] ipv4_daddr IP protocol address.
 */
static VALUE
packet_in_ipv4_daddr( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_IPV4 ) ) {
    PACKET_IN_RETURN_IP( ipv4_daddr );
  }
  else {
    return Qnil;
  }
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
  PACKET_IN_RETURN_NUM( NW_ICMPV4, UINT2NUM, icmpv4_type );
}


/*
 * The ICMPv4 message code.
 *
 * @return [Integer] icmpv4_code The ICMPv4 message code.
 */
static VALUE
packet_in_icmpv4_code( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_ICMPV4, UINT2NUM, icmpv4_code );
}


/*
 * The ICMPv4 message checksum.
 *
 * @return [Integer] icmpv4_checksum The ICMPv4 message checksum.
 */
static VALUE
packet_in_icmpv4_checksum( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_ICMPV4, UINT2NUM, icmpv4_checksum );
}


/*
 * The identifier of ICMPv4 echo.
 *
 * @return [Integer] icmpv4_id The identifier of ICMPv4 echo.
 */
static VALUE
packet_in_icmpv4_id( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_ICMPV4, UINT2NUM, icmpv4_id );
}


/*
 * The sequence number of ICMPv4 echo.
 *
 * @return [Integer] icmpv4_id The sequence number of ICMPv4 echo.
 */
static VALUE
packet_in_icmpv4_seq( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_ICMPV4, UINT2NUM, icmpv4_seq );
}


/*
 * The gateway address of ICMPv4 redicect.
 *
 * @return [Trema::IP] icmp_gateway The gateway address of ICMPv4 redicect.
 */
static VALUE
packet_in_icmpv4_gateway( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_ICMPV4 ) ) {
    PACKET_IN_RETURN_IP( icmpv4_gateway );
  }
  else {
    return Qnil;
  }
}


/*
 * Is an ICMPv4 echo reply packet?
 *
 * @return [bool] icmpv4_echo_reply? Is an ICMP echo reply packet?
 */
static VALUE
packet_in_is_icmpv4_echo_reply( VALUE self ) {
  if ( packet_type_icmpv4_echo_reply( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an ICMPv4 destination unreachable packet?
 *
 * @return [bool] icmpv4_dst_unreach? Is an ICMP destination unreachable packet?
 */
static VALUE
packet_in_is_icmpv4_dst_unreach( VALUE self ) {
  if ( packet_type_icmpv4_dst_unreach( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an ICMPv4 redirect packet?
 *
 * @return [bool] icmpv4_redirect? Is an ICMP redirect packet?
 */
static VALUE
packet_in_is_icmpv4_redirect( VALUE self ) {
  if ( packet_type_icmpv4_redirect( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


/*
 * Is an ICMPv4 echo request packet?
 *
 * @return [bool] icmpv4_echo_request? Is an ICMP echo request packet?
 */
static VALUE
packet_in_is_icmpv4_echo_request( VALUE self ) {
  if ( packet_type_icmpv4_echo_request( get_packet_in( self )->data ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
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
  PACKET_IN_RETURN_NUM( NW_IGMP, UINT2NUM, igmp_type );
}


/*
 * The IGMP group address.
 *
 * @return [Trema::IP] igmp_group an IGMP group address.
 */
static VALUE
packet_in_igmp_group( VALUE self ) {
  if ( ( get_packet_in_info( self )->format & NW_IGMP ) ) {
    PACKET_IN_RETURN_IP( igmp_group );
  }
  else {
    return Qnil;
  }
}


/*
 * The IGMP checksum.
 *
 * @return [Integer] igmp_checksum a IGMP checksum.
 */
static VALUE
packet_in_igmp_checksum( VALUE self ) {
  PACKET_IN_RETURN_NUM( NW_IGMP, UINT2NUM, igmp_checksum );
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
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_src_port );
}


/*
 * The TCP destination port.
 *
 * @return [Integer] tcp_dst_port TCP port.
 */
static VALUE
packet_in_tcp_dst_port( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_dst_port );
}


/*
 * The TCP sequence number.
 *
 * @return [Integer] tcp_seq_no a TCP sequence number.
 */
static VALUE
packet_in_tcp_seq_no( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, ULONG2NUM, tcp_seq_no );
}


/*
 * The TCP acknowledge number.
 *
 * @return [Integer] tcp_ack_no a TCP acknowkedge number.
 */
static VALUE
packet_in_tcp_ack_no( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, ULONG2NUM, tcp_ack_no );
}


/*
 * The TCP offset.
 *
 * @return [Integer] tcp_offset a TCP offset.
 */
static VALUE
packet_in_tcp_offset( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_offset );
}


/*
 * The TCP flags.
 *
 * @return [Integer] tcp_flags TCP flags.
 */
static VALUE
packet_in_tcp_flags( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_flags );
}


/*
 * The TCP window.
 *
 * @return [Integer] tcp_window a TCP window.
 */
static VALUE
packet_in_tcp_window( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_window );
}


/*
 * The TCP checksum.
 *
 * @return [Integer] tcp_checksum a TCP checksum.
 */
static VALUE
packet_in_tcp_checksum( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_checksum );
}


/*
 * The TCP urgent.
 *
 * @return [Integer] tcp_urgent a TCP urgent.
 */
static VALUE
packet_in_tcp_urgent( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_TCP, UINT2NUM, tcp_urgent );
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
  PACKET_IN_RETURN_NUM( TP_UDP, UINT2NUM, udp_src_port );
}


/*
 * The UDP destination port.
 *
 * @return [Integer] udp_dst_port UDP port.
 */
static VALUE
packet_in_udp_dst_port( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_UDP, UINT2NUM, udp_dst_port );
}


/*
 * The UDP length.
 *
 * @return [Integer] udp_len a UDP length.
 */
static VALUE
packet_in_udp_len( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_UDP, UINT2NUM, udp_len );
}


/*
 * The UDP checksum.
 *
 * @return [Integer] udp_checksum a UDP checksum.
 */
static VALUE
packet_in_udp_checksum( VALUE self ) {
  PACKET_IN_RETURN_NUM( TP_UDP, UINT2NUM, udp_checksum );
}


void
Init_packet_in() {
  rb_require( "trema/ip" );
  rb_require( "trema/mac" );
  cPacketIn = rb_define_class_under( mTrema, "PacketIn", rb_cObject );
  rb_define_alloc_func( cPacketIn, packet_in_alloc );

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

  rb_define_method( cPacketIn, "vtag?", packet_in_is_vtag, 0 );
  rb_define_method( cPacketIn, "arp?", packet_in_is_arp, 0 );
  rb_define_method( cPacketIn, "ipv4?", packet_in_is_ipv4, 0 );
  rb_define_method( cPacketIn, "icmpv4?", packet_in_is_icmpv4, 0 );
  rb_define_method( cPacketIn, "igmp?", packet_in_is_igmp, 0 );
  rb_define_method( cPacketIn, "tcp?", packet_in_is_tcp, 0 );
  rb_define_method( cPacketIn, "udp?", packet_in_is_udp, 0 );

  rb_define_method( cPacketIn, "vlan_tpid", packet_in_vlan_tpid, 0 );
  rb_define_method( cPacketIn, "vlan_tci", packet_in_vlan_tci, 0 );
  rb_define_method( cPacketIn, "vlan_prio", packet_in_vlan_prio, 0 );
  rb_define_method( cPacketIn, "vlan_cfi", packet_in_vlan_cfi, 0 );
  rb_define_method( cPacketIn, "vlan_vid", packet_in_vlan_vid, 0 );

  rb_define_method( cPacketIn, "arp_oper", packet_in_arp_oper, 0 );
  rb_define_method( cPacketIn, "arp_sha", packet_in_arp_sha, 0 );
  rb_define_method( cPacketIn, "arp_spa", packet_in_arp_spa, 0 );
  rb_define_method( cPacketIn, "arp_tha", packet_in_arp_tha, 0 );
  rb_define_method( cPacketIn, "arp_tpa", packet_in_arp_tpa, 0 );
  rb_define_method( cPacketIn, "arp_request?", packet_in_is_arp_request, 0 );
  rb_define_method( cPacketIn, "arp_reply?", packet_in_is_arp_reply, 0 );

  rb_define_method( cPacketIn, "ipv4_version", packet_in_ipv4_version, 0 );
  rb_define_method( cPacketIn, "ipv4_ihl", packet_in_ipv4_ihl, 0 );
  rb_define_method( cPacketIn, "ipv4_tos", packet_in_ipv4_tos, 0 );
  rb_define_method( cPacketIn, "ipv4_tot_len", packet_in_ipv4_tot_len, 0 );
  rb_define_method( cPacketIn, "ipv4_id", packet_in_ipv4_id, 0 );
  rb_define_method( cPacketIn, "ipv4_frag_off", packet_in_ipv4_frag_off, 0 );
  rb_define_method( cPacketIn, "ipv4_ttl", packet_in_ipv4_ttl, 0 );
  rb_define_method( cPacketIn, "ipv4_protocol", packet_in_ipv4_protocol, 0 );
  rb_define_method( cPacketIn, "ipv4_checksum", packet_in_ipv4_checksum, 0 );
  rb_define_method( cPacketIn, "ipv4_saddr", packet_in_ipv4_saddr, 0 );
  rb_define_method( cPacketIn, "ipv4_daddr", packet_in_ipv4_daddr, 0 );

  rb_define_method( cPacketIn, "icmpv4_type", packet_in_icmpv4_type, 0 );
  rb_define_method( cPacketIn, "icmpv4_code", packet_in_icmpv4_code, 0 );
  rb_define_method( cPacketIn, "icmpv4_checksum", packet_in_icmpv4_checksum, 0 );
  rb_define_method( cPacketIn, "icmpv4_id", packet_in_icmpv4_id, 0 );
  rb_define_method( cPacketIn, "icmpv4_seq", packet_in_icmpv4_seq, 0 );
  rb_define_method( cPacketIn, "icmpv4_gateway", packet_in_icmpv4_gateway, 0 );
  rb_define_method( cPacketIn, "icmpv4_echo_reply?", packet_in_is_icmpv4_echo_reply, 0 );
  rb_define_method( cPacketIn, "icmpv4_dst_unreach?", packet_in_is_icmpv4_dst_unreach, 0 );
  rb_define_method( cPacketIn, "icmpv4_redirect?", packet_in_is_icmpv4_redirect, 0 );
  rb_define_method( cPacketIn, "icmpv4_echo_request?", packet_in_is_icmpv4_echo_request, 0 );

  rb_define_method( cPacketIn, "igmp_type", packet_in_igmp_type, 0 );
  rb_define_method( cPacketIn, "igmp_group", packet_in_igmp_group, 0 );
  rb_define_method( cPacketIn, "igmp_checksum", packet_in_igmp_checksum, 0 );
  rb_define_method( cPacketIn, "igmp_membership_query?", packet_in_is_igmp_membership_query, 0 );
  rb_define_method( cPacketIn, "igmp_v1_membership_report?", packet_in_is_igmp_v1_membership_report, 0 );
  rb_define_method( cPacketIn, "igmp_v2_membership_report?", packet_in_is_igmp_v2_membership_report, 0 );
  rb_define_method( cPacketIn, "igmp_v2_leave_group?", packet_in_is_igmp_v2_leave_group, 0 );
  rb_define_method( cPacketIn, "igmp_v3_membership_report?", packet_in_is_igmp_v3_membership_report, 0 );

  rb_define_method( cPacketIn, "tcp_src_port", packet_in_tcp_src_port, 0 );
  rb_define_method( cPacketIn, "tcp_dst_port", packet_in_tcp_dst_port, 0 );
  rb_define_method( cPacketIn, "tcp_seq_no", packet_in_tcp_seq_no, 0 );
  rb_define_method( cPacketIn, "tcp_ack_no", packet_in_tcp_ack_no, 0 );
  rb_define_method( cPacketIn, "tcp_offset", packet_in_tcp_offset, 0 );
  rb_define_method( cPacketIn, "tcp_flags", packet_in_tcp_flags, 0 );
  rb_define_method( cPacketIn, "tcp_window", packet_in_tcp_window, 0 );
  rb_define_method( cPacketIn, "tcp_checksum", packet_in_tcp_checksum, 0 );
  rb_define_method( cPacketIn, "tcp_urgent", packet_in_tcp_urgent, 0 );

  rb_define_method( cPacketIn, "udp_payload", packet_in_udp_payload, 0 );
  rb_define_method( cPacketIn, "udp_src_port", packet_in_udp_src_port, 0 );
  rb_define_method( cPacketIn, "udp_dst_port", packet_in_udp_dst_port, 0 );
  rb_define_method( cPacketIn, "udp_checksum", packet_in_udp_checksum, 0 );
  rb_define_method( cPacketIn, "udp_len", packet_in_udp_len, 0 );
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

  rb_funcall( controller, rb_intern( "packet_in" ), 2, ULL2NUM( datapath_id ), r_message );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
