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

#ifndef _CMDIF_H_
#define _CMDIF_H_

#include <stdint.h>
#include "eth.h"
#include "stats.h"

#define CMDIF_SERVER_SOCK_FILE "/tmp/.cmdif_s"
#define CMDIF_CLIENT_SOCK_FILE "/tmp/.cmdif_c"
#define CMDIF_MSG_LEN 8192

#define CMDIF_SOCK_RMEM 1048576
#define CMDIF_SOCK_WMEM 1048576

#define CMDIF_MIN_USLEEP 100.0

#define CMDIF_SELECT_TIMEOUT 1000 /* in usec */

#define CMDIF_SEND_THREADS_MAX 8

#define CMDIF_SEARCH_MAC_BY_ARP

enum cmdif_cmd {
    CMDIF_CMD_SET_HOST_ADDR = 0,
    CMDIF_CMD_SEND_PACKETS,
    CMDIF_CMD_ADD_ARP_ENTRY,
    CMDIF_CMD_DELETE_ARP_ENTRY,
    CMDIF_CMD_RESET_STATS,
    CMDIF_CMD_SHOW_STATS,
    CMDIF_CMD_SET_PROMISC,
    CMDIF_CMD_MAX
};

enum cmdif_status {
    CMDIF_STATUS_OK = 0,
    CMDIF_STATUS_NG,
    CNDIF_STATUS_MAX
};

typedef struct cmdif_request_hdr {
    uint32_t xid;
    uint8_t cmd;
    uint8_t padding;
    uint16_t length;
} __attribute__ ((packed)) cmdif_request_hdr;

typedef struct cmdif_request_set_host_addr {
    cmdif_request_hdr hdr;
    uint32_t ip_addr;
    uint32_t ip_mask;
    uint8_t mac_addr[ETH_ADDR_LEN];
    uint8_t padding[2];  
} __attribute__ ((packed)) cmdif_request_set_host_addr;

#define CMDIF_CMD_SP_OPTS_INCREMENT_SIP 0x0001
#define CMDIF_CMD_SP_OPTS_INCREMENT_DIP 0x0002
#define CMDIF_CMD_SP_OPTS_INCREMENT_SP  0x0004
#define CMDIF_CMD_SP_OPTS_INCREMENT_DP  0x0008
#define CMDIF_CMD_SP_OPTS_INCREMENT_PL  0x0010
#define CMDIF_CMD_SP_OPTS_NONBLOCKING   0x0020
#define CMDIF_CMD_SP_OPTS_BACKGROUND    0x0040

typedef struct cmdif_request_send_packets {
    cmdif_request_hdr hdr;
    uint16_t cmd_options;
    uint8_t padding[2];
    uint32_t ip_src;
    uint32_t ip_dst;
    uint16_t tp_src;
    uint16_t tp_dst;
    uint32_t inc_ip_src_n;
    uint32_t inc_ip_dst_n;
    uint16_t inc_tp_src_n;
    uint16_t inc_tp_dst_n;
    uint32_t inc_payload_n;
    uint16_t payload_length;
    uint16_t duration;
    uint32_t pps;
    uint32_t count;
} __attribute__ ((packed)) cmdif_request_send_packets;

typedef struct cmdif_request_add_arp_entry {
    cmdif_request_hdr hdr;
    uint32_t ip_addr;
    uint8_t mac_addr[ETH_ADDR_LEN];
    uint8_t padding[2];
} __attribute__ ((packed)) cmdif_request_add_arp_entry;

typedef struct cmdif_request_delete_arp_entry {
    cmdif_request_hdr hdr;
    uint32_t ip_addr;
} __attribute__ ((packed)) cmdif_request_delete_arp_entry;

#define CMDIF_CMD_STATS_OPTS_TX  0x0001
#define CMDIF_CMD_STATS_OPTS_RX  0x0002
#define CMDIF_CMD_STATS_CONTINUE 0x0100

typedef struct cmdif_request_reset_stats {
    cmdif_request_hdr hdr;
    uint16_t cmd_options;
    uint8_t padding[2];
} __attribute__ ((packed)) cmdif_request_reset_stats;

typedef struct cmdif_request_show_stats {
    cmdif_request_hdr hdr;
    uint16_t cmd_options;
    uint8_t padding[2];
} __attribute__ ((packed)) cmdif_request_show_stats;

#define CMDIF_CMD_PROMISC_ENABLE  0x0001
#define CMDIF_CMD_PROMISC_DISABLE 0x0002

typedef struct cmdif_request_promiscuous {
    cmdif_request_hdr hdr;
    uint16_t cmd_options;
} __attribute__ ((packed)) cmdif_request_promiscuous;

typedef struct cmdif_reply_hdr {
    uint32_t xid;
    int8_t status;
    uint8_t padding;
    uint16_t length;
} __attribute__ ((packed)) cmdif_reply_hdr;

typedef struct cmdif_reply_set_host_addr {
    cmdif_reply_hdr hdr;
} __attribute__ ((packed)) cmdif_reply_set_host_addr;

typedef struct cmdif_reply_send_packets {
    cmdif_reply_hdr hdr;
    uint32_t n_pkts;
    uint32_t duration_sec;
    uint32_t duration_usec;
} __attribute__ ((packed)) cmdif_reply_send_packets;

typedef struct cmdif_reply_add_arp_entry {
    cmdif_reply_hdr hdr;
} __attribute__ ((packed)) cmdif_reply_add_arp_entry;

typedef struct cmdif_reply_delete_arp_entry {
    cmdif_reply_hdr hdr;
} __attribute__ ((packed)) cmdif_reply_delete_arp_entry;

typedef struct cmdif_reply_reset_stats {
    cmdif_reply_hdr hdr;
} __attribute__ ((packed)) cmdif_reply_reset_stats;

typedef struct cmdif_tp_stats {
    uint32_t lip;
    uint16_t lport;
    uint8_t padding[2];
    uint32_t rip;
    uint16_t rport;
    uint8_t padding2[2];
    uint32_t n_pkts;
    uint64_t n_octets;
} __attribute__ ((packed)) cmdif_tp_stat;

#define CMDIF_CMD_STATS_REPLY_SIZE 256

typedef struct cmdif_reply_show_stats {
    cmdif_reply_hdr hdr;
    uint16_t cmd_options;
    uint8_t padding[2];
    uint32_t n_stats;
    cmdif_tp_stat st[CMDIF_CMD_STATS_REPLY_SIZE];
} __attribute__ ((packed)) cmdif_reply_show_stats;

typedef struct cmdif_reply_promiscuous {
    cmdif_reply_hdr hdr;
} __attribute__ ((packed)) cmdif_reply_promiscuous;

int cmdif_init(const char *instance, void *send_packet_callback);
int cmdif_close();
int cmdif_set_send_packet_callback(void *callback);
int cmdif_handle_request(uint8_t *request, uint32_t *length);
int cmdif_set_host_addr(cmdif_request_set_host_addr *request);
int cmdif_send_packets(cmdif_request_send_packets *request);
void cmdif_do_send_packets(void *request);
int cmdif_add_arp_entry(cmdif_request_add_arp_entry *request);
int cmdif_delete_arp_entry(cmdif_request_delete_arp_entry *request);
int cmdif_reset_stats(cmdif_request_reset_stats *request);
int cmdif_show_stats(cmdif_request_show_stats *request);
int cmdif_set_promiscuous(cmdif_request_promiscuous *request);
int cmdif_run();
int cmdif_all();
int cmdif_send(void *reply, uint32_t *length);
int cmdif_recv(void *request, uint32_t *length);

#endif /* _CMDIF_H_ */
