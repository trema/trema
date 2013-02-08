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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "eth.h"
#include "tap.h"
#include "ethdev.h"
#include "trx.h"
#include "arp.h"
#include "ipv4.h"
#include "udp.h"
#include "stats.h"
#include "cmdif.h"
#include "phost.h"
#include "log.h"
#include "common.h"

static uint8_t host_mac_addr[ETH_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint32_t host_ip_addr = 0xc0a80002;
static uint32_t host_ip_mask = 0xffffff00;

static int phost_netdev_max_backlog = 0;
static int phost_max_dgram_qlen = 0;

static int phost_promiscuous = 0;

static int run = 1;
static char *pkt_dump;
static char program_name[PATH_MAX];
static char pid_dir[PATH_MAX];
static char log_dir[PATH_MAX];
static char host_name[PATH_MAX];

int main(int argc, char **argv)
{
    char dev_if[16];
    int opt;
    int err = 0;
    int daemonize = 0;
    int log_level = LOG_WARN;
    char *log_file = NULL;

    memset(dev_if, '\0', sizeof(dev_if));
    strncpy(dev_if, PHOST_DEFAULT_TAP_DEVICE, sizeof(dev_if) - 1);

    phost_set_program_name(basename(argv[0]));

    /* parse options */
    while(1){
        opt = getopt(argc, argv, "Dd:i:p:l:n:v");
        if(opt < 0){
            break;
        }

        switch(opt){
        case 'D':
            daemonize = 1;
            break;
        case 'd':
            if(optarg){
                if(atoi(optarg) < LOG_LEVEL_MAX){
                    log_level = atoi(optarg);
                }
                else{
                    phost_print_usage();
                    exit(1);
                }
            }
            else{
                err |= 1;
            }
            break;
        case 'i':
            if(optarg){
                memset(dev_if, '\0', sizeof(dev_if));
                strncpy(dev_if, optarg, sizeof(dev_if) - 1);
            }
            else{
                err |= 1;
            }
            break;
        case 'p':
            if(optarg){
                memset(pid_dir, '\0', sizeof(pid_dir));
                strncpy(pid_dir, optarg, sizeof(pid_dir) - 1);
            }
            else{
                err |= 1;
            }
            break;
        case 'l':
            if(optarg){
                memset(log_dir, '\0', sizeof(log_dir));
                strncpy(log_dir, optarg, sizeof(log_dir) - 1);
            }
            else{
                err |= 1;
            }
            break;
        case 'n':
            if(optarg){
                memset(host_name, '\0', sizeof(host_name));
                strncpy(host_name, optarg, sizeof(host_name) - 1);
            }
            else{
                err |= 1;
            }
            break;
        case 'v':
            log_file = LOG_OUT_STDOUT;
            log_level = LOG_DEBUG;
            break;
        default:
            err |= 1;
        }
    }

    if(err){
        phost_print_usage();
        exit(1);
    }

    if(log_file == NULL){
        log_file = (char*)malloc(sizeof(char)*(strlen(log_dir)+strlen(PHOST_LOG_FILE)+strlen(dev_if)+3));
        sprintf(log_file, "%s/%s.%s", log_dir, PHOST_LOG_FILE, dev_if);
    }

    /* check if this process is run with root privilege */
    if(getuid() != 0){
        fprintf(stderr, "[ERROR] `%s' must be run with root privilege.\n", program_name);
        exit(1);
    }

    /* register signal handler */
    signal(SIGINT, phost_handle_signals);
    signal(SIGTERM, phost_handle_signals);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);

    /* initializations */
    if(log_init(log_level, log_file) < 0){
        fprintf(stderr, "[ERROR] cannot initialize logger.\n");
        exit(1);
    }

    if(daemonize && (phost_daemonize() < 0)){
        fprintf(stderr, "[ERROR] cannot daemonize.\n");
        exit(1);
    }

    if(strncmp(dev_if, "tap", 3) != 0){
        if(ethdev_init(dev_if) < 0){
            fprintf(stderr, "[ERROR] cannot initialize Ethernet device (%s).\n", dev_if);
            exit(1);
        }
        trx_init(ethdev_read, ethdev_send);
    }
    else{
        if(tap_init(dev_if) < 0){
            fprintf(stderr, "[ERROR] cannot create tap device (%s).\n", dev_if);
            exit(1);
        }
        trx_init(tap_read, tap_send);
    }

    phost_set_global_params();
    phost_create_pid_file(host_name);

    cmdif_init(dev_if, stats_udp_send_update);
    arp_init(host_mac_addr, host_ip_addr);
    ipv4_init(host_ip_addr, host_ip_mask);
    udp_init(stats_udp_recv_update);

    pkt_dump = (char*)malloc(sizeof(char)*PKT_BUF_SIZE*2);

    while(run){
        phost_run();
        cmdif_run();
        arp_age_entries();
    }

    free(pkt_dump);

    cmdif_close();
    tap_close();
    phost_delete_pid_file(host_name);
    phost_unset_global_params();
    log_close();

    return 0;
}

int phost_run()
{
    int count = 0;
    eth *eth;

    trx_rx();

    eth = trx_rxq_pop();

    while((eth != NULL) && (count < PHOST_RUN_LOOP_COUNT)){
        /*
        log_debug("new packet recived: %s", eth_dump(eth, pkt_dump));
        memset(pkt_dump, 0, sizeof(char)*PKT_BUF_SIZE*2);
        */

        /* note that multicast frame is not supported. */
        if(phost_promiscuous ||
           (memcmp(eth->dst, arp_host_mac_addr, ETH_ADDR_LEN) == 0) ||
           (memcmp(eth->dst, eth_mac_addr_bc, ETH_ADDR_LEN) == 0)){
            switch(eth->type){
            case ETH_TYPE_ARP:
                arp_handle_message(eth);
                break;
            case ETH_TYPE_IPV4:
                ipv4_handle_message(eth);
                break;
            default:
                log_debug("unsupported ether type.");
                break;
            }
        }
        eth_destroy(eth);
        count++;
        eth = trx_rxq_pop();
    }

    trx_all();

    return 0;
}

int phost_daemonize()
{
    pid_t pid, sid;

    pid = fork();
    if(pid < 0){
        return -1;
    }

    if(pid > 0){
        exit(0);
    }

    sid = setsid();
    if(sid < 0){
        return -1;
    }

    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
}

int phost_set_program_name(const char *name)
{
    memset(program_name, '\0', sizeof(program_name));
    strncpy(program_name, name, PATH_MAX - 1);

    return 0;
}

int phost_create_pid_file(const char *instance)
{
    pid_t pid;
    char file[PATH_MAX];

    memset(file, '\0', sizeof(file));
    snprintf(file, PATH_MAX - 1, "%s/phost.%s.pid", pid_dir, instance);

    pid = getpid();

    FILE *fp = fopen(file, "w");

    if(fp == NULL){
        log_err( "cannot create pid file: %s", file );
        return -1;
    }

    fprintf(fp, "%u\n", pid);

    fclose(fp);

    return 0;
}

int phost_delete_pid_file(const char *instance)
{
    char file[PATH_MAX];

    memset(file, '\0', sizeof(file));
    snprintf(file, PATH_MAX - 1, "%s/phost.%s.pid", pid_dir, instance);

    return unlink(file);
}

int phost_set_global_params()
{
    int fd;
    int ret;
    char buff[32];

    /* netdev_max_backlog */
    fd = open(PHOST_NETDEV_MAX_BACKLOG_FILE, O_RDWR);
    if(fd < 0){
        log_err("cannot open %s", PHOST_NETDEV_MAX_BACKLOG_FILE);
        return -1;
    }

    memset(buff, '\0', sizeof(buff));

    ret = read(fd, buff, sizeof(buff));
    if(ret <= 0){
        log_err("cannot read %s", PHOST_NETDEV_MAX_BACKLOG_FILE);
        return -1;
    }
    phost_netdev_max_backlog = atoi(buff);

    memset(buff, '\0', sizeof(buff));
    snprintf(buff, 32, "%d\n", PHOST_NETDEV_MAX_BACKLOG);

    ret = write(fd, buff, strlen(buff));
    if(ret != strlen(buff)){
        log_err("cannot write %s", PHOST_NETDEV_MAX_BACKLOG_FILE);
        return -1;
    }
    close(fd);

    /* max_dgram_qlen */
    fd = open(PHOST_MAX_DGRAM_QLEN_FILE, O_RDWR);
    if(fd < 0){
        log_err("cannot open %s", PHOST_MAX_DGRAM_QLEN_FILE);
        return -1;
    }

    memset(buff, '\0', sizeof(buff));

    ret = read(fd, buff, sizeof(buff));
    if(ret <= 0){
        log_err("cannot read %s", PHOST_MAX_DGRAM_QLEN);
        return -1;
    }

    phost_max_dgram_qlen = atoi(buff);

    memset(buff, '\0', sizeof(buff));
    snprintf(buff, 32, "%d\n", PHOST_MAX_DGRAM_QLEN);

    ret = write(fd, buff, strlen(buff));
    if(ret != strlen(buff)){
        log_err("cannot write %s", PHOST_MAX_DGRAM_QLEN_FILE);
        return -1;
    }
    close(fd);

    return 0;
}

int phost_unset_global_params()
{
    int fd;
    int ret;
    char buff[32];

    /* netdev_max_backlog */
    fd = open(PHOST_NETDEV_MAX_BACKLOG_FILE, O_RDWR);
    if(fd < 0){
        log_err("cannot open %s", PHOST_NETDEV_MAX_BACKLOG_FILE);
        return -1;
    }

    memset(buff, '\0', sizeof(buff));
    snprintf(buff, 32, "%d\n", phost_netdev_max_backlog);

    ret = write(fd, buff, strlen(buff));
    if(ret != strlen(buff)){
        log_err("cannot write %s", PHOST_NETDEV_MAX_BACKLOG_FILE);
        return -1;
    }
    close(fd);

    /* max_dgram_qlen */
    fd = open(PHOST_MAX_DGRAM_QLEN_FILE, O_RDWR);
    if(fd < 0){
        log_err("cannot open %s", PHOST_MAX_DGRAM_QLEN_FILE);
        return -1;
    }

    memset(buff, '\0', sizeof(buff));
    snprintf(buff, 32, "%d\n", phost_max_dgram_qlen);

    ret = write(fd, buff, strlen(buff));
    if(ret != strlen(buff)){
        log_err("cannot write %s", PHOST_MAX_DGRAM_QLEN_FILE);
        return -1;
    }
    close(fd);

    return 0;
}

int phost_enable_promiscuous()
{
    phost_promiscuous = 1;
    return 0;
}

int phost_disable_promiscuous()
{
    phost_promiscuous = 0;
    return 0;
}

void phost_handle_signals(int signum)
{
    log_debug("signal received: signum = %u", signum);
    run = 0;
}

int phost_print_usage()
{
    printf("usage: %s [-i dev] [-d debug_level] [-p pid_dir] [-l log_dir] [-n host_name] [-D] [-v]\n", program_name);
    return 0;
}
