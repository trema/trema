/*
 * Copyright (C) 2011 NEC Corporation
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


#ifndef PATH_H
#define PATH_H


#include <inttypes.h>
#include "trema.h"

typedef struct {
  uint64_t datapath_id;
  uint16_t in_port;
  uint16_t out_port;
  openflow_actions *extra_actions;
} hop;

typedef struct {
  struct ofp_match match;
  uint16_t priority;
  uint16_t idle_timeout;
  uint16_t hard_timeout;
  int n_hops;
  list_element *hops;
} path;

enum {
  SETUP_SUCCEEDED,
  SETUP_CONFLICTED_ENTRY,
  SETUP_SWITCH_ERROR,
  SETUP_UNKNOWN_ERROR,
};

typedef void ( *setup_handler )(
  int status,
  const path *path,
  void *user_data
);

enum {
  TEARDOWN_TIMEOUT,
  TEARDOWN_MANUALLY_REQUESTED,
};

typedef void ( *teardown_handler )(
  int reason,
  const path *path,
  void *user_data
);

/*
 * Create new hop.
 *
 * @param [uint64_t] datapath_id datapath id.
 *
 * @param [uint16_t] in_port in_port.
 *
 * @param [uint16_t] out_port out_port.
 *
 * @param [openflow_actions] openflow_actions openflow_actions.
 *
 * @return [hop] created hop.
 */
hop *create_hop( uint64_t datapath_id, uint16_t in_port, uint16_t out_port, openflow_actions *extra_actions );

/*
 * Delete a hop.
 *
 * @param [hop *] hop hop pointer.
 *
 * @return [void] return void.
 */
void delete_hop( hop *hop );

/*
 * Copy a hop.
 *
 * @param [hop *] hop hop pointer.
 *
 * @return [hop] return copied hop.
 */
hop *copy_hop( const hop *hop );

/*
 * Create a path.
 *
 * @param [struct ofp_match] match match.
 *
 * @param [uint16_t] priority priority.
 *
 * @param [uint16_t] idle_timeout idle_timeout.
 *
 * @param [uint16_t] hard_timeout hard_timeout.
 *
 * @return [path] created path.
 */
path *create_path( struct ofp_match match, uint16_t priority, uint16_t idle_timeout, uint16_t hard_timeout );

/*
 * Append a hop to a path.
 *
 * @param [path *] path path pointer.
 *
 * @param [hop *] hop hop pointer.
 *
 * @return [void] void.
 */
void append_hop_to_path( path *path, hop *hop );

/*
 * Delete a path.
 *
 * @param [path *] path path pointer.
 *
 * @return [void] void.
 */
void delete_path( path *path );

/*
 * Copy a path.
 *
 * @param [path *] path path pointer.
 *
 * @return [path] copied path.
 */
path *copy_path( const path *path );

/*
 * Setup a path.
 *
 * @param [path *] path path pointer.
 *
 * @param [setup_handler] setup_callback setup_callback.
 *
 * @param [void *] setup_user_data any user data you want to pass to the handler.
 *
 * @param [teardown_handler] teardown_callback teardown_callback.
 *
 * @param [void *] teardown_user_data any user data you want to pass to the handler.
 *
 * @return [bool] The result of if its setup signal was sent. This doesn't represent that setup was success. Please check it through setup_callback.
 */
bool setup_path( path *path, setup_handler setup_callback, void *setup_user_data,
                 teardown_handler teardown_callback, void *teardown_user_data );

/*
 * Teardown a path.
 *
 * @param [uint64_t] in_datapath_id in_datapath_id.
 *
 * @param [struct ofp_match] match match.
 *
 * @param [uint16_t] priority priority.
 *
 * @return [bool] The result of if its teardown signal was sent. This doesn't represent that teardown was success. Please check it through teardown_callback.
 */
bool teardown_path( uint64_t in_datapath_id, struct ofp_match match, uint16_t priority );

/*
 * Teardown a path by a match.
 *
 * @param [struct ofp_match match] match match.
 *
 * @return [bool] The result of if its teardown signal was sent. This doesn't represent that teardown was success. Please check it through teardown_callback.
 */
bool teardown_path_by_match( struct ofp_match match );

/*
 * Lookup a path.
 *
 * @param [uint64_t] in_datapath_id in_datapath_id.
 *
 * @param [struct ofp_match] match match.
 *
 * @param [uint16_t] priority priority.
 *
 * @return [path *] path.
 */
const path *lookup_path( uint64_t in_datapath_id, struct ofp_match match, uint16_t priority );

/*
 * Lookup a path.
 *
 * @param [ struct ofp_match] match match.
 *
 * @param [int *] n_paths The number of paths.
 *
 * @param [path **] paths The passes you looked up.
 *
 * @return [bool] The result of if the lookup was success or not.
 */
bool lookup_path_by_match( struct ofp_match match, int *n_paths, path **paths );

/*
 * Initalize this function. Please init before you use it.
 *
 * @return [bool] The result of if the init was success or not.
 */
bool init_path( void );

/*
 * Finalize this function. Please finalize after you use it.
 *
 * @return [bool] The result of if the finalize was success or not.
 */
bool finalize_path( void );


#endif // PATH_H
