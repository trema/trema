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

#ifndef _TRX_H_
#define _TRX_H_

#include "eth.h"

#define TRX_TRX_LOOP_COUNT 4
#define TRX_RX_LOOP_COUNT 128
#define TRX_TX_LOOP_COUNT 128
#define TRX_ARP_WAITLIST_TIMEOUT 2 /* in seconds */

int trx_init(void *recv, void *send);
int trx_txq_push(eth *eth);
int trx_txq_arp_waitlist_push(eth *eth, uint32_t ip_addr);
int trx_rxq_push(uint8_t *buffer, uint32_t length);
eth *trx_rxq_pop();
int trx_all();
int trx_tx();
int trx_tx_one();
int trx_tx_immediately(eth *eth);
int trx_tx_arp_waitlist();
int trx_rx();
int trx_rx_one();
uint32_t trx_get_txq_size();
uint32_t trx_get_txq_arp_waitlist_size();
uint32_t trx_get_rxq_size();

#endif /* _TRX_H_ */
