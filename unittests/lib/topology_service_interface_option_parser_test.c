/*
 * libtopology_test.c
 *
 *  Created on: 2012/11/22
 *      Author: y-higuchi
 */

#include <assert.h>



#include "checks.h"
#include "cmockery_trema.h"
#include "trema.h"
#include "topology_service_interface_option_parser.h"



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


//void topology_service_interface_usage( const char *progname, const char *description, const char *additional_options );

//void init_topology_service_interface_options( int *argc, char **argv[] );
//void finalize_topology_service_interface_options( void );
//const char *get_topology_service_interface_name( void );
static void
test_init_topology_service_interface_options_no_args() {
  int argc = 1;
  char progname[256] = "progname";
  char *argv_[] = {progname, };
  char **argv = argv_;
  init_topology_service_interface_options( &argc, &argv );

  assert_string_equal("topology", get_topology_service_interface_name() );

  finalize_topology_service_interface_options();
}

static void
test_init_topology_service_interface_options_specify_topology_service_name_short() {
  int argc = 3;
  char progname[256] = "progname";
  char option[3] = "-t";
  char topo_name[256] = "topology_custom";
  char *argv_[] = {progname, option, topo_name, };
  char **argv = argv_;

  init_topology_service_interface_options( &argc, &argv );

  assert_string_equal("topology_custom", get_topology_service_interface_name() );

  finalize_topology_service_interface_options();
}

static void
test_init_topology_service_interface_options_specify_topology_service_name_long() {
  int argc = 2;
  char progname[256] = "progname";
  char option[256] = "--topology=topology_custom";
  char *argv_[] = {progname, option, };
  char **argv = argv_;

  init_topology_service_interface_options( &argc, &argv );

  assert_string_equal("topology_custom", get_topology_service_interface_name() );

  finalize_topology_service_interface_options();
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
      unit_test( test_init_topology_service_interface_options_no_args ),
      unit_test( test_init_topology_service_interface_options_specify_topology_service_name_short ),
      unit_test( test_init_topology_service_interface_options_specify_topology_service_name_long ),
  };
  setup_leak_detector();
  return run_tests( tests );
}

