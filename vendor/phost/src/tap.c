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
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "tap.h"
#include "log.h"
#include "common.h"

static int fd = -1;
static char tap_name[IFNAMSIZ];
static pthread_mutex_t tap_send_mutex;

int tap_init(const char *name)
{
    int flags;
    int nfd;
    struct ifreq ifr;

    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&tap_send_mutex, &mutexattr);

    memset(&ifr, 0, sizeof(ifr));

    if(fd >= 0){
        return -1;
    }

    fd = open(TAP_DEV, O_RDWR);
    if(fd < 0){
        return -1;
    }
    
    ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    if(ioctl(fd, TUNSETIFF, (void *)&ifr) < 0){
        return -1;
    }

    flags = fcntl(fd, F_GETFL);
    if(fcntl(fd, F_SETFL, O_NONBLOCK|flags) < 0){
        // check errno
        return -1;
    }

    nfd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_flags = 0;
    if(ioctl(nfd, SIOCGIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot get interface flags.");
        return -1;
    }
    ifr.ifr_flags |= IFF_UP|IFF_RUNNING;
    if(ioctl(nfd, SIOCSIFFLAGS, (void *)&ifr) < 0){
        log_err("cannot set interface flags.");
        return -1;
    }

    ifr.ifr_qlen = TAP_DEV_TXQ_LEN;
    if(ioctl(nfd, SIOCSIFTXQLEN, (void *)&ifr) < 0){
        log_err("cannot set txqueuelen.");
        return -1;
    }

    close(nfd);

    memset(tap_name, '\0', IFNAMSIZ);
    strncpy(tap_name, name, IFNAMSIZ);

    return 0;
}

int tap_close()
{
    if(fd < 0){
        // already closed
        return -1;
    }

    return close(fd);
}

int tap_read(uint8_t *data, uint32_t *length)
{
    int ret;
    fd_set fdset;
    struct timeval tv;
  
    tv.tv_sec = TAP_SELECT_TIMEOUT/1000000;
    tv.tv_usec = TAP_SELECT_TIMEOUT - tv.tv_sec * 1000000;
    
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

int tap_send(const uint8_t *data, uint32_t length)
{
    int ret;
/*
    fd_set fdset;
    struct timeval tv;

    tv.tv_sec = TAP_SELECT_TIMEOUT/1000000;
    tv.tv_usec = TAP_SELECT_TIMEOUT - tv.tv_sec * 1000000;
    
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
    pthread_mutex_lock(&tap_send_mutex);

    ret = write(fd, data, length);
    if(ret < 0){
        if(ret == EAGAIN){
            log_warn("EAGAIN");
        }
        pthread_mutex_unlock(&tap_send_mutex);
        return -1;
    }

    if(ret != length){
        log_warn("only a part of packet is send (pkt_len = %u, sent_len = %u.",
                 length, ret);
    }

    pthread_mutex_unlock(&tap_send_mutex);

    return ret;
}

int tap_enable_promiscuous()
{
    int nfd;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    nfd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, tap_name, IFNAMSIZ);
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

int tap_disable_promiscuous()
{
    int nfd;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    nfd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, tap_name, IFNAMSIZ);
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
