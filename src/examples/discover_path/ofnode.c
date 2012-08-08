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


#include "ofnode.h"

#define STRLEN ( 256 )

/* Switch Info Table */
struct ofnode *ofnode_table[ NODE_MAX ];

static void do_remove( struct ofnode * );
static char *mac2str( uint8_t *, int );

void
ofnode_init_table() {
  int i;

  for ( i = 0; i < NODE_MAX; i++ ) {
    ofnode_table[ i ] = NULL;
  }
}

struct ofnode *
ofnode_get_switch( uint64_t dpid ) {
  struct ofnode *node = NULL;
  int i;

  for ( i = 0; i < NODE_MAX; i++ ) {
    struct ofnode *tnode = ofnode_table[ i ];

    if ( ( tnode != NULL ) 
        && ( tnode->type == TYPE_SWITCH ) 
        && ( tnode->switch_info.dpid == dpid ) ) {
      node = tnode;
      break;
    }
  }

  return node;
}

struct ofnode *
ofnode_get_host( uint32_t nwaddr ) {
  struct ofnode *node = NULL;
  int i;

  for ( i = 0; i < NODE_MAX; i++ ) {
    struct ofnode *tnode = ofnode_table[ i ];

    if ( ( tnode != NULL ) 
        && ( tnode->type == TYPE_HOST ) 
        && ( tnode->host_info.nwaddr == nwaddr ) ) {
      node = tnode;
      break;
    }
  }

  return node;
}

struct ofnode *
ofnode_create_switch( uint64_t dpid, uint16_t len ) {
  struct ofnode *node = NULL;
  int i;

  /* checking the ofnode object that has required feature is */
  node = ofnode_get_switch( dpid );
  if ( node != NULL) {
    return node;
  }

  for ( i = 0; i < NODE_MAX; i++ ) {
    if ( ofnode_table[ i ] == NULL ) {
      node = xmalloc( sizeof( struct ofnode ) );

      node->type = TYPE_SWITCH;
      node->switch_info.dpid = dpid;
      node->switch_info.port_len = len;
      node->switch_info.ports = xmalloc( sizeof( struct ofnode * ) * len );
      memset( node->switch_info.ports, 0, sizeof( struct ofnode * ) * len );

      ofnode_table[ i ] = node;

      break;
    }
  }

  return node;
}

struct ofnode *
ofnode_create_host( uint8_t *mac_addr, uint32_t nw_addr ) {
  struct ofnode *node = NULL;
  int i;

  /* checking the ofnode object that has required feature is */
  node = ofnode_get_host( nw_addr );
  if ( node != NULL ) {
    return node;
  }

  for ( i = 0; i < NODE_MAX; i++ ) {
    if ( ofnode_table[ i ] == NULL ) {
      node = xmalloc( sizeof( struct ofnode ) );

      node->type = TYPE_HOST;
      node->host_info.nwaddr = nw_addr;
      memcpy( node->host_info.macaddr, mac_addr, ETH_ADDRLEN );

      ofnode_table[ i ] = node;

      break;
    }
  }

  return node;
}

void 
ofnode_remove( struct ofnode *node ) {
  int i;

  for ( i = 0; i < NODE_MAX; i++ ) {
    if ( ofnode_table[ i ] == node ) {
      do_remove( node );

      ofnode_table[ i ] = NULL;
    }
  }
}

void
ofnode_destroy() {
  int i;

  for ( i = 0; i < NODE_MAX; i++ ) {
    do_remove( ofnode_table[ i ] );

    ofnode_table[ i ] = NULL;
  }
}

void
ofnode_show_table() {
  int i;

  for ( i = 0; i < NODE_MAX; i++ ) {
    struct ofnode *node = ofnode_table[ i ];
    if ( ( node != NULL ) && ( node->type == TYPE_SWITCH ) ) {

      int p_num;
      for ( p_num = 1; p_num < node->switch_info.port_len; p_num++ ) {
        char str[ STRLEN ];
        struct ofnode *tonode = node->switch_info.ports[ p_num ];
        int offset;

        offset = sprintf( str, "[ ofnode_show_table ] dpid:%llx, port_no:%d => ", 
            node->switch_info.dpid, p_num );

        if ( tonode != NULL ) {
          if ( tonode->type == TYPE_SWITCH ) {
            /* seting switch infomation */

            offset += sprintf( str + offset, "switch ( dpid:%llx )", tonode->switch_info.dpid );

          } else if ( tonode->type == TYPE_HOST ) {
            /* setting host infomation */

            struct in_addr addr;
            addr.s_addr = ntohl( tonode->host_info.nwaddr );

            offset += sprintf( str + offset, "host ( macaddr:%s, nw_addr:%s )", 
                mac2str( tonode->host_info.macaddr, ETH_ADDRLEN ), inet_ntoa( addr ) );

          }
        }

        info( "%s", str );
      }
    }
  }
}

static void 
do_remove( struct ofnode *node ) {
  if ( node != NULL ) {
    if ( node->type == TYPE_SWITCH ) {
      xfree( node->switch_info.ports );
    }

    xfree( node );
  }
}

static char *
mac2str( uint8_t *addr, int len ) {
  static char str[ STRLEN ];
  int i, offset;

  memset( str, 0, STRLEN );
  for ( i = offset = 0; i < len; i++ ) {
    if ( i == ( len - 1 ) ) {
      sprintf( str + offset, "%02x", addr[ i ] );
    } else {
      offset += sprintf( str + offset, "%02x:", addr[ i ] );
    }
  }

  return str;
}
