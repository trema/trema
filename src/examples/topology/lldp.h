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


#include "probe_timer_table.h"


#define LLDP_TLV_INFO_MAX_LEN 255U

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
#define LLDP_LEN( type_length ) ( ntohs( type_length ) & 0x1f )

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


bool send_lldp( probe_timer_entry *port );
bool init_lldp( void );
bool finalize_lldp( void );

#ifdef UNIT_TESTING

void _handle_packet_in( uint64_t datapath_id,
                        uint32_t transaction_id,
                        uint32_t buffer_id,
                        uint16_t total_len,
                        uint16_t in_port,
                        uint8_t reason,
                        const buffer *m,
                        void *user_data );
buffer *_create_lldp_frame( const uint8_t *mac, uint64_t dpid,
                            uint16_t port_no );

#endif


#endif // LLDP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
