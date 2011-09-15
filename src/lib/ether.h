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
 * @file
 *
 * @brief Function declarations and type definitions for Ethernet header implementation
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

//Ethernet payload types
#define ETH_ETHTYPE_8023 0x05dc /*!<IEEE 802.3*/
#define ETH_ETHTYPE_IPV4 0x0800 /*!<IPV4 Protocol*/
#define ETH_ETHTYPE_ARP 0x0806 /*!<Address Resolution Protocol*/
#define ETH_ETHTYPE_TPID 0x8100 /*!<IEEE 802.1Q VLAN Tagging*/
#define ETH_ETHTYPE_EAPOL 0x88c7 /*!<802.11i Pre-Authentication*/
#define ETH_ETHTYPE_LLDP 0x88cc /*!<IEEE 802.1 Link Layer Discovery Protocol*/
#define ETH_ETHTYPE_UKNOWN 0xffff /*!<Maximum valid ethernet type, Reserved*/


/**
 * Ethernet header definitions
 * @see http://www.ieee802.org/3/
 * @see http://www.ieee802.org/1/
 */
typedef struct ether_headr {
  uint16_t prepad;
  uint8_t macda[ ETH_ADDRLEN ];
  uint8_t macsa[ ETH_ADDRLEN ];
  uint16_t type;
} ether_header_t;

/**
 * VLAN tagging header definitions
 */
typedef struct vlantag_header {
  uint16_t tci;
  uint16_t type;
} vlantag_header_t;

/**
 * Tag Control Information(TCI) definitions
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
 * Subnetwork Access Protocol header definitions
 */
typedef struct snap_header {
  uint8_t llc[ 3 ];
  uint8_t oui[ 3 ];
  uint16_t type;
} snap_header_t;


#define TCI_GET_PRIO( _tci )                           \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->prio;         \
  }                                                    \
  ) /*!<Gets the User Priority from tag control information*/

#define TCI_SET_PRIO( _tci, _prio )                    \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->prio = _prio; \
    _tci_value;                                        \
  }                                                    \
  ) /*!<Sets the User Priority in tag control information to mentioned User Priority*/

#define TCI_GET_CFI( _tci )                            \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->cfi;          \
  }                                                    \
  ) /*!<Gets the Canonical Format Indicator from tag control information*/

#define TCI_SET_CFI( _tci, _cfi )                      \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->cfi = _cfi;   \
    _tci_value;                                        \
  }                                                    \
  ) /*!<Sets the Canonical Format Indicator in tag control information to mentioned Canonical Format Indicator*/

#define TCI_GET_VID( _tci )                            \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->vid;          \
  }                                                    \
  ) /*!<Gets the VLAN ID from tag control information*/

#define TCI_SET_VID( _tci, _vid )                      \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->vid = _vid;   \
    _tci_value;                                        \
  }                                                    \
  ) /*!<Sets the VLAN ID in tag control information to mentioned VLAN ID*/


uint16_t fill_ether_padding( buffer *buf );
bool parse_ether( buffer *buf );


#endif // ETHER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
