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

#ifndef _ARP_H_
#define _ARP_H_

#include <time.h>
#include "eth.h"

#define ARP_MESSAGE_SIZE_ETH_IP 46

#define ARP_HW_TYPE_ETH 0x0001
#define ARP_PROTOCOL_TYPE_IP 0x0800
#define ARP_HW_SIZE_ETH 0x06
#define ARP_PROTOCOL_SIZE 0x04
#define ARP_OPCODE_REQUEST 0x0001
#define ARP_OPCODE_REPLY 0x0002

#define ARP_TABLE_HASH_SIZE 65536
#define ARP_TABLE_HASH_MASK 0xffff

#define ARP_TABLE_AGE_TIMEOUT 300

/*
#define ARP_TABLE_HASH_SHA1
*/

typedef struct arp_entry {
    uint8_t mac_addr[ETH_ADDR_LEN];
    uint32_t ip_addr;
    time_t last_update;
    struct arp_entry *prev;
    struct arp_entry *next;
} arp_entry;

extern uint8_t arp_host_mac_addr[ETH_ADDR_LEN];

int arp_init(uint8_t mac_addr[ETH_ADDR_LEN], uint32_t ip_addr);
int arp_uninit();
int arp_handle_message(eth *eth);
int arp_send_reply(eth *eth);
int arp_send_request(uint32_t ip_addr);

int arp_get_mac_by_ip(uint32_t ip_addr, uint8_t *mac_addr);
arp_entry *arp_get_entry_by_ip(uint32_t ip_addr);
arp_entry *arp_add_entry(uint32_t ip_addr, uint8_t mac_addr[ETH_ADDR_LEN], time_t now);
int arp_update_entry(uint32_t ip_addr, uint8_t mac_addr[ETH_ADDR_LEN],
                     time_t now);
int arp_delete_entry(uint32_t ip_addr);
int arp_age_entries();
uint32_t arp_get_hash(uint32_t ip_addr);

#endif /* _ARP_H */
