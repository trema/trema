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
#include <stdio.h>
#include <pcap.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "packet_info.h"
#include "packet_parser.h"
#include "wrapper.h"


/******************************************************************************
 * 
 ******************************************************************************/

static bool
store_packet_to_buffer( buffer *buffer, const char *filename ) {
  assert( buffer != NULL );
  assert( filename != NULL );

  FILE *fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    // "Can't open a file of test data."
    return false;
  }

  struct pcap_file_header dummy_buffer;
  size_t size = fread( &dummy_buffer, 1, 
                       sizeof( struct pcap_file_header ), fp );
  if ( size < sizeof( struct pcap_file_header ) ) {
    return false;
  }  
  
  uint32_t dummy_buffer2[4];
  size = fread( &dummy_buffer2, 1, sizeof( dummy_buffer2 ), fp );
  if ( size < sizeof( dummy_buffer2 ) ) {
    return false;
  }  

  buffer->length = dummy_buffer2[2];
  buffer->data = xcalloc( 1, buffer->length );
  size = fread( buffer->data, 1, buffer->length, fp );
  if ( size < buffer->length ) {
    return false;
  }  

  return true;
}


/******************************************************************************
 * Setup and teardown function.
 ******************************************************************************/

static buffer *
setup_dummy_ether_arp_packet() {
  buffer *arp_buffer = alloc_buffer();

  store_packet_to_buffer( arp_buffer, "./unittests/lib/test_packets/arp.cap" );
  
  return arp_buffer;
}


/******************************************************************************
 * ether arp Tests.
 ******************************************************************************/

static void
test_parse_packet_ether_arp_succeeds() {
  buffer *arp_buffer = setup_dummy_ether_arp_packet();

  assert_true( parse_packet( arp_buffer ) );

  packet_info *packet_info0 = arp_buffer->user_data;
  assert_true( packet_info0->eth_type == ETH_ETHTYPE_ARP );

  free_buffer( arp_buffer );
}



/******************************************************************************
 * Run tests.
 ******************************************************************************/

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
