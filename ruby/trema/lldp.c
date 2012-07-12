/*
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


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/ether.h>
#include <assert.h>
#include "lldp.h"
#include "ruby.h"
#include "trema.h"
extern VALUE mTrema;


/*
 * This function is called from send_packet_out_lldp().
 */
void
make_lldp_and_send_message( const uint64_t datapath_id, const long int port, const char *src_mac_address, const char *dst_mac_address ) {
  struct ether_addr src_mac_addr;
  struct ether_addr dst_mac_addr;

  struct tlv *chassis_id_tlv;
  uint32_t chassis_id_tlv_len = 0;
  uint32_t chassis_id_strlen = 0;
  char *info;
  
  struct tlv *port_id_tlv;
  uint32_t port_id_tlv_len = 0;
  uint32_t port_id_strlen = 0;

  size_t padding_length = 0;
  size_t frame_len = 0;
  char dpid_str[ LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN ];
  char port_no_str[ LLDP_TLV_PORT_ID_INFO_MAX_LEN ];

  struct tlv *ttl_tlv;
  uint16_t *ttl_val;
  struct tlv *end_tlv;

  ether_aton_r( src_mac_address, &src_mac_addr );
  ether_aton_r( dst_mac_address, &dst_mac_addr );

  // Convert Chassis ID into string
  chassis_id_strlen = ( uint32_t ) snprintf( dpid_str, ( LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN - 1 ), "%#" PRIx64, datapath_id );
  assert ( chassis_id_strlen <= ( LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN - 1 ) );
  chassis_id_tlv_len = LLDP_TLV_HEAD_LEN + LLDP_SUBTYPE_LEN + chassis_id_strlen;

  // Convert Port ID into string
  port_id_strlen = ( uint32_t ) snprintf( port_no_str, ( LLDP_TLV_PORT_ID_INFO_MAX_LEN - 1 ), "%ld", port );
  assert ( port_id_strlen <= ( LLDP_TLV_PORT_ID_INFO_MAX_LEN - 1 ) );
  port_id_tlv_len = LLDP_TLV_HEAD_LEN + LLDP_SUBTYPE_LEN + port_id_strlen;

  frame_len = sizeof( ether_header_t ) + chassis_id_tlv_len + port_id_tlv_len + LLDP_TTL_LEN;

  // padding
  if ( frame_len + ETH_FCS_LENGTH < ETH_MINIMUM_LENGTH ) {
    padding_length = ETH_MINIMUM_LENGTH - ( frame_len + ETH_FCS_LENGTH );
    frame_len += padding_length;
  }

  buffer *frame = alloc_buffer_with_length( frame_len );
  
  // Create ether frame header
  ether_header_t *ether = append_back_buffer( frame, sizeof( ether_header_t ) ); 
  memcpy( ether->macsa, src_mac_addr.ether_addr_octet, ETH_ADDRLEN );
  memcpy( ether->macda, dst_mac_addr.ether_addr_octet, ETH_ADDRLEN );  
  ether->type = htons( ETH_ETHTYPE_LLDP ); 

  // Create Chassis ID TLV
  chassis_id_tlv = append_back_buffer( frame, chassis_id_tlv_len );
  chassis_id_tlv->type_len = LLDP_TL( LLDP_TYPE_CHASSIS_ID, chassis_id_tlv_len );
  info = chassis_id_tlv->val;
  *info = CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED;
  info += LLDP_SUBTYPE_LEN;
  strncpy( info, dpid_str, chassis_id_strlen );

  // Create Port ID TLV
  port_id_tlv = append_back_buffer( frame, port_id_tlv_len );
  port_id_tlv->type_len = LLDP_TL( LLDP_TYPE_PORT_ID, port_id_tlv_len );
  info = port_id_tlv->val;
  *info = PORT_ID_SUBTYPE_LOCALLY_ASSINED;
  info += LLDP_SUBTYPE_LEN;
  strncpy( info, port_no_str, port_id_strlen );

  // Create TTL TLV
  ttl_tlv = append_back_buffer( frame, LLDP_TTL_LEN );
  ttl_tlv->type_len = LLDP_TL( LLDP_TYPE_TTL, LLDP_TTL_LEN );
  ttl_val = ( uint16_t * ) ttl_tlv->val;
  *ttl_val = htons( LLDP_DEFAULT_TTL );

  // Create END OF LLDPDU TLV
  end_tlv = append_back_buffer( frame, LLDP_END_PDU_LEN );
  end_tlv->type_len = LLDP_TL( LLDP_TYPE_END, LLDP_TLV_HEAD_LEN );

  if ( padding_length > 0 ) {
    fill_ether_padding( frame );
  }

  openflow_actions *actions = create_actions(); 
  append_action_output( actions, port, UINT16_MAX ); 
  buffer *pout = create_packet_out( get_transaction_id(), UINT32_MAX, OFPP_NONE, actions, frame ); 
  send_openflow_message( datapath_id, pout ); 
  
  free_buffer( pout ); 
  free_buffer( frame ); 
  delete_actions( actions ); 
}


/*
 * Parse hex string.
 * @return [Nmber] the value of chassis id.
 */
uint64_t
parse_lldp_ull( void *str, uint32_t len ) {
  unsigned int i;
  uint64_t value;
  char buf[ LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN + 1];
  char *p = ( char *) str;
  char *end_p;
  
  for ( i = 0; i < len; i++ ) {
    buf[ i ] = *p;
    p++;
  }
  buf[ len ] = '\0';

  value = strtoull( buf, &end_p, 0 );
  if ( *end_p != '\0' ) {
    // uknown format of topology_discovery cassis id
    debug( "Failed to strtoull." );
    return -1;
  }

  return value;
}


/*
 * Send LLDP frame.
 * This is wrapped by Trema::LLDP.probe.
 */
VALUE
send_packet_out_lldp( VALUE self, VALUE rb_datapath_id, VALUE rb_src_macaddr, VALUE rb_port ) {
  uint64_t datapath_id
#if __WORDSIZE == 64 /* 64 bit  CPU */
  = NUM2ULONG( rb_datapath_id );
#else /* 32 bit  CPU */
  = NUM2ULL( rb_datapath_id );
#endif
  long int port = NUM2INT( rb_port );

  char *src_macaddr = STR2CSTR( rb_src_macaddr );
  char *dst_macaddr = "01:80:c2:00:00:0e";

  make_lldp_and_send_message( datapath_id, port, src_macaddr, dst_macaddr );
}


/*
 * Get LLDP chassis id.
 * @return [number] the Ruby object of number.
 */
VALUE
lldp_src_datapath_id( VALUE self ) {
  uint64_t dpid;
  size_t remain_len = 0;
  uint16_t *lldp_tlv = NULL;
  uint8_t *lldp_du;
  uint32_t type = 0;
  uint32_t len = 0;
  uint8_t subtype = 0;


  packet_in *cpacket;
  Data_Get_Struct( self, packet_in, cpacket );
  packet_info *packet_info = cpacket->data->user_data  ;


  assert( packet_info != NULL );
  remain_len = cpacket->data->length;

  if ( packet_type_eth_vtag( cpacket->data ) ) {
    error( "no vlan tag is supported" );
    return Qfalse;
  }

  remain_len -= sizeof( ether_header_t );

  if ( packet_info->eth_type == ETH_ETHTYPE_LLDP ) {
    lldp_tlv = packet_info->l2_payload;
  }

  do {
    if ( remain_len < LLDP_TLV_HEAD_LEN ) {
      info( "Missing size of LLDP tlv header." );
      return Qfalse;
    }
    assert( lldp_tlv );
    type = LLDP_TYPE( *lldp_tlv );
    len = ( uint32_t ) LLDP_LEN( *lldp_tlv );
    if ( len > LLDP_TLV_INFO_MAX_LEN ) {
      info( "Missing length of LLDP tlv header ( len = %u ).", len );
      return Qfalse;
    }

    remain_len -= LLDP_TLV_HEAD_LEN;
    if ( remain_len < len ) {
      info( "Missing size of LLDP data unit." );
      return Qfalse;
    }

    lldp_du = ( uint8_t * ) lldp_tlv + LLDP_TLV_HEAD_LEN;

    switch ( type ) {
    case LLDP_TYPE_END:
      if ( len != 0 ) {
        info( "Missing length of LLDP END ( len = %u ).", len );
        return Qfalse;
      }
      break;

    case LLDP_TYPE_CHASSIS_ID:
      if ( len > ( LLDP_SUBTYPE_LEN + LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN ) ) {
        info( "Missing length of LLDP CHASSIS ID ( len = %u ).", len );
        return Qfalse;
      }
      subtype = *( ( uint8_t * ) lldp_du );
      lldp_du += LLDP_SUBTYPE_LEN;
      switch ( subtype ) {
      case CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED:
        dpid = parse_lldp_ull( lldp_du, len - LLDP_SUBTYPE_LEN );
        break;

      default:
        break;
      }
      break;
      

    case LLDP_TYPE_PORT_ID:
    case LLDP_TYPE_TTL:
    case LLDP_TYPE_PORT_DESC:
    case LLDP_TYPE_SYS_NAME:
    case LLDP_TYPE_SYS_DESC:
    case LLDP_TYPE_SYS_CAPABILITES:
    case LLDP_TYPE_MNG_ADDR:
    case LLDP_TYPE_ORGANIZE_SPEC:
      break;

    default:
      info( "Unknown LLDP type (%#x).", type );
      return Qfalse;
    }

    remain_len -= len;
    lldp_tlv = ( uint16_t * ) ( ( uint8_t * ) lldp_tlv + LLDP_TLV_HEAD_LEN + len );
  } while ( type != LLDP_TYPE_END );

  return INT2NUM(dpid);
}


void
Init_lldp() {
  VALUE cLldp;
  VALUE cPacketIn;

  cLldp = rb_define_class_under( mTrema, "LLDP", rb_cObject );
  rb_define_method( cLldp, "send_packet_out_lldp", send_packet_out_lldp, 3 );
  rb_require( "trema/lldp" );

  cPacketIn = rb_define_class_under(mTrema, "PacketIn", rb_cObject );
  rb_define_method(cPacketIn, "lldp_cid", lldp_src_datapath_id, 0);
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
