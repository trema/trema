/*
 * Unit tests for packetin_filer.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unittest.h"
#include "trema.h"
#include "match_table.h"


void usage();
void handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
  uint32_t buffer_id, uint16_t total_len,
  uint16_t in_port, uint8_t reason, const buffer *data,
  void *user_data );
void register_dl_type_filter( uint16_t dl_type, uint16_t priority,
  const char *service_name, const char *entry_name );
void register_any_filter( uint16_t priority, const char *service_name,
  const char *entry_name );
int packetin_filter_main( int argc, char *argv[] );


/*************************************************************************
 * Setup and teardown function.
 *************************************************************************/


static void
setup() {
  init_log( "packetin_filter_test", false );
}


static void
teardown() {
}


/*************************************************************************
 * Mock
 *************************************************************************/


void
mock_die( char *format, ... ) {
  UNUSED( format );
}


int
mock_printf2( char *format, ... ) {
  UNUSED( format );

  return ( int ) mock();
}


void
mock_error( const char *format, ... ) {
  char buffer[ 1024 ];
  va_list args;
  va_start( args, format );
  vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
  va_end( args );

  check_expected( buffer );
  ( void ) mock();
}


void
mock_set_match_from_packet( struct ofp_match *match, const uint16_t in_port,
  const uint32_t wildcards, /* const */ buffer *packet ) {
  uint32_t in_port32 = in_port;

  check_expected( match );
  check_expected( in_port32 );
  check_expected( wildcards );
  check_expected( packet );

  memset( match, 0, sizeof( struct ofp_match ) );
  match->in_port = in_port;
  match->wildcards = wildcards;

  ( void ) mock();
}


buffer *
mock_create_packet_in( const uint32_t transaction_id, const uint32_t buffer_id,
  const uint16_t total_len, uint16_t in_port,
  const uint8_t reason, /* const */ buffer *data ) {
  uint32_t total_len32 = total_len;
  uint32_t in_port32 = in_port;
  uint32_t reason32 = reason;

  check_expected( transaction_id );
  check_expected( buffer_id );
  check_expected( total_len32 );
  check_expected( in_port32 );
  check_expected( reason32 );
  check_expected( data );

  return ( buffer * ) mock();
}


void
mock_insert_match_entry( struct ofp_match *ofp_match, uint16_t priority,
  /* const */ char *service_name, /* const */ char *entry_name ) {
  uint32_t priority32 = priority;

  check_expected( ofp_match );
  check_expected( priority32 );
  check_expected( service_name );
  check_expected( entry_name );

  ( void ) mock();
}


match_entry *
mock_lookup_match_entry( struct ofp_match *match ) {
  check_expected( match );

  return ( match_entry * ) mock();
}


bool
mock_send_message( /* const */ char *service_name, const uint16_t tag,
  /* const */ void *data,
  size_t len ) {
  uint32_t tag32 = tag;

  check_expected( service_name );
  check_expected( tag32 );
  check_expected( data );
  check_expected( len );

  return ( bool ) mock();
}


void
mock_init_trema( int *argc, char ***argv ) {
  UNUSED( argc );
  UNUSED( argv );

  ( void ) mock();
}


bool
mock_set_packet_in_handler( packet_in_handler callback, void *user_data ) {
  UNUSED( callback );
  UNUSED( user_data );

  return ( bool ) mock();
}


void
mock_start_trema( void ) {
  ( void ) mock();
}


const char *
mock_get_executable_name( void ) {
  return "packetin_filter";
}


/*************************************************************************
 * Test functions.
 *************************************************************************/

static void
test_usage() {
  setup();

  will_return( mock_printf2, 1 );

  usage();

  teardown();
}


static void
test_handle_packet_in_successed() {
  setup();

  uint64_t datapath_id = 0x101;
  uint32_t transaction_id = 1234;
  uint32_t buffer_id = 0;
  uint32_t total_len32 = 0;
  uint32_t in_port32 = 1;
  uint32_t reason32 = 0;
  buffer *data;
  void *user_data = NULL;

  match_entry match_entry;
  buffer *buf;

  data = alloc_buffer();
  expect_not_value( mock_set_match_from_packet, match, NULL );
  expect_value( mock_set_match_from_packet, in_port32, in_port32 );
  expect_value( mock_set_match_from_packet, wildcards, 0 );
  expect_value( mock_set_match_from_packet, packet, data );
  will_return_void( mock_set_match_from_packet );

  memset( &match_entry, 0, sizeof( match_entry ) );
  match_entry.service_name = ( char * ) ( uintptr_t ) ( "service_name" );
  match_entry.entry_name = ( char * ) ( uintptr_t ) ( "entry_name" );
  expect_not_value( mock_lookup_match_entry, match, NULL );
  will_return( mock_lookup_match_entry, &match_entry );

  buf = alloc_buffer();
  expect_value( mock_create_packet_in, transaction_id, transaction_id );
  expect_value( mock_create_packet_in, buffer_id, buffer_id );
  expect_value( mock_create_packet_in, total_len32, total_len32 );
  expect_value( mock_create_packet_in, in_port32, in_port32 );
  expect_value( mock_create_packet_in, reason32, reason32 );
  expect_value( mock_create_packet_in, data, data );
  will_return( mock_create_packet_in, buf );

  expect_value( mock_send_message, service_name, match_entry.service_name );
  expect_value( mock_send_message, tag32, MESSENGER_OPENFLOW_MESSAGE );
  expect_not_value( mock_send_message, data, NULL );
  expect_not_value( mock_send_message, len, 0 );
  will_return( mock_send_message, true );

  handle_packet_in( datapath_id, transaction_id, buffer_id, ( uint16_t ) total_len32,
    ( uint16_t ) in_port32, ( uint8_t ) reason32, data, user_data );

  free_buffer( data );

  teardown();
}


static void
test_handle_packet_in_lookup_failed() {
  setup();

  uint64_t datapath_id = 0x101;
  uint32_t transaction_id = 1234;
  uint32_t buffer_id = 0;
  uint16_t total_len = 0;
  uint32_t in_port32 = 1;
  uint8_t reason = 0;
  buffer *data;
  void *user_data = NULL;

  data = alloc_buffer();

  expect_not_value( mock_set_match_from_packet, match, NULL );
  expect_value( mock_set_match_from_packet, in_port32, in_port32 );
  expect_value( mock_set_match_from_packet, wildcards, 0 );
  expect_value( mock_set_match_from_packet, packet, data );
  will_return_void( mock_set_match_from_packet );

  expect_not_value( mock_lookup_match_entry, match, NULL );
  will_return( mock_lookup_match_entry, NULL );

  handle_packet_in( datapath_id, transaction_id, buffer_id, total_len,
    ( uint16_t ) in_port32, reason, data, user_data );

  free_buffer( data );

  teardown();
}


static void
test_handle_packet_in_send_failed() {
  setup();

  uint64_t datapath_id = 0x101;
  uint32_t transaction_id = 1234;
  uint32_t buffer_id = 0;
  uint32_t total_len32 = 0;
  uint32_t in_port32 = 1;
  uint32_t reason32 = 0;
  buffer *data;
  void *user_data = NULL;

  match_entry match_entry;
  buffer *buf;

  data = alloc_buffer();
  expect_not_value( mock_set_match_from_packet, match, NULL );
  expect_value( mock_set_match_from_packet, in_port32, in_port32 );
  expect_value( mock_set_match_from_packet, wildcards, 0 );
  expect_value( mock_set_match_from_packet, packet, data );
  will_return_void( mock_set_match_from_packet );

  memset( &match_entry, 0, sizeof( match_entry ) );
  match_entry.service_name = ( char * ) ( uintptr_t ) ( "service_name" );
  match_entry.entry_name = ( char * ) ( uintptr_t ) ( "entry_name" );
  expect_not_value( mock_lookup_match_entry, match, NULL );
  will_return( mock_lookup_match_entry, &match_entry );

  buf = alloc_buffer();
  expect_value( mock_create_packet_in, transaction_id, transaction_id );
  expect_value( mock_create_packet_in, buffer_id, buffer_id );
  expect_value( mock_create_packet_in, total_len32, total_len32 );
  expect_value( mock_create_packet_in, in_port32, in_port32 );
  expect_value( mock_create_packet_in, reason32, reason32 );
  expect_value( mock_create_packet_in, data, data );
  will_return( mock_create_packet_in, buf );

  expect_string( mock_send_message, service_name, match_entry.service_name );
  expect_value( mock_send_message, tag32, MESSENGER_OPENFLOW_MESSAGE );
  expect_not_value( mock_send_message, data, NULL );
  expect_not_value( mock_send_message, len, 0 );
  will_return( mock_send_message, false );

  expect_string( mock_error, buffer, "Failed to send a message to service_name ( entry_name = entry_name, match = wildcards = 0, in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0, nw_dst = 0.0.0.0, tp_src = 0, tp_dst = 0 )." );
  will_return_void( mock_error );

  handle_packet_in( datapath_id, transaction_id, buffer_id, ( uint16_t ) total_len32,
                    ( uint16_t ) in_port32, ( uint8_t ) reason32, data, user_data );

  free_buffer( data );

  teardown();
}


static void
test_register_dl_type_filter() {
  setup();

  uint32_t dl_type32 = 1;
  uint32_t priority32 = 2;

  expect_not_value( mock_insert_match_entry, ofp_match, NULL );
  expect_value( mock_insert_match_entry, priority32, priority32 );
  expect_string( mock_insert_match_entry, service_name, "service_name" );
  expect_string( mock_insert_match_entry, entry_name, "entry_name" );
  will_return_void( mock_insert_match_entry );

  register_dl_type_filter( ( uint16_t ) dl_type32, ( uint16_t ) priority32, "service_name", "entry_name" );

  teardown();
}


static void
test_register_any_filter() {
  setup();

  uint32_t priority32 = 3;

  expect_not_value( mock_insert_match_entry, ofp_match, NULL );
  expect_value( mock_insert_match_entry, priority32, priority32 );
  expect_string( mock_insert_match_entry, service_name, "service_name" );
  expect_string( mock_insert_match_entry, entry_name, "entry_name" );
  will_return_void( mock_insert_match_entry );

  register_any_filter( ( uint16_t ) priority32, "service_name", "entry_name" );

  teardown();
}


static void
test_packetin_filter_main_successed() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "packetin_filter",
      ( char * ) ( uintptr_t ) "lldp::topo",
      ( char * ) ( uintptr_t ) "packet_in::hub",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  expect_not_value( mock_insert_match_entry, ofp_match, NULL );
  expect_value( mock_insert_match_entry, priority32, 0x8000 );
  expect_string( mock_insert_match_entry, service_name, "topo" );
  expect_string( mock_insert_match_entry, entry_name, "filter-lldp" );
  will_return_void( mock_insert_match_entry );

  expect_not_value( mock_insert_match_entry, ofp_match, NULL );
  expect_value( mock_insert_match_entry, priority32, 0 );
  expect_string( mock_insert_match_entry, service_name, "hub" );
  expect_string( mock_insert_match_entry, entry_name, "filter-any" );
  will_return_void( mock_insert_match_entry );

  will_return_void( mock_init_trema );
  will_return( mock_set_packet_in_handler, true );
  will_return_void( mock_start_trema );

  optind = 1;
  ret = packetin_filter_main( argc, argv );

  assert_true( ret == EXIT_SUCCESS );

  teardown();
}


static void
test_packetin_filter_main_invalid_match_type() {
  setup();

  char *argv[] = {
      ( char * ) ( uintptr_t ) "packetin_filter",
      ( char * ) ( uintptr_t ) "INVALID_MATCH_TYPE::dummy_service_name",
      NULL,
    };
  int argc = ARRAY_SIZE( argv ) - 1;
  int ret;

  will_return( mock_printf2, 1 );
  will_return_void( mock_init_trema );

  optind = 1;
  ret = packetin_filter_main( argc, argv );

  assert_true( ret == EXIT_FAILURE );

  teardown();
}


/*************************************************************************
 * Run tests.
 *************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_usage ),
    unit_test( test_handle_packet_in_successed ),
    unit_test( test_handle_packet_in_lookup_failed ),
    unit_test( test_handle_packet_in_send_failed ),
    unit_test( test_register_dl_type_filter ),
    unit_test( test_register_any_filter ),
    unit_test( test_packetin_filter_main_successed ),
    unit_test( test_packetin_filter_main_invalid_match_type ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
