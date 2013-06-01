/*
  Copyright (C) 2009-2013 NEC Corporation

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _STATS_H_
#define _STATS_H_

#include <stdint.h>
#include "ipv4.h"
#include "udp.h"

#define STATS_TP_HASH_SIZE (UINT16_MAX + 1)
#define STATS_TP_HASH_MASK UINT16_MAX

typedef struct tp_stats {
    uint32_t lip;
    uint16_t lport;
    uint32_t rip;
    uint16_t rport;
    uint32_t n_pkts;
    uint64_t n_octets;
    struct tp_stats *next;
} tp_stats;

int stats_init();
int stats_udp_send_uninit();
int stats_udp_recv_uninit();
int stats_tp_uninit();
void stats_udp_send_update(ipv4 *ip, udp *udp);
void stats_udp_recv_update(ipv4 *ip, udp *udp);
int stats_tp_update(tp_stats **st, uint32_t key, ipv4 *ip, udp *udp);
tp_stats *stats_udp_send_get(uint32_t *size);
tp_stats *stats_udp_recv_get(uint32_t *size);
int stats_udp_send_dump();
int stats_udp_recv_dump();
int stats_tp_dump(tp_stats **st, uint32_t hash_size);

#endif /* _STATS_H_ */
