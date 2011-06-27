/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


#include <arpa/inet.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "bool.h"
#include "log.h"
#include "utility.h"


static void
_die( const char *format, ... ) {
  char err[ 1024 ];

  assert( format != NULL );
  va_list args;
  va_start( args, format );
  vsnprintf( err, sizeof( err ), format, args );
  va_end( args );

  critical( err );
  abort();
}
void ( *die )( const char *format, ... ) = _die;


bool
compare_string( const void *x, const void *y ) {
  return strcmp( x, y ) == 0 ? true : false;
}


/**
 * Generates a hash value from a string.
 *
 * FNV-1a is used for hashing. See http://isthe.com/chongo/tech/comp/fnv/index.html.
 */
unsigned int
hash_string( const void *key ) {
  // 32 bit offset_basis
  uint32_t hash_value = 0x811c9dc5UL;
  // 32 bit FNV_prime
  const uint32_t prime = 0x01000193UL;
  const char *skey = key;
  int key_size = ( int ) strlen( skey );

  for ( int i = 0; i < key_size; i++ ) {
    hash_value ^= ( unsigned char ) skey[ i ];
    hash_value *= prime;
  }

  return ( unsigned int ) hash_value;
}


bool
compare_mac( const void *x, const void *y ) {
  return memcmp( x, y, OFP_ETH_ALEN ) == 0 ? true : false;
}


// The lowest 4 bytes of the mac address is used as a hash value.
unsigned int
hash_mac( const void *mac ) {
  uint8_t mac_copy[ OFP_ETH_ALEN ];
  unsigned int value;

  memcpy( mac_copy, mac, sizeof( uint8_t ) * OFP_ETH_ALEN );
  memcpy( &value, ( char * ) mac_copy + 2, sizeof( value ) );

  return value;
}


bool
compare_uint32( const void *x, const void *y ) {
  return *( ( const uint32_t * ) x ) == *( ( const uint32_t * ) y ) ? true : false;
}


unsigned int
hash_uint32( const void *key ) {
  return ( *( ( const uint32_t * ) key ) % UINT_MAX );
}


bool
compare_datapath_id( const void *x, const void *y ) {
  return *( ( const uint64_t * ) x ) == *( ( const uint64_t * ) y ) ? true : false;
}


unsigned int
hash_datapath_id( const void *key ) {
  const uint32_t *datapath_id = ( const uint32_t * ) key;
  return ( unsigned int ) datapath_id[ 0 ] ^ datapath_id[ 1 ];
}


bool
string_to_datapath_id( const char *str, uint64_t *datapath_id ) {
  char *endp = NULL;
  *datapath_id = ( uint64_t ) strtoull( str, &endp, 0 );
  if ( *endp != '\0' ) {
    return false;
  }
  return true;
}


bool
match_to_string( const struct ofp_match *match, char *str, size_t size ) {
  assert( match != NULL );
  assert( str != NULL );

  char nw_src[ 16 ];
  char nw_dst[ 16 ];
  struct in_addr addr;

  addr.s_addr = htonl( match->nw_src );
  memset( nw_src, '\0', sizeof( nw_src ) );
  inet_ntop( AF_INET, &addr, nw_src, sizeof( nw_src ) );
  addr.s_addr = htonl( match->nw_dst );
  memset( nw_dst, '\0', sizeof( nw_dst ) );
  inet_ntop( AF_INET, &addr, nw_dst, sizeof( nw_dst ) );

  memset( str, '\0', size );

  int ret = snprintf(
              str,
              size,
              "wildcards = %#x, in_port = %u, "
              "dl_src = %02x:%02x:%02x:%02x:%02x:%02x, "
              "dl_dst = %02x:%02x:%02x:%02x:%02x:%02x, "
              "dl_vlan = %u, dl_vlan_pcp = %u, dl_type = %#x, "
              "nw_tos = %u, nw_proto = %u, nw_src = %s, nw_dst = %s, "
              "tp_src = %u, tp_dst = %u",
              match->wildcards, match->in_port,
              match->dl_src[ 0 ], match->dl_src[ 1 ], match->dl_src[ 2 ],
              match->dl_src[ 3 ], match->dl_src[ 4 ], match->dl_src[ 5 ],
              match->dl_dst[ 0 ], match->dl_dst[ 1 ], match->dl_dst[ 2 ],
              match->dl_dst[ 3 ], match->dl_dst[ 4 ], match->dl_dst[ 5 ],
              match->dl_vlan, match->dl_vlan_pcp, match->dl_type,
              match->nw_tos, match->nw_proto, nw_src, nw_dst,
              match->tp_src, match->tp_dst
            );

  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
phy_port_to_string( const struct ofp_phy_port *phy_port, char *str, size_t size ) {
  assert( phy_port != NULL );
  assert( str != NULL );

  memset( str, '\0', size );

  int ret = snprintf(
              str,
              size,
              "port_no = %u, hw_addr =  %02x:%02x:%02x:%02x:%02x:%02x, "
              "name = %s, config = %#x, state = %#x, "
              "curr = %#x, advertised = %#x, supported = %#x, peer = %#x",
              phy_port->port_no,
              phy_port->hw_addr[ 0 ], phy_port->hw_addr[ 1 ], phy_port->hw_addr[ 2 ],
              phy_port->hw_addr[ 3 ], phy_port->hw_addr[ 4 ], phy_port->hw_addr[ 5 ],
              phy_port->name, phy_port->config, phy_port->state,
              phy_port->curr, phy_port->advertised, phy_port->supported, phy_port->peer
            );

  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
