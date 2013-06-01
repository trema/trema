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
#include "trx.h"
#include "utils.h"
#include "common.h"
#include "log.h"

int icmp_handle_message(ipv4 *ip)
{
    if(ip == NULL){
        log_err("ip is null.");
        return -1;
    }

    if(ip->protocol != IPV4_PROTOCOL_ICMP){
        log_err("ip protocol is not equal to icmp.");
        return -1;
    }

    uint8_t *p;
    uint8_t type;

    p = ip->payload;

    memcpy(&type, p, sizeof(type));
    p += ICMP_TYPE_LEN;

    switch(type){
    case ICMP_TYPE_ECHO_REQUEST:
        log_debug("icmp echo request received.");
        if(icmp_send_echo_reply(ip) == -2){
            return -1;
        }
        break;
    default:
        log_warn("icmp type %u is not implemented yet.", type);
        break;
    }

    return 0;
}

int icmp_send_echo_reply(ipv4 *ip)
{
    uint8_t *p, *payload;
    uint8_t type, code;
    uint16_t chksum, id, seqnum;
    eth *eth;
    ipv4 *reply;

    log_debug("icmp echo request received.");

    p = ip->payload;

    memcpy(&type, p, sizeof(type));
    p += ICMP_TYPE_LEN;
    memcpy(&code, p, sizeof(code));
    p += ICMP_CODE_LEN;
    memcpy(&chksum, p, sizeof(chksum));
    p += ICMP_CHECKSUM_LEN;
    memcpy(&id, p, sizeof(id));
    p += ICMP_ID_LEN;
    memcpy(&seqnum, p, sizeof(seqnum));

    payload = (uint8_t*)malloc(sizeof(uint8_t)*ip->payload_length);
    p = payload;

    memcpy(p, ip->payload, sizeof(uint8_t)*ip->payload_length);

    type = ICMP_TYPE_ECHO_REPLY;
    memcpy(p, &type, sizeof(type));
    p += ICMP_TYPE_LEN;
    p += ICMP_CODE_LEN;
    chksum = 0;
    memcpy(p, &chksum, sizeof(chksum));

    reply = ipv4_create(ipv4_get_host_addr(), ip->src, IPV4_PROTOCOL_ICMP,
                        payload, ip->payload_length);

    uint8_t macda[ETH_ADDR_LEN];
    uint8_t macsa[ETH_ADDR_LEN];
    uint8_t ip_raw[PKT_BUF_SIZE];

    memset(macda, 0, sizeof(macda));
    memset(macsa, 0, sizeof(macda));
    memset(ip_raw, 0, sizeof(ip_raw));

    int ret = arp_get_mac_by_ip(ip->src, macda);
    if(ret < 0){
        log_debug("cannot get destination mac address.");
        arp_send_request(ip->src);
    }

    uint32_t length = PKT_BUF_SIZE;
    ipv4_get_packet(reply, ip_raw, &length);

    /* update icmp checksum */
    chksum = calc_checksum(ip_raw, reply->hdr_length + reply->payload_length);
    log_debug("new icmp checksum = 0x%04x", chksum);
    memcpy((ip_raw + reply->hdr_length + ICMP_TYPE_LEN + ICMP_CODE_LEN),
           &chksum, sizeof(chksum));

    eth = eth_create(macsa, macda, ETH_TYPE_IPV4, ip_raw, length);

    if(ret < 0){
        trx_txq_arp_waitlist_push(eth, reply->dst);
    }
    else{
        trx_txq_push(eth);
    }

    free(payload);
    ipv4_destroy(reply);
    eth_destroy(eth);

    return 0;
}
