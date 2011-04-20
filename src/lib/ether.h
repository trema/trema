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


#ifndef ETHER_H
#define ETHER_H


#include <endian.h>
#include <stdint.h>
#include "bool.h"
#include "buffer.h"


#define ETH_ADDRLEN 6
#define ETH_PREPADLEN 2
#define ETH_FCS_LENGTH 4
#define ETH_MINIMUM_LENGTH 64

#define ETH_ETHTYPE_8023 0x05dc
#define ETH_ETHTYPE_IPV4 0x0800
#define ETH_ETHTYPE_ARP 0x0806
#define ETH_ETHTYPE_TPID 0x8100
#define ETH_ETHTYPE_EAPOL 0x88c7
#define ETH_ETHTYPE_LLDP 0x88cc
#define ETH_ETHTYPE_UKNOWN 0xffff


/*
 * Ethernet header definitions.
 * See http://www.ieee802.org/3/ and http://www.ieee802.org/1/ for details.
 */

typedef struct ether_headr {
  uint16_t prepad;
  uint8_t macda[ ETH_ADDRLEN ];
  uint8_t macsa[ ETH_ADDRLEN ];
  uint16_t type;
} ether_header_t;

typedef struct vlantag_header {
  uint16_t tci;
  uint16_t type;
} vlantag_header_t;

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
  )

#define TCI_SET_PRIO( _tci, _prio )                    \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->prio = _prio; \
    _tci_value;                                        \
  }                                                    \
  )

#define TCI_GET_CFI( _tci )                            \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->cfi;          \
  }                                                    \
  )

#define TCI_SET_CFI( _tci, _cfi )                      \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->cfi = _cfi;   \
    _tci_value;                                        \
  }                                                    \
  )

#define TCI_GET_VID( _tci )                            \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->vid;          \
  }                                                    \
  )

#define TCI_SET_VID( _tci, _vid )                      \
  ( {                                                  \
    uint16_t _tci_value = _tci;                        \
    ( ( vlantag_tci_t * ) &_tci_value )->vid = _vid;   \
    _tci_value;                                        \
  }                                                    \
  )


void fill_ether_padding( buffer *buf );
bool parse_ether( buffer *buf );


#endif // ETHER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
