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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#ifdef ARP_TABLE_HASH_SHA1
#include <openssl/sha.h>
#endif
#include "arp.h"
#include "eth.h"
#include "trx.h"
#include "utils.h"
#include "common.h"
#include "log.h"

uint8_t arp_host_mac_addr[ETH_ADDR_LEN];
static uint32_t arp_host_ip_addr = 0;
static uint8_t arp_initialized = 0;

static arp_entry *arp_table[ARP_TABLE_HASH_SIZE];

int arp_init(uint8_t mac_addr[ETH_ADDR_LEN], uint32_t ip_addr)
{
    int ret;

    if(arp_initialized){
        arp_uninit();
    }

    memcpy(arp_host_mac_addr, mac_addr, ETH_ADDR_LEN);
    memcpy(&arp_host_ip_addr, &ip_addr, sizeof(uint32_t));
    memset(arp_table, 0, sizeof(arp_table));

    ret = arp_update_entry(ip_addr, mac_addr, UINT32_MAX);
    if(ret < 0){
        return -1;
    }

    arp_initialized = 1;

    return 0;
}

int arp_uninit()
{
    arp_delete_entry(arp_host_ip_addr);
    arp_initialized = 0;

    return 0;
}

int arp_handle_message(eth *eth)
{
    if(eth == NULL){
        log_err("eth is null.");
        return -1;
    }

    uint8_t mac_addr[ETH_ADDR_LEN];
    uint16_t opcode;
    uint32_t sender;

    if(eth->type == ETH_TYPE_ARP){
        if(eth->payload != NULL &&
           eth->length >= ARP_MESSAGE_SIZE_ETH_IP){
            log_debug("arp request received.");
            memcpy(&opcode, eth->payload + 6, 2);
            opcode = ntohs(opcode);
            if(opcode == ARP_OPCODE_REQUEST){
                arp_send_reply(eth);
            }
        }
        else{
            log_err("malformed arp message (length = %u).",
                    eth->length);
            return -1;
        }

        memcpy(mac_addr, eth->payload + 8, ETH_ADDR_LEN);
        memcpy(&sender, eth->payload + 8+6, 4);
        sender = ntohl(sender);
        arp_update_entry(sender, mac_addr, time(NULL));
    }

    return 0;
}

/*
int arp_entry_update(eth *eth)
{
    if(eth == NULL){
        log_err("eth is null.");
        return -1;
    }

    uint8_t mac_addr[ETH_ADDR_LEN];
    uint16_t opcode;
    uint32_t sender;
    arp_entry *entry;

    if(eth->payload != NULL &&
       eth->length == ARP_MESSAGE_SIZE_ETH_IP){
        memcpy(&opcode, eth->payload + 6, 2);
        opcode = ntohs(opcode);
        memcpy(mac_addr, eth->payload + 8, ETH_ADDR_LEN);
        memcpy(&sender, eth->payload + 8+6, 4);
        sender = ntohl(sender);

        entry = arp_get_entry_by_ip(sender);
        if(entry != NULL){
            entry->last_update = time(NULL);
            memcpy(entry->mac_addr, mac_addr, ETH_ADDR_LEN);
        }
        else{
            arp_add_entry(sender, mac_addr);
        }
    }
    else{
        log_err("malformed arp message.");
        return -1;
    }

    return 0;
}
*/

int arp_update_entry(uint32_t ip_addr, uint8_t mac_addr[ETH_ADDR_LEN], time_t now)
{
    arp_entry *entry;

    entry = arp_get_entry_by_ip(ip_addr);
    if(entry != NULL){
        /* static entry can only be updated by another static entry */
        if((entry->last_update == UINT32_MAX) &&
           (now != UINT32_MAX)){
            return -1;
        }
        entry->last_update = now;
        memcpy(entry->mac_addr, mac_addr, ETH_ADDR_LEN);
        return 0;
    }

    entry = arp_add_entry(ip_addr, mac_addr, now);
    if(entry == NULL){
        return -1;
    }

    return 0;
}

int arp_send_reply(eth *eth)
{
    if(eth == NULL){
        log_err("eth is null.");
        return -1;
    }

    if(!arp_initialized){
        log_err("arp module is not initialized yet.");
        return -1;
    }

    uint16_t opcode;
    uint32_t target;

    if(eth->payload != NULL &&
       eth->length == ARP_MESSAGE_SIZE_ETH_IP){
        memcpy(&opcode, eth->payload + 6, 2);
        opcode = ntohs(opcode);
        memcpy(&target, eth->payload + 24, 4);
        target = ntohl(target);

        if(!(opcode == ARP_OPCODE_REQUEST &&
             target == arp_host_ip_addr)){
            return -1;
        }
    }
    else{
        log_err("invalid arp request.");
        return -1;
    }

    uint16_t u16;
    uint32_t u32;
    uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t)*ARP_MESSAGE_SIZE_ETH_IP);
    uint8_t *p = buffer;
    struct eth *reply;

    reply = eth_create(arp_host_mac_addr, eth->src, ETH_TYPE_ARP, NULL, 0);

    if(reply == NULL){
        free(buffer);
        return -1;
    }

    u16 = htons(ARP_HW_TYPE_ETH);
    memcpy(p, &u16, 2);
    p += 2;
    u16 = htons(ARP_PROTOCOL_TYPE_IP);
    memcpy(p, &u16, 2);
    p += 2;
    *p = ARP_HW_SIZE_ETH;
    p++;
    *p = ARP_PROTOCOL_SIZE;
    p++;
    u16 = htons(ARP_OPCODE_REPLY);
    memcpy(p, &u16, 2);
    p += 2;
    memcpy(p, arp_host_mac_addr, ETH_ADDR_LEN);
    p += 6;
    u32 = htonl(arp_host_ip_addr);
    memcpy(p, &u32, 4);
    p += 4;
    memcpy(p, eth->payload + 8, 10);
    p += 10;

    eth_set_payload_nocopy(reply, buffer, ARP_MESSAGE_SIZE_ETH_IP);

    if(trx_txq_push(reply) < 0){
        log_err("ARP reply is not queued.");
        eth_destroy(reply); /* buffer should be copied by trx_txq_push(). */
        return -1;
    }
    eth_destroy(reply); /* buffer should be copied by trx_txq_push(). */

    return 0;
}
        
int arp_send_request(uint32_t ip_addr)
{
    if(!arp_initialized){
        log_err("arp module is not initialized yet.");
        return -1;
    }

    uint16_t u16;
    uint32_t u32;
    uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t)*ARP_MESSAGE_SIZE_ETH_IP);
    uint8_t *p = buffer;
    struct eth *request;

    request = eth_create(arp_host_mac_addr, eth_mac_addr_bc, ETH_TYPE_ARP, NULL, 0);

    if(request == NULL){
        free(buffer);
        return -1;
    }

    memset(buffer, 0, sizeof(uint8_t)*ARP_MESSAGE_SIZE_ETH_IP);
    u16 = htons(ARP_HW_TYPE_ETH);
    memcpy(p, &u16, sizeof(u16));
    p += 2;
    u16 = htons(ARP_PROTOCOL_TYPE_IP);
    memcpy(p, &u16, sizeof(u16));
    p += 2;
    *p = ARP_HW_SIZE_ETH;
    p++;
    *p = ARP_PROTOCOL_SIZE;
    p++;
    u16 = htons(ARP_OPCODE_REQUEST);
    memcpy(p, &u16, sizeof(u16));
    p += 2;
    memcpy(p, arp_host_mac_addr, ETH_ADDR_LEN);
    p += 6;
    u32 = htonl(arp_host_ip_addr);
    memcpy(p, &u32, sizeof(u32));
    p += 4;
    p += 6;
    u32 = htonl(ip_addr);
    memcpy(p, &u32, sizeof(u32));

    eth_set_payload_nocopy(request, buffer, ARP_MESSAGE_SIZE_ETH_IP);

    if(trx_txq_push(request) < 0){
        log_err("ARP request is not queued.");
        eth_destroy(request); /* buffer should be copied by trx_txq_push(). */
        return -1;
    }
    eth_destroy(request); /* buffer should be copied by trx_txq_push(). */

    return 0;
}

int arp_get_mac_by_ip(uint32_t ip_addr, uint8_t *mac_addr)
{
    arp_entry *entry;

    entry = arp_get_entry_by_ip(ip_addr);

    if(entry == NULL){
        return -1;
    }

    if(mac_addr == NULL){
        log_err("mac_addr must be allocated by caller.");
        return -1;
    }

    memcpy(mac_addr, entry->mac_addr, ETH_ADDR_LEN);

    return 0;
}

arp_entry *arp_get_entry_by_ip(uint32_t ip_addr)
{
    int found = 0;
    uint32_t hash;
    arp_entry *p;

    hash = arp_get_hash(ip_addr);

    if(arp_table[hash] == NULL){
        log_debug("entry does not exist.");
        return NULL;
    }

    p = arp_table[hash];

    if(p->ip_addr == ip_addr){
        found = 1;
    }
    else{
        while(p->next != NULL){
            if(p->ip_addr == ip_addr){
                found = 1;
                break;
            }
            else{
                p = p->next;
            }
        }
    }

    if(!found){
        log_debug("entry does not exist.");
        return NULL;
    }

    if(log_get_level() >= LOG_DEBUG){
        char mac[ETH_ADDR_LEN*2 + 1];
        memset(mac, 0, sizeof(mac));
        hexdump(p->mac_addr, ETH_ADDR_LEN, mac);
        
        log_debug("entry found: ip addr = 0x%x, mac addr = %s", p->ip_addr, mac);
    }

    return p;
}

arp_entry *arp_add_entry(uint32_t ip_addr, uint8_t mac_addr[ETH_ADDR_LEN], time_t now)
{
    uint32_t hash;
    arp_entry *entry;
    arp_entry *p;

    hash = arp_get_hash(ip_addr);

    entry = (arp_entry*)malloc(sizeof(arp_entry));
    memcpy(entry->mac_addr, mac_addr, ETH_ADDR_LEN);
    entry->ip_addr = ip_addr;
    entry->last_update = now;
    entry->prev = NULL;
    entry->next = NULL;

    if(arp_table[hash] == NULL){
        arp_table[hash] = entry;
    }
    else{
        p = arp_table[hash];
        while(p->next != NULL){
            p = p->next;
        }
        entry->prev = p;
        p->next = entry;
    }

    return entry;
}

int arp_delete_entry(uint32_t ip_addr)
{
    int found = 0;
    uint32_t hash;
    arp_entry *p;

    hash = arp_get_hash(ip_addr);

    if(arp_table[hash] == NULL){
        log_debug("entry does not exist.");
        return -1;
    }

    p = arp_table[hash];

    if(p->ip_addr == ip_addr){
        found = 1;
    }
    else{
        while(p->next != NULL){
            if(p->ip_addr == ip_addr){
                found = 1;
                break;
            }
            else{
                p = p->next;
            }
        }
    }
    if(found){
        if(p->next != NULL && p->prev != NULL){
            // middle
            p->prev->next = p->next;
        }
        else if(p->next == NULL && p->prev != NULL){
            // last
            p->prev->next = NULL;
        }
        else if(p->next != NULL && p->prev == NULL){
            // head
            arp_table[hash] = p->next;
        }
        else{
            arp_table[hash] = NULL;
        }
        free(p);
    }
    else{
        log_debug("entry does not exist.");
        return -1;
    }

    return 0;
}

int arp_age_entries()
{
    int i = 0;
    arp_entry *entry;
    uint32_t ip_addr;

    for(i=0; i<ARP_TABLE_HASH_SIZE; i++){
        entry = arp_table[i];
        while(entry != NULL){
            if((entry->last_update > 0) &&
               (entry->last_update < (time(NULL) - ARP_TABLE_AGE_TIMEOUT))){
                ip_addr = entry->ip_addr;
                entry = entry->next;
                arp_delete_entry(ip_addr);
            }
            else{
                entry = entry->next;
            }
        }
    }

    return 0;
}

uint32_t arp_get_hash(uint32_t ip_addr)
{
    uint32_t hash;

#ifdef ARP_TABLE_HASH_SHA1
    unsigned char *sha;

    sha = (unsigned char*)malloc(sizeof(unsigned char)*SHA_DIGEST_LENGTH);

    memset(sha, 0, sizeof(unsigned char)*SHA_DIGEST_LENGTH);

    sha = SHA1((unsigned char*)&ip_addr, sizeof(ip_addr), sha);

    if(sha == NULL){
        log_err("cannot calculate hash value.");
        free(sha);
        return NULL;
    }

    memcpy(&hash, sha, sizeof(hash));
    free(sha);
#else
    hash = (ip_addr & 0xffff0000 >> 16) + (ip_addr & 0x0000ffff);
#endif

    hash &= ARP_TABLE_HASH_MASK;

    log_debug("hash value = %u", hash);

    return hash;
}
