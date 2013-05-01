/*
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
#include "checks.h"
#include "log.h"
#include "trema_wrapper.h"
#include "utility.h"
#include "wrapper.h"

static void
_die( const char *format, ... ) {
  char err[ 1024 ];

  assert( format != NULL );
  va_list args;
  va_start( args, format );
  vsnprintf( err, sizeof( err ), format, args );
  va_end( args );

  critical( "%s", err );
  trema_abort();
}
void ( *die )( const char *format, ... ) = _die;


bool
compare_string( const void *x, const void *y ) {
  return strcmp( x, y ) == 0 ? true : false;
}


/**
 * Generates a hash value
 *
 * FNV-1a is used for hashing. See http://isthe.com/chongo/tech/comp/fnv/index.html.
 */
unsigned int
hash_core( const void *key, int size ) {
  // 32 bit offset_basis
  uint32_t hash_value = 0x811c9dc5UL;
  // 32 bit FNV_prime
  const uint32_t prime = 0x01000193UL;
  const unsigned char *c = key;

  for ( int i = 0; i < size; i++ ) {
    hash_value ^= ( unsigned char ) c[ i ];
    hash_value *= prime;
  }

  return ( unsigned int ) hash_value;
}


unsigned int
hash_string( const void *key ) {
  return hash_core( key, ( int ) strlen( key ) );
}


bool
compare_mac( const void *x, const void *y ) {
  return memcmp( x, y, OFP_ETH_ALEN ) == 0 ? true : false;
}


unsigned int
hash_mac( const void *mac ) {
  return hash_core( mac, OFP_ETH_ALEN );
}


uint64_t
mac_to_uint64( const uint8_t *mac ) {
  return ( ( uint64_t ) mac[ 0 ] << 40 ) +
         ( ( uint64_t ) mac[ 1 ] << 32 ) +
         ( ( uint64_t ) mac[ 2 ] << 24 ) +
         ( ( uint64_t ) mac[ 3 ] << 16 ) +
         ( ( uint64_t ) mac[ 4 ] << 8 ) +
         ( ( uint64_t ) mac[ 5 ] );
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
  return hash_core( key, ( int ) sizeof( uint64_t ) );
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


static bool
append_string( char *buf, size_t length, const char *string ) {
  assert( buf != NULL );
  assert( length > 0 );
  assert( string != NULL );

  size_t current_length = strlen( buf );
  size_t length_to_append = strlen( string );
  if ( ( current_length + length_to_append + 1 ) > length ) {
    return false;
  }
  strncpy( buf + current_length, string, length_to_append + 1 );

  return true;
}


bool
wildcards_to_string( uint32_t wildcards, char *str, size_t size ) {
  assert( str != NULL );
  assert( size > 0 );

  memset( str, '\0', size );

  bool ret = true;
  if ( ( wildcards & OFPFW_ALL ) == 0 ) {
    ret &= append_string( str, size, "none" );
    return ret;
  }

  uint32_t nw_src_mask = ( wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;
  uint32_t nw_dst_mask = ( wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;
  uint32_t mask = OFPFW_ALL & ~OFPFW_NW_SRC_MASK & ~OFPFW_NW_DST_MASK;
  if ( ( wildcards & mask ) == mask && nw_src_mask >= 32 && nw_dst_mask >= 32 ) {
    ret &= append_string( str, size, "all" );
    return ret;
  }

  if ( wildcards & OFPFW_IN_PORT ) {
    ret &= append_string( str, size, "in_port|" );
  }
  if ( wildcards & OFPFW_DL_SRC ) {
    ret &= append_string( str, size, "dl_src|" );
  }
  if ( wildcards & OFPFW_DL_DST ) {
    ret &= append_string( str, size, "dl_dst|" );
  }
  if ( wildcards & OFPFW_DL_TYPE ) {
    ret &= append_string( str, size, "dl_type|" );
  }
  if ( wildcards & OFPFW_DL_VLAN ) {
    ret &= append_string( str, size, "dl_vlan|" );
  }
  if ( wildcards & OFPFW_DL_VLAN_PCP ) {
    ret &= append_string( str, size, "dl_vlan_pcp|" );
  }
  if ( wildcards & OFPFW_NW_PROTO ) {
    ret &= append_string( str, size, "nw_proto|" );
  }
  if ( wildcards & OFPFW_NW_TOS ) {
    ret &= append_string( str, size, "nw_tos|" );
  }

  char mask_str[ 16 ];
  if ( nw_src_mask > 0 ) {
    snprintf( mask_str, sizeof( mask_str ), "nw_src(%u)|", nw_src_mask );
    ret &= append_string( str, size, mask_str );
  }
  if ( nw_dst_mask > 0 ) {
    snprintf( mask_str, sizeof( mask_str ), "nw_dst(%u)|", nw_dst_mask );
    ret &= append_string( str, size, mask_str );
  }
  if ( wildcards & OFPFW_TP_SRC ) {
    ret &= append_string( str, size, "tp_src|" );
  }
  if ( wildcards & OFPFW_TP_DST ) {
    ret &= append_string( str, size, "tp_dst|" );
  }

  if ( strlen( str ) > 0 ) {
    str[ strlen( str ) - 1 ] = '\0';
  }

  return ret;
}


bool
match_to_string( const struct ofp_match *match, char *str, size_t size ) {
  assert( match != NULL );
  assert( str != NULL );

  char wildcards_str[ 256 ];
  char nw_src[ 16 ];
  char nw_dst[ 16 ];
  struct in_addr addr;
  unsigned int masklen;
  unsigned int nw_src_prefixlen;
  unsigned int nw_dst_prefixlen;

  wildcards_to_string( match->wildcards, wildcards_str, sizeof( wildcards_str ) );
  addr.s_addr = htonl( match->nw_src );
  memset( nw_src, '\0', sizeof( nw_src ) );
  inet_ntop( AF_INET, &addr, nw_src, sizeof( nw_src ) );
  addr.s_addr = htonl( match->nw_dst );
  memset( nw_dst, '\0', sizeof( nw_dst ) );
  inet_ntop( AF_INET, &addr, nw_dst, sizeof( nw_dst ) );
  masklen = ( match->wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;
  nw_src_prefixlen = ( masklen >= 32 ? 0 : 32 - masklen );
  masklen = ( match->wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;
  nw_dst_prefixlen = ( masklen >= 32 ? 0 : 32 - masklen );

  memset( str, '\0', size );

  int ret = snprintf(
              str,
              size,
              "wildcards = %#x(%s), in_port = %u, "
              "dl_src = %02x:%02x:%02x:%02x:%02x:%02x, "
              "dl_dst = %02x:%02x:%02x:%02x:%02x:%02x, "
              "dl_vlan = %#x, dl_vlan_pcp = %#x, dl_type = %#x, "
              "nw_tos = %u, nw_proto = %u, nw_src = %s/%u, nw_dst = %s/%u, "
              "tp_src = %u, tp_dst = %u",
              match->wildcards, wildcards_str, match->in_port,
              match->dl_src[ 0 ], match->dl_src[ 1 ], match->dl_src[ 2 ],
              match->dl_src[ 3 ], match->dl_src[ 4 ], match->dl_src[ 5 ],
              match->dl_dst[ 0 ], match->dl_dst[ 1 ], match->dl_dst[ 2 ],
              match->dl_dst[ 3 ], match->dl_dst[ 4 ], match->dl_dst[ 5 ],
              match->dl_vlan, match->dl_vlan_pcp, match->dl_type,
              match->nw_tos, match->nw_proto, nw_src, nw_src_prefixlen,
              nw_dst, nw_dst_prefixlen, match->tp_src, match->tp_dst
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
              "port_no = %u, hw_addr = %02x:%02x:%02x:%02x:%02x:%02x, "
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


bool
action_output_to_string( const struct ofp_action_output *action, char *str, size_t size ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "output: port=%u max_len=%u", action->port, action->max_len );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_set_vlan_vid_to_string( const struct ofp_action_vlan_vid *action, char *str, size_t size ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "set_vlan_vid: vlan_vid=%#x", action->vlan_vid );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_set_vlan_pcp_to_string( const struct ofp_action_vlan_pcp *action, char *str, size_t size ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "set_vlan_pcp: vlan_pcp=%#x", action->vlan_pcp );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_strip_vlan_to_string( const struct ofp_action_header *action, char *str, size_t size ) {
  UNUSED( action );
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "strip_vlan" );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


static bool
action_dl_addr_to_string( const struct ofp_action_dl_addr *action, char *str, size_t size, uint16_t type ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "set_dl_%s: dl_addr=%02x:%02x:%02x:%02x:%02x:%02x",
                      type == OFPAT_SET_DL_SRC ? "src" : "dst",
                      action->dl_addr[ 0 ], action->dl_addr[ 1 ], action->dl_addr[ 2 ],
                      action->dl_addr[ 3 ], action->dl_addr[ 4 ], action->dl_addr[ 5 ] );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_set_dl_src_to_string( const struct ofp_action_dl_addr *action, char *str, size_t size ) {
  return action_dl_addr_to_string( action, str, size, OFPAT_SET_DL_SRC );
}


bool
action_set_dl_dst_to_string( const struct ofp_action_dl_addr *action, char *str, size_t size ) {
  return action_dl_addr_to_string( action, str, size, OFPAT_SET_DL_DST );
}


static bool
action_nw_addr_to_string( const struct ofp_action_nw_addr *action, char *str, size_t size, uint16_t type ) {
  assert( action != NULL );
  assert( str != NULL );

  struct in_addr addr;
  addr.s_addr = htonl( action->nw_addr );
  char nw_addr[ 16 ];
  memset( nw_addr, '\0', sizeof( nw_addr ) );
  inet_ntop( AF_INET, &addr, nw_addr, sizeof( nw_addr ) );
  int ret = snprintf( str, size, "set_nw_%s: nw_addr=%s",
                      type == OFPAT_SET_NW_SRC ? "src" : "dst", nw_addr );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_set_nw_src_to_string( const struct ofp_action_nw_addr *action, char *str, size_t size ) {
  return action_nw_addr_to_string( action, str, size, OFPAT_SET_NW_SRC );
}


bool
action_set_nw_dst_to_string( const struct ofp_action_nw_addr *action, char *str, size_t size ) {
  return action_nw_addr_to_string( action, str, size, OFPAT_SET_NW_DST );
}


bool
action_set_nw_tos_to_string( const struct ofp_action_nw_tos *action, char *str, size_t size ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "set_nw_tos: nw_tos=%#x", action->nw_tos );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


static bool
action_tp_port_to_string( const struct ofp_action_tp_port *action, char *str, size_t size, uint16_t type ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "set_tp_%s: tp_port=%u",
                      type == OFPAT_SET_TP_SRC ? "src" : "dst", action->tp_port );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_set_tp_src_to_string( const struct ofp_action_tp_port *action, char *str, size_t size ) {
  return action_tp_port_to_string( action, str, size, OFPAT_SET_TP_SRC );
}


bool
action_set_tp_dst_to_string( const struct ofp_action_tp_port *action, char *str, size_t size ) {
  return action_tp_port_to_string( action, str, size, OFPAT_SET_TP_DST );
}


bool
action_enqueue_to_string( const struct ofp_action_enqueue *action, char *str, size_t size ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "enqueue: port=%u queue_id=%u", action->port, action->queue_id );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
action_vendor_to_string( const struct ofp_action_vendor_header *action, char *str, size_t size ) {
  assert( action != NULL );
  assert( str != NULL );

  int ret = snprintf( str, size, "vendor: vendor=%#x", action->vendor );
  if ( ( ret >= ( int ) size ) || ( ret < 0 ) ) {
    return false;
  }

  return true;
}


bool
actions_to_string( const struct ofp_action_header *actions, uint16_t actions_length, char *str, size_t str_length ) {
  assert( actions != NULL );
  assert( str != NULL );
  assert( actions_length > 0 );
  assert( str_length > 0 );

  memset( str, '\0', str_length );

  bool ret = true;
  size_t offset = 0;
  while ( ( actions_length - offset ) >= sizeof( struct ofp_action_header ) ) {
    size_t current_str_length = strlen( str );
    size_t remaining_str_length = str_length - current_str_length;
    if ( current_str_length > 0 && remaining_str_length > 2 ) {
      snprintf( str + current_str_length, remaining_str_length, ", " );
      remaining_str_length -= 2;
      current_str_length += 2;
    }
    char *p = str + current_str_length;
    const struct ofp_action_header *header = ( const struct ofp_action_header * ) ( ( const char * ) actions + offset );
    switch ( header->type ) {
      case OFPAT_OUTPUT:
        ret = action_output_to_string( ( const struct ofp_action_output * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_VLAN_VID:
        ret = action_set_vlan_vid_to_string( ( const struct ofp_action_vlan_vid * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_VLAN_PCP:
        ret = action_set_vlan_pcp_to_string( ( const struct ofp_action_vlan_pcp * ) header, p, remaining_str_length );
        break;
      case OFPAT_STRIP_VLAN:
        ret = action_strip_vlan_to_string( ( const struct ofp_action_header * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_DL_SRC:
        ret = action_set_dl_src_to_string( ( const struct ofp_action_dl_addr * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_DL_DST:
        ret = action_set_dl_dst_to_string( ( const struct ofp_action_dl_addr * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_NW_SRC:
        ret = action_set_nw_src_to_string( ( const struct ofp_action_nw_addr * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_NW_DST:
        ret = action_set_nw_dst_to_string( ( const struct ofp_action_nw_addr * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_NW_TOS:
        ret = action_set_nw_tos_to_string( ( const struct ofp_action_nw_tos * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_TP_SRC:
        ret = action_set_tp_src_to_string( ( const struct ofp_action_tp_port * ) header, p, remaining_str_length );
        break;
      case OFPAT_SET_TP_DST:
        ret = action_set_tp_dst_to_string( ( const struct ofp_action_tp_port * ) header, p, remaining_str_length );
        break;
      case OFPAT_ENQUEUE:
        ret = action_enqueue_to_string( ( const struct ofp_action_enqueue * ) header, p, remaining_str_length );
        break;
      case OFPAT_VENDOR:
        ret = action_vendor_to_string( ( const struct ofp_action_vendor_header * ) header, p, remaining_str_length );
        break;
      default:
        snprintf( p, remaining_str_length, "undefined: type=%#x", header->type );
        break;
    }

    if ( ret == false ) {
      break;
    }
    offset += header->len;
  }

  str[ str_length - 1 ] = '\0';

  return ret;
}


uint16_t
get_checksum( uint16_t *pos, uint32_t size ) {
  assert( pos != NULL );

  uint32_t csum = 0;
  for (; 2 <= size; pos++, size -= 2 ) {
    csum += *pos;
  }
  if ( size == 1 ) {
    csum += *( unsigned char * ) pos;
  }
  while ( csum & 0xffff0000 ) {
    csum = ( csum & 0x0000ffff ) + ( csum >> 16 );
  }

  return ( uint16_t ) ~csum;
}


void
xfree_data( void *data, void *user_data ) {
  UNUSED( user_data );
  xfree( data );
}


bool
string_equal( void *data, void *user_data ) {
  return strcmp( data, user_data ) == 0;
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
