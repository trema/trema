/*
 * Unit tests for packet_parser functions and macros.
 *
 * Author: Kazuya Suzuki
 *
 * Copyright (C) 2008-2011 NEC Corporation
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


#include <assert.h>
#include <string.h>
#include <pcap.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "packet_info.h"
#include "packet_parser.h"
#include "wrapper.h"



/******************************************************************************
 * Callback function for pcap_dispatch().
 ******************************************************************************/

static void *
store_packet_to_buffer( u_char *user_data,
                        const struct pcap_pkthdr *header,
                        const u_char *length ) {
  assert( user_data != NULL );
  assert( header != NULL );
  assert( length != NULL );

  buffer *buffer0 = ( buffer * )user_data;

  buffer0->length = header->len;
  buffer0->data = xcalloc( 1, header->len );
  memcpy( buffer0->data, header + 1, header->len );

  return NULL;
}


/******************************************************************************
 * Setup and teardown function.
 ******************************************************************************/
static buffer *
setup_dummy_ether_arp_packet() {
  buffer *arp_buffer = alloc_buffer();

  char error[ PCAP_ERRBUF_SIZE ];
  pcap_t *pcap = pcap_open_offline( "./test_packets/arp.cap", error );
  assert( pcap != NULL );
  
  int count = pcap_dispatch( pcap, 1, store_packet_to_buffer, arp_buffer );
  assert( count == 1 );

  return arp_buffer;
}


/********************************************************************************
 * ether arp Tests.
 ********************************************************************************/

static void
test_parse_packet_ether_arp_succeeds() {
  buffer *arp_buffer = setup_dummy_ether_arp_packet( );

  assert_true( parse_packet( arp_buffer ) );

  packet_info packet_info0 = ( packet_info * )buf->user_data;

  free_buffer( arp_buffer );
}



/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_parse_packet_ether_arp_succeeds ),

  };
  stub_logger();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
