#include <assert.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <mcheck.h>
#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "path.h"
#include "event_handler.h"
#include "messenger.h"
#include "path_utils.h"
#include "flow_manager_interface.h"
#include "match.h"
#include "openflow.h"
#include "trema.h"

#define FLOW_MANAGER_NAME "flow_manager_test_service"
#define INIT_PATH "./tmp/sock"

//This structure exists in "path.c" also.
//Here, we include it for the test.
typedef struct {
  path public;
  uint64_t id;
  uint64_t in_datapath_id;
  setup_handler setup_callback;
  void *setup_user_data;
  teardown_handler teardown_callback;
  void *teardown_user_data;
} path_private;

int _argc;
char **_argv;

/*******************************
 * Test helper functions
 *******************************/
static bool
compare_path_private( const void *x, const void *y ) {
  const path_private *path_x = x;
  const path_private *path_y = y;

  if ( compare_match_strict( &path_x->public.match, &path_y->public.match ) &&
       ( path_x->public.priority == path_y->public.priority ) &&
       ( path_x->in_datapath_id == path_y->in_datapath_id ) ) {
    return true;
  }

  return false;
}

static void
start_flow_manager_for_test()
{
  system( "./build.rb" );
  system( "./trema run -c src/examples/flow_manager_example/flow_manager_example.conf -d" );
  start_flow_manager();
}

static void
stop_flow_manager_for_test()
{
  system("./trema killall");
}


/***********************************
 * Test functions
 ***********************************/
static void
test_init_and_finalize_path()
{
  init_messenger( "/tmp" );
  init_timer();

  assert_true(init_path());
  assert_true(finalize_path());

  finalize_timer();
  finalize_messenger();
}

static void
test_create_hop_with_actions()
{
  init_messenger( "/tmp" );
  init_timer();
  init_path();

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;
  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );

  hop *h = create_hop(datapath_id, in_port, out_port, actions);
  assert_int_equal(h->datapath_id, datapath_id);
  assert_int_equal(h->in_port, in_port);
  assert_int_equal(h->out_port, out_port);
  assert_int_equal(h->extra_actions->n_actions, actions->n_actions);

  struct ofp_action_header *ah = h->extra_actions->list->data;
  assert_int_equal(ah->type, OFPAT_OUTPUT);
  struct ofp_action_output *ofp_output = (struct ofp_action_output*)h->extra_actions->list->data;
  assert_int_equal(ofp_output->port, out_port);
  assert_int_equal(ofp_output->max_len, UINT16_MAX);

  delete_hop(h);
  delete_actions(actions);

  finalize_path();
  finalize_timer();
  finalize_messenger();
}

static void
test_delete_hop_with_actions()
{
  init_messenger( "/tmp" );
  init_timer();
  init_path();

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;
  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );

  hop *h = create_hop(datapath_id, in_port, out_port, actions);

  //The test is confirmed with no memory leak.
  delete_hop(h);
  delete_actions(actions);

  finalize_path();
  finalize_timer();
  finalize_messenger();
}

static void
test_create_hop_without_actions()
{
  init_messenger( "/tmp" );
  init_timer();
  init_path();

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;

  hop *h = create_hop(datapath_id, in_port, out_port, NULL);
  assert_int_equal(h->datapath_id, datapath_id);
  assert_int_equal(h->in_port, in_port);
  assert_int_equal(h->out_port, out_port);
  assert_true(h->extra_actions == NULL);

  delete_hop(h);

  finalize_path();
  finalize_timer();
  finalize_messenger();
}

static void
test_delete_hop_without_actions()
{
  init_messenger( "/tmp" );
  init_timer();
  init_path();

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;

  hop *h = create_hop(datapath_id, in_port, out_port, NULL);

  //The test is confirmed with no memory leak.
  delete_hop(h);

  finalize_path();
  finalize_timer();
  finalize_messenger();
}

static void
test_copy_hop()
{
  init_messenger( "/tmp" );
  init_timer();
  assert_true(init_path());

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;

  openflow_actions *actions = create_actions();
  append_action_output( actions, 2, UINT16_MAX );

  hop *h = create_hop(datapath_id, in_port, out_port, actions);
  hop *h2 = copy_hop(h);
  assert_int_equal(h2->datapath_id, datapath_id);
  assert_int_equal(h2->in_port, in_port);
  assert_int_equal(h2->out_port, out_port);
  assert_int_equal(h2->extra_actions->n_actions, actions->n_actions);
  assert_int_not_equal(h, h2);

  struct ofp_action_header *ah = h->extra_actions->list->data;
  assert_int_equal(ah->type, OFPAT_OUTPUT);
  struct ofp_action_output *ofp_output = (struct ofp_action_output*)h->extra_actions->list->data;
  assert_int_equal(ofp_output->port, out_port);
  assert_int_equal(ofp_output->max_len, UINT16_MAX);

  delete_hop(h);
  delete_actions(actions);
  delete_hop(h2);

  assert_true(finalize_path());
  finalize_timer();
  finalize_messenger();
}

static void
test_create_path()
{
  init_messenger( "/tmp" );
  init_timer();
  assert_true(init_path());

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 20;

  path *p = create_path( match, priority, idle_timeout, hard_timeout );
  assert( p != NULL );
  assert_int_equal(p->priority, priority);
  assert_int_equal(p->idle_timeout, idle_timeout);
  assert_int_equal(p->hard_timeout, hard_timeout);
  assert_int_equal(p->n_hops, 0);
  assert_memory_equal(&p->match, &match, sizeof( struct ofp_match ) );

  delete_path(p);

  assert_true(finalize_path());
  finalize_timer();
  finalize_messenger();
}

static void
test_append_hop_to_path()
{
  init_messenger( "/tmp" );
  init_timer();
  assert_true(init_path());

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;

  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );
  hop *h = create_hop(datapath_id, in_port, out_port, actions);

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;
  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 20;
  path *p = create_path( match, priority, idle_timeout, hard_timeout );

  append_hop_to_path(p, h);
  assert_int_equal(p->n_hops, 1);
  hop *h2 = (hop*)p->hops->data;

  assert_int_equal(h2->datapath_id, datapath_id);
  assert_int_equal(h2->in_port, in_port);
  assert_int_equal(h2->out_port, out_port);

  struct ofp_action_header *ah = h2->extra_actions->list->data;
  assert_int_equal(ah->type, OFPAT_OUTPUT);
  struct ofp_action_output *ofp_output = (struct ofp_action_output*)h2->extra_actions->list->data;
  assert_int_equal(ofp_output->port, out_port);
  assert_int_equal(ofp_output->max_len, UINT16_MAX);

  delete_path(p);
  delete_actions(actions);

  assert_true(finalize_path());
  finalize_timer();
  finalize_messenger();
}

static void
test_delete_path()
{
  init_messenger( "/tmp" );
  init_timer();
  assert_true(init_path());

  openflow_actions *actions = create_actions();
  append_action_output( actions, 2, UINT16_MAX );
  hop *h = create_hop(0x1, 1, 2, actions);

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;
  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 20;
  path *p = create_path( match, priority, idle_timeout, hard_timeout );

  append_hop_to_path(p, h);

  delete_path(p);
  delete_actions(actions);

  finalize_path();
  finalize_timer();
  finalize_messenger();
}

static void
test_copy_path()
{
  init_messenger( "/tmp" );
  init_timer();
  assert_true(init_path());

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;

  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );
  hop *h = create_hop(datapath_id, in_port, out_port, actions);

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 20;
  path *p = create_path( match, priority, idle_timeout, hard_timeout );

  append_hop_to_path(p, h);

  path *p2 = copy_path(p);

  assert_int_equal(p2->priority, p->priority);
  assert_int_equal(p2->idle_timeout, p->idle_timeout);
  assert_int_equal(p2->hard_timeout, p->hard_timeout);
  assert_int_equal(p2->n_hops, p->n_hops);
  assert_int_not_equal(p, p2);
  assert_memory_equal(&p2->match, &p->match, sizeof(struct ofp_match));

  hop *post_hop1 = (hop*)p->hops->data;
  hop *post_hop2 = (hop*)p2->hops->data;

  assert_int_equal(post_hop1->datapath_id, post_hop2->datapath_id);
  assert_int_equal(post_hop1->in_port, post_hop2->in_port);
  assert_int_equal(post_hop1->out_port, post_hop2->out_port);

  struct ofp_action_header *ah1 = post_hop1->extra_actions->list->data;
  struct ofp_action_header *ah2 = post_hop2->extra_actions->list->data;
  assert_int_equal(ah1->type, ah2->type);
  struct ofp_action_output *ofp_output1 = (struct ofp_action_output*)post_hop1->extra_actions->list->data;
  struct ofp_action_output *ofp_output2 = (struct ofp_action_output*)post_hop2->extra_actions->list->data;
  assert_memory_equal(ofp_output1, ofp_output2, sizeof(struct ofp_action_output));

  delete_path(p);
  delete_path(p2);
  delete_actions(actions);

  assert_true(finalize_path());
  finalize_timer();
  finalize_messenger();
}

/*
 * Test for setup_path
 */
static void
handle_setup( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  check_expected(status);
  check_expected(_path);
  UNUSED(user_data);
}


static void
handle_teardown( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  check_expected(reason);
  check_expected(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry( void *user_data ){

  UNUSED( user_data );

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;

  openflow_actions *actions = create_actions();
  append_action_output( actions, out_port, UINT16_MAX );
  hop *h = create_hop(datapath_id, in_port, out_port, actions);

  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 5;
  uint16_t hard_timeout = 10;

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;
  path *p = create_path( match, priority, idle_timeout, hard_timeout );
  append_hop_to_path( p, h );

  expect_value(handle_setup, status, SUCCEEDED);
  //We put -4 because the last 4 bits are pointer and they don't match.
  expect_memory(handle_setup, _path, p, sizeof(path) - 4);
  expect_value(handle_teardown, reason, SUCCEEDED);
  //We put -4 because the last 4 bits are pointer and they don't match.
  expect_memory(handle_teardown, _path, p, sizeof(path) - 4);

  setup_path( p, handle_setup, NULL, handle_teardown, NULL );

  delete_path(p);
  delete_actions(actions);
}


static void
test_setup_path()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry, NULL );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry, NULL);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();

}
/*
 * end
 */

/*
 * Test for setup_path with duplicate path
 */
static void
install_duplicate_flow_entry( void *user_data );

static void
handle_teardown_duplicate( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(user_data);
  UNUSED(_path);

  stop_event_handler();
  stop_messenger();
}

static void
handle_setup_duplicate( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(_path);
  UNUSED(user_data);

  path* p = (path *)user_data;

  setup_path( p, NULL, NULL, NULL, NULL);
}

static void
install_duplicate_flow_entry( void *user_data ){

  UNUSED( user_data );

  path* p = (path *)user_data;

  setup_path( p, handle_setup_duplicate, user_data, handle_teardown_duplicate, user_data );
}

static void
test_setup_path_duplicate()
{

  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  uint64_t datapath_id = 0x1;
  uint16_t in_port = 1;
  uint16_t out_port = 2;
  hop *h = create_hop(datapath_id, in_port, out_port, NULL);

  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 5;
  uint16_t hard_timeout = 5;
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;
  path *p = create_path( match, priority, idle_timeout, hard_timeout );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_duplicate_flow_entry, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_duplicate_flow_entry, NULL);
  delete_path(p);
  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();

}
/*
 * end
 */

/*
 * Test for teardown_path
 */

static void
handle_setup_for_teardown_test( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(user_data);

  path_private* pp = (path_private*)user_data;

  assert_true(teardown_path(pp->in_datapath_id, _path->match, _path->priority));
}


static void
handle_teardown_for_teardown_test( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_teardown_test( void *user_data ){

  path *p = (path*)user_data;

  setup_path( p, handle_setup_for_teardown_test, p, handle_teardown_for_teardown_test, p);

}

static void
test_teardown_path()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 300, 300 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_teardown_test, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry, NULL);

  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}

/*
 * end
 */

/*
 * Test for teardown_path with not found
 */

static void
handle_setup_for_teardown_test_not_found( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(_path);

  path_private* pp = (path_private*)user_data;

  uint64_t in_datapath_id = 0x5;
  uint16_t priority = 100;
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  assert_false(teardown_path(in_datapath_id, match, priority));
  assert_true(teardown_path(pp->in_datapath_id, _path->match, _path->priority));
}


static void
handle_teardown_for_teardown_test_not_found( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_teardown_test_not_found( void *user_data ){

  path *p = (path*)user_data;

  setup_path( p, handle_setup_for_teardown_test_not_found, p, handle_teardown_for_teardown_test_not_found, p);

}

static void
test_teardown_path_not_found()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 10, 10 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_teardown_test_not_found, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry_for_teardown_test_not_found, NULL);

  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}

/*
 * end
 */

/*
 * Test for teardown_path_by_match
 */
static void
handle_setup_for_teardown_path_by_match_test( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(user_data);

  assert_true(teardown_path_by_match(_path->match));
}


static void
handle_teardown_for_teardown_path_by_match_test( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_teardown_path_by_match_test( void *user_data ){

  path *p = (path*)user_data;
  setup_path( p, handle_setup_for_teardown_path_by_match_test, p, handle_teardown_for_teardown_path_by_match_test, p);

}

static void
test_teardown_path_by_match()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 300, 300 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_teardown_path_by_match_test, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry, NULL);

  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}
/*
 * end
 */

/*
 * Test for test_lookup_path
 */

static void
handle_setup_for_test_lookup_path( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  path_private* pp = (path_private*)user_data;

  const path *p = lookup_path(pp->in_datapath_id, _path->match ,_path->priority);
  assert_true(compare_path_private(p, _path));
}


static void
handle_teardown_for_test_lookup_path( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_test_lookup_path( void *user_data ){

  path *p = (path*)user_data;
  setup_path( p, handle_setup_for_test_lookup_path, p, handle_teardown_for_test_lookup_path, p);

}

static void
test_lookup_path()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 5, 10 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_test_lookup_path, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry, NULL);
  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}

/*
 * end
 */

/*
 * Test for test_lookup_path_not_found
 */

static void
handle_setup_for_test_lookup_path_not_found( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(user_data);

  uint64_t in_datapath_id = 0x5;
  uint16_t priority = 100;
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  path_private* pp = (path_private*)user_data;

  const path *p = lookup_path(in_datapath_id, match , priority);
  assert_true(p == NULL);

  const path *p2 = lookup_path(pp->in_datapath_id, _path->match ,_path->priority);
  assert_true(compare_path_private(p2, _path));
}


static void
handle_teardown_for_test_lookup_path_not_found( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_test_lookup_path_not_found( void *user_data ){

  path *p = (path*)user_data;
  setup_path( p, handle_setup_for_test_lookup_path_not_found, p, handle_teardown_for_test_lookup_path_not_found, p);

}

static void
test_lookup_path_not_found()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 5, 10 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_test_lookup_path_not_found, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry_for_test_lookup_path_not_found, NULL);
  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}

/*
 * end
 */

/*
 * Test for test_lookup_path_not_found
 */

static void
handle_setup_for_test_lookup_path_by_match( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(user_data);

  path *paths[16];
  int n_paths = 16;

  assert_true(lookup_path_by_match(_path->match, &n_paths, paths));

  for(int i = 0; i < n_paths; i++)
  {
    assert_true(compare_path_private(_path, paths[i]));
  }
}


static void
handle_teardown_for_test_lookup_path_by_match( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_test_lookup_path_by_match( void *user_data ){

  path *p = (path*)user_data;
  setup_path( p, handle_setup_for_test_lookup_path_by_match, p, handle_teardown_for_test_lookup_path_by_match, p);

}

static void
test_lookup_path_by_match()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 5, 10 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_test_lookup_path_by_match, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry_for_test_lookup_path_by_match, NULL);
  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}

/*
 * end
 */


/*
 * Test for test_lookup_path_not_found
 */

static void
handle_setup_for_test_lookup_path_by_match_max_paths_is_too_short( int status, const path *_path, void *user_data ) {
  puts( "**** Path setup completed *** ");
  UNUSED(status);
  UNUSED(user_data);

  path *paths[1];
  int n_paths = 0;

  assert_false(lookup_path_by_match(_path->match, &n_paths, paths));
}


static void
handle_teardown_for_test_lookup_path_by_match_max_paths_is_too_short( int reason, const path *_path, void *user_data ) {
  puts( "**** Path teardown completed *** ");
  UNUSED(reason);
  UNUSED(_path);
  UNUSED(user_data);

  stop_event_handler();
  stop_messenger();
}

static void
install_flow_entry_for_test_lookup_path_by_match_max_paths_is_too_short( void *user_data ){

  path *p = (path*)user_data;
  setup_path( p, handle_setup_for_test_lookup_path_by_match_max_paths_is_too_short, p, handle_teardown_for_test_lookup_path_by_match_max_paths_is_too_short, p);

}

static void
test_lookup_path_by_match_max_paths_is_too_short()
{
  init_messenger( INIT_PATH );
  init_timer();
  init_path();

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  hop *h = create_hop( 0x1, 1, 2, NULL );
  path *p = create_path( match, UINT16_MAX, 5, 10 );
  append_hop_to_path( p, h );

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry_for_test_lookup_path_by_match_max_paths_is_too_short, p );

  // Main loop
  start_messenger();
  start_event_handler();

  delete_timer_event(install_flow_entry_for_test_lookup_path_by_match_max_paths_is_too_short, NULL);
  delete_path(p);

  // Finalize path management library (path)
  finalize_path();
  finalize_timer();
  finalize_messenger();
}

/*
 * end
 */

/************************************************************
 *  Test for flow_manager.c
 *
 *  enum {
 *   MESSENGER_FLOW_ENTRY_GROUP_SETUP_REQUEST = 0x9000,
 *   MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
 *   MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REQUEST,
 *   MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
 *   MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN,
 *  };
 ************************************************************/

static void
handle_notification_for_flow_entry_group_setup_request( uint16_t tag, void *data, size_t length ) {

  check_expected(tag);
  check_expected(length);

  flow_entry_group_teardown *notification = data;
  uint64_t id = notification->id;
  uint8_t reason = notification->reason;

  check_expected(id);
  check_expected(reason);

  stop_event_handler();
  stop_messenger();
}

static void
handle_reply_for_flow_entry_group_setup_request( uint16_t tag, void *data, size_t length, void *user_data ) {

  check_expected(tag);
  check_expected(length);

  flow_entry_group_setup_reply *reply = data;
  uint64_t id = reply->id;
  uint8_t status = reply->status;

  check_expected(id);
  check_expected(status);
  UNUSED(user_data);
}

static void
test_flow_manager_flow_entry_group_setup_request() {

  void* user_data = NULL;

  init_messenger( INIT_PATH );
  init_timer();

  assert_true(add_message_received_callback( FLOW_MANAGER_NAME, handle_notification_for_flow_entry_group_setup_request ));
  assert_true(add_message_replied_callback( FLOW_MANAGER_NAME, handle_reply_for_flow_entry_group_setup_request ));

  buffer *entries = alloc_buffer_with_length( 1024 );
  if(entries == NULL)
  {
    assert( 0 );
  }

  uint64_t id = 1;
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;
  uint16_t priority = 1;
  uint16_t idle_timeout = 5;
  uint16_t hard_timeout = 5;
  openflow_actions *actions = create_actions();
  append_action_output( actions, 2, UINT16_MAX );
  buffer *entry = create_flow_entry( id, match, priority, idle_timeout, hard_timeout, actions );
  void *p = append_back_buffer( entries, entry->length );
  memcpy( p, entry->data, entry->length );
  uint64_t gid = 10000;
  buffer *request = create_flow_entry_group_setup_request( gid, FLOW_MANAGER_NAME,
                                                           1, entries );

  expect_value(handle_reply_for_flow_entry_group_setup_request, tag, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY);
  expect_value(handle_reply_for_flow_entry_group_setup_request, length, sizeof( flow_entry_group_setup_reply ));
  expect_value(handle_reply_for_flow_entry_group_setup_request, status, 0);
  expect_value(handle_reply_for_flow_entry_group_setup_request, id, gid );

  expect_value(handle_notification_for_flow_entry_group_setup_request, tag, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN);
  expect_value(handle_notification_for_flow_entry_group_setup_request, length, sizeof( flow_entry_group_teardown ) );
  expect_value(handle_notification_for_flow_entry_group_setup_request, reason, 0);
  expect_value(handle_notification_for_flow_entry_group_setup_request, id, gid );

  assert_true( send_request_message( FLOW_MANAGEMENT_SERVICE,
                                      FLOW_MANAGER_NAME,
                                    MESSENGER_FLOW_ENTRY_GROUP_SETUP_REQUEST,
                                    request->data, request->length, user_data ) );

  free_buffer(entries);
  delete_actions(actions);
  free_buffer( entry );
  free_buffer( request );

  start_messenger();
  start_event_handler();

  assert_true(delete_message_received_callback( FLOW_MANAGER_NAME, handle_notification_for_flow_entry_group_setup_request ));
  assert_true(delete_message_replied_callback( FLOW_MANAGER_NAME, handle_reply_for_flow_entry_group_setup_request ));

  finalize_timer();
  finalize_messenger();
}

static void
handle_notification_for_flow_entry_group_teardown_request( uint16_t tag, void *data, size_t length ) {

  check_expected(tag);
  check_expected(length);

  flow_entry_group_teardown *notification = data;
  uint64_t id = notification->id;
  uint8_t reason = notification->reason;

  check_expected(id);
  check_expected(reason);

  stop_event_handler();
  stop_messenger();
}

static void
handle_reply_for_flow_entry_group_teardown_request( uint16_t tag, void *data, size_t length, void *user_data ) {

  check_expected(tag);
  check_expected(length);

  flow_entry_group_setup_reply *reply = data;
  uint64_t id = reply->id;
  uint8_t status = reply->status;

  check_expected(id);
  check_expected(status);

  if(status == 4){
    stop_event_handler();
    stop_messenger();
  }

  UNUSED(user_data);
}

static void
test_flow_manager_flow_entry_group_teardown_request() {

  void* user_data = NULL;

  init_messenger( INIT_PATH );
  init_timer();

  assert_true(add_message_received_callback( FLOW_MANAGER_NAME, handle_notification_for_flow_entry_group_teardown_request ));
  assert_true(add_message_replied_callback( FLOW_MANAGER_NAME, handle_reply_for_flow_entry_group_teardown_request ));

  uint64_t gid = 10000;
  buffer *trerdown_request = create_flow_entry_group_teardown_request(gid);

  expect_value(handle_reply_for_flow_entry_group_teardown_request, tag, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY);
  expect_value(handle_reply_for_flow_entry_group_teardown_request, length, sizeof( flow_entry_group_teardown_reply ));
  expect_value(handle_reply_for_flow_entry_group_teardown_request, status, 4);
  expect_value(handle_reply_for_flow_entry_group_teardown_request, id, gid );

  /*
  expect_value(handle_notification_for_flow_entry_group_teardown_request, tag, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN);
  expect_value(handle_notification_for_flow_entry_group_teardown_request, length, sizeof( flow_entry_group_teardown ) );
  expect_value(handle_notification_for_flow_entry_group_teardown_request, reason, 4);
  expect_value(handle_notification_for_flow_entry_group_teardown_request, id, gid );
  */

  assert_true( send_request_message( FLOW_MANAGEMENT_SERVICE,
                                      FLOW_MANAGER_NAME,
                                      MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REQUEST,
                                      trerdown_request->data, trerdown_request->length, user_data ) );

  free_buffer( trerdown_request );

  start_messenger();
  start_event_handler();

  assert_true(delete_message_received_callback( FLOW_MANAGER_NAME, handle_notification_for_flow_entry_group_teardown_request ));
  assert_true(delete_message_replied_callback( FLOW_MANAGER_NAME, handle_reply_for_flow_entry_group_teardown_request ));

  finalize_timer();
  finalize_messenger();
}

/*
 * Pleaase check the result by manual.
 */
static void
test_flow_entry_request_undefined(){

  void* user_data = NULL;

  init_messenger( INIT_PATH );
  init_timer();

  assert_true( send_request_message( FLOW_MANAGEMENT_SERVICE,
                                      FLOW_MANAGER_NAME,
                                    MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                                    user_data, 0, user_data ) );

  start_messenger();
  start_event_handler();

  finalize_timer();
  finalize_messenger();
}

/*
 * Pleaase check the result by manual.
 */
static void
test_flow_entry_group_teardown_request_too_short(){
  void* user_data = NULL;

  init_messenger( INIT_PATH );
  init_timer();

  assert_true( send_request_message( FLOW_MANAGEMENT_SERVICE,
                                      FLOW_MANAGER_NAME,
                                    MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REQUEST,
                                    user_data, 0, user_data ) );

  start_messenger();
  start_event_handler();

  finalize_timer();
  finalize_messenger();
}

/*
 * Pleaase check the result by manual.
 */
static void
test_flow_entry_group_setup_request_too_short(){
  void* user_data = NULL;

  init_messenger( INIT_PATH );
  init_timer();

  assert_true( send_request_message( FLOW_MANAGEMENT_SERVICE,
                                      FLOW_MANAGER_NAME,
                                    MESSENGER_FLOW_ENTRY_GROUP_SETUP_REQUEST,
                                    user_data, 0, user_data ) );

  start_messenger();
  start_event_handler();

  finalize_timer();
  finalize_messenger();
}

/***********************************
 * test for path_utils.h
 ***********************************/

static void
test_dump_hop()
{
  //This is output method, so please test it by manual.

  hop *h = create_hop(0x1, 1, 2, NULL);
  dump_hop(h);

  delete_hop(h);
}

static void
test_dump_hop_with_extra_actions()
{
  //This is output method, so please test it by manual.
  openflow_actions *actions = create_actions();
  append_action_output( actions, 1, UINT16_MAX );

  hop *h = create_hop(0x1, 1, 2, actions);
  dump_hop(h);

  delete_actions(actions);
  delete_hop(h);
}

static void
test_dump_path()
{
  ///This is output method, so please test it by manual.

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  uint16_t priority = UINT16_MAX;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 20;
  path *p = create_path( match, priority, idle_timeout, hard_timeout );

  hop *h1 = create_hop( 0x1, 1, 2, NULL);
  append_hop_to_path( p, h1 );

  dump_path(p);
  delete_path(p);
}

static void
test_dump_match()
{
  ///This is output method, so please test it by manual.
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  dump_match(&match);
}

static void
test_status_to_string()
{
  //This value is for getting undefined value.
  int undefined_number = 5;

  assert_string_equal(status_to_string(SETUP_SUCCEEDED), "succeeded");
  assert_string_equal(status_to_string(SETUP_CONFLICTED_ENTRY), "conflicted entry");
  assert_string_equal(status_to_string(SETUP_SWITCH_ERROR), "switch error");
  assert_string_equal(status_to_string(undefined_number), "undefined");
}

static void
test_reason_to_string()
{
  //This value is for getting undefined value.
  int undefined_number = 5;

  assert_string_equal(reason_to_string(0), "timeout");
  assert_string_equal(reason_to_string(1), "manually requested");
  assert_string_equal(reason_to_string(undefined_number), "undefined");
}

/******************************************
 * test for flow_manager_interface.h
 ******************************************/
static void
test_get_flow_entry_group_id()
{
  //TODO
}

static void
test_create_flow_entry()
{
  uint64_t id = 1;

  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  uint16_t priority = 1;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 10;

  openflow_actions *actions = create_actions();
  append_action_output( actions, 2, UINT16_MAX );

  buffer *entries = alloc_buffer_with_length( 1024 );

  buffer *entry = create_flow_entry( id, match, priority, idle_timeout, hard_timeout, actions );
  void *p = append_back_buffer( entries, entry->length );
  memcpy( p, entry->data, entry->length );
  flow_entry *fe = (flow_entry*)p;

  struct ofp_action_header *action_header;
  action_header = fe->actions;

  assert_int_equal(fe->priority, 1);
  assert_int_equal(fe->idle_timeout, 10);
  assert_int_equal(fe->hard_timeout, 10);
  assert_int_equal(fe->datapath_id, 1);
  assert_true( compare_match( &match, &fe->match ) );
  assert_int_equal(fe->actions_length, sizeof(struct ofp_action_output) );
  assert_int_equal(action_header->len, 8);
  assert_int_equal(action_header->type, 0);

  free_buffer(entries);
  free_buffer(entry);
  delete_actions(actions);
}

static void
test_create_flow_entry_with_actions_null()
{
  uint64_t id = 1;
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  uint16_t priority = 1;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 10;

  buffer *entries = alloc_buffer_with_length( 1024 );

  buffer *entry = create_flow_entry( id, match, priority, idle_timeout, hard_timeout, NULL );
  void *p = append_back_buffer( entries, entry->length );
  memcpy( p, entry->data, entry->length );
  flow_entry *fe = (flow_entry*)p;

  assert_int_equal(fe->priority, 1);
  assert_int_equal(fe->idle_timeout, 10);
  assert_int_equal(fe->hard_timeout, 10);
  assert_int_equal(fe->datapath_id, 1);
  assert_true( compare_match( &match, &fe->match ) );
  assert_int_equal(fe->actions_length, 0 );

  free_buffer(entries);
  free_buffer(entry);
}

static void
test_create_flow_entry_group_setup_request()
{
  buffer *entries = alloc_buffer_with_length( 1024 );
  buffer *buf = alloc_buffer_with_length( 1024 );

  uint64_t datapath_id = 1;
  struct ofp_match match;
  memset( &match, 0, sizeof( struct ofp_match ) );
  match.wildcards = OFPFW_ALL;

  uint16_t priority = 1;
  uint16_t idle_timeout = 10;
  uint16_t hard_timeout = 10;
  openflow_actions *actions = create_actions();
  append_action_output( actions, 2, UINT16_MAX );

  uint64_t group_id = 10000;
  uint16_t n_hops = 1;

  buffer *entry = create_flow_entry( datapath_id, match, priority, idle_timeout, hard_timeout, actions );
  void *p = append_back_buffer( entries, entry->length );
  memcpy( p, entry->data, entry->length );
  flow_entry *fe = (flow_entry*)p;

  buffer *request = create_flow_entry_group_setup_request( group_id, "test", n_hops, entries );
  void *p2 = append_back_buffer( buf, request->length );
  memcpy( p2, request->data, request->length );
  flow_entry_group_setup_request *fg = (flow_entry_group_setup_request*)p2;

  assert_int_equal(fg->id, group_id);
  assert_int_equal(fg->n_entries, n_hops);
  assert_int_equal(fg->entries_length, entries->length);
  assert_string_equal(fg->owner, "test");
  assert_memory_equal(fg->entries, fe, sizeof(flow_entry));

  free_buffer(entries);
  free_buffer(entry);
  free_buffer(buf);
  free_buffer(request);
  delete_actions(actions);
}

static void
test_create_flow_entry_group_setup_reply()
{
  uint64_t id = 1;
  buffer *replies = alloc_buffer_with_length( 1024 );
  buffer *reply = create_flow_entry_group_setup_reply( id, SUCCEEDED );

  void *p = append_back_buffer( replies, reply->length );
  memcpy( p, reply->data, reply->length );
  flow_entry_group_teardown_reply *fe = (flow_entry_group_teardown_reply*)p;

  assert_int_equal(fe->id, id);
  assert_int_equal(fe->status, SUCCEEDED);

  free_buffer(replies);
  free_buffer(reply);
}


static void
test_create_flow_entry_group_teardown_request()
{
  uint64_t id = 1;
  buffer *requests = alloc_buffer_with_length( 1024 );
  buffer *request = create_flow_entry_group_teardown_request( id );

  void *p = append_back_buffer( requests, request->length );
  memcpy( p, request->data, request->length );
  flow_entry_group_teardown_request *fe = (flow_entry_group_teardown_request*)p;

  assert_int_equal(fe->id, id);

  free_buffer(requests);
  free_buffer(request);
}

static void
test_create_flow_entry_group_teardown_reply()
{
  uint64_t id = 1;
  buffer *replies = alloc_buffer_with_length( 1024 );
  buffer *reply = create_flow_entry_group_teardown_reply( id, SUCCEEDED );

  void *p = append_back_buffer( replies, reply->length );
  memcpy( p, reply->data, reply->length );
  flow_entry_group_teardown_reply *fe = (flow_entry_group_teardown_reply*)p;


  assert_int_equal(fe->id, id);
  assert_int_equal(fe->status, SUCCEEDED);

  free_buffer(replies);
  free_buffer(reply);
}


static void
test_create_flow_entry_group_teardown()
{
  uint64_t id = 1;
  buffer *nitifications = alloc_buffer_with_length( 1024 );
  buffer *notification = create_flow_entry_group_teardown( id, TIMEOUT );

  void *p = append_back_buffer( nitifications, notification->length );
  memcpy( p, notification->data, notification->length );
  flow_entry_group_teardown *fe = (flow_entry_group_teardown*)p;


  assert_int_equal(fe->id, id);
  assert_int_equal(fe->reason, TIMEOUT);

  free_buffer(nitifications);
  free_buffer(notification);
}

/*
static void
test_setup_path()
{
  mtrace();

  init_trema(&_argc, &_argv);
  init_path();

  expect_value(handle_setup, status, SUCCEEDED);
  expect_value(handle_teardown, reason, SUCCEEDED);

  struct itimerspec spec;
  memset( &spec, 0, sizeof( struct itimerspec ) );
  spec.it_value.tv_sec = 1;
  add_timer_event_callback( &spec, install_flow_entry, NULL );

  // Main loop
  start_trema();

  delete_timer_event(install_flow_entry, NULL);

  // Finalize path management library (path)
  finalize_path();

  muntrace();
}
*/

/********************************************************************************
 * Run tests.
 ********************************************************************************/

int main( int argc, char *argv[] ) {

  _argc = argc;
  _argv = argv;

  const UnitTest tests[] = {
    unit_test( test_status_to_string ),
    unit_test( test_reason_to_string ),
    unit_test( test_get_flow_entry_group_id ),
    unit_test( test_dump_hop ),
    unit_test( test_dump_hop_with_extra_actions ),
    unit_test( test_dump_path ),
    unit_test( test_dump_match ),
    unit_test( test_create_path ),
    unit_test( test_delete_path ),
    unit_test( test_append_hop_to_path ),
    unit_test( test_copy_hop ),
    unit_test( test_copy_path ),
    unit_test( test_create_flow_entry ),
    unit_test( test_create_flow_entry_with_actions_null ),
    unit_test( test_create_flow_entry_group_setup_request ),
    unit_test( test_create_flow_entry_group_setup_reply ),
    unit_test( test_create_flow_entry_group_teardown_request ),
    unit_test( test_create_flow_entry_group_teardown_reply ),
    unit_test( test_create_flow_entry_group_teardown ),
    unit_test( test_create_hop_with_actions ),
    unit_test( test_create_hop_without_actions ),
    unit_test( test_delete_hop_without_actions ),
    unit_test( test_delete_hop_with_actions),
    unit_test( test_init_and_finalize_path),
    unit_test_setup_teardown( test_setup_path, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_setup_path_duplicate, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_flow_manager_flow_entry_group_setup_request, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_flow_manager_flow_entry_group_teardown_request, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_teardown_path, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_teardown_path_by_match, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_teardown_path_not_found, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_lookup_path, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_lookup_path_not_found, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_lookup_path_by_match, start_flow_manager_for_test, stop_flow_manager_for_test),
    unit_test_setup_teardown( test_lookup_path_by_match_max_paths_is_too_short, start_flow_manager_for_test, stop_flow_manager_for_test),

    //Please test below functions by manual
    //unit_test( test_flow_entry_request_undefined ),
    //unit_test( test_flow_entry_group_setup_request_too_short ),
    //unit_test( test_flow_entry_group_teardown_request_too_short ),
  };

  UNUSED( test_flow_manager_flow_entry_group_setup_request );
  UNUSED( test_flow_manager_flow_entry_group_teardown_request);
  UNUSED( test_setup_path );
  UNUSED( test_setup_path_duplicate);
  UNUSED( test_teardown_path );
  UNUSED( test_teardown_path_by_match);
  UNUSED( test_teardown_path_not_found );
  UNUSED( test_lookup_path );
  UNUSED( test_lookup_path_not_found );
  UNUSED( test_lookup_path_by_match );
  UNUSED( test_lookup_path_by_match_max_paths_is_too_short );

  UNUSED( test_flow_entry_request_undefined );
  UNUSED( test_flow_entry_group_setup_request_too_short );
  UNUSED( test_flow_entry_group_teardown_request_too_short );

  setup_leak_detector();
  return run_tests( tests );
}
