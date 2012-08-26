/*
 * Author: Hiroyasu OHYAMA
 *
 * Copyright (C) 2012 Univ. of tsukuba
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


#ifndef __ofswitch_h__
#define __ofswitch_h__

#include "trema.h"

#define NODE_MAX ( 512 )

#define TYPE_SWITCH ( 0x1 )
#define TYPE_HOST ( 0x2 )

/* OpenFlow node, which means switch or host, infomation */
struct ofnode {
  uint32_t type;
  union {
    struct {
      uint64_t dpid;
      uint16_t port_len;
      struct ofnode **ports;
    } switch_info;

    struct {
      uint8_t macaddr[ ETH_ADDRLEN ];
      uint32_t nwaddr;
      void *data;
    } host_info;
  };
};

void ofnode_init_table( void );
struct ofnode *ofnode_get_switch( uint64_t );
struct ofnode *ofnode_get_host( uint32_t );
struct ofnode *ofnode_create_switch( uint64_t, uint16_t );
struct ofnode *ofnode_create_host( uint8_t *, uint32_t );
void ofnode_remove( struct ofnode * );
void ofnode_destroy( void );
void ofnode_show_table( void );

#endif
