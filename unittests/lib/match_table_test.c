/*
 * Unit tests for match table.
 *
 * Author: Kazushi SUGYO
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


#include <net/ethernet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "ether.h"
#include "log.h"
#include "match_table.h"


typedef struct match_table {
  hash_table *exact_table; // no wildcards are set
  list_element *wildcard_table; // wildcard flags are set
  pthread_mutex_t *mutex;
} match_table;


extern match_table match_table_head;


/*************************************************************************
 * Setup and teardown function.                                          
 *************************************************************************/


static void
setup() {
  // FIXME: fake logging!
  init_log( "match_table_test", "/tmp", true );
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


/*************************************************************************
 * Test functions.
 *************************************************************************/

static void
test_init_and_finalize_match_table_successed() {
  setup();

  assert_true( match_table_head.exact_table == NULL );
  assert_true( match_table_head.wildcard_table == NULL );

  init_match_table();
  assert_true( match_table_head.exact_table != NULL );
  assert_true( match_table_head.wildcard_table == NULL );

  finalize_match_table();
  assert_true( match_table_head.exact_table == NULL );
  assert_true( match_table_head.wildcard_table == NULL );

  teardown();
}


static void
set_any_match_entry( struct ofp_match *match ) {
  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = OFPFW_ALL;
}


static void
set_lldp_match_entry( struct ofp_match *match ) {
  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = OFPFW_ALL & ~OFPFW_DL_TYPE;
  match->dl_type = ETH_ETHTYPE_LLDP;
}


static void
set_ipv4_match_entry( struct ofp_match *match ) {
  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = OFPFW_ALL & ~OFPFW_DL_TYPE;
  match->dl_type = ETHERTYPE_IP;
}


#define ANY_MATCH_PRIORITY 0x0
#define ANY_MATCH_SERVICE_NAME "service-name-any"
#define ANY_MATCH_ENTRY_NAME "entry-name-any"

#define LLDP_MATCH_PRIORITY 0x8000
#define LLDP_MATCH_SERVICE_NAME "service-name-lldp"
#define LLDP_MATCH_ENTRY_NAME "entry-name-lldp"

#define IPV4_MATCH_PRIORITY 0x4000
#define IPV4_MATCH_SERVICE_NAME "service-name-ipv4"
#define IPV4_MATCH_ENTRY_NAME "entry-name-ipv4"

static void
intsert_any_match_entry() {
  struct ofp_match match;

  set_any_match_entry( &match );
  insert_match_entry( &match, ANY_MATCH_PRIORITY, ANY_MATCH_SERVICE_NAME, ANY_MATCH_ENTRY_NAME );
}


static void
delete_any_match_entry() {
  struct ofp_match match;

  set_any_match_entry( &match );
  delete_match_entry( &match );
}


static void
intsert_lldp_match_entry() {
  struct ofp_match match;

  set_lldp_match_entry( &match );
  insert_match_entry( &match, LLDP_MATCH_PRIORITY, LLDP_MATCH_SERVICE_NAME, LLDP_MATCH_ENTRY_NAME );
}


static void
delete_lldp_match_entry() {
  struct ofp_match match;

  set_lldp_match_entry( &match );
  delete_match_entry( &match );
}


static void
intsert_ipv4_match_entry() {
  struct ofp_match match;

  set_ipv4_match_entry( &match );
  insert_match_entry( &match, IPV4_MATCH_PRIORITY, IPV4_MATCH_SERVICE_NAME, IPV4_MATCH_ENTRY_NAME );
}


static void
delete_ipv4_match_entry() {
  struct ofp_match match;

  set_ipv4_match_entry( &match );
  delete_match_entry( &match );
}


static void
test_insert_and_lookup_of_wildcard_any_entry() {
  setup();

  struct ofp_match match, lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_any_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  set_any_match_entry( &match );
  assert_memory_equal( &match_entry->ofp_match, &match, sizeof( struct ofp_match ) );
  assert_true( match_entry->priority == ANY_MATCH_PRIORITY );
  assert_string_equal( match_entry->service_name, ANY_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ANY_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_wildcard_any_lldp_entry() {
  setup();

  struct ofp_match match, lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_any_match_entry();
  intsert_lldp_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  set_any_match_entry( &match );
  assert_memory_equal( &match_entry->ofp_match, &match, sizeof( struct ofp_match ) );
  assert_true( match_entry->priority == ANY_MATCH_PRIORITY );
  assert_string_equal( match_entry->service_name, ANY_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ANY_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_wildcard_lldp_any_entry() {
  setup();

  struct ofp_match match, lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_lldp_match_entry();
  intsert_any_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  set_any_match_entry( &match );
  assert_memory_equal( &match_entry->ofp_match, &match, sizeof( struct ofp_match ) );
  assert_true( match_entry->priority == ANY_MATCH_PRIORITY );
  assert_string_equal( match_entry->service_name, ANY_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ANY_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_wildcard_any_lldp_ipv4_entry() {
  setup();

  struct ofp_match match, lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_any_match_entry();
  intsert_lldp_match_entry();
  intsert_ipv4_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  set_any_match_entry( &match );
  assert_memory_equal( &match_entry->ofp_match, &match, sizeof( struct ofp_match ) );
  assert_true( match_entry->priority == ANY_MATCH_PRIORITY );
  assert_string_equal( match_entry->service_name, ANY_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ANY_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_wildcard_lldp_entry() {
  setup();

  struct ofp_match match, lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_lldp_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );
  lookup_match.dl_type = ETH_ETHTYPE_LLDP;

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  set_lldp_match_entry( &match );
  assert_memory_equal( &match_entry->ofp_match, &match, sizeof( struct ofp_match ) );
  assert_true( match_entry->priority == LLDP_MATCH_PRIORITY );
  assert_string_equal( match_entry->service_name, LLDP_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, LLDP_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_lookup_of_wildcard_lldp_entry_failed() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_wildcard_lldp_entry_failed() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_lldp_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


static void
test_delete_of_wildcard_any_entry_failed() {
  setup();

  init_match_table();

  delete_any_match_entry();
  assert_true( match_table_head.wildcard_table == NULL );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_delete_of_wildcard_any_entry() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_any_match_entry();
  delete_any_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_delete_of_wildcard_any_lldp_ipv4_entry() {
  setup();

  struct ofp_match match, lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_any_match_entry();
  intsert_lldp_match_entry();
  intsert_ipv4_match_entry();

  delete_ipv4_match_entry();
  delete_any_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );
  lookup_match.dl_type = ETH_ETHTYPE_LLDP;

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  set_lldp_match_entry( &match );
  assert_memory_equal( &match_entry->ofp_match, &match, sizeof( struct ofp_match ) );
  assert_true( match_entry->priority == LLDP_MATCH_PRIORITY );
  assert_string_equal( match_entry->service_name, LLDP_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, LLDP_MATCH_ENTRY_NAME );

  delete_lldp_match_entry();

  memset( &lookup_match, 0, sizeof( struct ofp_match ) );
  lookup_match.dl_type = ETH_ETHTYPE_LLDP;

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


static void
set_alice_match_entry( struct ofp_match *match ) {
  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = 0;
  match->dl_type = ETHERTYPE_IP;
  match->nw_src = 0x0a000001;
  match->nw_dst = 0x0a000002;
}


static void
set_bob_match_entry( struct ofp_match *match ) {
  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = 0;
  match->dl_type = ETHERTYPE_IP;
  match->nw_src = 0x0a000002;
  match->nw_dst = 0x0a000001;
}


static void
set_carol_match_entry( struct ofp_match *match ) {
  memset( match, 0, sizeof( struct ofp_match ) );
  match->wildcards = 0;
  match->dl_type = ETHERTYPE_IP;
  match->nw_src = 0x0a000003;
  match->nw_dst = 0xffffffff;
}


#define ALICE_MATCH_PRIORITY 0x0
#define ALICE_MATCH_SERVICE_NAME "service-name-alice"
#define ALICE_MATCH_ENTRY_NAME "entry-name-alice"

#define BOB_MATCH_PRIORITY 0x0
#define BOB_MATCH_SERVICE_NAME "service-name-bob"
#define BOB_MATCH_ENTRY_NAME "entry-name-bob"

#define CAROL_MATCH_PRIORITY 0x0
#define CAROL_MATCH_SERVICE_NAME "service-name-carol"
#define CAROL_MATCH_ENTRY_NAME "entry-name-carol"

#define MALLORY_MATCH_PRIORITY 0x0
#define MALLORY_MATCH_SERVICE_NAME "service-name-mallory"
#define MALLORY_MATCH_ENTRY_NAME "entry-name-mallory"

static void
intsert_alice_match_entry() {
  struct ofp_match match;

  set_alice_match_entry( &match );
  insert_match_entry( &match, ALICE_MATCH_PRIORITY, ALICE_MATCH_SERVICE_NAME, ALICE_MATCH_ENTRY_NAME );
}


static void
delete_alice_match_entry() {
  struct ofp_match match;

  set_alice_match_entry( &match );
  delete_match_entry( &match );
}


static void
intsert_bob_match_entry() {
  struct ofp_match match;

  set_bob_match_entry( &match );
  insert_match_entry( &match, BOB_MATCH_PRIORITY, BOB_MATCH_SERVICE_NAME, BOB_MATCH_ENTRY_NAME );
}


static void
delete_bob_match_entry() {
  struct ofp_match match;

  set_bob_match_entry( &match );
  delete_match_entry( &match );
}


static void
intsert_carol_match_entry() {
  struct ofp_match match;

  set_carol_match_entry( &match );
  insert_match_entry( &match, CAROL_MATCH_PRIORITY, CAROL_MATCH_SERVICE_NAME, CAROL_MATCH_ENTRY_NAME );
}


static void
delete_carol_match_entry() {
  struct ofp_match match;

  set_carol_match_entry( &match );
  delete_match_entry( &match );
}


static void
intsert_mallory_match_entry() {
  struct ofp_match match;

  set_alice_match_entry( &match );
  insert_match_entry( &match, MALLORY_MATCH_PRIORITY, MALLORY_MATCH_SERVICE_NAME, MALLORY_MATCH_ENTRY_NAME );
}


static void
test_insert_and_lookup_of_exact_alice_entry() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_alice_match_entry();

  set_alice_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  assert_memory_equal( &match_entry->ofp_match, &lookup_match, sizeof( struct ofp_match ) );
  assert_string_equal( match_entry->service_name, ALICE_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ALICE_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_exact_alice_entry_conflict() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_alice_match_entry();
  intsert_mallory_match_entry();

  set_alice_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  assert_memory_equal( &match_entry->ofp_match, &lookup_match, sizeof( struct ofp_match ) );
  assert_string_equal( match_entry->service_name, ALICE_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ALICE_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_exact_all_entry() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_alice_match_entry();
  intsert_bob_match_entry();
  intsert_carol_match_entry();

  set_alice_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  assert_memory_equal( &match_entry->ofp_match, &lookup_match, sizeof( struct ofp_match ) );
  assert_string_equal( match_entry->service_name, ALICE_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, ALICE_MATCH_ENTRY_NAME );

  set_bob_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  assert_memory_equal( &match_entry->ofp_match, &lookup_match, sizeof( struct ofp_match ) );
  assert_string_equal( match_entry->service_name, BOB_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, BOB_MATCH_ENTRY_NAME );

  set_carol_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry != NULL );

  assert_memory_equal( &match_entry->ofp_match, &lookup_match, sizeof( struct ofp_match ) );
  assert_string_equal( match_entry->service_name, CAROL_MATCH_SERVICE_NAME );
  assert_string_equal( match_entry->entry_name, CAROL_MATCH_ENTRY_NAME );

  finalize_match_table();

  teardown();
}


static void
test_lookup_of_exact_alice_entry_failed() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  set_alice_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


static void
test_insert_and_lookup_of_exact_alice_entry_failed() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_bob_match_entry();
  intsert_carol_match_entry();

  set_alice_match_entry( &lookup_match );

  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


static void
test_delete_of_exact_alice_entry_failed() {
  setup();

  init_match_table();

  delete_alice_match_entry();

  finalize_match_table();

  teardown();
}


static void
test_insert_and_delete_of_exact_all_entry_failed() {
  setup();

  struct ofp_match lookup_match;
  match_entry *match_entry;

  init_match_table();

  intsert_alice_match_entry();
  intsert_bob_match_entry();
  intsert_carol_match_entry();

  delete_carol_match_entry();
  set_carol_match_entry( &lookup_match );
  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  delete_bob_match_entry();
  set_bob_match_entry( &lookup_match );
  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  delete_alice_match_entry();
  set_alice_match_entry( &lookup_match );
  match_entry = lookup_match_entry( &lookup_match );
  assert_true( match_entry == NULL );

  finalize_match_table();

  teardown();
}


/*************************************************************************
 * Run tests.
 *************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_init_and_finalize_match_table_successed ),
    unit_test( test_insert_and_lookup_of_wildcard_any_entry ),
    unit_test( test_insert_and_lookup_of_wildcard_any_lldp_entry ),
    unit_test( test_insert_and_lookup_of_wildcard_any_lldp_ipv4_entry ),
    unit_test( test_insert_and_lookup_of_wildcard_lldp_any_entry ),
    unit_test( test_insert_and_lookup_of_wildcard_lldp_entry ),
    unit_test( test_lookup_of_wildcard_lldp_entry_failed ),
    unit_test( test_insert_and_lookup_of_wildcard_lldp_entry_failed ),
    unit_test( test_delete_of_wildcard_any_entry_failed ),
    unit_test( test_insert_and_delete_of_wildcard_any_entry ),
    unit_test( test_insert_and_delete_of_wildcard_any_lldp_ipv4_entry ),
    unit_test( test_insert_and_lookup_of_exact_alice_entry ),
    unit_test( test_insert_and_lookup_of_exact_alice_entry_conflict ),
    unit_test( test_insert_and_lookup_of_exact_all_entry ),
    unit_test( test_lookup_of_exact_alice_entry_failed ),
    unit_test( test_insert_and_lookup_of_exact_alice_entry_failed ),
    unit_test( test_delete_of_exact_alice_entry_failed ),
    unit_test( test_insert_and_delete_of_exact_all_entry_failed ),
  };

  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
