/*
 * Utility functions for converting byteorder.
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


#ifndef BYTEORDER_H
#define BYTEORDER_H


#include <byteswap.h>
#include <endian.h>
#include <openflow.h>


#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll( _x ) ( _x )
#define htonll( _x ) ( _x )
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohll( _x ) bswap_64( _x )
#define htonll( _x ) bswap_64( _x )
#endif


void ntoh_match( struct ofp_match *dst, const struct ofp_match *src );
#define hton_match ntoh_match

void ntoh_phy_port( struct ofp_phy_port *dst, const struct ofp_phy_port *src );
#define hton_phy_port ntoh_phy_port

void ntoh_action_output( struct ofp_action_output *dst, const struct ofp_action_output *src );
#define hton_action_output ntoh_action_output

void ntoh_action_vlan_vid( struct ofp_action_vlan_vid *dst, const struct ofp_action_vlan_vid *src );
#define hton_action_vlan_vid ntoh_action_vlan_vid

void ntoh_action_vlan_pcp( struct ofp_action_vlan_pcp *dst, const struct ofp_action_vlan_pcp *src );
#define hton_action_vlan_pcp ntoh_action_vlan_pcp

void ntoh_action_strip_vlan( struct ofp_action_header *dst, const struct ofp_action_header *src );
#define hton_action_strip_vlan ntoh_action_strip_vlan

void ntoh_action_dl_addr( struct ofp_action_dl_addr *dst, const struct ofp_action_dl_addr *src );
#define hton_action_dl_addr ntoh_action_dl_addr

void ntoh_action_nw_addr( struct ofp_action_nw_addr *dst, const struct ofp_action_nw_addr *src );
#define hton_action_nw_addr ntoh_action_nw_addr

void ntoh_action_nw_tos( struct ofp_action_nw_tos *dst, const struct ofp_action_nw_tos *src );
#define hton_action_nw_tos ntoh_action_nw_tos

void ntoh_action_tp_port( struct ofp_action_tp_port *dst, const struct ofp_action_tp_port *src );
#define hton_action_tp_port ntoh_action_tp_port

void ntoh_action_enqueue( struct ofp_action_enqueue *dst, const struct ofp_action_enqueue *src );
#define hton_action_enqueue ntoh_action_enqueue

void ntoh_action_vendor( struct ofp_action_vendor_header *dst, const struct ofp_action_vendor_header *src );
void hton_action_vendor( struct ofp_action_vendor_header *dst, const struct ofp_action_vendor_header *src );

void ntoh_action( struct ofp_action_header *dst, const struct ofp_action_header *src );
void hton_action( struct ofp_action_header *dst, const struct ofp_action_header *src );

void ntoh_flow_stats( struct ofp_flow_stats *dst, const struct ofp_flow_stats *src );
void hton_flow_stats( struct ofp_flow_stats *dst, const struct ofp_flow_stats *src );

void ntoh_aggregate_stats( struct ofp_aggregate_stats_reply *dst, const struct ofp_aggregate_stats_reply *src );
#define hton_aggregate_stats ntoh_aggregate_stats

void ntoh_table_stats( struct ofp_table_stats *dst, const struct ofp_table_stats *src );
#define hton_table_stats ntoh_table_stats

void ntoh_port_stats( struct ofp_port_stats *dst, const struct ofp_port_stats *src );
#define hton_port_stats ntoh_port_stats

void ntoh_queue_stats( struct ofp_queue_stats *dst, const struct ofp_queue_stats *src );
#define hton_queue_stats ntoh_queue_stats

void ntoh_queue_property( struct ofp_queue_prop_header *dst, const struct ofp_queue_prop_header *src );
void hton_queue_property( struct ofp_queue_prop_header *dst, const struct ofp_queue_prop_header *src );

void ntoh_packet_queue( struct ofp_packet_queue *dst, const struct ofp_packet_queue *src );
void hton_packet_queue( struct ofp_packet_queue *dst, const struct ofp_packet_queue *src );


#endif // BYTEORDER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
