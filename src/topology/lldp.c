/*
 * Author: Shuji Ishii
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


#include <assert.h>
#include <inttypes.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "etherip.h"
#include "trema.h"
#include "probe_timer_table.h"
#include "lldp.h"


static int recv_lldp( uint64_t *dpid, uint16_t *port, const buffer *buf );
static buffer *create_lldp_frame( const uint8_t *mac, uint64_t dpid,
                                  uint16_t port_no );
static int parse_lldp_ull( void *str, uint64_t *value, uint32_t len );
static int parse_lldp_us( void *str, uint16_t *value, uint32_t len );

static uint16_t lldp_ethtype = ETH_ETHTYPE_LLDP;
static uint8_t lldp_mac_dst[ ETH_ADDRLEN ];
static bool lldp_over_ip = false;
static uint32_t lldp_ip_src = 0;
static uint32_t lldp_ip_dst = 0;


bool
send_lldp( probe_timer_entry *port ) {
  buffer *lldp;

  lldp = create_lldp_frame( port->mac, port->datapath_id, port->port_no );

  openflow_actions *actions = create_actions();
  if ( !append_action_output( actions, port->port_no, UINT16_MAX ) ) {
    free_buffer( lldp );
    error( "Failed to sent LLDP frame(%#" PRIx64 ", %u)", port->datapath_id, port->port_no );
    return false;
  }

  uint32_t transaction_id = get_transaction_id();

  buffer *packetout = create_packet_out( transaction_id, UINT32_MAX,
                                         OFPP_NONE, actions, lldp );
  if ( !send_openflow_message( port->datapath_id, packetout ) ) {
    free_buffer( lldp );
    free_buffer( packetout );
    delete_actions( actions );
    die( "send_openflow_message" );
  }

  free_buffer( lldp );
  free_buffer( packetout );
  delete_actions( actions );

  debug( "Sent LLDP frame(%#" PRIx64 ", %u)", port->datapath_id, port->port_no );
  return true;
}


static int
recv_lldp( uint64_t *dpid, uint16_t *port_no, const buffer *buf ) {
  int ret;
  size_t remain_len = 0;
  uint16_t *lldp_tlv = NULL;
  uint8_t *lldp_du;
  uint32_t type = 0;
  uint32_t len = 0;
  uint8_t subtype = 0;

  debug( "Receiving LLDP frame." );

  packet_info *packet_info = buf->user_data;
  assert( packet_info != NULL );
  remain_len = buf->length;

  if ( packet_type_eth_vtag( buf ) ) {
    error( "no vlan tag is supported" );
    return -1;
  }

  remain_len -= sizeof( ether_header_t );

  if ( packet_info->eth_type == ETH_ETHTYPE_LLDP ) {
    if ( memcmp( packet_info->eth_macda, lldp_mac_dst, ETH_ADDRLEN ) != 0 ) {
      debug( "Unknown MAC address ( %s ).", ether_ntoa( ( const struct ether_addr *) packet_info->eth_macda ) );
      return -1;
    }
    lldp_tlv = packet_info->l2_payload;
  }
  else if ( packet_info->eth_type == ETH_ETHTYPE_IPV4 ) {
    if ( packet_info->ipv4_daddr != lldp_ip_dst ||
         packet_info->ipv4_saddr != lldp_ip_src ) {
      warn( "Unknown IP address ( src = %#x, dst = %#x ).", packet_info->ipv4_saddr, packet_info->ipv4_daddr );
      return -1;
    }

    remain_len -= sizeof( ipv4_header_t ) + sizeof( etherip_header ) + sizeof( ether_header_t );
    lldp_tlv = ( uint16_t * ) ( ( char * ) packet_info->l3_payload + sizeof( etherip_header ) + sizeof( ether_header_t ) );
  }
  else {
    error( "Unsupported ether type ( %#x ).", packet_info->eth_type );
    return -1;
  }

  do {
    if ( remain_len < LLDP_TLV_HEAD_LEN ) {
      info( "Missing size of LLDP tlv header." );
      return -1;
    }
    assert( lldp_tlv );
    type = LLDP_TYPE( *lldp_tlv );
    len = ( uint32_t ) LLDP_LEN( *lldp_tlv );
    if ( len > LLDP_TLV_INFO_MAX_LEN ) {
      info( "Missing length of LLDP tlv header ( len = %u ).", len );
      return -1;
    }

    remain_len -= LLDP_TLV_HEAD_LEN;
    if ( remain_len < len ) {
      info( "Missing size of LLDP data unit." );
      return -1;
    }

    lldp_du = ( uint8_t * ) lldp_tlv + LLDP_TLV_HEAD_LEN;

    switch ( type ) {
    case LLDP_TYPE_END:
      if ( len != 0 ) {
        info( "Missing length of LLDP END ( len = %u ).", len );
        return -1;
      }
      break;

    case LLDP_TYPE_CHASSIS_ID:
      if ( len > ( LLDP_SUBTYPE_LEN + LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN ) ) {
        info( "Missing length of LLDP CHASSIS ID ( len = %u ).", len );
        return -1;
      }
      subtype = *( ( uint8_t * ) lldp_du );
      lldp_du += LLDP_SUBTYPE_LEN;
      switch ( subtype ) {
      case CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED:
        ret = parse_lldp_ull( lldp_du, dpid, len - LLDP_SUBTYPE_LEN );
        if ( ret < 0 ) {
          info( "Failed to parse dpid." );
          return -1;
        }
        break;

      default:
        break;
      }
      break;

    case LLDP_TYPE_PORT_ID:
      if ( len > ( LLDP_SUBTYPE_LEN + LLDP_TLV_PORT_ID_INFO_MAX_LEN ) ) {
        info( "Missing length of LLDP PORT ID ( len = %u ).", len );
        return -1;
      }
      subtype = *( ( uint8_t * ) lldp_du );
      lldp_du += LLDP_SUBTYPE_LEN;
      switch ( subtype ) {
      case PORT_ID_SUBTYPE_LOCALLY_ASSINED:
        ret = parse_lldp_us( lldp_du, port_no, len - LLDP_SUBTYPE_LEN );
        if ( ret < 0 ) {
          info( "Failed to parse port_no." );
          return -1;
        }
        break;

      default:
        break;
      }
      break;

    case LLDP_TYPE_TTL:
      if ( len != 2 ) {
        info( "Missing length of LLDP TTL ( len = %u ).", len );
        return -1;
      }
      break;

    case LLDP_TYPE_PORT_DESC:
    case LLDP_TYPE_SYS_NAME:
    case LLDP_TYPE_SYS_DESC:
    case LLDP_TYPE_SYS_CAPABILITES:
    case LLDP_TYPE_MNG_ADDR:
    case LLDP_TYPE_ORGANIZE_SPEC:
      break;

    default:
      info( "Unknown LLDP type (%#x).", type );
      return -1;
    }

    remain_len -= len;
    lldp_tlv = ( uint16_t * ) ( ( uint8_t * ) lldp_tlv + LLDP_TLV_HEAD_LEN + len );
  } while ( type != LLDP_TYPE_END );

  return 0;
}


static buffer *
create_lldp_frame( const uint8_t *mac, uint64_t dpid, uint16_t port_no ) {
  buffer *lldp_buf;
  size_t lldp_buf_len = 0;
  ether_header_t *ether;

  struct tlv *chassis_id_tlv;
  uint32_t chassis_id_tlv_len = 0;
  uint32_t chassis_id_strlen = 0;
  struct tlv *port_id_tlv;
  uint32_t port_id_tlv_len = 0;
  uint32_t port_id_strlen = 0;
  struct tlv *ttl_tlv;
  struct tlv *end_tlv;
  char *info;
  char dpid_str[ LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN ];
  char port_no_str[ LLDP_TLV_PORT_ID_INFO_MAX_LEN ];
  uint16_t *ttl_val;
  size_t padding_length = 0;

  debug( "Creating LLDP Frame." );

  // Convert Chassis ID into string
  chassis_id_strlen = ( uint32_t ) snprintf( dpid_str, ( LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN - 1 ), "%#" PRIx64, dpid );

  assert ( chassis_id_strlen <= ( LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN - 1 ) );

  chassis_id_tlv_len = LLDP_TLV_HEAD_LEN + LLDP_SUBTYPE_LEN + chassis_id_strlen;

  // Convert Port ID into string
  port_id_strlen = ( uint32_t ) snprintf( port_no_str, ( LLDP_TLV_PORT_ID_INFO_MAX_LEN - 1 ), "%d", port_no );
  assert ( port_id_strlen <= ( LLDP_TLV_PORT_ID_INFO_MAX_LEN - 1 ) );
  port_id_tlv_len = LLDP_TLV_HEAD_LEN + LLDP_SUBTYPE_LEN + port_id_strlen;

  lldp_buf_len = sizeof( ether_header_t ) + chassis_id_tlv_len + port_id_tlv_len
    + LLDP_TTL_LEN;

  if ( lldp_buf_len + ETH_FCS_LENGTH < ETH_MINIMUM_LENGTH ) {
    padding_length = ETH_MINIMUM_LENGTH - ( lldp_buf_len + ETH_FCS_LENGTH );
    lldp_buf_len += padding_length;
  }
  if ( lldp_over_ip ) {
    lldp_ethtype = ETH_ETHTYPE_IPV4;
    lldp_buf_len += sizeof( ipv4_header_t ) + sizeof( etherip_header ) + sizeof( ether_header_t );
  }

  lldp_buf = alloc_buffer_with_length( lldp_buf_len );

  if ( lldp_over_ip ) {
    ether = append_back_buffer( lldp_buf, sizeof( ether_header_t ) );
    memset( ether->macda, 0xff, ETH_ADDRLEN );
    memcpy( ether->macsa, mac, ETH_ADDRLEN );
    ether->type = htons( ETH_ETHTYPE_IPV4 );

    ipv4_header_t *ip = append_back_buffer( lldp_buf, sizeof( ipv4_header_t ) );
    memset( ip, 0, sizeof( ipv4_header_t ) );
    ip->ihl = 5;
    ip->version = 4;
    ip->ttl = 1;
    ip->protocol = IPPROTO_ETHERIP;
    ip->tot_len = htons( ( uint16_t ) ( lldp_buf_len - sizeof( ether_header_t ) ) );
    ip->saddr = htonl( lldp_ip_src );
    ip->daddr = htonl( lldp_ip_dst );
    ip->csum = get_checksum( ( uint16_t * ) ip, sizeof( ipv4_header_t ) );
    etherip_header *etherip = append_back_buffer( lldp_buf, sizeof( etherip_header ) );
    etherip->version = htons( ETHERIP_VERSION );
  }

  // Create ether frame header
  ether = append_back_buffer( lldp_buf, sizeof( ether_header_t ) );
  memcpy( ether->macda, lldp_mac_dst, ETH_ADDRLEN );
  memcpy( ether->macsa, mac, ETH_ADDRLEN );
  ether->type = htons( ETH_ETHTYPE_LLDP );

  // Create Chassis ID TLV
  chassis_id_tlv = append_back_buffer( lldp_buf, chassis_id_tlv_len );
  chassis_id_tlv->type_len = LLDP_TL( LLDP_TYPE_CHASSIS_ID, chassis_id_tlv_len );
  info = chassis_id_tlv->val;
  *info = CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED;
  info += LLDP_SUBTYPE_LEN;
  strncpy( info, dpid_str, chassis_id_strlen );

  // Create Port ID TLV
  port_id_tlv = append_back_buffer( lldp_buf, port_id_tlv_len );
  port_id_tlv->type_len = LLDP_TL( LLDP_TYPE_PORT_ID, port_id_tlv_len );
  info = port_id_tlv->val;
  *info = PORT_ID_SUBTYPE_LOCALLY_ASSINED;
  info += LLDP_SUBTYPE_LEN;
  strncpy( info, port_no_str, port_id_strlen );

  // Create TTL TLV
  ttl_tlv = append_back_buffer( lldp_buf, LLDP_TTL_LEN );
  ttl_tlv->type_len = LLDP_TL( LLDP_TYPE_TTL, LLDP_TTL_LEN );
  ttl_val = ( uint16_t * ) ttl_tlv->val;
  *ttl_val = htons( LLDP_DEFAULT_TTL );

  // Create END OF LLDPDU TLV
  end_tlv = append_back_buffer( lldp_buf, LLDP_END_PDU_LEN );
  end_tlv->type_len = LLDP_TL( LLDP_TYPE_END, LLDP_TLV_HEAD_LEN );

  if ( padding_length > 0 ) {
    fill_ether_padding( lldp_buf );
  }

  return lldp_buf;
}


static int
parse_lldp_ull( void *str, uint64_t *value, uint32_t len ) {
  unsigned int i;
  char *end_p;
  char buf[ LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN + 1];
  char *p = ( char * ) str;

  for ( i = 0; i < len; i++ ) {
    buf[ i ] = *( ( char * ) p );
    p++;
  }
  buf[ len ] = '\0';

  *value = strtoull( buf, &end_p, 0 );
  if ( *end_p != '\0' ) {
    // uknown format of topology_discovery cassis id
    debug( "Failed to strtoull." );
    return -1;
  }

  return 0;
}


static int
parse_lldp_us( void *str, uint16_t *value, uint32_t len ) {
  unsigned int i;
  char *end_p;
  char buf[ LLDP_TLV_PORT_ID_INFO_MAX_LEN + 1 ];
  char *p = ( char * ) str;

  for ( i = 0; i < len; i++ ) {
    buf[ i ] = *( ( char * ) p );
    p++;
  }
  buf[ len ] = '\0';

  *value = ( uint16_t ) strtoul( buf, &end_p, 0 );
  if ( *end_p != '\0' ) {
    // uknown format of topology_discovery port id
    debug( "Failed to strtoul." );
    return -1;
  }

  return 0;
}


static void
handle_packet_in( uint64_t dst_datapath_id,
                  uint32_t transaction_id __attribute__((unused)),
                  uint32_t buffer_id __attribute__((unused)),
                  uint16_t total_len __attribute__((unused)),
                  uint16_t dst_port_no,
                  uint8_t reason __attribute__((unused)),
                  const buffer *m,
                  void *user_data __attribute__((unused)) ) {
  packet_info *packet_info = m->user_data;
  assert( packet_info != NULL );

  // check if LLDP or not
  if ( packet_info->eth_type != lldp_ethtype ) {
    notice( "Non-LLDP packet is received ( type = %#x ).", packet_info->eth_type );
    return;
  }

  uint64_t src_datapath_id;
  uint16_t src_port_no;
  if ( recv_lldp( &src_datapath_id, &src_port_no, m ) < 0 ) {
    return;
  }

  debug( "Receive LLDP Frame (%#" PRIx64 ", %u) from (%#" PRIx64 ", %u).",
         dst_datapath_id, dst_port_no, src_datapath_id, src_port_no );
  probe_timer_entry *entry = delete_probe_timer_entry( &src_datapath_id,
                                                       src_port_no );
  if ( entry == NULL ) {
    debug( "Not found dst datapath_id (%#" PRIx64 ", %u).", src_datapath_id,
            src_port_no );
    return;
  }

  probe_request( entry, PROBE_TIMER_EVENT_RECV_LLDP, &dst_datapath_id, dst_port_no );
}


bool
init_lldp( lldp_options options ) {
  memcpy( lldp_mac_dst, options.lldp_mac_dst, ETH_ADDRLEN );
  lldp_over_ip = options.lldp_over_ip;
  lldp_ip_src = options.lldp_ip_src;
  lldp_ip_dst = options.lldp_ip_dst;

  init_openflow_application_interface( get_trema_name() );
  set_packet_in_handler( handle_packet_in, NULL );

  return true;
}


bool
finalize_lldp() {
  // do something here
  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
