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
#include <pthread.h>
#include <time.h>
#include "eth.h"
#include "tap.h"
#include "trx.h"
#include "arp.h"
#include "common.h"
#include "log.h"

struct trx_pktq {
    struct trx_pktq *prev;
    struct trx_pktq *next;
    uint8_t *buffer;
    uint32_t length;
};

struct trx_pktq_arp_wait {
    struct trx_pktq_arp_wait *prev;
    struct trx_pktq_arp_wait *next;
    uint8_t *buffer;
    uint32_t length;
    uint32_t ip_addr;
    time_t pushed;
};

static struct trx_pktq *trx_txq;
static struct trx_pktq *trx_txq_tail;
static struct trx_pktq_arp_wait *trx_txq_arp_wait;
static struct trx_pktq_arp_wait *trx_txq_arp_wait_tail;
static struct trx_pktq *trx_rxq;
static struct trx_pktq *trx_rxq_tail;
static uint32_t trx_txq_size;
static uint32_t trx_txq_arp_wait_size;
static uint32_t trx_rxq_size;

static pthread_mutex_t trx_txq_mutex;
static pthread_mutex_t trx_rxq_mutex;
static pthread_mutex_t trx_txq_arp_wait_mutex;

static int (*trx_send)(const uint8_t *data, uint32_t length) = NULL;
static int (*trx_read)(uint8_t *data, uint32_t *length) = NULL;

int trx_init(void *recv_callback, void *send_callback)
{
    log_debug("trx_init()");

    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&trx_txq_mutex, &mutexattr);
    pthread_mutex_init(&trx_rxq_mutex, &mutexattr);
    pthread_mutex_init(&trx_txq_arp_wait_mutex, &mutexattr);

    trx_txq = (struct trx_pktq*)malloc(sizeof(struct trx_pktq));
    trx_txq_arp_wait
        = (struct trx_pktq_arp_wait*)malloc(sizeof(struct trx_pktq_arp_wait));
    trx_rxq = (struct trx_pktq*)malloc(sizeof(struct trx_pktq));

    trx_txq->prev = NULL;
    trx_txq->next = NULL;
    trx_txq->buffer = NULL;
    trx_txq->length = 0;

    trx_txq_arp_wait->prev = NULL;
    trx_txq_arp_wait->next = NULL;
    trx_txq_arp_wait->buffer = NULL;
    trx_txq_arp_wait->length = 0;
    trx_txq_arp_wait->ip_addr = 0;
    trx_txq_arp_wait->pushed = 0;

    trx_rxq->prev = NULL;
    trx_rxq->next = NULL;
    trx_rxq->buffer = NULL;
    trx_rxq->length = 0;

    trx_rxq_tail = trx_rxq;
    trx_txq_tail = trx_txq;
    trx_txq_arp_wait_tail = trx_txq_arp_wait;

    trx_txq_size = 0;
    trx_txq_arp_wait_size = 0;
    trx_rxq_size = 0;

    trx_send = send_callback;
    trx_read = recv_callback;

    return 0;
}

int trx_txq_push(eth *eth)
{
    uint8_t *buffer;
    uint32_t length;

    length = PKT_BUF_SIZE;
    buffer = (uint8_t *)malloc(sizeof(uint8_t)*PKT_BUF_SIZE);

    buffer = eth_get_frame(eth, buffer, &length);

    if(length <= 0 || buffer == NULL){
        if(buffer != NULL){
            free(buffer);
        }
        return -1;
    }

    // push to tx queue
    pthread_mutex_lock(&trx_txq_mutex);

    struct trx_pktq *q = trx_txq_tail;
    if(q == NULL){
        pthread_mutex_unlock(&trx_txq_mutex);
        return -1;
    }

    struct trx_pktq *tx;
    if(q->prev != NULL){
        tx = (struct trx_pktq*)malloc(sizeof(struct trx_pktq));
        tx->buffer = buffer;
        tx->length = length;
        tx->prev = q;
        tx->next = NULL;
        q->next = tx;
        trx_txq_tail = tx;
    }
    else{
        trx_txq->buffer = buffer;
        trx_txq->length = length;
    }

    trx_txq_size++;

    pthread_mutex_unlock(&trx_txq_mutex);

    return 0;
}

int trx_txq_arp_waitlist_push(eth *eth, uint32_t ip_addr)
{
    uint8_t *buffer;
    uint32_t length;

    length = PKT_BUF_SIZE;
    buffer = (uint8_t *)malloc(sizeof(uint8_t)*PKT_BUF_SIZE);

    buffer = eth_get_frame(eth, buffer, &length);

    if(length <= 0 || buffer == NULL){
        if(buffer != NULL){
            free(buffer);
        }
        return -1;
    }

    // push to tx queue
    pthread_mutex_lock(&trx_txq_arp_wait_mutex);

    struct trx_pktq_arp_wait *q = trx_txq_arp_wait_tail;
    if(q == NULL){
        pthread_mutex_unlock(&trx_txq_arp_wait_mutex);
        return -1;
    }

    struct trx_pktq_arp_wait *tx;
    if(q->prev != NULL){
        tx = (struct trx_pktq_arp_wait*)malloc(sizeof(struct trx_pktq_arp_wait));
        tx->buffer = buffer;
        tx->length = length;
        tx->ip_addr = ip_addr;
        tx->pushed = time(NULL);
        tx->prev = q;
        tx->next = NULL;
        q->next = tx;
        trx_txq_arp_wait_tail = tx;
    }
    else{
        trx_txq_arp_wait->buffer = buffer;
        trx_txq_arp_wait->length = length;
        trx_txq_arp_wait->ip_addr = ip_addr;
        trx_txq_arp_wait->pushed = time(NULL);
    }

    trx_txq_arp_wait_size++;

    pthread_mutex_unlock(&trx_txq_arp_wait_mutex);

    return 0;
}


int trx_rxq_push(uint8_t *buffer, uint32_t length)
{
    if(buffer == NULL || length == 0){
        return -1;
    }

    pthread_mutex_lock(&trx_rxq_mutex);

    struct trx_pktq *q = trx_rxq_tail;

    if(q == NULL){
        pthread_mutex_unlock(&trx_rxq_mutex);
        return -1;
    }
     
    struct trx_pktq *rx;
    if(q->prev == NULL && q->next == NULL && q->length == 0){
        /* head == tail */
        trx_rxq->buffer = buffer;
        trx_rxq->length = length;
    }
    else if(q->next == NULL){
        rx = (struct trx_pktq*)malloc(sizeof(struct trx_pktq));
        rx->buffer = buffer;
        rx->length = length;
        rx->prev = q;
        rx->next = NULL;
        q->next = rx;
        trx_rxq_tail = rx;
    }

    trx_rxq_size++;

    pthread_mutex_unlock(&trx_rxq_mutex);

    return 0;
}

eth *trx_rxq_pop()
{
    uint8_t *buffer;
    uint32_t len;
    eth *eth = NULL;

    pthread_mutex_lock(&trx_rxq_mutex);

    struct trx_pktq *q = trx_rxq;

    if(q->length > 0){
        log_debug("q->length = %u", q->length);
    }

    if(q->length > 0){
        buffer = q->buffer;
        len = q->length;
        eth = eth_create_from_raw(buffer, len);
        if(q->next != NULL){
            trx_rxq = q->next;
            trx_rxq->prev = NULL;
            free(q->buffer);
            free(q);
        }
        else{
            free(q->buffer);
            q->buffer = NULL;
            q->length = 0;
            if(q->prev != NULL){
                /* should not reach */
                q->prev->next = NULL;
                free(q);
            }
        }
    }
    else{
        pthread_mutex_unlock(&trx_rxq_mutex);

        return NULL;
    }

    trx_rxq_size--;
    
    pthread_mutex_unlock(&trx_rxq_mutex);

    return eth;
}

int trx_all()
{
    int count = 0;
    int ret_rx = 0, ret_tx = 0, ret_tx_arp_waitlist = 0;

    while(count < TRX_TRX_LOOP_COUNT){
      if(ret_tx == 0){
          ret_tx = trx_tx();
      }
      if(ret_rx == 0){
          ret_rx = trx_rx();
      }
      if(ret_tx_arp_waitlist == 0){
          ret_tx_arp_waitlist = trx_tx_arp_waitlist();
      }
      if((ret_rx & ret_tx & ret_tx_arp_waitlist) != 0){
        break;
      }
      count++;
    }

    return 0;
}

int trx_tx()
{
    int count = 0;
    int ret = 0;

    do {
        ret = trx_tx_one();
        count++;
    } while((count < TRX_TX_LOOP_COUNT) && (ret >= 0));

    return 0;
}

int trx_tx_one()
{
    int sent;
    uint32_t length;
    uint8_t *buffer;

    pthread_mutex_lock(&trx_txq_mutex);

    struct trx_pktq *q = trx_txq;

    if(q->length > 0){
        buffer = q->buffer;
        length = q->length;

        sent = trx_send(buffer, length);

        if(sent == length){
            if(q->next != NULL){
                trx_txq = q->next;
                trx_txq->prev = NULL;
                free(q->buffer);
                free(q);
            }
            else{
                free(q->buffer);
                q->buffer = NULL;
                q->length = 0;
            }
            trx_txq_size--;
            pthread_mutex_unlock(&trx_txq_mutex);
            return 0;
        }
        else if(sent > 0 && sent < length){
            q->length = length - sent;
            uint8_t *new_buf = (uint8_t*)malloc(sizeof(uint8_t)*(q->length));
            memcpy(new_buf, q->buffer + sent, q->length);
            free(q->buffer);
            q->buffer = new_buf;
        }
    }

    pthread_mutex_unlock(&trx_txq_mutex);

    return -1;
}

int trx_tx_immediately(eth *eth)
{
    uint8_t *buffer;
    uint32_t length;
    uint32_t sent;

    length = PKT_BUF_SIZE;
    buffer = (uint8_t *)malloc(sizeof(uint8_t)*PKT_BUF_SIZE);

    buffer = eth_get_frame(eth, buffer, &length);

    if(length <= 0 || buffer == NULL){
        if(buffer != NULL){
            free(buffer);
        }
        return -1;
    }

    sent = trx_send(buffer, length);

    free(buffer);

    if(sent == length){
        return 0;
    }

    return -1;
}

int trx_tx_arp_waitlist()
{
    int sent = 0;
    uint32_t length;
    uint8_t *buffer;
    uint8_t mac_addr[ETH_ADDR_LEN];

    pthread_mutex_lock(&trx_txq_arp_wait_mutex);

    struct trx_pktq_arp_wait *q = trx_txq_arp_wait;

    if(q->length > 0){
        
        buffer = q->buffer;
        length = q->length;

        if((q->pushed + TRX_ARP_WAITLIST_TIMEOUT) > time(NULL)){
            memset(mac_addr, 0, sizeof(mac_addr));
            if(arp_get_mac_by_ip(q->ip_addr, mac_addr) == 0){
                memcpy(buffer, mac_addr, ETH_ADDR_LEN);
                sent = trx_send(buffer, length);
            }
        }
        else{
            sent = length; /* just remove from queue */
        }

        if(sent == length){
            if(q->next != NULL){
                trx_txq_arp_wait = q->next;
                trx_txq_arp_wait->prev = NULL;
                free(q->buffer);
                free(q);
            }
            else{
                free(q->buffer);
                q->buffer = NULL;
                q->length = 0;
                q->ip_addr = 0;
                q->pushed = 0;
            }
            trx_txq_arp_wait_size--;
            pthread_mutex_unlock(&trx_txq_arp_wait_mutex);
            return 0;
        }
        else if(sent > 0 && sent < length){
            q->length = length - sent;
            uint8_t *new_buf = (uint8_t*)malloc(sizeof(uint8_t)*(q->length));
            memcpy(new_buf, q->buffer + sent, q->length);
            free(q->buffer);
            q->buffer = new_buf;
        }
    }

    pthread_mutex_unlock(&trx_txq_arp_wait_mutex);

    return -1;
}

int trx_rx()
{
    int count = 0;
    int ret = 0;

    do {
        ret = trx_rx_one();
        count++;
    } while((count < TRX_RX_LOOP_COUNT) && (ret >= 0));

    return 0;
}

int trx_rx_one()
{
    int ret;
    uint32_t length = 0;
    uint8_t *buffer;

    buffer = (uint8_t *)malloc(sizeof(uint8_t)*PKT_BUF_SIZE);
    memset(buffer, 0, sizeof(uint8_t)*PKT_BUF_SIZE);

    ret = trx_read(buffer, &length);

    if(ret < 0 || length <= 0){
        free(buffer);
        return -1;
    }

    trx_rxq_push(buffer, length);

    return 0;
}

uint32_t trx_get_txq_size()
{
    return trx_txq_size;
}

uint32_t trx_get_txq_arp_waitlist_size()
{
    return trx_txq_arp_wait_size;
}

uint32_t trx_get_rxq_size()
{
    return trx_rxq_size;
}
