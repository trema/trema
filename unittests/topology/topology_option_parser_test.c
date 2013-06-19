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


#include <assert.h>

#include "checks.h"
#include "cmockery_trema.h"

#include "topology_option_parser.h"


/********************************************************************************
 * Common function.
 ********************************************************************************/


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

#define swap_original( funcname ) \
  original_##funcname = funcname;\
  funcname = mock_##funcname;

#define revert_original( funcname ) \
  funcname = original_##funcname;


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/


/********************************************************************************
 * Tests.
 ********************************************************************************/
static uint8_t lldp_default_dst[ ETH_ADDRLEN ] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e };

static void
test_default() {
  char progname[256] = "progname";
  char *argv_[] = {progname, };
  char **argv = argv_;
  int argc = 1;


  topology_options options;
  parse_options( &options, &argc, &argv );

  assert_int_equal( options.service.ping_ageout_cycles, 5 );
  assert_int_equal( options.service.ping_interval_sec, 60 );

  assert_false( options.discovery.always_enabled );
  assert_false( options.discovery.lldp.lldp_over_ip );
  assert_memory_equal( options.discovery.lldp.lldp_mac_dst, lldp_default_dst, ETH_ADDRLEN );
}

//{ "liveness_wait", required_argument, NULL, 'w'},
//{ "liveness_limit", required_argument, NULL, 'e'},
static void
test_ping_interval_long() {
  char progname[256] = "progname";
  char liveness_wait[] = "--liveness_wait";
  char liveness_wait_arg[] = "15";
  char liveness_limit[] = "--liveness_limit";
  char liveness_limit_arg[] = "1";
  char *argv_[] = {
      progname,
      liveness_wait, liveness_wait_arg,
      liveness_limit, liveness_limit_arg,
  };
  char **argv = argv_;
  int argc = 1+2+2;

  topology_options options;
  parse_options( &options, &argc, &argv );

  assert_int_equal( options.service.ping_ageout_cycles, 1 );
  assert_int_equal( options.service.ping_interval_sec, 15 );

  assert_false( options.discovery.always_enabled );
  assert_false( options.discovery.lldp.lldp_over_ip );
  assert_memory_equal( options.discovery.lldp.lldp_mac_dst, lldp_default_dst, ETH_ADDRLEN );
}


//{ "always_run_discovery", no_argument, NULL, 'a'},
static void
test_always_run_discovery() {
  char progname[256] = "progname";
  char always_run_discovery[] = "--always_run_discovery";
  char *argv_[] = { progname, always_run_discovery };
  char **argv = argv_;
  int argc = 1+1;


  topology_options options;
  parse_options( &options, &argc, &argv );

  assert_int_equal( options.service.ping_ageout_cycles, 5 );
  assert_int_equal( options.service.ping_interval_sec, 60 );

  assert_true( options.discovery.always_enabled );
  assert_false( options.discovery.lldp.lldp_over_ip );
  assert_memory_equal( options.discovery.lldp.lldp_mac_dst, lldp_default_dst, ETH_ADDRLEN );
}

//{ "lldp_mac_dst", required_argument, NULL, 'm' },
static void
test_lldp_mac_dst() {
  char progname[256] = "progname";
  char lldp_mac_dst[] = "--lldp_mac_dst";
  char lldp_mac_dst_arg[] = "01:02:03:04:05:06";
  char *argv_[] = { progname, lldp_mac_dst, lldp_mac_dst_arg };
  char **argv = argv_;
  int argc = 1+2;

  static const uint8_t lldp_mac_dst_expected[ ETH_ADDRLEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };

  topology_options options;
  parse_options( &options, &argc, &argv );

  assert_int_equal( options.service.ping_ageout_cycles, 5 );
  assert_int_equal( options.service.ping_interval_sec, 60 );

  assert_false( options.discovery.always_enabled );
  assert_false( options.discovery.lldp.lldp_over_ip );
  assert_memory_equal( options.discovery.lldp.lldp_mac_dst, lldp_mac_dst_expected, ETH_ADDRLEN );
}

//{ "lldp_over_ip", no_argument, NULL, 'i' },
//{ "lldp_ip_src", required_argument, NULL, 'o' },
//{ "lldp_ip_dst", required_argument, NULL, 'r' },
static void
test_lldp_over_ip() {
  char progname[256] = "progname";
  char lldp_over_ip[] = "--lldp_over_ip";
  char lldp_ip_src[] = "--lldp_ip_src";
  char lldp_ip_src_arg[] = "127.0.0.1";
  char lldp_ip_dst[] = "--lldp_ip_dst";
  char lldp_ip_dst_arg[] = "192.168.0.1";
  char *argv_[] = {
      progname,
      lldp_over_ip,
      lldp_ip_src, lldp_ip_src_arg,
      lldp_ip_dst, lldp_ip_dst_arg,
  };
  char **argv = argv_;
  int argc = 1+1+2+2;

  uint32_t lldp_ip_src_expected = 0;
  lldp_ip_src_expected = 127;
  lldp_ip_src_expected <<= 8;
  lldp_ip_src_expected += 0;
  lldp_ip_src_expected <<= 8;
  lldp_ip_src_expected += 0;
  lldp_ip_src_expected <<= 8;
  lldp_ip_src_expected += 1;

  uint32_t lldp_ip_dst_expected = 0;
  lldp_ip_dst_expected = 192;
  lldp_ip_dst_expected <<= 8;
  lldp_ip_dst_expected += 168;
  lldp_ip_dst_expected <<= 8;
  lldp_ip_dst_expected += 0;
  lldp_ip_dst_expected <<= 8;
  lldp_ip_dst_expected += 1;

  topology_options options;
  parse_options( &options, &argc, &argv );

  assert_int_equal( options.service.ping_ageout_cycles, 5 );
  assert_int_equal( options.service.ping_interval_sec, 60 );

  assert_false( options.discovery.always_enabled );
  assert_true( options.discovery.lldp.lldp_over_ip );
  assert_memory_equal( options.discovery.lldp.lldp_mac_dst, lldp_default_dst, ETH_ADDRLEN );

  assert_int_equal( options.discovery.lldp.lldp_ip_src, lldp_ip_src_expected );
  assert_int_equal( options.discovery.lldp.lldp_ip_dst, lldp_ip_dst_expected );
}
/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test( test_default ),
      unit_test( test_ping_interval_long ),
      unit_test( test_always_run_discovery ),
      unit_test( test_lldp_mac_dst ),
      unit_test( test_lldp_over_ip ),
  };

  setup_leak_detector();
  return run_tests( tests );
}

