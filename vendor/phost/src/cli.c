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

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/param.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "cmdif.h"
#include "cli.h"
#include "utils.h"
#include "common.h"
#include "log.h"

static struct cli_cmds cli_cmds_array[] = {
    /* { .cmd, .cmd_str, .parser_callback } */
    { CLI_CMD_SET_HOST_ADDR, CLI_CMD_SET_HOST_ADDR_STR, cli_parse_set_host_addr },
    { CLI_CMD_SEND_PACKETS, CLI_CMD_SEND_PACKETS_STR, cli_parse_send_packets },
    { CLI_CMD_ADD_ARP_ENTRY, CLI_CMD_ADD_ARP_ENTRY_STR, cli_parse_add_arp_entry },
    { CLI_CMD_DELETE_ARP_ENTRY, CLI_CMD_DELETE_ARP_ENTRY_STR, cli_parse_delete_arp_entry },
    { CLI_CMD_RESET_STATS, CLI_CMD_RESET_STATS_STR, cli_parse_reset_stats },
    { CLI_CMD_SHOW_STATS, CLI_CMD_SHOW_STATS_STR, cli_parse_show_stats },
    { CLI_CMD_ENABLE_PROMISC, CLI_CMD_ENABLE_PROMISC_STR, cli_parse_enable_promiscuous },
    { CLI_CMD_DISABLE_PROMISC, CLI_CMD_DISABLE_PROMISC_STR, cli_parse_disable_promiscuous },
    { UINT8_MAX, NULL, NULL }
};

static struct option cli_cmd_sha_options[] = {
    { CLI_CMD_SHA_IP_ADDR_STR, required_argument, NULL, CLI_CMD_SHA_IP_ADDR },
    { CLI_CMD_SHA_IP_MASK_STR, required_argument, NULL, CLI_CMD_SHA_IP_MASK },
    { CLI_CMD_SHA_MAC_ADDR_STR, required_argument, NULL, CLI_CMD_SHA_MAC_ADDR },
    { 0, 0, 0, 0 }
};

static struct option cli_cmd_sp_options[] = {
    { CLI_CMD_SP_IP_SRC_STR, required_argument, NULL, CLI_CMD_SP_IP_SRC },
    { CLI_CMD_SP_IP_DST_STR, required_argument, NULL, CLI_CMD_SP_IP_DST },
    { CLI_CMD_SP_TP_SRC_STR, required_argument, NULL, CLI_CMD_SP_TP_SRC },
    { CLI_CMD_SP_TP_DST_STR, required_argument, NULL, CLI_CMD_SP_TP_DST },
    { CLI_CMD_SP_DURATION_STR, required_argument, NULL, CLI_CMD_SP_DURATION },
    { CLI_CMD_SP_PPS_STR, required_argument, NULL, CLI_CMD_SP_PPS },
    { CLI_CMD_SP_LENGTH_STR, required_argument, NULL, CLI_CMD_SP_LENGTH },
    { CLI_CMD_SP_INC_IP_SRC_STR, optional_argument, NULL, CLI_CMD_SP_INC_IP_SRC },
    { CLI_CMD_SP_INC_IP_DST_STR, optional_argument, NULL, CLI_CMD_SP_INC_IP_DST },
    { CLI_CMD_SP_INC_TP_SRC_STR, optional_argument, NULL, CLI_CMD_SP_INC_TP_SRC },
    { CLI_CMD_SP_INC_TP_DST_STR, optional_argument, NULL, CLI_CMD_SP_INC_TP_DST },
    { CLI_CMD_SP_INC_PAYLOAD_STR, optional_argument, NULL, CLI_CMD_SP_INC_PAYLOAD },
    { CLI_CMD_SP_NONBLOCK_STR, no_argument, NULL, CLI_CMD_SP_NONBLOCK },
    { CLI_CMD_SP_N_PKTS_STR, required_argument, NULL, CLI_CMD_SP_N_PKTS },
    { CLI_CMD_SP_BACKGROUND_STR, no_argument, NULL, CLI_CMD_SP_BACKGROUND },
    { 0, 0, 0, 0 }
};

static struct option cli_cmd_aae_options[] = {
    { CLI_CMD_AAE_IP_ADDR_STR, required_argument, NULL, CLI_CMD_AAE_IP_ADDR },
    { CLI_CMD_AAE_MAC_ADDR_STR, required_argument, NULL, CLI_CMD_AAE_MAC_ADDR },
    { 0, 0, 0, 0 }
};

static struct option cli_cmd_dae_options[] = {
    { CLI_CMD_DAE_IP_ADDR_STR, required_argument, NULL, CLI_CMD_DAE_IP_ADDR },
    { 0, 0, 0, 0 }
};

static struct option cli_cmd_rs_options[] = {
    { CLI_CMD_RS_TX_STR, no_argument, NULL, CLI_CMD_RS_TX },
    { CLI_CMD_RS_RX_STR, no_argument, NULL, CLI_CMD_RS_RX },
    { 0, 0, 0, 0 }
};

static struct option cli_cmd_ss_options[] = {
    { CLI_CMD_SS_TX_STR, no_argument, NULL, CLI_CMD_SS_TX },
    { CLI_CMD_SS_RX_STR, no_argument, NULL, CLI_CMD_SS_RX },
    { 0, 0, 0, 0 }
};


static int cli_fd = -1;
static char program_name[PATH_MAX];
static char cli_server_sock_file[PATH_MAX];
static char cli_client_sock_file[PATH_MAX];

int main(int argc, char **argv)
{
    char instance[PATH_MAX];
    int ret;

    memset(instance, '\0', sizeof(instance));

    log_init(LOG_WARN, LOG_OUT_STDOUT);

    cli_set_program_name(basename(argv[0]));

    if((argc > 2) && (strcmp(argv[1], "-i") == 0)){
        strncpy(instance, argv[2], PATH_MAX - 1);
        argc -= 2;
        argv += 2;
    }
    else{
        strncpy(instance, CLI_DEFAULT_INSTANCE, PATH_MAX - 1);
    }

    cli_init(instance);

    ret = cli_parse_args(argc, argv);
    if(ret < 0){
        log_err("cannot execute.");
    }

    cli_close();
    log_close();

    return 0;
}

int cli_init(const char *instance)
{
    int ret;
    int flags;
    struct sockaddr_un saddr;

    if(instance == NULL){
        log_err("instance must be specified.");
        return -1;
    }

    memset(cli_server_sock_file, '\0', sizeof(cli_server_sock_file));
    snprintf(cli_server_sock_file, PATH_MAX - 1, "%s.%s",
             CMDIF_SERVER_SOCK_FILE, instance);

    memset(cli_client_sock_file, '\0', sizeof(cli_client_sock_file));
    snprintf(cli_client_sock_file, PATH_MAX - 1, "%s.%s",
             CMDIF_CLIENT_SOCK_FILE, instance);

    if(cli_fd >= 0){
        cli_close();
    }

    cli_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    
    if(cli_fd < 0){
        log_err("cannot create socket.");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, cli_client_sock_file);

    unlink(cli_client_sock_file);

    ret = bind(cli_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret < 0){
        log_err("cannot bind socket.");
        return -1;
    }

    flags = fcntl(cli_fd, F_GETFL);
    ret = fcntl(cli_fd,  F_SETFL, O_NONBLOCK|flags);

    if(ret < 0){
        log_err("cannot turn on non-blocking mode.");
        return -1;
    }

    srand(time(NULL));

    return 0;
}

int cli_close()
{
    if(cli_fd < 0){
        return -1;
    }

    close(cli_fd);
    cli_fd = -1;
    unlink(cli_client_sock_file);

    return 0;
}

int cli_exec_cmd(void *request, uint32_t *request_length,
                 void *reply, uint32_t *reply_length)
{
    int ret;
    struct timeval tv;

    tv.tv_sec = CLI_CMD_REPLY_TIMEOUT/1000;
    tv.tv_usec = (CLI_CMD_REPLY_TIMEOUT - tv.tv_sec * 1000) * 1000;

    ret = cli_send(request, request_length);
    if(ret <= 0){
        log_err("cannot send a request to server.");
        return -1;
    }
    ret = cli_recv(reply, reply_length, tv);
    if(ret <= 0){
        log_err("cannot get a reply from server.");
        return -1;
    }

    if(((cmdif_request_hdr*)request)->xid != ((cmdif_reply_hdr*)reply)->xid){
        log_err("xid mismatch (%u != %u).",
                ((cmdif_request_hdr*)request)->xid,
                ((cmdif_reply_hdr*)reply)->xid);
        return -1;
    }

    return 0;
}

int cli_exec_cmd_with_to(void *request, uint32_t *request_length,
                         void *reply, uint32_t *reply_length,
                         int timeout)
{
    int ret;
    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    ret = cli_send(request, request_length);
    if(ret <= 0){
        log_err("cannot send a request to server.");
        return -1;
    }
    ret = cli_recv(reply, reply_length, tv);
    if(ret <= 0){
        log_err("cannot get a reply from server.");
        return -1;
    }

    if(((cmdif_request_hdr*)request)->xid != ((cmdif_reply_hdr*)reply)->xid){
        log_err("xid mismatch (%u != %u).",
                ((cmdif_request_hdr*)request)->xid,
                ((cmdif_reply_hdr*)reply)->xid);
        return -1;
    }

    return 0;
}

int cli_exec_cmd_more_reply(void *request, uint32_t *request_length,
                            void *reply, uint32_t *reply_length)
{
    int ret;
    struct timeval tv;

    tv.tv_sec = CLI_CMD_REPLY_TIMEOUT/1000;
    tv.tv_usec = (CLI_CMD_REPLY_TIMEOUT - tv.tv_sec * 1000) * 1000;

    ret = cli_recv(reply, reply_length, tv);
    if(ret <= 0){
        log_err("cannot get a reply from server.");
        return -1;
    }

    if(((cmdif_request_hdr*)request)->xid != ((cmdif_reply_hdr*)reply)->xid){
        log_err("xid mismatch.");
        return -1;
    }

    return 0;
}

int cli_send(void *request, uint32_t *length)
{
    if(cli_fd < 0){
        return -1;
    }

    int ret;
    fd_set fdset;
    char buf[256];
    struct timeval tv;
    struct sockaddr_un addr;

    tv.tv_sec = CLI_CMD_REQUEST_TIMEOUT/1000;
    tv.tv_usec = (CLI_CMD_REQUEST_TIMEOUT - tv.tv_sec * 1000) * 1000;

    FD_ZERO(&fdset);
    FD_SET(cli_fd, &fdset);

    ret = select(cli_fd + 1, NULL, &fdset, NULL, &tv);
    if(ret <= 0){
        return ret;
    }

    if(FD_ISSET(cli_fd, &fdset) == 0){
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, cli_server_sock_file);

    ret = sendto(cli_fd, request, *length, 0, (struct sockaddr*)&addr,
                 sizeof(addr));
    if(ret < 0){
        if(ret == EAGAIN){
           // EAGAIN
        }
        else{
            strerror_r(errno, buf, sizeof(buf));
            log_err("cannot send a message: ret = %d, errno = %s (%d)",
                    ret, buf, errno);
        }
        return -1;
    }

    if(ret != sizeof(addr)){
        // warn
    }

    return ret;
}

int cli_recv(void *reply, uint32_t *length, struct timeval timeout)
{
    if(cli_fd < 0){
        return -1;
    }

    int ret;
    fd_set fdset;

    FD_ZERO(&fdset);
    FD_SET(cli_fd, &fdset);

    ret = select(cli_fd + 1, &fdset, NULL, NULL, &timeout);
    if(ret <= 0) {
        return ret;
    }

    if(FD_ISSET(cli_fd, &fdset) == 0){
        return 0;
    }

    ret = read(cli_fd, reply, *length);
    if(ret == -1){
        if(errno == EAGAIN){
            return 0;
        }
        return -1;
    }

    log_debug("ret = %u, length = %u", ret, *length);

    *length = ret;

    return ret;
}

static void cli_print_set_host_addr_usage()
{
    printf("usage: %s set_host_addr [-v] [-h]\n"
           "       [--ip_addr IP_ADDRESS] [--ip_mask NETMASK] [--mac_addr MAC_ADDRESS]\n",
           program_name);
}

static void cli_print_send_packets_usage()
{
    printf("usage: %s send_packets [-v] [-h]\n"
           "       [--ip_src source IP_ADDRESS] [--ip_dst destination IP_ADDRESS]\n"
           "       [--tp_src SOURCE_PORT] [--tp_dst DESTINATION_PORT]\n"
           "       [--duration DURATION] [--pps PPS] [--length PAYLOAD_LENGTH]\n"
           "       [--inc_ip_src] [--inc_ip_dst] [--inc_tp_src] [--inc_tp_dst] [--inc_payload]\n"
           "       [--nonblock]  [--n_pkts NUMBER_OF_PACKETS] [--background]\n",
           program_name);
}

static void cli_print_add_arp_entry_usage()
{
    printf("usage: %s add_arp_entry [-v] [-h] [--ip_addr IP_ADDRESS] [--mac_addr MAC_ADDRESS]\n",
           program_name);
}

static void cli_print_delete_arp_entry_usage()
{
    printf("usage: %s delete_arp_entry [-v] [-h] [--ip_addr IP_ADDRESS]\n",
           program_name);
}

static void cli_print_reset_stats_usage()
{
    printf("usage: %s reset_stats [-v] [-h] [--tx] [--rx]\n",
           program_name);
}

static void cli_print_show_stats_usage()
{
    printf("usage: %s show_stats [-v] [-h] [--tx] [--rx]\n",
           program_name);
}

int cli_print_usage()
{
    printf("usage: %s [-i instance] {set_host_addr|add_arp_entry|"
           "delete_arp_entry|send_packets|reset_stats|show_stats|"
           "enable_promisc|disable_promisc} OPTIONS\n",
           program_name);
    return 0;
}

int cli_parse_args(int argc, char **argv)
{
    if(argc < 2){
        cli_print_usage();
        return -1;
    }

    int i = 0;
    int found = 0;
    
    while(cli_cmds_array[i].cmd != UINT8_MAX){
        if(strncmp(argv[1], cli_cmds_array[i].cmd_str,
                   strlen(cli_cmds_array[i].cmd_str)) == 0){
            found = 1;
            break;
        }
        i++;
    }

    if(!found){
        cli_print_usage();
        return -1;
    }

    argc--;
    argv++;

    return (*cli_cmds_array[i].parser_callback)(argc, argv);
}

int cli_parse_set_host_addr(int argc, char **argv)
{
    int ret = 0;
    int opt;
    int opt_idx = 0;
    uint8_t mac_addr[ETH_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t request_length, reply_length;
    uint32_t ip_addr;
    cmdif_request_set_host_addr request;
    cmdif_reply_set_host_addr reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = htonl(UINT32_MAX * rand());
    request.hdr.cmd = CMDIF_CMD_SET_HOST_ADDR;
    request.hdr.length = htons(sizeof(request));
    memset(&ip_addr, 0, sizeof(ip_addr));

    /*
    request.ip_addr = htonl(0xc0a80001);
    memcpy(&(request.mac_addr), mac_addr, sizeof(mac_addr));
    */

    while(1){
        opt = getopt_long(argc, argv, "hv", cli_cmd_sha_options, &opt_idx);
        if(opt == -1){
            break;
        }

        switch(opt){
        case CLI_CMD_SHA_IP_ADDR:
            if(optarg){
                ret = ipaddrtoul(optarg, &ip_addr);
                if(ret < 0){
                    return -1;
                }
                request.ip_addr = htonl(ip_addr);
                log_debug("ip_addr = 0x%08x", ntohl(request.ip_addr));
            }
            break;
        case CLI_CMD_SHA_IP_MASK:
            if(optarg){
                ret = ipaddrtoul(optarg, &ip_addr);
                if(ret < 0){
                    return -1;
                }
                request.ip_mask = htonl(ip_addr);
                log_debug("ip_mask = 0x%08x", ntohl(request.ip_mask));
            }
            break;
        case CLI_CMD_SHA_MAC_ADDR:
            if(optarg){
                if(strtomac(optarg, mac_addr) == 0){
                    memcpy(request.mac_addr, mac_addr, sizeof(mac_addr));
                }
                else{
                    return -1;
                }
            }
              
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
        default:
            ret = -1;
            break;
        }
    }

    if(ret < 0){
        cli_print_set_host_addr_usage();
        return -1;
    }

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
    }

    log_debug("reply: xid = %u, status = %u",
              ntohl(reply.hdr.xid), reply.hdr.status);
    return 0;
}

int cli_parse_send_packets(int argc, char **argv)
{
    if(argc < 2){
        return -1;
    }

    int opt;
    int ret = 0;
    int opt_idx = 0;
    int to = 0;
    uint32_t request_length, reply_length;
    uint32_t ip_addr;
    cmdif_request_send_packets request;
    cmdif_reply_send_packets reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = UINT32_MAX * rand();
    request.hdr.cmd = CMDIF_CMD_SEND_PACKETS;
    request.hdr.length = htons(sizeof(request));
    memset(&ip_addr, 0, sizeof(ip_addr));

    /*
      TBD: set default values
    request.ip_src = htonl(0xc0a80002);
    request.ip_dst = htonl(0xc0a80001);
    request.tp_src = htons(4096);
    request.tp_dst = htons(8192);
    request.payload_length = htons(128);
    request.duration = htons(4);
    request.pps = htonl(100);
    */

    request.inc_ip_src_n = htonl(-1);
    request.inc_ip_dst_n = htonl(-1);
    request.inc_tp_src_n = htons(-1);
    request.inc_tp_dst_n = htons(-1);
    request.inc_payload_n = htonl(-1);
    request.duration = htons(0);
    request.pps = htonl(0);
    request.count = htonl(0);

    while(1){
        opt = getopt_long(argc, argv, "hv", cli_cmd_sp_options, &opt_idx);
        if(opt == -1){
            break;
        }

        switch(opt){
        case CLI_CMD_SP_IP_SRC:
            if(optarg){
                ret = ipaddrtoul(optarg, &ip_addr);
                if(ret < 0){
                    return -1;
                }
                request.ip_src = htonl(ip_addr);
                log_debug("ip_src = 0x%08x", ntohl(request.ip_src));
            }
            break;
        case CLI_CMD_SP_IP_DST:
            if(optarg){
                ret = ipaddrtoul(optarg, &ip_addr);
                if(ret < 0){
                    return -1;
                }
                request.ip_dst = htonl(ip_addr);
                log_debug("ip_dst = 0x%08x", ntohl(request.ip_dst));
            }
            break;
        case CLI_CMD_SP_TP_SRC:
            if(optarg){
                request.tp_src = htons(strtoul(optarg, NULL, 0));
                log_debug("tp_src = %u", ntohs(request.tp_src));
            }
            break;
        case CLI_CMD_SP_TP_DST:
            if(optarg){
                request.tp_dst = htons(strtoul(optarg, NULL, 0));
                log_debug("tp_dst = %u", ntohs(request.tp_dst));
            }
            break;
        case CLI_CMD_SP_DURATION:
            if(optarg){
                request.duration = htons(strtoul(optarg, NULL, 0));
                log_debug("duration = %u", ntohs(request.duration));
            }
            break;
        case CLI_CMD_SP_PPS:
            if(optarg){
                request.pps = htonl(strtoul(optarg, NULL, 0));
                log_debug("pps = %u", ntohl(request.pps));
            }
            break;
        case CLI_CMD_SP_LENGTH:
            if(optarg){
                request.payload_length = htons(strtoul(optarg, NULL, 0));
                log_debug("length = %u", ntohs(request.payload_length));
            }
            break;
        case CLI_CMD_SP_INC_IP_SRC:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_INCREMENT_SIP;
            if(optarg){
                request.inc_ip_src_n = htonl(strtoul(optarg, NULL, 0));
            }
            log_debug("inc_ip_src_n = %u", ntohl(request.inc_ip_src_n));
            break;
        case CLI_CMD_SP_INC_IP_DST:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_INCREMENT_DIP;
            if(optarg){
                request.inc_ip_dst_n = htonl(strtoul(optarg, NULL, 0));
            }
            log_debug("inc_ip_dst_n = %u", ntohl(request.inc_ip_dst_n));
            break;
        case CLI_CMD_SP_INC_TP_SRC:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_INCREMENT_SP;
            if(optarg){
                request.inc_tp_src_n = htons(strtoul(optarg, NULL, 0));
            }
            log_debug("inc_tp_src_n = %u", ntohs(request.inc_tp_src_n));
            break;
        case CLI_CMD_SP_INC_TP_DST:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_INCREMENT_DP;
            if(optarg){
                request.inc_tp_dst_n = htons(strtoul(optarg, NULL, 0));
            }
            log_debug("inc_tp_dst_n = %u", ntohs(request.inc_tp_dst_n));
            break;
        case CLI_CMD_SP_INC_PAYLOAD:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_INCREMENT_PL;
            if(optarg){
                request.inc_payload_n = htonl(strtoul(optarg, NULL, 0));
            }
            log_debug("inc_payload_n = %u", ntohl(request.inc_payload_n));
            break;
        case CLI_CMD_SP_NONBLOCK:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_NONBLOCKING;
            break;
        case CLI_CMD_SP_N_PKTS:
            if(optarg){
                request.count = htonl(strtoul(optarg, NULL, 0));
                log_debug("n_pkts = %u", ntohl(request.count));
            }
            break;
        case CLI_CMD_SP_BACKGROUND:
            request.cmd_options |= CMDIF_CMD_SP_OPTS_BACKGROUND;
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
        default:
            ret = -1;
            break;
        }
    }

    if(ret < 0){
        cli_print_send_packets_usage();
        return -1;
    }

    request.cmd_options = htons(request.cmd_options);

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    if(ntohl(request.count) > 0){
        to = 2 * (0.5 + ntohl(request.count) / ntohl(request.pps));
    }
    else{
        to = ntohl(request.duration) * 2;
    }

    request_length = sizeof(request);
    reply_length = sizeof(reply);

    ret = cli_exec_cmd_with_to(&request, &request_length,
                               &reply, &reply_length, to);

    if(ret < 0){
        log_err("cannot execute a command.");
        return -1;
    }

    log_debug("reply: xid = %u, status = %u, n_pkts = %u, "
              "duration = %u.%06u", ntohl(reply.hdr.xid),
              reply.hdr.status, ntohl(reply.n_pkts),
              ntohl(reply.duration_sec), ntohl(reply.duration_usec));

    return ret;
}

int cli_parse_add_arp_entry(int argc, char **argv)
{
    int ret = 0;
    int opt;
    int opt_idx = 0;
    uint8_t mac_addr[ETH_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x01 };
    uint32_t request_length, reply_length;
    uint32_t ip_addr;
    cmdif_request_add_arp_entry request;
    cmdif_reply_add_arp_entry reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = htonl(UINT32_MAX * rand());
    request.hdr.cmd = CMDIF_CMD_ADD_ARP_ENTRY;
    request.hdr.length = htons(sizeof(request));
    memset(&ip_addr, 0, sizeof(ip_addr));

    /*
    request.ip_addr = htonl(0xc0a80001);
    memcpy(&(request.mac_addr), mac_addr, sizeof(mac_addr));
    */

    while(1){
        opt = getopt_long(argc, argv, "hv", cli_cmd_aae_options, &opt_idx);
        if(opt == -1){
            break;
        }

        switch(opt){
        case CLI_CMD_AAE_IP_ADDR:
            if(optarg){
                ret = ipaddrtoul(optarg, &ip_addr);
                if(ret < 0){
                    return -1;
                }
                request.ip_addr = htonl(ip_addr);
                log_debug("ip_addr = 0x%08x", ntohl(request.ip_addr));
            }
            break;
        case CLI_CMD_AAE_MAC_ADDR:
            if(optarg){
                if(strtomac(optarg, mac_addr) == 0){
                    memcpy(request.mac_addr, mac_addr, sizeof(mac_addr));
                }
                else{
                    return -1;
                }
            }
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
        default:
            ret = -1;
            break;
        }
    }

    if(ret < 0){
        cli_print_add_arp_entry_usage();
        return -1;
    }

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
    }

    log_debug("reply: xid = %u, status = %u",
              ntohl(reply.hdr.xid), reply.hdr.status);
    return 0;
}

int cli_parse_delete_arp_entry(int argc, char **argv)
{
    int ret = 0;
    int opt;
    int opt_idx = 0;
    uint32_t request_length, reply_length;
    uint32_t ip_addr;
    cmdif_request_delete_arp_entry request;
    cmdif_reply_delete_arp_entry reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = UINT32_MAX * rand();
    request.hdr.cmd = CMDIF_CMD_DELETE_ARP_ENTRY;
    request.hdr.length = htons(sizeof(request));
    memset(&ip_addr, 0, sizeof(ip_addr));

    /*
    request.ip_addr = htonl(0xc0a80001);
    */

    while(1){
        opt = getopt_long(argc, argv, "hv", cli_cmd_dae_options, &opt_idx);
        if(opt == -1){
            break;
        }

        switch(opt){
        case CLI_CMD_DAE_IP_ADDR:
            if(optarg){
                ret = ipaddrtoul(optarg, &ip_addr);
                if(ret < 0){
                    return -1;
                }
                request.ip_addr = htonl(ip_addr);
                log_debug("arg = 0x%08x", ntohl(request.ip_addr));
            }
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
        default:
            ret = -1;
            break;
        }
    }

    if(ret < 0){
        cli_print_delete_arp_entry_usage();
        return -1;
    }

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
    }

    log_debug("reply: xid = %u, status = %u",
              ntohl(reply.hdr.xid), reply.hdr.status);

    return 0;
}

int cli_parse_reset_stats(int argc, char **argv)
{
    int ret = 0;
    int opt;
    int opt_idx = 0;
    uint32_t request_length, reply_length;
    cmdif_request_reset_stats request;
    cmdif_reply_reset_stats reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = htonl(UINT32_MAX * rand());
    request.hdr.cmd = CMDIF_CMD_RESET_STATS;
    request.hdr.length = htons(sizeof(request));

    while(1){
        opt = getopt_long(argc, argv, "hv", cli_cmd_rs_options, &opt_idx);
        if(opt == -1){
            break;
        }

        switch(opt){
        case CLI_CMD_RS_TX:
            request.cmd_options = CMDIF_CMD_STATS_OPTS_TX;
            break;
        case CLI_CMD_RS_RX:
            request.cmd_options = CMDIF_CMD_STATS_OPTS_RX;
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
        default:
            ret = -1;
            break;
        }
    }

    if(ret < 0){
        cli_print_reset_stats_usage();
        return -1;
    }

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request.cmd_options = htons(request.cmd_options);

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
    }

    log_debug("reply: xid = %u, status = %u",
              ntohl(reply.hdr.xid), reply.hdr.status);
    return 0;
}

int cli_parse_show_stats(int argc, char **argv)
{
    int ret = 0;
    int msgs = 0;
    int opt;
    int opt_idx = 0;
    char lip_addr[16], rip_addr[16];
    uint32_t i = 0;
    uint32_t request_length, reply_length;
    cmdif_request_show_stats request;
    cmdif_reply_show_stats reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = htonl(UINT32_MAX * rand());
    request.hdr.cmd = CMDIF_CMD_SHOW_STATS;
    request.hdr.length = htons(sizeof(request));
    memset(&reply, 0, sizeof(reply));
    memset(lip_addr, '\0', sizeof(lip_addr));
    memset(rip_addr, '\0', sizeof(rip_addr));

    while(1){
        opt = getopt_long(argc, argv, "hv", cli_cmd_ss_options, &opt_idx);
        if(opt == -1){
            break;
        }

        switch(opt){
        case CLI_CMD_RS_TX:
            request.cmd_options = CMDIF_CMD_STATS_OPTS_TX;
            break;
        case CLI_CMD_RS_RX:
            request.cmd_options = CMDIF_CMD_STATS_OPTS_RX;
            break;
        case 'v':
            log_set_level(LOG_DEBUG);
            break;
        case 'h':
        default:
            ret = -1;
            break;
        }
    }

    if(ret < 0){
        cli_print_show_stats_usage();
        return -1;
    }

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request.cmd_options = htons(request.cmd_options);

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
        return -1;
    }
    msgs++;

    log_debug("reply: xid = %u, status = %u, options = 0x%04x, n_stats = %u",
              ntohl(reply.hdr.xid), reply.hdr.status, ntohs(reply.cmd_options),
              ntohl(reply.n_stats));

    if(ntohl(reply.n_stats) == 0){
        return 0;
    }

    printf("ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets\n");

    while(reply.hdr.status == CMDIF_STATUS_OK){
        for(i=0; i<ntohl(reply.n_stats); i++){
/*
            printf("%08x,%u,%08x,%u,%u,%llu\n",
                   ntohl(reply.st[i].lip), ntohs(reply.st[i].lport),
                   ntohl(reply.st[i].rip), ntohs(reply.st[i].rport),
                   ntohl(reply.st[i].n_pkts), ntohll(reply.st[i].n_octets));
*/
            printf("%s,%u,%s,%u,%u,%"PRIu64"\n",
                   ultoipaddr(ntohl(reply.st[i].lip), lip_addr),
                   ntohs(reply.st[i].lport),
                   ultoipaddr(ntohl(reply.st[i].rip), rip_addr),
                   ntohs(reply.st[i].rport),
                   ntohl(reply.st[i].n_pkts), ntohll(reply.st[i].n_octets));
        }

        log_debug("cmd_options = %x", ntohs(reply.cmd_options));
        if((ntohs(reply.cmd_options) & CMDIF_CMD_STATS_CONTINUE) == 0){
            break;
        }

        memset(&reply, 0, sizeof(reply));
        ret = cli_exec_cmd_more_reply(&request, &request_length,
                                      &reply, &reply_length);
        if(ret < 0){
            log_err("cannot execute a command.");
            log_err("%u messages received.", msgs);
            return -1;
        }
        
        msgs++;

        log_debug("reply: xid = %u, status = %u, options = 0x%04x, n_stats = %u",
                  ntohl(reply.hdr.xid), reply.hdr.status, ntohs(reply.cmd_options),
                  ntohl(reply.n_stats));
    }

    log_debug("%u messages received.", msgs);

    return 0;
}

int cli_parse_enable_promiscuous(int argc, char **argv)
{
    int ret = 0;
    uint32_t request_length, reply_length;
    cmdif_request_promiscuous request;
    cmdif_reply_promiscuous reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = htonl(UINT32_MAX * rand());
    request.hdr.cmd = CMDIF_CMD_SET_PROMISC;
    request.hdr.length = htons(sizeof(request));
    request.cmd_options = htons(CMDIF_CMD_PROMISC_ENABLE);

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
    }

    log_debug("reply: xid = %u, status = %u",
              ntohl(reply.hdr.xid), reply.hdr.status);

    return 0;
}

int cli_parse_disable_promiscuous(int argc, char **argv)
{
    int ret = 0;
    uint32_t request_length, reply_length;
    cmdif_request_promiscuous request;
    cmdif_reply_promiscuous reply;

    memset(&request, 0, sizeof(request));
    request.hdr.xid = htonl(UINT32_MAX * rand());
    request.hdr.cmd = CMDIF_CMD_SET_PROMISC;
    request.cmd_options = htons(CMDIF_CMD_PROMISC_DISABLE);
    request.hdr.length = htons(sizeof(request));

    log_debug("request: xid = %u", ntohl(request.hdr.xid));

    request_length = sizeof(request);
    reply_length = sizeof(reply);
    ret = cli_exec_cmd(&request, &request_length,
                       &reply, &reply_length);
    if(ret < 0){
        log_err("cannot execute a command.");
    }

    log_debug("reply: xid = %u, status = %u",
              ntohl(reply.hdr.xid), reply.hdr.status);

    return 0;
}

int cli_set_program_name(const char *name)
{
    memset(program_name, '\0', sizeof(program_name));
    strncpy(program_name, name, PATH_MAX - 1);

    return 0;
}
