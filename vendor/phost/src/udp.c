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
#include "common.h"
#include "log.h"

static void (*udp_recv_callback)(ipv4*, udp*) = NULL;

int udp_init(void *recv_callback)
{
    return udp_set_recv_callback(recv_callback);
}

int udp_set_recv_callback(void* callback)
{
    udp_recv_callback = callback;

    return 0;
}

int udp_handle_message(ipv4 *ip)
{
    if(ip == NULL){
        log_err("ip is null.");
        return -1;
    }

    if(ip->protocol != IPV4_PROTOCOL_UDP){
        log_err("ip protocol is not equal to udp.");
        return -1;
    }

    udp *udp;

    udp = udp_create_from_raw(ip->payload, ip->payload_length);

    if(udp == NULL){
        log_err("cannot create udp object.");
        return -1;
    }

    log_debug("udp message received: 0x%0x:%u -> 0x%08x:%u (length = %u)",
              ip->src, udp->src, ip->dst, udp->dst, udp->payload_length);

    if(udp_recv_callback != NULL){
        log_debug("calling callback function.");
        (*udp_recv_callback)(ip, udp);
    }

    udp_destroy(udp);

    return 0;
}

udp *udp_create(uint16_t src, uint16_t dst, uint8_t *payload,
                uint32_t payload_length)
{
    if(payload == NULL && payload_length > 0){
        log_err("payload_length is not zero but payload is null.");
        return NULL;
    }

    udp *udp;

    udp = (struct udp*)malloc(sizeof(struct udp));

    udp->src = src;
    udp->dst = dst;
    udp->payload_length = payload_length;

    udp->payload = (uint8_t*)malloc(sizeof(uint8_t)*payload_length);

    memcpy(udp->payload, payload, sizeof(uint8_t)*payload_length);

    return udp;
}

udp *udp_create_from_raw(uint8_t *packet, uint32_t length)
{
    uint8_t *p;
    uint16_t u16;
    udp *udp;

    if(packet == NULL || length < UDP_HDR_LEN){
        log_err("malformed udp packet.");
        return NULL;
    }

    p = packet;

    udp = (struct udp*)malloc(sizeof(struct udp));

    memcpy(&u16, p, sizeof(u16));
    udp->src = ntohs(u16);
    p += UDP_SRC_PORT_LEN;

    memcpy(&u16, p, sizeof(u16));
    udp->dst = ntohs(u16);
    p += UDP_DST_PORT_LEN;

    memcpy(&u16, p, sizeof(u16));
    udp->payload_length = ntohs(u16) - UDP_HDR_LEN;
    p += UDP_LEN_LEN;
    p += UDP_CHECKSUM_LEN;

    udp->payload = (uint8_t*)malloc(sizeof(uint8_t)*udp->payload_length);
    memcpy(udp->payload, p, udp->payload_length);

    return udp;
}

int udp_destroy(udp *udp)
{
    if(udp == NULL){
        log_err("udp is null.");
        return -1;
    }

    if(udp->payload != NULL){
        free(udp->payload);
    }

    free(udp);

    return 0;
}

uint8_t *udp_get_packet(udp *udp, uint8_t *buffer, uint32_t *length)
{
    if(udp == NULL){
        log_err("udp is null.");
        return NULL;
    }

    *length = UDP_HDR_LEN + udp->payload_length;
    if(buffer == NULL){
        buffer = (uint8_t *)malloc(sizeof(uint8_t)*(*length));
    }

    uint8_t *p;
    uint16_t u16;

    p = buffer;

    memset(p, 0, sizeof(uint8_t)*(*length));

    u16 = htons(udp->src);
    memcpy(p, &u16, sizeof(u16));
    p += UDP_SRC_PORT_LEN;

    u16 = htons(udp->dst);
    memcpy(p, &u16, sizeof(u16));
    p += UDP_DST_PORT_LEN;

    u16 = htons(udp->payload_length + UDP_HDR_LEN);
    memcpy(p, &u16, sizeof(u16));
    p += UDP_LEN_LEN;
    p += UDP_CHECKSUM_LEN;

    memcpy(p, udp->payload, sizeof(uint8_t)*(udp->payload_length));

    return buffer;
}

