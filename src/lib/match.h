/*
 * OpenFlow flow matching library
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


#ifndef MATCH_H
#define MATCH_H


#include <openflow.h>


#define COMPARE_MAC( _x, _y )                                                      \
  ( ( ( ( uint16_t * ) &( _x ) )[ 0 ] == ( ( uint16_t * ) &( _y ) )[ 0 ] )         \
   && ( ( ( uint16_t * ) &( _x ) )[ 1 ] == ( ( uint16_t * ) &( _y ) )[ 1 ] )       \
   && ( ( ( uint16_t * ) &( _x ) )[ 2 ] == ( ( uint16_t * ) &( _y ) )[ 2 ] ) )

#define create_nw_src_mask( _w )                                                   \
  ( {                                                                              \
    uint32_t _m = ( ( _w ) & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;            \
      ( _m >= 32 ) ? 0 : 0xffffffff << _m;                                         \
  }                                                                                \
  )

#define create_nw_dst_mask( _w )                                                   \
  ( {                                                                              \
    uint32_t _m = ( ( _w ) & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;            \
      ( _m >= 32 ) ? 0 : 0xffffffff << _m;                                         \
  }                                                                                \
  )

#define compare_match( _x, _y )                                                    \
  ( {                                                                              \
    uint32_t _w = ( _x )->wildcards | ( _y )->wildcards;                           \
    uint32_t _sm = create_nw_src_mask( ( _x )->wildcards )                         \
                                      & create_nw_src_mask( ( _y )->wildcards );   \
    uint32_t _dm = create_nw_dst_mask( ( _x )->wildcards )                         \
                                      & create_nw_dst_mask( ( _y )->wildcards );   \
      ( ( _w & OFPFW_IN_PORT || ( _x )->in_port == ( _y )->in_port )               \
       && ( _w & OFPFW_DL_VLAN || ( _x )->dl_vlan == ( _y )->dl_vlan )             \
       && ( _w & OFPFW_DL_VLAN_PCP || ( _x )->dl_vlan_pcp == ( _y )->dl_vlan_pcp ) \
       && ( _w & OFPFW_DL_SRC || COMPARE_MAC( ( _x )->dl_src, ( _y )->dl_src ) )   \
       && ( _w & OFPFW_DL_DST || COMPARE_MAC( ( _x )->dl_dst, ( _y )->dl_dst ) )   \
       && ( _w & OFPFW_DL_TYPE || ( _x )->dl_type == ( _y )->dl_type )             \
       && !( ( ( _x )->nw_src ^ ( _y )->nw_src ) & _sm )                           \
       && !( ( ( _x )->nw_dst ^ ( _y )->nw_dst ) & _dm )                           \
       && ( _w & OFPFW_NW_TOS || ( _x )->nw_tos == ( _y )->nw_tos )                \
       && ( _w & OFPFW_NW_PROTO || ( _x )->nw_proto == ( _y )->nw_proto )          \
       && ( _w & OFPFW_TP_SRC || ( _x )->tp_src == ( _y )->tp_src )                \
       && ( _w & OFPFW_TP_DST || ( _x )->tp_dst == ( _y )->tp_dst ) );             \
  }                                                                                \
  )

#define compare_match_strict( _x, _y )                                             \
  ( ( ( ( ( _x )->wildcards ^ ( _y )->wildcards ) & OFPFW_ALL ) == 0 )             \
    && compare_match( ( _x ), ( _y ) ) )


#endif // MATCH_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
