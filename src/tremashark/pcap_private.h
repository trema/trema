/*
 * Copyright (C) 2008-2013 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef PCAP_PRIVATE_H
#define PCAP_PRIVATE_H


#include <pcap.h>


// 32bit compatible pcap_pkthdr definition for 64bit environments
typedef struct pcap_pkthdr_private {
  struct {
    bpf_int32 tv_sec;
    bpf_int32 tv_usec;
  } ts;
  bpf_u_int32 caplen;
  bpf_u_int32 len;
} pcap_pkthdr_private;


#endif // PCAP_PRIVATE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
