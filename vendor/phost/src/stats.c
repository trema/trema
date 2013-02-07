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
#include <pthread.h>
#include "ipv4.h"
#include "udp.h"
#include "stats.h"
#include "common.h"
#include "log.h"

static tp_stats *stats_udp_send[STATS_TP_HASH_SIZE];
static tp_stats *stats_udp_recv[STATS_TP_HASH_SIZE];
static uint32_t stats_udp_send_n;
static uint32_t stats_udp_recv_n;
static pthread_mutex_t stats_udp_send_mutex;

int stats_init()
{
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&stats_udp_send_mutex, &mutexattr);

    memset(stats_udp_send, 0, sizeof(stats_udp_send));
    memset(stats_udp_recv, 0, sizeof(stats_udp_recv));

    stats_udp_send_n = 0;
    stats_udp_recv_n = 0;

    return 0;
}

int stats_uninit()
{
    return (stats_udp_send_uninit() | stats_udp_recv_uninit());
}

int stats_udp_send_uninit()
{
    int ret;

    pthread_mutex_lock(&stats_udp_send_mutex);
    stats_udp_send_n = 0;
    ret = stats_tp_uninit(stats_udp_send, STATS_TP_HASH_SIZE);
    pthread_mutex_unlock(&stats_udp_send_mutex);

    return ret;
}

int stats_udp_recv_uninit()
{
    stats_udp_recv_n = 0;

    return stats_tp_uninit(stats_udp_recv, STATS_TP_HASH_SIZE);
}

int stats_tp_uninit(tp_stats **st, uint32_t hash_size)
{
    if(hash_size == 0 || st == NULL){
        return -1;
    }

    int i;
    tp_stats *p, *pp;

    for(i=0; i<hash_size; i++){
        if(st[i] != NULL){
            p = st[i];
            do {
                pp = p;
                p = p->next;
                free(pp);
            } while(p != NULL);
            st[i] = NULL;
        }
    }

    return 0;
}

void stats_udp_send_update(ipv4 *ip, udp *udp)
{
    if(ip->protocol != IPV4_PROTOCOL_UDP){
        log_err("only udp packet can be handled.");
        return;
    }

    int ret;
    uint32_t key;

    key = ip->src + ip->dst + (udp->src << 16) + udp->dst;
    key &= STATS_TP_HASH_MASK;

    log_debug("udp message sent: 0x%0x:%u -> 0x%08x:%u (length = %u, "
              "hash key = %u)", ip->src, udp->src, ip->dst, udp->dst,
              udp->payload_length, key);

    pthread_mutex_lock(&stats_udp_send_mutex);
    ret = stats_tp_update(stats_udp_send, key, ip, udp);
    stats_udp_send_n += ret;
    pthread_mutex_unlock(&stats_udp_send_mutex);


    return;
}

void stats_udp_recv_update(ipv4 *ip, udp *udp)
{
    if(ip->protocol != IPV4_PROTOCOL_UDP){
        log_err("only udp packet can be handled.");
        return;
    }

    int ret;
    uint32_t key;

    key = ip->src + ip->dst + (udp->src << 16) + udp->dst;
    key &= STATS_TP_HASH_MASK;

    log_debug("udp message received: 0x%0x:%u -> 0x%08x:%u (length = %u, "
              "hash key = %u)", ip->src, udp->src, ip->dst, udp->dst,
              udp->payload_length, key);

    ret = stats_tp_update(stats_udp_recv, key, ip, udp);
    stats_udp_recv_n += ret;

    return;
}

int stats_tp_update(tp_stats **st, uint32_t key, ipv4 *ip, udp *udp)
{
    int found = 0;
    int new_entry = 0;
    tp_stats *pp = NULL;
    tp_stats *p = st[key];

    if(p == NULL){
        p = (struct tp_stats*)malloc(sizeof(struct tp_stats));
        memset(p, 0, sizeof(struct tp_stats));
        p->lip = ip->dst;
        p->lport = udp->dst;
        p->rip = ip->src;
        p->rport = udp->src;
        p->n_pkts = 1;
        p->n_octets = ip->hdr_length + ip->payload_length;
        p->next = NULL;
        st[key] = p;
        found = 1;
        new_entry = 1;
    }

    while((p != NULL) && !found){
        if((p->lip == ip->dst) && (p->rip == ip->src) &&
           (p->lport == udp->dst) && (p->rport == udp->src)){
             p->n_pkts++;
             p->n_octets += ip->hdr_length + ip->payload_length;
             found = 1;
             break;
        }
        pp = p;
        p = p->next;
    }

    if((p == NULL) && !found){
        p = (struct tp_stats*)malloc(sizeof(struct tp_stats));
        memset(p, 0, sizeof(struct tp_stats));
        p->lip = ip->dst;
        p->lport = udp->dst;
        p->rip = ip->src;
        p->rport = udp->src;
        p->n_pkts = 1;
        p->n_octets = ip->hdr_length + ip->payload_length;
        p->next = NULL;
        pp->next = p;
        found = 1;
        new_entry = 1;
    }

    if(found){
        log_debug("0x%08x:%u -> 0x%08x:%u, n_pkts = %u, n_octets = %u",
                  p->rip, p->rport, p->lip, p->lport, p->n_pkts, p->n_octets);
    }

    return new_entry;
}

tp_stats *stats_udp_send_get(uint32_t *size)
{
    int i, n;
    tp_stats *p;
    tp_stats *stats, *stats_tmp;

    *size = 0;
    if(stats_udp_send_n == 0){
        return NULL;
    }

    pthread_mutex_lock(&stats_udp_send_mutex);

    stats = (tp_stats *)malloc(sizeof(tp_stats) * stats_udp_send_n);
    if(stats == NULL){
        log_err("Failed to allocate memory.");
        pthread_mutex_unlock(&stats_udp_send_mutex);
        return NULL;
    }
    memset(stats, 0, sizeof(tp_stats) * stats_udp_send_n);
    stats_tmp = stats;

    n = 0;
    for(i=0; i<STATS_TP_HASH_SIZE; i++){
        if(stats_udp_send[i] != NULL){
            p = stats_udp_send[i];
            do {
                if(n > stats_udp_send_n){
                    break;
                }
                memcpy(stats_tmp, p, sizeof(tp_stats));
                stats_tmp++;
                n++;
                p = p->next;
            } while(p != NULL);
        }
        if(n > stats_udp_send_n){
            break;
        }
    }

    pthread_mutex_unlock(&stats_udp_send_mutex);

    *size = n;

    return stats;
}

tp_stats *stats_udp_recv_get(uint32_t *size)
{
    int i, n;
    tp_stats *p;
    tp_stats *stats, *stats_tmp;

    *size = 0;
    if(stats_udp_recv_n == 0){
        return NULL;
    }

    stats = (tp_stats *)malloc(sizeof(tp_stats) * stats_udp_recv_n);
    if(stats == NULL){
        log_err("Failed to allocate memory.");
        return NULL;
    }
    memset(stats, 0, sizeof(tp_stats) * stats_udp_recv_n);
    stats_tmp = stats;

    n = 0;
    for(i=0; i<STATS_TP_HASH_SIZE; i++){
        if(stats_udp_recv[i] != NULL){
            p = stats_udp_recv[i];
            do {
                if(n > stats_udp_recv_n){
                    break;
                }
                memcpy(stats_tmp, p, sizeof(tp_stats));
                stats_tmp++;
                n++;
                p = p->next;
            } while(p != NULL);
        }
        if(n > stats_udp_recv_n){
            break;
        }
    }

    *size = n;

    return stats;
}

int stats_udp_send_dump()
{
    int ret;

    pthread_mutex_lock(&stats_udp_send_mutex);
    ret = stats_tp_dump(stats_udp_send, STATS_TP_HASH_SIZE);
    pthread_mutex_unlock(&stats_udp_send_mutex);

    return ret;
}

int stats_udp_recv_dump()
{
    return stats_tp_dump(stats_udp_recv, STATS_TP_HASH_SIZE);
}

int stats_tp_dump(tp_stats **st, uint32_t hash_size)
{
    int i = 0;
    tp_stats *p;

    for(i=0; i<hash_size; i++){
        if(st[i] == NULL){
            continue;
        }
        p = st[i];
        do {
            log_debug("0x%08x:%u -> 0x%08x:%u, n_pkts = %u, n_octets = %u",
                      p->rip, p->rport, p->lip, p->lport, p->n_pkts,
                      p->n_octets);
            p = p->next;
        } while(p != NULL);
    }

    return 0;
}
