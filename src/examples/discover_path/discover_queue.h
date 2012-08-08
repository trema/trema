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


#ifndef __DISCOVERY_QUEUE__
#define __DISCOVERY_QUEUE__

typedef struct {
  uint8_t type;
  uint64_t target_dpid;
  uint16_t in_port;

  // attribute for switch
  uint64_t from_dpid;

  // attribute for host
  uint8_t mac_addr[ ETH_ADDRLEN ];
  uint32_t nw_addr;
} discover_queue_entry;

struct discover_queue_head {
  pthread_mutex_t *mutex;
  list_element *head;
};

void discover_queue_init( void );
void discover_queue_destroy( void );
void discover_queue_push_switch( uint64_t, uint64_t, uint16_t );
void discover_queue_push_host( uint64_t, uint8_t *, uint32_t, uint16_t );
void discover_queue_push( discover_queue_entry * );
discover_queue_entry *discover_queue_pop();

#endif
