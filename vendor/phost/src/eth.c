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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "eth.h"
#include "log.h"

uint8_t eth_mac_addr_bc[ETH_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

eth *eth_create(uint8_t src[ETH_ADDR_LEN], uint8_t dst[ETH_ADDR_LEN],
               uint16_t type, uint8_t *payload, uint32_t length)
{
    eth *eth = (struct eth*)malloc(sizeof(struct eth));

    memcpy(eth->src, src, ETH_ADDR_LEN);
    memcpy(eth->dst, dst, ETH_ADDR_LEN);
    eth->type = type;
    eth->payload = NULL;
    eth->length = 0;

    if(payload != NULL && length > 0){
        eth->payload = (uint8_t *)malloc(sizeof(uint8_t)*length);
        memcpy(eth->payload, payload, length);
        eth->length = length;
    }

    return eth;
}

eth *eth_create_from_raw(uint8_t *frame, uint32_t length)
{
    eth *eth = (struct eth*)malloc(sizeof(struct eth));

    memcpy(eth->dst, frame, ETH_ADDR_LEN);
    frame += ETH_ADDR_LEN;
    memcpy(eth->src, frame, ETH_ADDR_LEN);
    frame += ETH_ADDR_LEN;
    memcpy(&(eth->type), frame, ETH_TYPE_LEN);
    eth->type = ntohs(eth->type);
    frame += ETH_TYPE_LEN;
    eth->length = length - 2 * ETH_ADDR_LEN - ETH_TYPE_LEN;

    log_debug("eth->length = %u", eth->length);

    if(eth->length > 0){
        eth->payload = (uint8_t *)malloc(sizeof(uint8_t)*(eth->length));
        memcpy(eth->payload, frame, eth->length);
    }
    else{
        eth->payload = NULL;
    }

    return eth;
}

int eth_destroy(eth *eth)
{
    if(eth == NULL){
        return -1;
    }

    if(eth->payload != NULL){
        free(eth->payload);
    }
    free(eth);

    return 0;
}

int eth_set_src(eth *eth, uint8_t *src)
{
    if(eth == NULL){
        return -1;
    }

    memcpy(eth->src, src, ETH_ADDR_LEN);

    return 0;
}

int eth_set_dst(eth *eth, uint8_t *dst)
{
    if(eth == NULL){
        return -1;
    }

    memcpy(eth->dst, dst, ETH_ADDR_LEN);

    return 0;
}

int eth_set_type(eth *eth, uint16_t type)
{
    if(eth == NULL){
        return -1;
    }

    eth->type = type;

    return 0;
}

int eth_set_payload(eth *eth, uint8_t *payload, uint32_t length)
{
    if(eth == NULL){
        return -1;
    }

    if(payload == NULL && length != 0){
        return -1;
    }

    if(payload == NULL && length == 0){
        eth->length = 0;
        free(eth->payload);
        eth->payload = NULL;

        return 0;
    }

    if(eth->payload != NULL){
        free(eth->payload);
    }

    eth->payload = (uint8_t *)malloc(sizeof(uint8_t)*length);
    memcpy(eth->payload, payload, length);
    eth->length = length;

    return 0;
}

int eth_set_payload_nocopy(eth *eth, uint8_t *payload, uint32_t length)
{
    if(eth == NULL){
        return -1;
    }

    if(payload == NULL && length != 0){
        return -1;
    }

    eth->payload = payload;
    eth->length = length;

    return 0;
}

int eth_get_src(eth *eth, uint8_t *src)
{
    if(eth == NULL){
        return -1;
    }

    src = eth->src;

    return 0;
}

int eth_get_dst(eth *eth, uint8_t *dst)
{
    if(eth == NULL){
        return -1;
    }

    dst = eth->dst;

    return 0;
}

int eth_get_type(eth *eth, uint16_t *type)
{
    if(eth == NULL){
        return -1;
    }

    type = &(eth->type);

    return 0;
}

int eth_get_payload(eth *eth, uint8_t *payload, uint32_t *length)
{
    if(eth == NULL){
        return -1;
    }

    payload = eth->payload;
    length = &(eth->length);

    return 0;
}

uint8_t *eth_get_frame(eth *eth, uint8_t *buffer, uint32_t *length)
{
    if(eth == NULL){
        log_err("eth is null.");
        return NULL;
    }

    *length = eth->length + ETH_ADDR_LEN * 2 + ETH_TYPE_LEN;
    if(buffer == NULL){
        buffer = (uint8_t *)malloc(sizeof(uint8_t)*(*length));
    }

    uint8_t *p;
    uint16_t u16;

    p = buffer;

    memset(p, 0, sizeof(uint8_t)*(*length));

    memcpy(p, eth->dst, ETH_ADDR_LEN);
    p += ETH_ADDR_LEN;
    memcpy(p, eth->src, ETH_ADDR_LEN);
    p += ETH_ADDR_LEN;
    u16 = htons(eth->type);
    memcpy(p, &u16, ETH_TYPE_LEN);
    p += ETH_TYPE_LEN;

    memcpy(p, eth->payload, sizeof(uint8_t)*(eth->length));

    return buffer;
}

char *eth_dump(eth *eth, char *dump)
{
    int i;
    char *p = dump;

    if(p == NULL){
        log_err("buffer must be prepared by caller.");
        return NULL;
    }

    sprintf(p, "eth_dst=");
    p += 8;
    for(i=0; i<ETH_ADDR_LEN; i++){
        sprintf(p, "%02x", eth->dst[i]);
        p += 2;
    }
    sprintf(p, ",eth_src=");
    p += 9;
    for(i=0; i<ETH_ADDR_LEN; i++){
        sprintf(p, "%02x", eth->src[i]);
        p += 2;
    }
    sprintf(p, ",eth_type=%04x", eth->type);
    p += 14;
    sprintf(p, ",payload=");
    p += 9;
    for(i=0; i<eth->length; i++){
        sprintf(p, "%02x", eth->payload[i]);
        p += 2;
    }

    return dump;
}
