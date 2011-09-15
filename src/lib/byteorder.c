/*
 * Author: Yasunobu Chiba
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
 * @brief Byte order conversion implementation
 *
 * File containing functions to convert network byteorder to host byteorder
 * and vice versa.
 * @code
 * //  Converts network byteorder to host byteorder for match entry
 * ntoh_match( &match, &flow_removed->match );
 * ...
 * // Converts network byteorder to host byteorder for phy port.
 * ntoh_phy_port( &phy_port, &port_status->desc );
 * ...
 * // Converts packet queue from host to network byteorder.
 * hton_packet_queue( packet_queue, pq );
 * ...
 * // Converts the flow stats from host byteorder to network byteorder.
 * hton_flow_stats( flow_stats, fs );
 * @endcode
 */
#include <assert.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>
#include "byteorder.h"
#include "log.h"
#include "wrapper.h"

/**
 * Converts network byteorder to host byteorder for match entry. It is called when 
 * validation of flow occurs.
 * @param dst Pointer to open flow match compatible with host machine
 * @param src Pointer to open flow match compatible with network
 * @return None
 */
void
ntoh_match( struct ofp_match *dst, const struct ofp_match *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->wildcards = ntohl( src->wildcards );
  dst->in_port = ntohs( src->in_port );
  if ( src != dst ) {
    memcpy( dst->dl_src, src->dl_src, OFP_ETH_ALEN );
    memcpy( dst->dl_dst, src->dl_dst, OFP_ETH_ALEN );
  }
  dst->dl_vlan = ntohs( src->dl_vlan );
  dst->dl_vlan_pcp = src->dl_vlan_pcp;
  dst->dl_type = ntohs( src->dl_type );
  dst->nw_tos = src->nw_tos;
  dst->nw_proto = src->nw_proto;
  memset( dst->pad1, 0, sizeof( dst->pad1 ) );
  memset( dst->pad2, 0, sizeof( dst->pad2 ) );
  dst->nw_src = ntohl( src->nw_src );
  dst->nw_dst = ntohl( src->nw_dst );
  dst->tp_src = ntohs( src->tp_src );
  dst->tp_dst = ntohs( src->tp_dst );
}


/**
 * Converts network byteorder to host byteorder for phy port. It is called in process
 * of validating phy port.
 * @param dst Pointer to phy port compatible with host machine
 * @param src Pointer to phy port compatible with network 
 * @return None
 */
void
ntoh_phy_port( struct ofp_phy_port *dst, const struct ofp_phy_port *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->port_no = ntohs( src->port_no );

  if ( src != dst ) {
    memcpy( dst->hw_addr, src->hw_addr, OFP_ETH_ALEN );
    memcpy( dst->name, src->name, OFP_MAX_PORT_NAME_LEN );
  }

  dst->config = ntohl( src->config );
  dst->state = ntohl( src->state );

  dst->curr = ntohl( src->curr );
  dst->advertised = ntohl( src->advertised );
  dst->supported = ntohl( src->supported );
  dst->peer = ntohl( src->peer );
}


/**
 * Converts the ofp_action_header struture from network byteorder to host byteorder,
 * when packet type is OFPAT_OUTPUT.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_output( struct ofp_action_output *dst,
                    const struct ofp_action_output *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->port = ntohs( src->port );
  dst->max_len = ntohs( src->max_len );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_SET_VLAN_VID. 
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_vlan_vid( struct ofp_action_vlan_vid *dst,
                      const struct ofp_action_vlan_vid *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->vlan_vid = ntohs( src->vlan_vid );
  memset( dst->pad, 0, sizeof( dst->pad ) );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_SET_VLAN_PCP.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_vlan_pcp( struct ofp_action_vlan_pcp *dst,
                      const struct ofp_action_vlan_pcp *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->vlan_pcp = src->vlan_pcp;
  memset( dst->pad, 0, sizeof( dst->pad ) );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_STRIP_VLAN.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_strip_vlan( struct ofp_action_header *dst,
                        const struct ofp_action_header *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  memset( dst->pad, 0, sizeof( dst->pad ) );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_SET_DL_DST or OFPAT_SET_DL_SRC.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_dl_addr( struct ofp_action_dl_addr *dst,
                     const struct ofp_action_dl_addr *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  if ( src != dst ) {
    memcpy( dst->dl_addr, src->dl_addr, OFP_ETH_ALEN );
  }
  memset( dst->pad, 0, sizeof( dst->pad ) );
}


/**
 * Converts the ofp_action_header structure  network byteorder to host byteorder,
 * when packet type is OFPAT_SET_NW_SRC or OFPAT_SET_NW_DST.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_nw_addr( struct ofp_action_nw_addr *dst,
                     const struct ofp_action_nw_addr *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->nw_addr = ntohl( src->nw_addr );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_SET_NW_TOS.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_nw_tos( struct ofp_action_nw_tos *dst,
                    const struct ofp_action_nw_tos *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->nw_tos = src->nw_tos;
  memset( dst->pad, 0, sizeof( dst->pad ) );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_SET_TP_SRC or OFPAT_SET_TP_DST.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_tp_port( struct ofp_action_tp_port *dst,
                     const struct ofp_action_tp_port *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->tp_port = ntohs( src->tp_port );
  memset( dst->pad, 0, sizeof( dst->pad ) );
}


/**
 * Converts the ofp_action_header structure network byteorder to host byteorder,
 * when packet type is OFPAT_ENQUEUE.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_enqueue( struct ofp_action_enqueue *dst,
                     const struct ofp_action_enqueue *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->port = ntohs( src->port );
  memset( dst->pad, 0, sizeof( dst->pad ) );
  dst->queue_id = ntohl( src->queue_id );
}


/**
 * Converts the ofp_action_header structure from network byteorder to host byteorder, 
 * when packet type is OFPAT_VENDOR 
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action_vendor( struct ofp_action_vendor_header *dst,
                    const struct ofp_action_vendor_header *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->type = ntohs( src->type );
  dst->len = ntohs( src->len );
  dst->vendor = ntohl( src->vendor );
}


/**
 * Converts the ofp_action_header from network byteorder to host byteorder. 
 * This is a helper function of ntoh_flow_stats.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
ntoh_action( struct ofp_action_header *dst,
             const struct ofp_action_header *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  switch ( ntohs( src->type ) ) {
  case OFPAT_OUTPUT:
    ntoh_action_output( ( struct ofp_action_output * ) dst,
                        ( const struct ofp_action_output * ) src );
    break;
  case OFPAT_SET_VLAN_VID:
    ntoh_action_vlan_vid( ( struct ofp_action_vlan_vid * ) dst,
                          ( const struct ofp_action_vlan_vid * ) src );
    break;
  case OFPAT_SET_VLAN_PCP:
    ntoh_action_vlan_pcp( ( struct ofp_action_vlan_pcp * ) dst,
                          ( const struct ofp_action_vlan_pcp * ) src );
    break;
  case OFPAT_STRIP_VLAN:
    ntoh_action_strip_vlan( ( struct ofp_action_header * ) dst,
                            ( const struct ofp_action_header * ) src );
    break;
  case OFPAT_SET_DL_SRC:
  case OFPAT_SET_DL_DST:
    ntoh_action_dl_addr( ( struct ofp_action_dl_addr * ) dst,
                         ( const struct ofp_action_dl_addr * ) src );
    break;
  case OFPAT_SET_NW_SRC:
  case OFPAT_SET_NW_DST:
    ntoh_action_nw_addr( ( struct ofp_action_nw_addr * ) dst,
                         ( const struct ofp_action_nw_addr * ) src );
    break;
  case OFPAT_SET_NW_TOS:
    ntoh_action_nw_tos( ( struct ofp_action_nw_tos * ) dst,
                        ( const struct ofp_action_nw_tos * ) src );
    break;
  case OFPAT_SET_TP_SRC:
  case OFPAT_SET_TP_DST:
    ntoh_action_tp_port( ( struct ofp_action_tp_port * ) dst,
                         ( const struct ofp_action_tp_port * ) src );
    break;
  case OFPAT_ENQUEUE:
    ntoh_action_enqueue( ( struct ofp_action_enqueue * ) dst,
                         ( const struct ofp_action_enqueue * ) src );
    break;
  case OFPAT_VENDOR:
    ntoh_action_vendor( ( struct ofp_action_vendor_header * ) dst,
                        ( const struct ofp_action_vendor_header * ) src );
    break;
  default:
    die( "Undefined action type ( type = %d ).", ntohs( src->type ) );
    break;
  }
}


/**
 * Converts the ofp_action_header from host byteorder to network byteorder.
 * This is a helper function of hton_flow_stats.
 * @param dst Pointer to ofp_action_header structure compatible with network 
 * @param src Pointer to ofp_action_header structure compatible with host machine
 * @return None
 */
void
hton_action( struct ofp_action_header *dst,
             const struct ofp_action_header *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  switch ( src->type ) {
  case OFPAT_OUTPUT:
    hton_action_output( ( struct ofp_action_output * ) dst,
                        ( const struct ofp_action_output * ) src );
    break;
  case OFPAT_SET_VLAN_VID:
    hton_action_vlan_vid( ( struct ofp_action_vlan_vid * ) dst,
                          ( const struct ofp_action_vlan_vid * ) src );
    break;
  case OFPAT_SET_VLAN_PCP:
    hton_action_vlan_pcp( ( struct ofp_action_vlan_pcp * ) dst,
                          ( const struct ofp_action_vlan_pcp * ) src );
    break;
  case OFPAT_STRIP_VLAN:
    hton_action_strip_vlan( ( struct ofp_action_header * ) dst,
                            ( const struct ofp_action_header * ) src );
    break;
  case OFPAT_SET_DL_SRC:
  case OFPAT_SET_DL_DST:
    hton_action_dl_addr( ( struct ofp_action_dl_addr * ) dst,
                         ( const struct ofp_action_dl_addr * ) src );
    break;
  case OFPAT_SET_NW_SRC:
  case OFPAT_SET_NW_DST:
    hton_action_nw_addr( ( struct ofp_action_nw_addr * ) dst,
                         ( const struct ofp_action_nw_addr * ) src );
    break;
  case OFPAT_SET_NW_TOS:
    hton_action_nw_tos( ( struct ofp_action_nw_tos * ) dst,
                        ( const struct ofp_action_nw_tos * ) src );
    break;
  case OFPAT_SET_TP_SRC:
  case OFPAT_SET_TP_DST:
    hton_action_tp_port( ( struct ofp_action_tp_port * ) dst,
                         ( const struct ofp_action_tp_port * ) src );
    break;
  case OFPAT_ENQUEUE:
    hton_action_enqueue( ( struct ofp_action_enqueue * ) dst,
                         ( const struct ofp_action_enqueue * ) src );
    break;
  case OFPAT_VENDOR:
    hton_action_vendor( ( struct ofp_action_vendor_header * ) dst,
                        ( const struct ofp_action_vendor_header * ) src );
    break;
  default:
    die( "Undefined action type ( type = %d ).", src->type );
    break;
  }
}


/**
 * Converts the flow stats from network byteorder to host byteorder.
 * @param dst Pointer to flow stat compatible with host machine
 * @param src Pointer to flow stat compatible with network
 * @return None
 */
void
ntoh_flow_stats( struct ofp_flow_stats *dst, const struct ofp_flow_stats *src ) {
  uint16_t actions_length;
  struct ofp_flow_stats *fs;
  struct ofp_action_header *ah_src, *ah_dst;

  assert( src != NULL );
  assert( dst != NULL );

  fs = ( struct ofp_flow_stats * ) xcalloc( 1, ntohs( src->length ) );
  memcpy( fs, src, ntohs( src->length ) );

  dst->length = ntohs( fs->length );
  dst->table_id = fs->table_id;
  dst->pad = 0;
  ntoh_match( &dst->match, &fs->match );
  dst->duration_sec = ntohl( fs->duration_sec );
  dst->duration_nsec = ntohl( fs->duration_nsec );
  dst->priority = ntohs( fs->priority );
  dst->idle_timeout = ntohs( fs->idle_timeout );
  dst->hard_timeout = ntohs( fs->hard_timeout );
  memset( &dst->pad2, 0, sizeof( dst->pad2 ) );
  dst->cookie = ntohll( fs->cookie );
  dst->packet_count = ntohll( fs->packet_count );
  dst->byte_count = ntohll( fs->byte_count );
  
  actions_length = ( uint16_t ) ( ntohs( fs->length )
                                  - offsetof( struct ofp_flow_stats, actions ) );

  ah_src = ( struct ofp_action_header * ) fs->actions;
  ah_dst = ( struct ofp_action_header * ) dst->actions;
  
  while ( actions_length > 0 ) {
    ntoh_action( ah_dst, ah_src );
    actions_length = ( uint16_t ) ( actions_length - ah_dst->len );
    ah_src = ( struct ofp_action_header * ) ( ( char * ) ah_src + ah_dst->len );
    ah_dst = ( struct ofp_action_header * ) ( ( char * ) ah_dst + ah_dst->len );
  }

  xfree( fs );
}


/**
 * Converts the flow stats from host byteorder to network byteorder.
 * @param dst Pointer to flow stat compatible with network
 * @param src Pointer to flow stat compatible with host machine
 * @return None
 */
void
hton_flow_stats( struct ofp_flow_stats *dst, const struct ofp_flow_stats *src ) {
  uint16_t actions_length;
  struct ofp_flow_stats *fs;
  struct ofp_action_header *ah_src, *ah_dst;

  assert( src != NULL );
  assert( dst != NULL );

  fs = ( struct ofp_flow_stats * ) xcalloc( 1, src->length );
  memcpy( fs, src, src->length );

  dst->length = htons( fs->length );
  dst->table_id = fs->table_id;
  dst->pad = 0;
  hton_match( &dst->match, &fs->match );
  dst->duration_sec = htonl( fs->duration_sec );
  dst->duration_nsec = htonl( fs->duration_nsec );
  dst->priority = htons( fs->priority );
  dst->idle_timeout = htons( fs->idle_timeout );
  dst->hard_timeout = htons( fs->hard_timeout );
  memset( &dst->pad2, 0, sizeof( dst->pad2 ) );
  dst->cookie = htonll( fs->cookie );
  dst->packet_count = htonll( fs->packet_count );
  dst->byte_count = htonll( fs->byte_count );

  actions_length = ( uint16_t ) ( fs->length
                                  - offsetof( struct ofp_flow_stats, actions ) );

  ah_src = ( struct ofp_action_header * ) fs->actions;
  ah_dst = ( struct ofp_action_header * ) dst->actions;
  
  while ( actions_length > 0 ) {
    hton_action( ah_dst, ah_src );
    actions_length = ( uint16_t ) ( actions_length - ah_src->len );
    ah_dst = ( struct ofp_action_header * ) ( ( char * ) ah_dst + ah_src->len );
    ah_src = ( struct ofp_action_header * ) ( ( char * ) ah_src + ah_src->len );
  }

  xfree( fs );
}


/**
 * Converts network byteorder to host byteorder all the members of 
 * structure ofp_aggegate_stats, this function is called while handling openflow messages.
 * @param dst Pointer to aggregate stats reply compatible with host machine
 * @param src Pointer to aggregate stats reply compatible with network
 * @return None
 */
void
ntoh_aggregate_stats( struct ofp_aggregate_stats_reply *dst,
                      const struct ofp_aggregate_stats_reply *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->packet_count = ntohll( src->packet_count );
  dst->byte_count = ntohll( src->byte_count );
  dst->flow_count = ntohl( src->flow_count );
  memset( &dst->pad, 0, sizeof( dst->pad ) );
}

/**
 * Helper function to handle stats reply, this function converts 
 * network byteorder to host byteorder the members of ofp_table_stats structure.
 * @param dst Pointer to table stats compatible with host machine
 * @param src Pointer to table stats compatible with network 
 * @return None
 */
void
ntoh_table_stats( struct ofp_table_stats *dst, const struct ofp_table_stats *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->table_id = src->table_id;
  memset( &dst->pad, 0, sizeof( dst->pad ) );
  if ( src != dst ) {
    memcpy( dst->name, src->name, OFP_MAX_TABLE_NAME_LEN );
  }
  dst->wildcards = ntohl( src->wildcards );
  dst->max_entries = ntohl( src->max_entries );
  dst->active_count = ntohl( src->active_count );
  dst->lookup_count = ntohll( src->lookup_count );
  dst->matched_count = ntohll( src->matched_count );
}


/**
 * Converts network byteorder to host byteorder all the members of ofp_port_stats structure.
 * @param dst Pointer to flow stats compatible with host machine
 * @param src Pointer to flow stats compatible with network
 * @return None
 */
void
ntoh_port_stats( struct ofp_port_stats *dst, const struct ofp_port_stats *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->port_no = ntohs( src->port_no );
  memset( &dst->pad, 0, sizeof( dst->pad ) );
  dst->rx_packets = ntohll( src->rx_packets );
  dst->tx_packets = ntohll( src->tx_packets );
  dst->rx_bytes = ntohll( src->rx_bytes );
  dst->tx_bytes = ntohll( src->tx_bytes );
  dst->rx_dropped = ntohll( src->rx_dropped );
  dst->tx_dropped = ntohll( src->tx_dropped );
  dst->rx_errors = ntohll( src->rx_errors );
  dst->tx_errors = ntohll( src->tx_errors );
  dst->rx_frame_err = ntohll( src->rx_frame_err );
  dst->rx_over_err = ntohll( src->rx_over_err );
  dst->rx_crc_err = ntohll( src->rx_crc_err );
  dst->collisions = ntohll( src->collisions );
}


/**
 * Converts network byteorder to host byteorder all the members of queue stats structure.
 * @param dst Pointer to flow stats compatible with host machine
 * @param src Pointer to flow stats compatible with network
 * @return None
 */
void
ntoh_queue_stats( struct ofp_queue_stats *dst, const struct ofp_queue_stats *src ) {
  assert( src != NULL );
  assert( dst != NULL );

  dst->port_no = htons( src->port_no );
  memset( &dst->pad, 0, sizeof( dst->pad ) );
  dst->queue_id = htonl( src->queue_id );
  dst->tx_bytes = htonll( src->tx_bytes );
  dst->tx_packets = htonll( src->tx_packets );
  dst->tx_errors = htonll( src->tx_errors );
}

/**
 * Converts queue properties from network byteorder to host byteorder. This is
 * helper function of ntoh_packet_queues. 
 * @param dst Pointer to queue property header compatible with host machine
 * @param src Pointer to queue property header compatible with network
 * @return None
 */
void
ntoh_queue_property( struct ofp_queue_prop_header *dst,
                     const struct ofp_queue_prop_header *src ) {
  struct ofp_queue_prop_header *ph;
  struct ofp_queue_prop_min_rate *mr_dst, *mr_src;

  assert( src != NULL );
  assert( dst != NULL );
  assert( ntohs( src->property ) <= OFPQT_MIN_RATE );
  assert( ntohs( src->len ) != 0 );

  ph = ( struct ofp_queue_prop_header * ) xcalloc( 1, ntohs( src->len ) );
  memcpy( ph, src, ntohs( src->len ) );

  dst->property = ntohs( ph->property );
  dst->len = ntohs( ph->len );
  
  if ( dst->property == OFPQT_MIN_RATE ) {
    mr_src = ( struct ofp_queue_prop_min_rate * ) ph;
    mr_dst = ( struct ofp_queue_prop_min_rate * ) dst;

    mr_dst->rate = ntohs( mr_src->rate );
  }

  xfree( ph );
}


/**
 * Converts queue properties from host to network byteorder. This is
 * helper function of ntoh_packet_queues. 
 * @param dst Pointer to queue property header compatible with network
 * @param src Pointer to queue property header compatible with host machine
 * @return None
 */
void
hton_queue_property( struct ofp_queue_prop_header *dst,
                     const struct ofp_queue_prop_header *src ) {
  struct ofp_queue_prop_header *ph;
  struct ofp_queue_prop_min_rate *mr_dst, *mr_src;

  assert( src != NULL );
  assert( dst != NULL );
  assert( src->property <= OFPQT_MIN_RATE );
  assert( src->len != 0 );

  ph = ( struct ofp_queue_prop_header * ) xcalloc( 1, src->len );
  memcpy( ph, src, src->len );

  dst->property = htons( ph->property );
  dst->len = htons( ph->len );
  
  if ( src->property == OFPQT_MIN_RATE ) {
    mr_src = ( struct ofp_queue_prop_min_rate * ) ph;
    mr_dst = ( struct ofp_queue_prop_min_rate * ) dst;

    mr_dst->rate = htons( mr_src->rate );
  }

  xfree( ph );
}

/**
 * Converts packet queue from network byteorder to host byteorder.
 * @param dst Pointer to packet queue compatible with host machine
 * @param src Pointer to packet queue compatible with network
 * @return None
 */
void
ntoh_packet_queue( struct ofp_packet_queue *dst,
                   const struct ofp_packet_queue *src) {
  /* Note that ofp_packet_queue is variable length.
   * Please make sure that dst and src have the same length.
   */
  size_t offset;
  uint16_t properties_length;
  struct ofp_packet_queue *pq;
  struct ofp_queue_prop_header *ph_dst, *ph_src;

  assert( src != NULL );
  assert( dst != NULL );
  assert( ntohs( src->len ) != 0 );

  pq = ( struct ofp_packet_queue * ) xcalloc( 1, ntohs( src->len ) );
  memcpy( pq, src, ntohs( src->len ) );

  dst->queue_id = ntohl( pq->queue_id );
  dst->len = ntohs( pq->len );
  memset( &dst->pad, 0, sizeof( dst->pad ) );

  offset = offsetof( struct ofp_packet_queue, properties );
  ph_src = ( struct ofp_queue_prop_header * ) ( ( char * ) pq + offset );
  ph_dst = ( struct ofp_queue_prop_header * ) ( ( char * ) dst + offset );

  properties_length = ( uint16_t ) ( dst->len - offset );

  while ( properties_length > 0 ) {
    ntoh_queue_property( ph_dst, ph_src );

    properties_length = ( uint16_t ) ( properties_length - ph_dst->len );

    ph_src = ( struct ofp_queue_prop_header * ) ( ( char * ) ph_src + ph_dst->len );
    ph_dst = ( struct ofp_queue_prop_header * ) ( ( char * ) ph_dst + ph_dst->len );
  }

  xfree( pq );
}


/**
 * Converts packet queue from host to network byteorder.
 * @param dst Pointer to packet queue compatible with network
 * @param src Pointer to packet queue compatible with host machine
 * @return None
 */
void
hton_packet_queue( struct ofp_packet_queue *dst,
                   const struct ofp_packet_queue *src) {
  /* Note that ofp_packet_queue is variable length.
   * Please make sure that dst and src have the same length.
   */
  size_t offset;
  uint16_t properties_length;
  struct ofp_packet_queue *pq;
  struct ofp_queue_prop_header *ph_dst, *ph_src;

  assert( src != NULL );
  assert( dst != NULL );
  assert( src->len != 0 );

  pq = ( struct ofp_packet_queue * ) xcalloc( 1, src->len );
  memcpy( pq, src, src->len );

  dst->queue_id = htonl( pq->queue_id );
  dst->len = htons( pq->len );
  memset( &dst->pad, 0, sizeof( dst->pad ) );

  offset = offsetof( struct ofp_packet_queue, properties );
  ph_src = ( struct ofp_queue_prop_header * ) ( ( char * ) pq + offset );
  ph_dst = ( struct ofp_queue_prop_header * ) ( ( char * ) dst + offset );

  properties_length = ( uint16_t ) ( src->len - offset );

  while ( properties_length > 0 ) {
    hton_queue_property( ph_dst, ph_src );

    properties_length = ( uint16_t ) ( properties_length - ph_src->len );

    ph_dst = ( struct ofp_queue_prop_header * ) ( ( char * ) ph_dst + ph_src->len );
    ph_src = ( struct ofp_queue_prop_header * ) ( ( char * ) ph_src + ph_src->len );
  }

  xfree( pq );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
