/*
 * Ethernet header definitions
 *
 * Copyright (C) 2008-2013 NEC Corporation
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


#ifndef ETHER_H
#define ETHER_H


#include <stdint.h>
#include "bool.h"
#include "buffer.h"


#define ETH_ADDRLEN 6
#define ETH_FCS_LENGTH 4
#define ETH_MINIMUM_LENGTH 64
#define ETH_MAXIMUM_LENGTH 1518
#define ETH_HDR_LENGTH sizeof( ether_header_t )
#define ETH_MTU ( ETH_MAXIMUM_LENGTH - ETH_HDR_LENGTH - ETH_FCS_LENGTH )


//Ethernet payload types
#define ETH_ETHTYPE_8023 0x05dc
#define ETH_ETHTYPE_IPV4 0x0800
#define ETH_ETHTYPE_ARP 0x0806
#define ETH_ETHTYPE_RARP 0x8035
#define ETH_ETHTYPE_TPID 0x8100
#define ETH_ETHTYPE_IPV6 0x86DD
#define ETH_ETHTYPE_EAPOL 0x88c7
#define ETH_ETHTYPE_LLDP 0x88cc
#define ETH_ETHTYPE_UKNOWN 0xffff


typedef struct ether_headr {
  uint8_t macda[ ETH_ADDRLEN ];
  uint8_t macsa[ ETH_ADDRLEN ];
  uint16_t type;
} ether_header_t;


typedef struct vlantag_header {
  uint16_t tci;
  uint16_t type;
} vlantag_header_t;


typedef struct snap_header {
  uint8_t llc[ 3 ];
  uint8_t oui[ 3 ];
  uint16_t type;
} snap_header_t;


#define TCI_GET_PRIO( _tci ) ( uint8_t )( ( ( _tci ) >> 13 ) & 7 )

#define TCI_GET_CFI( _tci ) ( uint8_t )( ( ( _tci ) >> 12 ) & 1 )

#define TCI_GET_VID( _tci ) ( uint16_t )( ( _tci ) & 0x0FFF )

#define TCI_CREATE( _prio, _cfi, _vid )         \
  ( uint16_t )( ( ( ( _prio ) & 7 ) << 13 ) |   \
                ( ( ( _cfi ) & 1 ) << 12 ) |    \
                ( ( _vid ) & 0x0FFF ) )

uint16_t fill_ether_padding( buffer *buf );


#endif // ETHER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
