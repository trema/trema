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

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include "ethdev.h"
#include "log.h"
#include "common.h"

static int fd = -1;
static char ethdev_name[IFNAMSIZ];
static int if_index = -1;
static pthread_mutex_t ethdev_send_mutex;

int ethdev_init(const char *name)
{
    struct ifreq ifr;
    struct sockaddr_ll sll;

    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&ethdev_send_mutex, &mutexattr);

    memset(&ifr, 0, sizeof(ifr));
    
    if(fd >= 0){
        return -1;
    }

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(fd < 0){
        return -1;
    }
    
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    if(ioctl(fd, SIOCGIFINDEX, (void *)&ifr) < 0){
        return -1;
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = ifr.ifr_ifindex;

    if_index = ifr.ifr_ifindex;

    if(bind(fd, (struct sockaddr*)&sll, sizeof(sll))){
        // check errno
        return -1;
    }

    ifr.ifr_flags = 0;
    if(ioctl(fd, SIOCGIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot get interface flags.");
        return -1;
    }
    ifr.ifr_flags |= IFF_UP|IFF_RUNNING;
    if(ioctl(fd, SIOCSIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot set interface flags.");
        return -1;
    }

    ifr.ifr_qlen = ETHDEV_DEV_TXQ_LEN;
    if(ioctl(fd, SIOCSIFTXQLEN, (void *)&ifr) < 0){
        log_err("cannot set txqueuelen.");
        return -1;
    }

    memset(ethdev_name, '\0', IFNAMSIZ);
    strncpy(ethdev_name, name, IFNAMSIZ);

    ethdev_enable_promiscuous();

    return 0;
}

int ethdev_close()
{
    ethdev_disable_promiscuous();

    if_index = -1;

    if(fd < 0){
        // already closed
        return -1;
    }

    return close(fd);
}

int ethdev_read(uint8_t *data, uint32_t *length)
{
    int ret;
    fd_set fdset;
    struct timeval tv;
  
    tv.tv_sec = ETHDEV_SELECT_TIMEOUT/1000000;
    tv.tv_usec = ETHDEV_SELECT_TIMEOUT - tv.tv_sec * 1000000;
    
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    ret = select(fd + 1, &fdset, NULL, NULL, &tv);
    if(ret <= 0) {
        return ret;
    }

    if(FD_ISSET(fd, &fdset) == 0){
        return 0;
    }

    ret = read(fd, data, PKT_BUF_SIZE);
    if(ret == -1){
        if(errno == EAGAIN){
            return 0;
        }
        return -1;
    }

    log_debug("packet received (length = %d)", ret);

    *length = ret;

    return 0;
}

int ethdev_send(const uint8_t *data, uint32_t length)
{
    int ret;
    struct sockaddr_ll sll;
/*
    fd_set fdset;
    struct timeval tv;

    tv.tv_sec = ETHDEV_SELECT_TIMEOUT/1000000;
    tv.tv_usec = ETHDEV_SELECT_TIMEOUT - tv.tv_sec * 1000000;
    
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    ret = select(fd + 1, NULL, &fdset, NULL, &tv);
    if(ret <= 0){
        return ret;
    }

    if(FD_ISSET(fd, &fdset) == 0){
        return 0;
    }
*/

    memset(&sll, 0, sizeof(sll));
    sll.sll_ifindex = if_index;

    pthread_mutex_lock(&ethdev_send_mutex);

    ret = sendto(fd, data, length, 0, (struct sockaddr*)&sll, sizeof(sll));
    if(ret < 0){
        if(ret == EAGAIN){
            log_warn("EAGAIN");
        }
        pthread_mutex_unlock(&ethdev_send_mutex);
        return -1;
    }

    if(ret != length){
        log_warn("only a part of packet is sent (pkt_len = %u, sent_len = %u.",
                 length, ret);
    }

    pthread_mutex_unlock(&ethdev_send_mutex);

    return ret;
}

int ethdev_enable_promiscuous()
{
    int nfd;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    nfd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, ethdev_name, IFNAMSIZ);
    if(ioctl(nfd, SIOCGIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot get interface flags.");
        return -1;
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if(ioctl(nfd, SIOCSIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot set interface flags.");
        return -1;
    }

    close(nfd);

    return 0;
}

int ethdev_disable_promiscuous()
{
    int nfd;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    nfd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, ethdev_name, IFNAMSIZ);
    if(ioctl(nfd, SIOCGIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot get interface flags.");
        return -1;
    }
    ifr.ifr_flags &= ~IFF_PROMISC;
    if(ioctl(nfd, SIOCSIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot set interface flags.");
        return -1;
    }

    close(nfd);

    return 0;
}
