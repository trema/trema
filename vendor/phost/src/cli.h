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

#ifndef _CLI_H_
#define _CLI_H_

#include "cmdif.h"

#define CLI_CMD_REQUEST_TIMEOUT 10000  /* in msec */
#define CLI_CMD_REPLY_TIMEOUT   10000  /* in msec */

#define CLI_SOCK_RMEM 1048576 /* 1MB */
#define CLI_SOCK_WMEM 1048576 /* 1MB */

#define CLI_DEFAULT_INSTANCE "tap0"

enum cli_cmds_idx {
    CLI_CMD_SET_HOST_ADDR = 0,
    CLI_CMD_SEND_PACKETS,
    CLI_CMD_ADD_ARP_ENTRY,
    CLI_CMD_DELETE_ARP_ENTRY,
    CLI_CMD_RESET_STATS,
    CLI_CMD_SHOW_STATS,
    CLI_CMD_ENABLE_PROMISC,
    CLI_CMD_DISABLE_PROMISC,
    CLI_CMD_MAX
};

#define CLI_CMD_SET_HOST_ADDR_STR        "set_host_addr"
#define CLI_CMD_SEND_PACKETS_STR         "send_packets"
#define CLI_CMD_ADD_ARP_ENTRY_STR        "add_arp_entry"
#define CLI_CMD_DELETE_ARP_ENTRY_STR     "delete_arp_entry"
#define CLI_CMD_RESET_STATS_STR          "reset_stats"
#define CLI_CMD_SHOW_STATS_STR           "show_stats"
#define CLI_CMD_ENABLE_PROMISC_STR   "enable_promisc"
#define CLI_CMD_DISABLE_PROMISC_STR  "disable_promisc"

enum cli_cmd_sha_opts {
    CLI_CMD_SHA_IP_ADDR = 0,
    CLI_CMD_SHA_IP_MASK,
    CLI_CMD_SHA_MAC_ADDR,
    CLI_CMD_SHA_MAX
};

#define CLI_CMD_SHA_IP_ADDR_STR  "ip_addr"
#define CLI_CMD_SHA_IP_MASK_STR  "ip_mask"
#define CLI_CMD_SHA_MAC_ADDR_STR "mac_addr"

enum cli_cmd_sp_opts {
    CLI_CMD_SP_IP_SRC = 0,
    CLI_CMD_SP_IP_DST,
    CLI_CMD_SP_TP_SRC,
    CLI_CMD_SP_TP_DST,
    CLI_CMD_SP_DURATION,
    CLI_CMD_SP_PPS,
    CLI_CMD_SP_LENGTH,
    CLI_CMD_SP_INC_IP_SRC,
    CLI_CMD_SP_INC_IP_DST,
    CLI_CMD_SP_INC_TP_SRC,
    CLI_CMD_SP_INC_TP_DST,
    CLI_CMD_SP_INC_PAYLOAD,
    CLI_CMD_SP_NONBLOCK,
    CLI_CMD_SP_N_PKTS,
    CLI_CMD_SP_BACKGROUND,
    CLI_CMD_SP_MAX
};

#define CLI_CMD_SP_IP_SRC_STR      "ip_src"
#define CLI_CMD_SP_IP_DST_STR      "ip_dst"
#define CLI_CMD_SP_TP_SRC_STR      "tp_src"
#define CLI_CMD_SP_TP_DST_STR      "tp_dst"
#define CLI_CMD_SP_DURATION_STR    "duration"
#define CLI_CMD_SP_PPS_STR         "pps"
#define CLI_CMD_SP_LENGTH_STR      "length"
#define CLI_CMD_SP_INC_IP_SRC_STR  "inc_ip_src"
#define CLI_CMD_SP_INC_IP_DST_STR  "inc_ip_dst"
#define CLI_CMD_SP_INC_TP_SRC_STR  "inc_tp_src"
#define CLI_CMD_SP_INC_TP_DST_STR  "inc_tp_dst"
#define CLI_CMD_SP_INC_PAYLOAD_STR "inc_payload"
#define CLI_CMD_SP_NONBLOCK_STR    "nonblock"
#define CLI_CMD_SP_N_PKTS_STR      "n_pkts"
#define CLI_CMD_SP_BACKGROUND_STR  "background"

enum cli_cmd_aae_opts {
    CLI_CMD_AAE_IP_ADDR = 0,
    CLI_CMD_AAE_MAC_ADDR,
    CLI_CMD_AAE_MAX
};

#define CLI_CMD_AAE_IP_ADDR_STR  "ip_addr"
#define CLI_CMD_AAE_MAC_ADDR_STR "mac_addr"

enum cli_cmd_dae_opts {
    CLI_CMD_DAE_IP_ADDR = 0,
    CLI_CMD_DAE_MAX
};

#define CLI_CMD_DAE_IP_ADDR_STR  "ip_addr"

enum cli_cmd_rs_opts {
    CLI_CMD_RS_TX = 0,
    CLI_CMD_RS_RX,
    CLI_CMD_RS_MAX
};

#define CLI_CMD_RS_TX_STR  "tx"
#define CLI_CMD_RS_RX_STR  "rx"

enum cli_cmd_ss_opts {
    CLI_CMD_SS_TX = 0,
    CLI_CMD_SS_RX,
    CLI_CMD_SS_MAX
};

#define CLI_CMD_SS_TX_STR  "tx"
#define CLI_CMD_SS_RX_STR  "rx"

typedef struct cli_cmds {
    uint8_t cmd;
    const char *cmd_str;
    int (*parser_callback)(int, char**);
} cli_cmds;

int cli_init(const char *instance);
int cli_close();
int cli_exec_cmd(void *request, uint32_t *request_length,
                 void *reply, uint32_t *reply_length);
int cli_exec_cmd_with_to(void *request, uint32_t *request_length,
                         void *reply, uint32_t *reply_length,
                         int timeout_sec);
int cli_exec_cmd_more_reply(void *request, uint32_t *request_length,
                            void *reply, uint32_t *reply_length);
int cli_send(void *request, uint32_t *length);
int cli_recv(void *reply, uint32_t *length, struct timeval timeout);
int cli_parse_args(int argc, char **argv);
int cli_parse_set_host_addr(int argc, char **argv);
int cli_parse_send_packets(int argc, char **argv);
int cli_parse_add_arp_entry(int argc, char **argv);
int cli_parse_delete_arp_entry(int argc, char **argv);
int cli_parse_reset_stats(int argc, char **argv);
int cli_parse_show_stats(int argc, char **argv);
int cli_parse_enable_promiscuous(int argc, char **argv);
int cli_parse_disable_promiscuous(int argc, char **argv);
int cli_set_program_name(const char *name);
int cli_print_usage();

#endif /* _CLI_H_ */
