/*
 * TCP header definitions
 *
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


#ifndef TCP_H
#define TCP_H


typedef struct tcp_header {
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t seq_no;
  uint32_t ack_no;
#if ( __BYTE_ORDER == __BIG_ENDIAN )
  uint8_t offset:4,
          reserved:4;
#else // _LITTLE_ENDIAN
  uint8_t reserved:4,
          offset:4;
#endif
  uint8_t flags;
  uint16_t window;
  uint16_t csum;
  uint16_t urgent;
} tcp_header_t;


#define TCP_FLAG_FIN ( 1 << 0 )
#define TCP_FLAG_SYN ( 1 << 1 )
#define TCP_FLAG_RST ( 1 << 2 )
#define TCP_FLAG_PSH ( 1 << 3 )
#define TCP_FLAG_ACK ( 1 << 4 )
#define TCP_FLAG_URG ( 1 << 5 )
#define TCP_FLAG_ECE ( 1 << 6 )
#define TCP_FLAG_CWR ( 1 << 7 )


#endif // TCP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
