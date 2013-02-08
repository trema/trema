#
# Copyright (C) 2009-2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

CC = gcc
#CFLAGS = -Wall -g
CFLAGS = -Wall -O2 -D_GNU_SOURCE -fno-strict-aliasing
#LDFLAGS_DAEMON = -lssl
LDFLAGS_DAEMON = -pthread
LDFLAGS_CLI =

TARGET_DAEMON = phost
SRCS_DAEMON = phost.c common.c tap.c eth.c arp.c ipv4.c icmp.c udp.c stats.c \
              ethdev.c cmdif.c trx.c log.c utils.c
OBJS_DAEMON = $(SRCS_DAEMON:.c=.o)

TARGET_CLI = cli
SRCS_CLI = cli.c common.c log.c utils.c
OBJS_CLI = $(SRCS_CLI:.c=.o)

TARGETS = $(TARGET_DAEMON) $(TARGET_CLI)
SRCS = $(SRCS_DAEMON) $(SRCS_CLI)
OBJS = $(OBJS_DAEMON) $(OBJS_CLI)
DEPENDS = .depends

.PHONY: all clean

.SUFFIXES: .c .o

all: depend $(TARGET_DAEMON) $(TARGET_CLI)

$(TARGET_DAEMON): $(OBJS_DAEMON)
	$(CC) $(LDFLAGS_DAEMON) $(OBJS_DAEMON) -o $@

$(TARGET_CLI): $(OBJS_CLI)
	$(CC) $(LDFLAGS_CLI) $(OBJS_CLI) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<

depend: 
	$(CC) -MM -MG $(SRCS) > $(DEPENDS)

clean:
	@rm -rf $(DEPENDS) $(OBJS) $(TARGETS) *~

-include $(DEPENDS)
