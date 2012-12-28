/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#ifndef PROBE_TIMER_TABLE_H
#define PROBE_TIMER_TABLE_H


#include <time.h>
#include "trema.h"


typedef struct probe_timer_entry {
  struct timespec expires;
  uint64_t datapath_id;
  uint16_t port_no;
  uint8_t mac[ ETH_ADDRLEN ];
  int state;
  int retry_count;
  bool link_up;
  uint64_t to_datapath_id;
  uint16_t to_port_no;
  bool dirty;
} probe_timer_entry;


enum probe_timer_state {
  PROBE_TIMER_STATE_INACTIVE = 0,
  PROBE_TIMER_STATE_SEND_DELAY,
  PROBE_TIMER_STATE_WAIT,
  PROBE_TIMER_STATE_CONFIRMED,
};


enum probe_timer_event {
  PROBE_TIMER_EVENT_UP,
  PROBE_TIMER_EVENT_DOWN,
  PROBE_TIMER_EVENT_RECV_LLDP,
  PROBE_TIMER_EVENT_TIMEOUT,
};


void probe_request( probe_timer_entry *entry, int event, uint64_t *dpid, uint16_t port_no );

void init_probe_timer_table( void );
void finalize_probe_timer_table( void );

probe_timer_entry *allocate_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no, const uint8_t *mac );
void free_probe_timer_entry( probe_timer_entry *free_entry );
void insert_probe_timer_entry( probe_timer_entry *entry );
probe_timer_entry *delete_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no );
probe_timer_entry *lookup_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no );


#endif // PROBE_TIMER_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
