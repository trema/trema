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


#include <assert.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "path.h"
#include "flow_manager_interface.h"
#include "path_utils.h"
#include "trema.h"

typedef struct {
  hop public;
  void *r_hop_pointer;
  void *r_extra_actions_pointer;
} hop_private;

typedef struct {
  path public;
  uint64_t id;
  uint64_t in_datapath_id;
  setup_handler setup_callback;
  void *setup_user_data;
  teardown_handler teardown_callback;
  void *teardown_user_data;
} path_private;


typedef struct {
  hash_table *id;
  hash_table *match;
} path_table;


static path_table path_db = { NULL, NULL };

static char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


static bool
compare_group_id( const void *x, const void *y ) {
  return ( *( const uint64_t * ) x == *( const uint64_t * ) y ) ? true : false;
}


static unsigned int
hash_group_id( const void *key ) {
  return hash_core( key, ( int ) sizeof( uint64_t ) );
}


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


static unsigned int
hash_path_private( const void *key ) {
  return hash_core( key, ( int ) offsetof( path, idle_timeout ) );
}


static bool
create_path_db( void ) {
  path_db.id = create_hash( compare_group_id, hash_group_id );
  path_db.match = create_hash( compare_path_private, hash_path_private );

  return true;
}


static bool
delete_path_db( void ) {
  hash_iterator iter;
  hash_entry *e;

  init_hash_iterator( path_db.id, &iter );

  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    void *value = delete_hash_entry( path_db.id, e->key );
    delete_hash_entry( path_db.match, value );
    xfree( value );
  }
  delete_hash( path_db.id );
  path_db.id = NULL;
  delete_hash( path_db.match );
  path_db.match = NULL;

  return true;
}


static path_private *
lookup_path_entry_by_id( uint64_t id ) {
  return lookup_hash_entry( path_db.id, &id );
}


static bool
add_path_entry( path_private *path ) {
  if ( lookup_path_entry_by_id( path->id ) != NULL ) {
    error( "Duplicated Path ID ( %#" PRIx64 " ).", path->id );
    return false;
  }

  insert_hash_entry( path_db.id, &path->id, path );
  insert_hash_entry( path_db.match, path, path );

  return true;
}


static bool
delete_path_entry( uint64_t id ) {

  /*
  if(path_db.id == NULL)
  {
	  return false;
  }
  */

  path_private *deleted = delete_hash_entry( path_db.id, &id );

  if ( deleted == NULL ) {
    return false;
  }
  delete_hash_entry( path_db.match, deleted );

  // NOTE: path entry must be freed by user.

  return true;
}


static void
set_service_name( void ) {
  snprintf( service_name, sizeof( service_name ), "libpath.%u", getpid() );
  service_name[ sizeof( service_name ) - 1 ] = '\0';
}


static const char *
get_service_name( void ) {
  return service_name;
}

/*
hop *
create_hop( uint64_t datapath_id, uint16_t in_port, uint16_t out_port, openflow_actions *extra_actions ) {
  hop *h = xmalloc( sizeof( hop ) );

  memset( h, 0, sizeof( hop ) );
  h->datapath_id = datapath_id;
  h->in_port = in_port;
  h->out_port = out_port;
  h->extra_actions = NULL;

  if ( extra_actions != NULL ) {
    h->extra_actions = create_actions();
    list_element *element = extra_actions->list;
    while ( element != NULL ) {
      struct ofp_action_header *ah = element->data;
      void *action = xmalloc( ah->len );
      memcpy( action, ah, ah->len );
      append_to_tail( &h->extra_actions->list, action );
      h->extra_actions->n_actions++;
      element = element->next;
    }
  }

  return h;
}

void
delete_hop( hop *hop ) {
  printf("hop %p will be deleted\n", hop );
  if ( hop->extra_actions != NULL ) {
    printf("actions %p will be deleted\n", hop->extra_actions );
    delete_actions( hop->extra_actions );
  }
  xfree( hop );
}
*/

hop *
create_hop( uint64_t datapath_id, uint16_t in_port, uint16_t out_port, openflow_actions *extra_actions ) {
  hop_private *h = xmalloc( sizeof( hop_private ) );

  memset( h, 0, sizeof( hop_private ) );
  h->public.datapath_id = datapath_id;
  h->public.in_port = in_port;
  h->public.out_port = out_port;
  h->public.extra_actions = NULL;

  if ( extra_actions != NULL ) {
    h->public.extra_actions = create_actions();
    list_element *element = extra_actions->list;
    while ( element != NULL ) {
      struct ofp_action_header *ah = element->data;
      void *action = xmalloc( ah->len );
      memcpy( action, ah, ah->len );
      append_to_tail( &h->public.extra_actions->list, action );
      h->public.extra_actions->n_actions++;
      element = element->next;
    }
  }

  return &h->public;
}


void
delete_hop( hop *hop ) {
  hop_private *private = ( hop_private * ) hop;
  if ( hop->extra_actions != NULL ) {
    delete_actions( hop->extra_actions );
  }
  xfree( private );
}


path *
create_path( struct ofp_match match, uint16_t priority, uint16_t idle_timeout, uint16_t hard_timeout ) {
  path_private *path = xmalloc( sizeof( path_private ) );

  memset( path, 0, sizeof( path_private ) );
  path->public.match = match;
  path->public.priority = priority;
  path->public.idle_timeout = idle_timeout;
  path->public.hard_timeout = hard_timeout;
  path->public.n_hops = 0;
  create_list( &path->public.hops );

  return &path->public;
}


void
append_hop_to_path( path *path, hop *hop ) {
  path_private *private = ( path_private * ) path;

  if ( path->n_hops == 0 ) {
    private->in_datapath_id = hop->datapath_id;
  }

  append_to_tail( &path->hops, hop );
  path->n_hops++;
}


void delete_path( path *path ) {

  path_private *private = ( path_private * ) path;
  list_element *element = path->hops;

  while ( element != NULL ) {
    delete_hop( element->data );
    element = element->next;
  }

  delete_list( path->hops );
  xfree( private );
}

/*
hop *
copy_hop( const hop *hop ) {
  return create_hop( hop->datapath_id, hop->in_port, hop->out_port, hop->extra_actions );
}
*/

hop *
copy_hop( const hop *hop ) {
  const hop_private *private = ( const hop_private * ) hop;
  hop_private *copied = ( hop_private * ) create_hop( hop->datapath_id, hop->in_port, hop->out_port, hop->extra_actions );

  copied->r_hop_pointer = private->r_hop_pointer;
  if(private->r_extra_actions_pointer != NULL)
  {
      copied->r_extra_actions_pointer = private->r_extra_actions_pointer;
  }
  return &copied->public;
}


path *
copy_path( const path *path ) {
  const path_private *private = ( const path_private * ) path;
  path_private *copied = ( path_private * ) create_path( path->match, path->priority,
                                                         path->idle_timeout,
                                                         path->hard_timeout );
  copied->id = private->id;
  copied->in_datapath_id = private->in_datapath_id;
  copied->setup_callback = private->setup_callback;
  copied->setup_user_data = private->setup_user_data;
  copied->teardown_callback = private->teardown_callback;
  copied->teardown_user_data = private->teardown_user_data;

  list_element *element = path->hops;
  while( element != NULL ) {
    append_hop_to_path( &copied->public, copy_hop( element->data ) );
    element = element->next;
  }

  return &copied->public;
}


static void
setup_completed( uint64_t id, int status, void *user_data ) {
  UNUSED( id );

  path_private *entry = user_data;

  if ( status == SUCCEEDED ) {
    bool ret = add_path_entry( entry );
    if ( ret == false ) {
      error( "Failed to add a path entry." );
    }
  }

  switch ( status ) {
    case SUCCEEDED:
      status = SETUP_SUCCEEDED;
      break;
    case CONFLICTED_ID:
    case CONFLICTED_ENTRY:
      status = SETUP_CONFLICTED_ENTRY;
      break;
    case SWITCH_ERROR:
      status = SETUP_SWITCH_ERROR;
      break;
    default:
      status = SETUP_UNKNOWN_ERROR;
      break;
  }

  if ( entry->setup_callback != NULL ) {
    entry->setup_callback( status, ( path * ) entry, entry->setup_user_data );
  }

  if ( status != SETUP_SUCCEEDED ) {
    delete_path( ( path * ) entry );
  }
}


static bool
send_setup_request( path_private *path ) {
  if ( path->public.n_hops <= 0 ) {
    error( "Invalid hop number ( n_hops = %d ).", path->public.n_hops );
    return false;
  }

  buffer *entries = alloc_buffer_with_length( 1024 );
  uint16_t margin = 0;
  list_element *element = path->public.hops;
  while ( element != NULL ) {
    hop *h = element->data;
    struct ofp_match match = path->public.match;
    match.in_port = h->in_port;
    match.wildcards &= ( OFPFW_ALL & ~OFPFW_IN_PORT );

    // TODO: Update match structure based on extra_actions

    uint16_t idle_timeout = path->public.idle_timeout;
    if ( idle_timeout != 0 ) {
      if ( idle_timeout + margin < UINT16_MAX ) {
        idle_timeout = ( uint16_t ) ( idle_timeout + margin );
      }
      else {
        idle_timeout = UINT16_MAX;
      }
    }
    uint16_t hard_timeout = path->public.hard_timeout;
    if ( hard_timeout != 0 ) {
      if ( hard_timeout + margin < UINT16_MAX ) {
        hard_timeout = ( uint16_t ) ( hard_timeout + margin );
      }
      else {
        hard_timeout = UINT16_MAX;
      }
    }
    margin++;

    openflow_actions *actions = h->extra_actions;
    if ( h->out_port != OFPP_NONE && h->out_port != 0 ) {
      if ( actions == NULL ) {
        actions = create_actions();
      }
      append_action_output( actions, h->out_port, UINT16_MAX );
    }
    buffer *entry = create_flow_entry( h->datapath_id, match, path->public.priority,
                                       idle_timeout, hard_timeout, actions );
    void *p = append_back_buffer( entries, entry->length );
    memcpy( p, entry->data, entry->length );
    if ( h->out_port != OFPP_NONE && h->out_port != 0 ) {
      if ( h->extra_actions == NULL ) {
        delete_actions( actions );
      }
      else {
        actions->n_actions--;
        // FIXME: free memory properly.
      }
    }
    free_buffer( entry );
    element = element->next;
  }

  buffer *request = create_flow_entry_group_setup_request( path->id, get_service_name(),
                                                           ( uint16_t ) path->public.n_hops, entries );
  free_buffer( entries );

  bool ret = send_request_message( FLOW_MANAGEMENT_SERVICE,
                                   get_service_name(),
                                   MESSENGER_FLOW_ENTRY_GROUP_SETUP_REQUEST,
                                   request->data, request->length, path );
  free_buffer( request );

  return ret;
}


bool
setup_path( path *p, setup_handler setup_callback, void *setup_user_data,
            teardown_handler teardown_callback, void *teardown_user_data ) {

  //path_private *private = ( path_private * ) copy_path( p );
  //uint64_t in_datapath_id = private->in_datapath_id;

  path_private *temp = (path_private *) p;
  uint64_t in_datapath_id = temp->in_datapath_id;

  if ( lookup_path( in_datapath_id, p->match, p->priority ) != NULL ) {
    error( "Duplicated path found." );
    return false;
  }

  temp = NULL;
  path_private *private = ( path_private * ) copy_path( p );

  private->id = get_flow_entry_group_id();
  private->setup_callback = setup_callback;
  private->setup_user_data = setup_user_data;
  private->teardown_callback = teardown_callback;
  private->teardown_user_data = teardown_user_data;

  return send_setup_request( private );
}


static void
teardown_completed( uint64_t id, int reason ) {
  path_private *entry = lookup_path_entry_by_id( id );
  if ( entry == NULL ) {
    return;
  }

  if ( entry->teardown_callback != NULL ) {
    entry->teardown_callback( reason, ( path * ) entry, entry->teardown_user_data );
  }

  delete_path_entry( id );
  delete_path( ( path * ) entry );
}


static bool
send_teardown_request( uint64_t id ) {
  buffer *request = create_flow_entry_group_teardown_request( id );

  bool ret = send_request_message( FLOW_MANAGEMENT_SERVICE,
                                   get_service_name(),
                                   MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REQUEST,
                                   request->data, request->length, NULL );
  free_buffer( request );

  return ret;
}


bool
teardown_path( uint64_t in_datapath_id, struct ofp_match match, uint16_t priority ) {
  const path_private *path = ( const path_private * ) lookup_path( in_datapath_id, match, priority );
  if ( path == NULL ) {
    return false;
  }

  return send_teardown_request( path->id );
}


bool
teardown_path_by_match( struct ofp_match match ) {
  bool ret = true;
  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( path_db.id, &iter );
  while ( ( ( e = iterate_hash_next( &iter ) ) != NULL ) ) {
    path_private *entry = e->value;
    if ( compare_match( &entry->public.match, &match ) ) {
      if ( !send_teardown_request( entry->id ) ) {
        ret = false;
      }
    }
  }

  return ret;
}


const path *
lookup_path( uint64_t in_datapath_id, struct ofp_match match, uint16_t priority ) {
  path_private criteria;
  criteria.public.match = match;
  criteria.public.priority = priority;
  criteria.in_datapath_id = in_datapath_id;

  path_private *entry = lookup_hash_entry( path_db.match, &criteria );
  if ( entry == NULL ) {
    return NULL;
  }

  return ( const path * ) entry;
}


bool
lookup_path_by_match( struct ofp_match match, int *n_paths, path **paths ) {
  int max_paths = *n_paths;
  *n_paths = 0;
  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( path_db.id, &iter );
  while ( ( ( e = iterate_hash_next( &iter ) ) != NULL ) ) {
    path_private *entry = e->value;
    if ( compare_match( &entry->public.match, &match ) ) {
      if ( *n_paths < max_paths ) {
        paths[ *n_paths ] = ( path * ) entry;
      }
      ( *n_paths )++;
    }
  }

  if ( *n_paths >= max_paths ) {
    warn( "Too many paths found ( n_paths = %d, max_paths = %d ).", *n_paths, max_paths );
    *n_paths = max_paths;

    return false;
  }

  return true;
}


static void
handle_notification( uint16_t tag, void *data, size_t length ) {
  if ( tag != MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN ) {
    return;
  }

  if ( length != sizeof( flow_entry_group_teardown ) ) {
    error( "Invalid teardown notification ( length = %u ).", length );
    return;
  }

  flow_entry_group_teardown *notification = data;
  teardown_completed( notification->id, notification->reason );
}


static void
handle_reply( uint16_t tag, void *data, size_t length, void *user_data ) {
  switch ( tag ) {
    case MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY:
    {
      if ( length != sizeof( flow_entry_group_setup_reply ) ) {
        error( "Invalid setup reply ( length = %u ).", length );
        return;
      }
      flow_entry_group_setup_reply *reply = data;
      if ( reply->status != SUCCEEDED ) {
        error( "Failed to setup reply ( id = %#" PRIx64 ", status = %#x ).", reply->id, reply->status );
      }
      setup_completed( reply->id, reply->status, user_data );
    }
    break;
    case MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY:
    {
      if ( length != sizeof( flow_entry_group_teardown_reply ) ) {
        error( "Invalid teardown reply ( length = %u ).", length );
        return;
      }
      flow_entry_group_teardown_reply *reply = data;
      if ( reply->status != SUCCEEDED ) {
        error( "Failed to tear down ( id = %#" PRIx64 ", status = %#x ).", reply->id, reply->status );
      }
      // FIXME: notify to caller here?
      path_private *entry = lookup_path_entry_by_id( reply->id );
      if ( entry != NULL ) {
        delete_path_entry( reply->id );
        delete_path( ( path * ) entry );
      }
    }
    break;
    default:
    {
      error( "Undefined reply tag ( tag = %#x, length = %u ).", tag, length );
    }
    return;
  }
}


bool
init_path( void ) {

  create_path_db();
  set_service_name();
  add_message_received_callback( get_service_name(), handle_notification );
  add_message_replied_callback( get_service_name(), handle_reply );

  return true;
}

static void
stop_flow_manager()
{
  char buf[50];
  FILE *fp;
  char *pid;
  char str[50];

  if((fp = fopen("./tmp/pid/flow_manager.pid", "r")) == NULL)
  {
     exit(2);
  }

  if((pid = fgets(buf, 50, fp)) != NULL)
  {
    sprintf(str, "kill -2 %s", pid );
    system(str);
  }

  fclose(fp);
  system("./trema killall");
}

bool
finalize_path( void ) {

  UNUSED(stop_flow_manager);
  delete_path_db();
  delete_message_received_callback( get_service_name(), handle_notification );
  delete_message_replied_callback( get_service_name(), handle_reply );
  return true;
}
