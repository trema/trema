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
#include <arpa/inet.h>
#include "eth.h"
#include "arp.h"
#include "ipv4.h"
#include "icmp.h"
#include "udp.h"
#include "trx.h"
#include "utils.h"
#include "common.h"
#include "log.h"

static uint32_t ipv4_host_ip_addr;
static uint32_t ipv4_host_ip_bcast;
static uint32_t ipv4_host_ip_mask;
static uint8_t ipv4_initialized = 0;
static int ipv4_promiscuous = 0;

int ipv4_init(uint32_t ip_addr, uint32_t ip_mask)
{
    if(ipv4_initialized){
        ipv4_uninit();
    }

    ipv4_host_ip_addr = ip_addr;
    ipv4_host_ip_bcast = ip_addr | ~ip_mask;
    ipv4_host_ip_mask = ip_mask;

    log_debug("addr = 0x%08x", ipv4_host_ip_addr);
    log_debug("mask = 0x%08x", ipv4_host_ip_mask);
    log_debug("bcast = 0x%08x", ipv4_host_ip_bcast);

    ipv4_initialized = 1;

    return 0;
}

int ipv4_uninit()
{
    ipv4_host_ip_addr = 0;
    ipv4_host_ip_bcast = 0;
    ipv4_host_ip_mask = 0;
    ipv4_initialized = 0;
    return 0;
}

int ipv4_handle_message(eth *eth)
{
    if(eth == NULL){
        log_err("eth is null.");
        return -1;
    }

    if(eth->type != ETH_TYPE_IPV4){
        log_err("ether type is not equal to ipv4.");
        return -1;
    }

    if(!ipv4_initialized){
        log_err("ipv4 is not initialized yet.");
        return -1;
    }

    ipv4 *ip;

    ip = ipv4_create_from_raw(eth->payload, eth->length);

    if(ip == NULL){
        log_err("cannot create ipv4 object.");
        return -1;
    }

    if(!ipv4_promiscuous && (ip->dst != ipv4_host_ip_addr) &&
       (ip->dst != ipv4_host_ip_bcast)){
        ipv4_destroy(ip);
        return 0;
    }

    switch(ip->protocol){
    case IPV4_PROTOCOL_ICMP:
        log_debug("icmp message found.");
        if((ip->dst == ipv4_host_ip_addr) ||
           (ip->dst == ipv4_host_ip_bcast)){
            if(icmp_handle_message(ip) < 0){
                /* push back to rx queue */
                /* TBI */
            }
        }
        break;
    case IPV4_PROTOCOL_TCP:
        log_warn("tcp is not implemented yet.");
        break;
    case IPV4_PROTOCOL_UDP:
        udp_handle_message(ip);
        break;
    default:
        log_warn("ip protocol %u is not implemented yet.",
                 ip->protocol);
        break;
    }

    ipv4_destroy(ip);

    return 0;
}


ipv4 *ipv4_create(uint32_t src, uint32_t dst, uint16_t protocol,
                  uint8_t *payload, uint16_t payload_length)
{
    if(payload == NULL && payload_length > 0){
        log_err("payload_length is not zero but payload is null.");
        return NULL;
    }

    ipv4 *ip;

    ip = (struct ipv4*)malloc(sizeof(struct ipv4));

    ip->src = src;
    ip->dst = dst;
    ip->protocol = protocol;
    ip->payload_length = payload_length;
    ip->hdr_length = IPV4_DEFAULT_HLEN * 4;

    ip->payload = (uint8_t*)malloc(sizeof(uint8_t)*payload_length);

    memcpy(ip->payload, payload, sizeof(uint8_t)*payload_length);

    return ip;
}

ipv4 *ipv4_create_from_raw(uint8_t *packet, uint32_t length)
{
    uint8_t *p;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    ipv4 *ip;

    p = packet;

    memcpy(&u8, p, sizeof(u8));

    if(((u8 & 0xf0) >> 4) != IPV4_VERSION){
        log_err("invalid version number");
        return NULL;
    }

    ip = (struct ipv4*)malloc(sizeof(struct ipv4));

    u8 &= 0x0f;
    ip->hdr_length = u8 * 4;
    if(length < ip->hdr_length){
        log_err("too short ipv4 packet (packet size = %u).", length);
        free(ip);
        return NULL;
    }
    p += IPV4_VERSION_HLEN_LEN; /* version + header length */
    p += IPV4_DSCP_LEN; /* dscp/tos */

    memcpy(&u16, p, sizeof(u16));
    ip->payload_length = ntohs(u16) - ip->hdr_length;

    log_debug("ip length = %u", ntohs(u16));
    log_debug("ip header length = %u", ip->hdr_length);
    log_debug("ip payload length = %u", ip->payload_length);

    if(ntohs(u16) > length){
        log_err("too short ipv4 packet (length in ipv4 header = %u, packet size = %u).",
                ntohs(u16), length);
        free(ip);
        return NULL;
    }
    p += IPV4_TOTAL_LEN_LEN; /* total length */
    p += IPV4_ID_LEN; /* id */
    p += IPV4_FRAGMENT_LEN; /* flags + fragment offset */
    p += IPV4_TTL_LEN; /* ttl */

    memcpy(&u8, p, sizeof(u8));
    ip->protocol = u8;
    p += IPV4_PROTOCOL_LEN; /* protocol */
    p += IPV4_CHECKSUM_LEN; /* checksum */

    memcpy(&u32, p, sizeof(u32));
    ip->src = ntohl(u32);
    p += IPV4_ADDR_LEN; /* ip src */

    memcpy(&u32, p, sizeof(u32));
    ip->dst = ntohl(u32);
    p += IPV4_ADDR_LEN; /* ip dst */

    ip->payload = (uint8_t*)malloc(sizeof(uint8_t)*ip->payload_length);
    memcpy(ip->payload, p, ip->payload_length);

    return ip;
}

int ipv4_set_payload(ipv4 *ip, uint8_t *payload, uint32_t length)
{
    if(ip == NULL){
        return -1;
    }

    if(payload == NULL && length != 0){
        return -1;
    }

    if(payload == NULL && length == 0){
        ip->payload_length = 0;
        free(ip->payload);
        ip->payload = NULL;

        return 0;
    }

    if(ip->payload != NULL){
        free(ip->payload);
    }

    ip->payload = (uint8_t *)malloc(sizeof(uint8_t)*length);
    memcpy(ip->payload, payload, length);
    ip->payload_length = length;

    return 0;
}

int ipv4_set_payload_nocopy(ipv4 *ip, uint8_t *payload, uint32_t length)
{
    if(ip == NULL){
        return -1;
    }

    if(payload == NULL && length != 0){
        return -1;
    }

    ip->payload = payload;
    ip->payload_length = length;

    return 0;
}

int ipv4_destroy(ipv4 *ip)
{
    if(ip == NULL){
        log_err("ip is null.");
        return -1;
    }

    if(ip->payload != NULL){
        free(ip->payload);
    }

    free(ip);

    return 0;
}

uint8_t *ipv4_get_packet(ipv4 *ip, uint8_t *buffer, uint32_t *length)
{
    if(ip == NULL){
        log_err("ip is null.");
        return NULL;
    }

    *length = ip->hdr_length + ip->payload_length;
    if(buffer == NULL){
        buffer = (uint8_t *)malloc(sizeof(uint8_t)*(*length));
    }

    uint8_t u8;
    uint8_t *p, *pchksum;
    uint16_t u16;
    uint32_t u32;

    p = buffer;

    memset(p, 0, sizeof(uint8_t)*(*length));

    u8 = IPV4_VERSION << 4 | IPV4_DEFAULT_HLEN;
    memcpy(p, &u8, sizeof(u8));
    p += IPV4_VERSION_HLEN_LEN;
    p += IPV4_DSCP_LEN;

    u16 = htons(ip->hdr_length + ip->payload_length);
    memcpy(p, &u16, sizeof(u16));
    p += IPV4_TOTAL_LEN_LEN;
    p += IPV4_ID_LEN;
    p += IPV4_FRAGMENT_LEN;

    u8 = IPV4_DEFAULT_TTL;
    memcpy(p, &u8, sizeof(u8));
    p += IPV4_TTL_LEN;

    u8 = ip->protocol;
    memcpy(p, &u8, sizeof(u8));
    p += IPV4_PROTOCOL_LEN;

    pchksum = p;
    p += IPV4_CHECKSUM_LEN;

    u32 = htonl(ip->src);
    memcpy(p, &u32, sizeof(u32));
    p += IPV4_ADDR_LEN;

    u32 = htonl(ip->dst);
    memcpy(p, &u32, sizeof(u32));
    p += IPV4_ADDR_LEN;

    memcpy(p, ip->payload, sizeof(uint8_t)*(ip->payload_length));

    /* update checksum */
    u16 = calc_checksum((void*)buffer, IPV4_DEFAULT_HLEN * 4);
    memcpy(pchksum, &u16, sizeof(u16));
    //log_debug("ip checksum = 0x%x", u16);

    return buffer;
}

uint32_t ipv4_get_host_addr()
{
    return ipv4_host_ip_addr;
}

int ipv4_enable_promiscuous()
{
    ipv4_promiscuous = 1;
    return 0;
}

int ipv4_disable_promiscuous()
{
    ipv4_promiscuous = 0;
    return 0;
}
