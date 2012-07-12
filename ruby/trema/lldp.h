/*
 * Topology Discovery
 *
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


#ifndef LLDP_H
#define LLDP_H

#include "trema.h"

#define LLDP_TLV_INFO_MAX_LEN 511U
#define LLDP_TLV_CHASSIS_ID_INFO_MAX_LEN 255U
#define LLDP_TLV_PORT_ID_INFO_MAX_LEN 255U

#define LLDP_TLV_HEAD_LEN 2U
#define LLDP_SUBTYPE_LEN 1U
#define LLDP_END_PDU_LEN 2U
#define LLDP_TTL_LEN 4U
#define LLDP_OUI_LEN 3U
#define LLDP_FLAG_LEN 1U
#define LLDP_VLAN_ID_LEN 2U

#define LLDP_PP_VLAN_LEN ( LLDP_TLV_HEAD_LEN \
    + LLDP_OUI_LEN                           \
    + LLDP_SUBTYPE_LEN                       \
    + LLDP_FLAG_LEN                          \
    + LLDP_VLAN_ID_LEN )

#define LLDP_DEFAULT_TTL 180

#define LLDP_TL( type, length ) htons( ( unsigned short ) ( ( ( type ) << 9 ) \
      | ( ( length ) - LLDP_TLV_HEAD_LEN ) ) )
#define LLDP_TYPE( type_length ) ( ntohs( type_length ) >> 9 )
#define LLDP_LEN( type_length ) ( ntohs( type_length ) & 0x1ff )

enum lldp_type {
  LLDP_TYPE_END,
  LLDP_TYPE_CHASSIS_ID,
  LLDP_TYPE_PORT_ID,
  LLDP_TYPE_TTL,
  LLDP_TYPE_PORT_DESC,
  LLDP_TYPE_SYS_NAME,
  LLDP_TYPE_SYS_DESC,
  LLDP_TYPE_SYS_CAPABILITES,
  LLDP_TYPE_MNG_ADDR,
  LLDP_TYPE_ORGANIZE_SPEC = 127,
};

enum chassis_id_subtype {
  CHASSIS_ID_SUBTYPE_CHASSIS_COMPONENT = 1,
  CHASSIS_ID_SUBTYPE_IF_ALIAS,
  CHASSIS_ID_SUBTYPE_PORT_COMPONENT,
  CHASSIS_ID_SUBTYPE_MAC_ADDR,
  CHASSIS_ID_SUBTYPE_NET_ADDR,
  CHASSIS_ID_SUBTYPE_IF_NAME,
  CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED,
};

enum port_id_subtype {
  PORT_ID_SUBTYPE_IF_ALIAS = 1,
  PORT_ID_SUBTYPE_PORT_COMPONENT,
  PORT_ID_SUBTYPE_MAC_ADDR,
  PORT_ID_SUBTYPE_NET_ADDR,
  PORT_ID_SUBTYPE_IF_NAME,
  PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID,
  PORT_ID_SUBTYPE_LOCALLY_ASSINED,
};

enum organize_spec_subtype {
  ORGANIZE_SPEC_SUBTYPE_PORT_VLAN = 1,
  ORGANIZE_SPEC_SUBTYPE_PORT_PROTOCOL_VLAN,
  ORGANIZE_SPEC_SUBTYPE_VLAN_NAME,
  ORGANIZE_SPEC_SUBTYPE_PROTOCOL_ID,
};

enum port_protocol_vlan_flags {
  PORT_PROTOCOL_VLAN_SUPPORTED = ( 1 << 1 ),
  PORT_PROTOCOL_VLAN_ENABLED = ( 1 << 2 ),
};

struct tlv {
  uint16_t type_len;
  char val[ 0 ];
};


typedef struct {
  uint8_t lldp_mac_dst[ ETH_ADDRLEN ];
  bool lldp_over_ip;
  uint32_t lldp_ip_src;
  uint32_t lldp_ip_dst;
} lldp_options;



#endif // LLDP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
