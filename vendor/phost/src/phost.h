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

#ifndef _PHOST_H_
#define _PHOST_H_

#define PHOST_LOG_FILE "phost.log"

#define PHOST_DEFAULT_TAP_DEVICE "tap0"

#define PHOST_RUN_LOOP_COUNT 16384

#define PHOST_NETDEV_MAX_BACKLOG_FILE "/proc/sys/net/core/netdev_max_backlog"
#define PHOST_NETDEV_MAX_BACKLOG 2048

#define PHOST_MAX_DGRAM_QLEN_FILE "/proc/sys/net/unix/max_dgram_qlen"
#define PHOST_MAX_DGRAM_QLEN 256

int phost_run();
int phost_daemonize();
int phost_set_program_name(const char *name);
int phost_create_pid_file(const char *instance);
int phost_delete_pid_file(const char *instance);
int phost_set_global_params();
int phost_unset_global_params();
int phost_enable_promiscuous();
int phost_disable_promiscuous();
void phost_handle_signals(int signal);
int phost_print_usage();

#endif /* _PHOST_H_*/

