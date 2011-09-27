/*
 * Ethernet header definitions
 *
 * Author: Naoyoshi Tada
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

/**
 * @file ether.h
 * This header file contain type definitions, type codes of Ethernet header and function declarations of ether.c file
 * @see ether.c
 */

#ifndef ETHER_H
#define ETHER_H


#include <endian.h>
#include <stdint.h>
#include "bool.h"
#include "buffer.h"


#define ETH_ADDRLEN 6 /*!<Length of ethernet address*/
#define ETH_PREPADLEN 2 /*!<Length of ethernet type field*/
#define ETH_FCS_LENGTH 4 /*!<Length of the ethernet CRC i.e, Frame Check Sequence*/
#define ETH_MINIMUM_LENGTH 64 /*!<Minimum frame length, including CRC*/
#define ETH_MAXIMUM_LENGTH 1518 /*!<Maximum frame length, including CRC*/
#define ETH_HDR_LENGTH sizeof( ether_header_t ) /*!<Length of ethernet header*/
#define ETH_MTU (ETH_MAXIMUM_LENGTH - ETH_HDR_LENGTH - ETH_FCS_LENGTH ) /*!<MTU of Ethernet*/

//Ethernet payload types
#define ETH_ETHTYPE_8023 0x05dc /*!<IEEE 802.3*/
#define ETH_ETHTYPE_IPV4 0x0800 /*!<IPV4 Protocol*/
#define ETH_ETHTYPE_ARP 0x0806 /*!<Address Resolution Protocol*/
#define ETH_ETHTYPE_TPID 0x8100 /*!<IEEE 802.1Q VLAN Tagging*/
#define ETH_ETHTYPE_EAPOL 0x88c7 /*!<802.11i Pre-Authentication*/
#define ETH_ETHTYPE_LLDP 0x88cc /*!<IEEE 802.1 Link Layer Discovery Protocol*/
#define ETH_ETHTYPE_UKNOWN 0xffff /*!<Maximum valid ethernet type, Reserved*/


/**
 * This is the type that specifies Ethernet header definitions
 * @see http://www.ieee802.org/3/
 * @see http://www.ieee802.org/1/
 */
typedef struct ether_headr {
  uint8_t macda[ ETH_ADDRLEN ];
  uint8_t macsa[ ETH_ADDRLEN ];
  uint16_t type;
} ether_header_t;

/**
 * This is the type that specifies VLAN tagging header definitions
 */
typedef struct vlantag_header {
  uint16_t tci;
  uint16_t type;
} vlantag_header_t;

/**
 * This is the type that specifies Tag Control Information(TCI) definitions
 */
typedef struct vlantag_tci {
#if ( __BYTE_ORDER == __BIG_ENDIAN )
  uint16_t prio:3,
           cfi:1,
           vid:12;
#else // __LITTLE_ENDIAN
  uint16_t vid:12,
           cfi:1,
           prio:3;
#endif
} vlantag_tci_t;

/**
 * This is the type that specifies Subnetwork Access Protocol header definitions
 */
typedef struct snap_header {
  uint8_t llc[ 3 ];
  uint8_t oui[ 3 ];
  uint16_t type;
} snap_header_t;

/*!<Gets the User Priority from tag control information*/
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
