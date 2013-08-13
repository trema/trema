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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include "tap.h"
#include "eth.h"
#include "arp.h"
#include "ipv4.h"
#include "udp.h"
#include "trx.h"
#include "stats.h"
#include "cmdif.h"
#include "phost.h"
#include "utils.h"
#include "common.h"
#include "log.h"

static int cmdif_fd = -1;
static void (*cmdif_send_packet_callback)(ipv4*, udp*) = NULL;
static char cmdif_server_sock_file[PATH_MAX];
static char cmdif_client_sock_file[PATH_MAX];
static pthread_mutex_t cmdif_send_mutex;
static int n_send_threads;

int cmdif_init(const char *instance, void *send_packet_callback)
{
    int ret;
    int flags;
    int val;
    struct sockaddr_un saddr;

    if(instance == NULL){
        log_err("instance must be specified.");
        return -1;
    }

    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&cmdif_send_mutex, &mutexattr);

    n_send_threads = 0;

    memset(cmdif_server_sock_file, '\0', sizeof(cmdif_server_sock_file));
    snprintf(cmdif_server_sock_file, PATH_MAX - 1, "%s.%s",
             CMDIF_SERVER_SOCK_FILE, instance);

    memset(cmdif_client_sock_file, '\0', sizeof(cmdif_client_sock_file));
    snprintf(cmdif_client_sock_file, PATH_MAX - 1, "%s.%s",
             CMDIF_CLIENT_SOCK_FILE, instance);

    if(cmdif_fd >= 0){
        cmdif_close();
    }

    cmdif_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    
    if(cmdif_fd < 0){
        log_err("cannot create socket.");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, cmdif_server_sock_file);

    unlink(cmdif_server_sock_file);

    ret = bind(cmdif_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret < 0){
        log_err("cannot bind socket.");
        return -1;
    }

    val = CMDIF_SOCK_RMEM;
    ret = setsockopt(cmdif_fd, SOL_SOCKET, SO_RCVBUFFORCE, &val, sizeof(val));
    if(ret < 0){
        log_err("cannot set rmem to %d.", val);
        return -1;
    }

    val = CMDIF_SOCK_WMEM;
    ret = setsockopt(cmdif_fd, SOL_SOCKET, SO_SNDBUFFORCE, &val, sizeof(val));
    if(ret < 0){
        log_err("cannot set wmem to %d.", val);
        return -1;
    }

    flags = fcntl(cmdif_fd, F_GETFL);
    ret = fcntl(cmdif_fd,  F_SETFL, O_NONBLOCK|flags);

    if(ret < 0){
        log_err("cannot turn on non-blocking mode.");
        return -1;
    }

    cmdif_set_send_packet_callback(send_packet_callback);

    return 0;
}

int cmdif_close()
{
    if(cmdif_fd < 0){
        return -1;
    }

    close(cmdif_fd);
    cmdif_fd = -1;
    unlink(cmdif_server_sock_file);
    n_send_threads = 0;

    return 0;
}

int cmdif_set_send_packet_callback(void *callback)
{
    cmdif_send_packet_callback = callback;

    return 0;
}

int cmdif_handle_request(uint8_t *request, uint32_t *length)
{
    if(*length < sizeof(cmdif_request_hdr)){
        log_err("too short request message.");
        return -1;
    }

    uint8_t cmd;
    memcpy(&cmd, request + sizeof(uint32_t), sizeof(cmd));

    switch(cmd){
    case CMDIF_CMD_SET_HOST_ADDR:
        cmdif_set_host_addr((struct cmdif_request_set_host_addr *)request);
        break;
    case CMDIF_CMD_SEND_PACKETS:
        cmdif_send_packets((struct cmdif_request_send_packets *)request);
        break;
    case CMDIF_CMD_ADD_ARP_ENTRY:
        cmdif_add_arp_entry((struct cmdif_request_add_arp_entry *)request);
        break;
    case CMDIF_CMD_DELETE_ARP_ENTRY:
        cmdif_delete_arp_entry((struct cmdif_request_delete_arp_entry *)request);
        break;
    case CMDIF_CMD_RESET_STATS:
        cmdif_reset_stats((struct cmdif_request_reset_stats *)request);
        break;
    case CMDIF_CMD_SHOW_STATS:
        cmdif_show_stats((struct cmdif_request_show_stats *)request);
        break;
    case CMDIF_CMD_SET_PROMISC:
        cmdif_set_promiscuous((struct cmdif_request_promiscuous *)request);
        break;
    default:
        log_warn("not implemented (cmd = %u)", cmd);
        break;
    }

    return 0;
}

int cmdif_set_host_addr(cmdif_request_set_host_addr *request)
{
    int ret;
    char mac_addr[ETH_ADDR_LEN*2 + 1];
    uint32_t length;
    uint32_t ip_addr, ip_mask;
    cmdif_reply_set_host_addr reply;

    memset(mac_addr, 0, sizeof(mac_addr));
    hexdump(request->mac_addr, ETH_ADDR_LEN, mac_addr);

    ip_addr = ntohl(request->ip_addr);
    ip_mask = ntohl(request->ip_mask);

    log_debug("set host address: ip_addr = 0x%08x/0x%08x, mac_addr = %s",
              ip_addr, ip_mask, mac_addr);

    ret = arp_init(request->mac_addr, ip_addr);
    ret |= ipv4_init(ip_addr, ip_mask);

    reply.hdr.xid = request->hdr.xid;
    if(ret == 0){
        reply.hdr.status = CMDIF_STATUS_OK;
    }
    else{
        reply.hdr.status = CMDIF_STATUS_NG;
    }
    length = sizeof(reply);
    cmdif_send((void*)&reply, &length);

    return 0;
}

int cmdif_send_packets(cmdif_request_send_packets *request)
{
    int ret = -1;
    size_t length;
    pthread_t thread;
    pthread_attr_t attr;
    cmdif_request_send_packets *req;
    cmdif_reply_send_packets reply;

    if(n_send_threads < CMDIF_SEND_THREADS_MAX){
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        req = (cmdif_request_send_packets*)malloc(sizeof(cmdif_request_send_packets));
        memcpy(req, request, sizeof(cmdif_request_send_packets));

        ret = pthread_create(&thread, &attr,
                             (void*)cmdif_do_send_packets,
                             (void*)req);

        if(ret != 0){
            log_err("Failed to create do_send_packets thread");
            ret = -1;
        }
    }

    if(ret < 0){
        memset(&reply, 0, sizeof(reply));
        reply.hdr.xid = request->hdr.xid;
        reply.hdr.status = CMDIF_STATUS_NG;
        reply.n_pkts = 0;
        reply.duration_sec = 0;
        reply.duration_usec = 0;
        length = sizeof(reply);
        cmdif_send((void*)&reply, (uint32_t*)&length);
    }

    return ret;
}

void cmdif_do_send_packets(void *req)
{
    int update_ip;
    uint8_t *payload;
    uint8_t udp_buf[PKT_BUF_SIZE];
    uint8_t ip_buf[PKT_BUF_SIZE];
    uint8_t macsa[ETH_ADDR_LEN];
    uint8_t macda[ETH_ADDR_LEN];
    uint16_t u16;
    uint32_t u32;
    uint32_t udp_len;
    uint32_t ip_len;
    uint32_t count;
    uint32_t count_sip, count_dip;
    uint32_t count_pl;
    uint16_t count_sp, count_dp;
    uint32_t length;
    uint32_t duration_sec, duration_usec;
    struct timeval tv;
    eth *eth;
    ipv4 *ip;
    udp *udp;
    cmdif_request_send_packets *request;
    cmdif_reply_send_packets reply;

    request = (struct cmdif_request_send_packets *)req;

    n_send_threads++;

    memset(macsa, 0, sizeof(macsa));
    memset(macda, 0, sizeof(macda));
    memset(&tv, 0, sizeof(tv));
    count_sip = 0;
    count_dip = 0;
    count_sp = 0;
    count_dp = 0;
    count_pl = 0;

    request->cmd_options = ntohs(request->cmd_options);
    request->ip_src = ntohl(request->ip_src);
    request->ip_dst = ntohl(request->ip_dst);
    request->tp_src = ntohs(request->tp_src);
    request->tp_dst = ntohs(request->tp_dst);
    request->inc_ip_src_n = ntohl(request->inc_ip_src_n);
    request->inc_ip_dst_n = ntohl(request->inc_ip_dst_n);
    request->inc_tp_src_n = ntohs(request->inc_tp_src_n);
    request->inc_tp_dst_n = ntohs(request->inc_tp_dst_n);
    request->inc_payload_n = ntohl(request->inc_payload_n);
    request->payload_length = ntohs(request->payload_length);
    request->duration = ntohs(request->duration);
    request->pps = ntohl(request->pps);
    request->count = ntohl(request->count);

    log_debug("sending udp packets: 0x%08x:%u -> 0x%08x:%u, "
              "length = %u, pps = %u, duration = %u, count = %u, "
              "inc_ip_src_n = %u, inc_ip_dst_n = %u, "
              "inc_tp_src_n = %u, inc_tp_dst_n = %u, "
              "inc_payload_n = %u",
              request->ip_src, request->tp_src,
              request->ip_dst, request->tp_dst,
              request->payload_length, request->pps,
              request->duration, request->count,
              request->inc_ip_src_n, request->inc_ip_dst_n,
              request->inc_tp_src_n, request->inc_tp_dst_n,
              request->inc_payload_n);

    if(request->cmd_options & CMDIF_CMD_SP_OPTS_BACKGROUND){
        /* send a reply immediately not to block CLI */
        memset(&reply, 0, sizeof(reply));
        reply.hdr.xid = request->hdr.xid;
        reply.hdr.status = CMDIF_STATUS_OK;
        reply.n_pkts = 0;
        reply.duration_sec = 0;
        reply.duration_usec = 0;
        length = sizeof(reply);
        cmdif_send((void*)&reply, &length);
    }

    payload = (uint8_t*)malloc(sizeof(uint8_t)*(request->payload_length));
    memset(payload, 0x00, sizeof(uint8_t)*(request->payload_length));

    udp = udp_create(request->tp_src, request->tp_dst,
                     payload, request->payload_length);
    udp_len = sizeof(udp_buf);
    udp_get_packet(udp, udp_buf, &udp_len);

    ip = ipv4_create(request->ip_src, request->ip_dst, IPV4_PROTOCOL_UDP,
                     udp_buf, udp_len);
    ip_len = sizeof(ip_buf);
    ipv4_get_packet(ip, ip_buf, &ip_len);

    arp_get_mac_by_ip(request->ip_src, macsa);

#ifndef CMDIF_SEARCH_MAC_BY_ARP
    arp_get_mac_by_ip(request->ip_dst, macda);
#else
    int arp_needed = arp_get_mac_by_ip(request->ip_dst, macda);
    if(arp_needed < 0){
        arp_send_request(request->ip_dst);
    }
#endif
    eth = eth_create(macsa, macda, ETH_TYPE_IPV4, ip_buf, ip_len);

    update_ip = 0;

    gettimeofday(&tv, NULL);
    time_t start_sec = tv.tv_sec;
    long double now;
    long double next;
    long double start = (long double)tv.tv_usec/1000000;
    long double sleep_usec;

    uint32_t n_pkts;

    if(request->count > 0){
        n_pkts = request->count;
    }
    else{
        n_pkts = request->duration * request->pps;
    }

    for(count=0; count<n_pkts; count++){
        //log_debug("sending packet: count = %u", count);
        if(cmdif_send_packet_callback != NULL){
            (*cmdif_send_packet_callback)(ip, udp);
        }
#ifndef CMDIF_SEARCH_MAC_BY_ARP
        trx_tx_immediately(eth);
#else
        if(arp_needed < 0){
            trx_txq_arp_waitlist_push(eth, ip->dst);
        }
        else{
            trx_tx_immediately(eth);
        }
        trx_tx_arp_waitlist();
#endif
/*
        gettimeofday(&tv, NULL);
        log_debug("sending udp packet: 0x%08x:%u -> 0x%08x:%u, "
                  "time = %u.%06u", ip->src, udp->src,
                  ip->dst, udp->dst, tv.tv_sec, tv.tv_usec);
*/
#if 0
        if(request->cmd_options & CMDIF_CMD_SP_OPTS_NONBLOCKING){
            phost_run();
        }
#endif

        if(request->cmd_options){
            if(request->cmd_options & CMDIF_CMD_SP_OPTS_INCREMENT_SIP){
                if(count_sip == request->inc_ip_src_n){
                    ip->src = request->ip_src;
                    count_sip = 0;
                }
                else{
                    ip->src++;
                    count_sip++;
                }
                arp_get_mac_by_ip(ip->src, macsa);
                u32 = htonl(ip->src);
                memcpy(eth->payload + 12, &u32, sizeof(u32));
                update_ip = 1;
            }
            if(request->cmd_options & CMDIF_CMD_SP_OPTS_INCREMENT_DIP){
                if(count_dip == request->inc_ip_src_n){
                    ip->dst = request->ip_dst;
                    count_dip = 0;
                }
                else{
                    ip->dst++;
                    count_dip++;
                }
                u32 = htonl(ip->dst);
                memcpy(eth->payload + 16, &u32, sizeof(u32));
                update_ip = 1;

#ifndef CMDIF_SEARCH_MAC_BY_ARP
                arp_get_mac_by_ip(ip->dst, macda);
#else
                arp_needed = arp_get_mac_by_ip(request->ip_dst, macda);
                if(arp_needed < 0){
                    arp_send_request(request->ip_dst);
                }
#endif
            }
            if(request->cmd_options & CMDIF_CMD_SP_OPTS_INCREMENT_SP){
                if(count_sp == request->inc_tp_src_n){
                    udp->src = request->tp_src;
                    count_sp = 0;
                }
                else{
                    udp->src++;
                    count_sp++;
                }
                u16 = htons(udp->src);
                memcpy(eth->payload + 20, &u16, sizeof(u16));
            }
            if(request->cmd_options & CMDIF_CMD_SP_OPTS_INCREMENT_DP){
                if(count_dp == request->inc_tp_dst_n){
                    udp->dst = request->tp_dst;
                    count_dp = 0;
                }
                else{
                    udp->dst++;
                    count_dp++;
                }
                u16 = htons(udp->dst);
                memcpy(eth->payload + 22, &u16, sizeof(u16));
            }
            if(request->cmd_options & CMDIF_CMD_SP_OPTS_INCREMENT_PL){
                if(count_pl == request->inc_payload_n){
                    count_pl = 0;
                }
                else{
                    count_pl++;
                }
                u32 = htonl(count_pl);
                memcpy(eth->payload + 28, &u32, sizeof(u32));
            }
            if(update_ip){
                memset(eth->payload + 10, 0, sizeof(uint16_t));
                u16 = calc_checksum((void*)eth->payload, IPV4_DEFAULT_HLEN * 4);
                memcpy(eth->payload + 10, &u16, sizeof(u16));
            }
        }
        gettimeofday(&tv, NULL);
        now = (long double)(tv.tv_sec - start_sec) +
            (long double)tv.tv_usec/1000000;
        next = start + (long double)(count+1)/request->pps;
        sleep_usec = 1000000.0*(next - now);
        if((sleep_usec > CMDIF_MIN_USLEEP) &&
           ((count+1) < n_pkts)){
            /*
            log_debug("sleeping: %u usec", (useconds_t)sleep_usec);
            */
            usleep((useconds_t)sleep_usec);
        }
    }
    gettimeofday(&tv, NULL);
    now = ((long double)(tv.tv_sec - start_sec) +
           (long double)tv.tv_usec/1000000) - (long double)start;

    duration_sec = (uint32_t)now;
    duration_usec = (uint32_t)(((long double)now -
                                (long double)duration_sec) * 1000000);

    log_debug("sent %u packets", count);
    log_debug("duration %u.%06u", duration_sec, duration_usec);

    free(payload);
    udp_destroy(udp);
    ipv4_destroy(ip);
    eth_destroy(eth);

    if(!(request->cmd_options & CMDIF_CMD_SP_OPTS_BACKGROUND)){
        memset(&reply, 0, sizeof(reply));
        reply.hdr.xid = request->hdr.xid;
        reply.hdr.status = CMDIF_STATUS_OK;
        reply.n_pkts = htonl(count);
        reply.duration_sec = htonl(duration_sec);
        reply.duration_usec = htonl(duration_usec);
        length = sizeof(reply);
        cmdif_send((void*)&reply, &length);
    }

    n_send_threads--;

    if(req){
        free(req);
    }
}

int cmdif_add_arp_entry(cmdif_request_add_arp_entry *request)
{
    int ret;
    char mac_addr[ETH_ADDR_LEN*2 + 1];
    uint32_t length;
    cmdif_reply_add_arp_entry reply;

    memset(mac_addr, 0, sizeof(mac_addr));
    hexdump(request->mac_addr, ETH_ADDR_LEN, mac_addr);

    log_debug("adding static arp entry: ip_addr = 0x%08x, mac_addr = %s",
              ntohl(request->ip_addr), mac_addr);

    ret = arp_update_entry(ntohl(request->ip_addr),
                           request->mac_addr, UINT32_MAX);

    reply.hdr.xid = request->hdr.xid;
    if(ret == 0){
        reply.hdr.status = CMDIF_STATUS_OK;
    }
    else{
        reply.hdr.status = CMDIF_STATUS_NG;
    }
    length = sizeof(reply);
    cmdif_send((void*)&reply, &length);

    return 0;
}

int cmdif_delete_arp_entry(cmdif_request_delete_arp_entry *request)
{
    int ret;
    uint32_t length;
    cmdif_reply_delete_arp_entry reply;

    ret = arp_delete_entry(ntohl(request->ip_addr));

    reply.hdr.xid = request->hdr.xid;
    if(ret == 0){
        reply.hdr.status = CMDIF_STATUS_OK;
    }
    else{
        reply.hdr.status = CMDIF_STATUS_NG;
    }
    length = sizeof(reply);
    cmdif_send((void*)&reply, &length);

    return 0;
}

int cmdif_reset_stats(cmdif_request_reset_stats *request)
{
    int ret;
    uint32_t length;
    cmdif_reply_reset_stats reply;

    request->cmd_options = ntohs(request->cmd_options);

    switch(request->cmd_options){
    case CMDIF_CMD_STATS_OPTS_TX:
        ret = stats_udp_send_uninit();
        break;
    case CMDIF_CMD_STATS_OPTS_RX:
        ret = stats_udp_recv_uninit();
        break;
    default:
        log_err("unknown stats");
        ret = -1;
    }

    reply.hdr.xid = request->hdr.xid;
    if(ret == 0){
        reply.hdr.status = CMDIF_STATUS_OK;
    }
    else{
        reply.hdr.status = CMDIF_STATUS_NG;
    }
    length = sizeof(reply);
    cmdif_send((void*)&reply, &length);

    return 0;
}

int cmdif_show_stats(cmdif_request_show_stats *request)
{
    int i;
    int ret;
    int msgs = 0;
    uint32_t count = 0;
    uint32_t length = 0;
    uint32_t size;
    cmdif_reply_show_stats reply;
    tp_stats *st = NULL;
    tp_stats *p;

    request->cmd_options = ntohs(request->cmd_options);

    switch(request->cmd_options){
    case CMDIF_CMD_STATS_OPTS_TX:
        st = stats_udp_send_get(&size);
        break;
    case CMDIF_CMD_STATS_OPTS_RX:
        st = stats_udp_recv_get(&size);
        break;
    default:
        log_err("unknown stats");
    }

    memset(&reply, 0, sizeof(reply));
    reply.hdr.xid = request->hdr.xid;
    if(st == NULL || size == 0){
        reply.hdr.status = CMDIF_STATUS_NG;
        reply.hdr.length = htons(sizeof(reply));
        length = sizeof(reply);
        cmdif_send((void*)&reply, &length);
        return 0;
    }

    reply.hdr.status = CMDIF_STATUS_OK;
    length = sizeof(cmdif_reply_show_stats);
    reply.hdr.length = htons(length);

    count = 0;
    for(i=0; i<size; i++){
        p = &(st[i]);
        reply.st[count].lip = htonl(p->lip);
        reply.st[count].lport = htons(p->lport);
        reply.st[count].rip = htonl(p->rip);
        reply.st[count].rport = htons(p->rport);
        reply.st[count].n_pkts = htonl(p->n_pkts);
        reply.st[count].n_octets = htonll(p->n_octets);
        count++;
        if(count == CMDIF_CMD_STATS_REPLY_SIZE){
            reply.cmd_options = htons(CMDIF_CMD_STATS_CONTINUE);
            reply.n_stats = htonl(count);
            ret = cmdif_send((void*)&reply, &length);
            if(ret < 0){
                log_err("cannot send a reply to clinet.");
                return -1;
            }
            count = 0;
            msgs++;
            usleep(10000); /* FIXME: fake flow control */
        }
    }

    reply.n_stats = htonl(count);
    reply.cmd_options = htons(0);
    ret = cmdif_send((void*)&reply, &length);
    if(ret < 0){
        log_err("cannot send a reply to clinet.");
        if(st){
            free(st);
        }
        return -1;
    }
    msgs++;

    log_debug("sent %u messages", msgs);

    if(st){
        free(st);
    }

    return 0;
}

int cmdif_set_promiscuous(cmdif_request_promiscuous *request)
{
    int ret = 0;
    uint32_t length = 0;
    cmdif_reply_promiscuous reply;

    request->cmd_options = ntohs(request->cmd_options);

    switch(request->cmd_options){
    case CMDIF_CMD_PROMISC_ENABLE:
        ret = ipv4_enable_promiscuous();
        ret |= phost_enable_promiscuous();
        break;
    case CMDIF_CMD_PROMISC_DISABLE:
        ret = ipv4_disable_promiscuous();
        ret |= phost_disable_promiscuous();
        break;
    default:
        log_err("unknown operation.");
    }

    reply.hdr.xid = request->hdr.xid;

    if(ret < 0){
        reply.hdr.status = CMDIF_STATUS_NG;
    }
    else{
        reply.hdr.status = CMDIF_STATUS_OK;
    }

    reply.hdr.length = htons(sizeof(reply));
    length = sizeof(reply);
    cmdif_send((void*)&reply, &length);

    return 0;
}

int cmdif_run()
{
    return cmdif_all();
}

int cmdif_all()
{
    int ret;
    uint8_t request[CMDIF_MSG_LEN];
    uint32_t length = CMDIF_MSG_LEN;

    memset(&request, 0, sizeof(request));
    
    ret = cmdif_recv(request, &length);
    if(ret > 0){
        cmdif_handle_request(request, &length);
    }

    return 0;
}

int cmdif_send(void *reply, uint32_t *length)
{
    if(cmdif_fd < 0){
        return -1;
    }

    int ret;
    fd_set fdset;
    struct timeval tv;
    struct sockaddr_un addr;

    tv.tv_sec = CMDIF_SELECT_TIMEOUT/1000000;
    tv.tv_usec = CMDIF_SELECT_TIMEOUT - tv.tv_sec * 1000000;

    pthread_mutex_lock(&cmdif_send_mutex);

    FD_ZERO(&fdset);
    FD_SET(cmdif_fd, &fdset);

    ret = select(cmdif_fd + 1, NULL, &fdset, NULL, &tv);
    if(ret <= 0){
        pthread_mutex_unlock(&cmdif_send_mutex);
        return ret;
    }

    if(FD_ISSET(cmdif_fd, &fdset) == 0){
        pthread_mutex_unlock(&cmdif_send_mutex);
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, cmdif_client_sock_file);

    ret = sendto(cmdif_fd, reply, *length, 0, (struct sockaddr*)&addr,
                 sizeof(addr));
    if(ret < 0){
        if(ret == EAGAIN){
           // EAGAIN
        }
        pthread_mutex_unlock(&cmdif_send_mutex);
        return -1;
    }

    if(ret != *length){
        // warn
        log_warn("only a part of message is sent (ret = %u, msg size = %u).",
                 ret, *length);
    }

    pthread_mutex_unlock(&cmdif_send_mutex);

    return ret;
}

int cmdif_recv(void *request, uint32_t *length)
{
    if(cmdif_fd < 0){
        return -1;
    }

    int ret;
    fd_set fdset;
    struct timeval tv;

    tv.tv_sec = CMDIF_SELECT_TIMEOUT/1000000;
    tv.tv_usec = CMDIF_SELECT_TIMEOUT - tv.tv_sec * 1000000;

    FD_ZERO(&fdset);
    FD_SET(cmdif_fd, &fdset);

    ret = select(cmdif_fd + 1, &fdset, NULL, NULL, &tv);
    if(ret <= 0) {
        return ret;
    }

    if(FD_ISSET(cmdif_fd, &fdset) == 0){
        return 0;
    }

    ret = read(cmdif_fd, request, *length);
    if(ret == -1){
        if(errno == EAGAIN){
            return 0;
        }
        return -1;
    }

    *length = ret;

    log_debug("command received (length = %d)", ret);

    return ret;
}
