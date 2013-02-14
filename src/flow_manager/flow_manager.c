/*
 * Flow manager
 *
 * Author: Yasunobu Chiba
 *
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
#include <inttypes.h>
#include <netinet/in.h>
#include "flow_manager_interface.h"
#include "trema.h"


#define ADD_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec + ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec + ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec >= 1000000000 ) {               \
      ( _return )->tv_sec++;                                  \
      ( _return )->tv_nsec -= 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )

#define TIMESPEC_GREATER_THAN( _a, _b )                          \
  ( ( ( _a )->tv_sec == ( _b )->tv_sec ) ?                    \
    ( ( _a )->tv_nsec > ( _b )->tv_nsec ) :                   \
    ( ( _a )->tv_sec > ( _b )->tv_sec ) )


typedef struct {
  hash_table *id;
  hash_table *entry;
  hash_table *entry_flow_mod_xid;
  hash_table *entry_barrier_xid;
} flow_entry_group_table;

flow_entry_group_table flow_entry_group_db = { NULL, NULL, NULL, NULL };

enum {
  INIT,
  INSTALL_IN_PROGRESS,
  INSTALL_CONFIRMED,
  INSTALL_FAILED,
  REMOVE_IN_PROGRESS,
  REMOVE_CONFIRMED,
  REMOVE_FAILED,
};

typedef struct {
  flow_entry *public;
  int state;
  uint32_t flow_mod_xid;
  uint32_t barrier_xid;
  uint64_t group_id;
} flow_entry_private;

typedef struct {
  uint64_t id;
  char owner[ MESSENGER_SERVICE_NAME_LENGTH ];
  int state;
  int n_entries;
  int n_active_entries;
  int n_barriers;
  messenger_context_handle *context_handle;
  list_element *entries;
  struct timespec expires_at;
} flow_entry_group;

static const struct timespec SETUP_TRANSACTION_TIMEOUT = { 5, 0 };
static const struct timespec TEARDOWN_TRANSACTION_TIMEOUT = { 5, 0 };
static const struct timespec FLOW_REMOVED_TRANSACTION_TIMEOUT = { 5, 0 };

static unsigned int hash_flow_entry_private( const void *key );

#define HAVE_NORMALIZE_MATCH 1
#ifndef HAVE_NORMALIZE_MATCH
#define HAVE_NORMALIZE_MATCH

static void
normalize_match( struct ofp_match *match ) {
  char match_string[ 1024 ];
  match_to_string( match, match_string, sizeof( match_string ) );
  debug( "Normalizing match structure ( original match = [%s] ).", match_string );

  memset( match->pad1, 0, sizeof( match->pad1 ) );
  memset( match->pad2, 0, sizeof( match->pad2 ) );

  match->wildcards &= OFPFW_ALL;

  if ( match->wildcards & OFPFW_IN_PORT ) {
    match->in_port = 0;
  }
  if ( match->wildcards & OFPFW_DL_VLAN ) {
    match->dl_vlan = 0;
  }
  else {
    if ( match->dl_vlan == UINT16_MAX ) {
      match->wildcards |= ( uint32_t ) OFPFW_DL_VLAN_PCP;
      match->dl_vlan_pcp = 0;
    }
  }
  if ( match->wildcards & OFPFW_DL_SRC ) {
    memset( match->dl_src, 0, sizeof( match->dl_src ) );
  }
  if ( match->wildcards & OFPFW_DL_DST ) {
    memset( match->dl_dst, 0, sizeof( match->dl_dst ) );
  }
  if ( match->wildcards & OFPFW_DL_TYPE ) {
    match->dl_type = 0;
    match->wildcards |= ( uint32_t ) OFPFW_NW_TOS;
    match->wildcards |= ( uint32_t ) OFPFW_NW_PROTO;
    match->wildcards |= ( uint32_t ) OFPFW_NW_SRC_MASK;
    match->wildcards |= ( uint32_t ) OFPFW_NW_DST_MASK;
    match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
    match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
    match->nw_tos = 0;
    match->nw_proto = 0;
    match->nw_src = 0;
    match->nw_dst = 0;
    match->tp_src = 0;
    match->tp_dst = 0;
  }
  else {
    if ( match->dl_type == ETH_ETHTYPE_ARP ) {
      match->wildcards |= ( uint32_t ) OFPFW_NW_TOS;
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->nw_tos = 0;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
    else if ( match->dl_type != ETH_ETHTYPE_IPV4 ) {
      match->wildcards |= ( uint32_t ) OFPFW_NW_TOS;
      match->wildcards |= ( uint32_t ) OFPFW_NW_PROTO;
      match->wildcards |= ( uint32_t ) OFPFW_NW_SRC_MASK;
      match->wildcards |= ( uint32_t ) OFPFW_NW_DST_MASK;
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->nw_tos = 0;
      match->nw_proto = 0;
      match->nw_src = 0;
      match->nw_dst = 0;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
  }
  if ( match->wildcards & OFPFW_NW_PROTO ) {
    match->nw_proto = 0;
    if ( match->dl_type != ETH_ETHTYPE_IPV4 ) {
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
  }
  else {
    if ( match->dl_type == ETH_ETHTYPE_IPV4 &&
	 match->nw_proto != IPPROTO_TCP && match->nw_proto != IPPROTO_UDP && match->nw_proto != IPPROTO_ICMP ) {
      match->wildcards |= ( uint32_t ) OFPFW_TP_SRC;
      match->wildcards |= ( uint32_t ) OFPFW_TP_DST;
      match->tp_src = 0;
      match->tp_dst = 0;
    }
  }

  if ( ( ( match->wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT ) > 32 ) {
    match->wildcards &= ( uint32_t ) ~OFPFW_NW_SRC_MASK;
    match->wildcards |= OFPFW_NW_SRC_ALL;
  }
  if ( ( ( match->wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT ) > 32 ) {
    match->wildcards &= ( uint32_t ) ~OFPFW_NW_DST_MASK;
    match->wildcards |= OFPFW_NW_DST_ALL;
  }
  if ( match->wildcards & OFPFW_TP_SRC ) {
    match->tp_src = 0;
  }
  if ( match->wildcards & OFPFW_TP_DST ) {
    match->tp_dst = 0;
  }

  if ( match->wildcards & OFPFW_DL_VLAN_PCP ) {
    match->dl_vlan_pcp = 0;
  }
  if ( match->wildcards & OFPFW_NW_TOS ) {
    match->nw_tos = 0;
  }

  match_to_string( match, match_string, sizeof( match_string ) );
  debug( "Normalization completed ( updated match = [%s] ).", match_string );
}
#endif // HAVE_NORMALIZE_MATCH


static messenger_context_handle *
copy_messenger_context_handle( const messenger_context_handle *handle ) {
  size_t length = offsetof( messenger_context_handle, service_name ) + handle->service_name_len;
  messenger_context_handle *copied = xmalloc( length );
  memcpy( copied, handle, length );

  return copied;
}


static void
free_messenger_context_handle( messenger_context_handle *handle ) {
  assert( handle != NULL );
  xfree( handle );
}


static unsigned int
hash_group_id( const void *key ) {
  return ( unsigned int ) *( const uint64_t * ) key;
}


static bool
compare_group_id( const void *x, const void *y ) {
  return ( *( const uint64_t * ) x == *( const uint64_t * ) y ) ? true : false;
}


static unsigned int
hash_flow_entry_private( const void *key ) {
  const flow_entry_private *private = key;
  flow_entry entry = *( private->public );

  normalize_match( &entry.match );

  return hash_core( &entry, offsetof( flow_entry, idle_timeout ) );
}


static bool
compare_flow_entry_private( const void *x, const void *y ) {
  const flow_entry *entry_x = ( ( const flow_entry_private * ) x )->public;
  const flow_entry *entry_y = ( ( const flow_entry_private * ) y )->public;

  struct ofp_match match_x = entry_x->match;
  struct ofp_match match_y = entry_y->match;
  normalize_match( &match_x );
  normalize_match( &match_y );

  if ( compare_match_strict( &match_x, &match_y ) &&
       ( entry_x->datapath_id == entry_y->datapath_id ) &&
       ( entry_x->priority == entry_y->priority ) ) {
    return true;
  }

  return false;
}


static unsigned int
hash_transaction_id( const void *key ) {
  return ( unsigned int ) *( const uint32_t * ) key;
}


static bool
compare_transaction_id( const void *x, const void *y ) {
  return ( *( const uint32_t * ) x  == *( const uint32_t * ) y ) ? true : false;
}


static void
create_flow_entry_group_db( void ) {
  flow_entry_group_db.id = create_hash( compare_group_id, hash_group_id );
  flow_entry_group_db.entry = create_hash( compare_flow_entry_private, hash_flow_entry_private );
  flow_entry_group_db.entry_flow_mod_xid = create_hash( compare_transaction_id,
                                                        hash_transaction_id );
  flow_entry_group_db.entry_barrier_xid = create_hash( compare_transaction_id,
                                                       hash_transaction_id );
}


static flow_entry_private *
lookup_flow_entry_by_flow_mod_xid( uint32_t transaction_id ) {
  debug( "Looking up flow entry by flow_mod transaction ( transaction_id = %#x ).",
         transaction_id );
  return lookup_hash_entry( flow_entry_group_db.entry_flow_mod_xid, &transaction_id );
}


static void
add_flow_mod_transaction( flow_entry_private *entry ) {
  assert( entry != NULL );

  if ( lookup_flow_entry_by_flow_mod_xid( entry->flow_mod_xid ) == NULL ) {
    debug( "Adding flow_mod transaction ( transaction_id = %#x ).", entry->barrier_xid );
    insert_hash_entry( flow_entry_group_db.entry_flow_mod_xid, &entry->flow_mod_xid, entry );
  }
}


static void
delete_flow_mod_transaction( uint32_t transaction_id ) {
  debug( "Deleting flow_mod transaction ( transaction_id = %#x ).", transaction_id );
  delete_hash_entry( flow_entry_group_db.entry_flow_mod_xid, &transaction_id );
}


static flow_entry_private *
lookup_flow_entry_by_barrier_xid( uint32_t transaction_id ) {
  debug( "Looking up flow entry by barrier transaction ( transaction_id = %#x ).",
         transaction_id );
  return lookup_hash_entry( flow_entry_group_db.entry_barrier_xid, &transaction_id );
}


static void
add_barrier_transaction( flow_entry_private *entry ) {
  assert( entry != NULL );

  if ( lookup_flow_entry_by_barrier_xid( entry->barrier_xid ) == NULL ) {
    debug( "Adding barrier transaction ( transaction_id = %#x ).", entry->barrier_xid );
    insert_hash_entry( flow_entry_group_db.entry_barrier_xid, &entry->barrier_xid, entry );
  }
}


static void
delete_barrier_transaction( uint32_t transaction_id ) {
  debug( "Deleting barrier transaction ( transaction_id = %#x ).", transaction_id );
  delete_hash_entry( flow_entry_group_db.entry_barrier_xid, &transaction_id );
}


static flow_entry_private *
lookup_flow_entry( uint64_t datapath_id, struct ofp_match match, uint16_t priority ) {
  flow_entry entry;
  entry.datapath_id = datapath_id;
  entry.match = match;
  entry.priority = priority;
  flow_entry_private private;
  private.public = &entry;

  char match_string[ 256 ];
  match_to_string( &match, match_string, sizeof( match_string ) );
  debug( "Looking up flow entry ( datapath_id = %#" PRIx64 ", match = [%s], priority = %u ).",
         datapath_id, match_string, priority );

  return lookup_hash_entry( flow_entry_group_db.entry, &private );
}


static void
add_flow_entry( flow_entry_private *entry ) {
  assert( entry != NULL );

  if ( lookup_flow_entry( entry->public->datapath_id, entry->public->match,
                          entry->public->priority ) == NULL ) {
    char match_string[ 256 ];
    match_to_string( &entry->public->match, match_string, sizeof( match_string ) );
    debug( "Adding flow entry ( datapath_id = %#" PRIx64 ", match = [%s], priority = %u ).",
           entry->public->datapath_id, match_string, entry->public->priority );
    insert_hash_entry( flow_entry_group_db.entry, entry, entry );
  }
}


static void
delete_flow_entry( uint64_t datapath_id, struct ofp_match match, uint16_t priority ) {
  flow_entry entry;
  entry.datapath_id = datapath_id;
  normalize_match( &match );
  entry.match = match;
  entry.priority = priority;
  flow_entry_private private;
  private.public = &entry;

  char match_string[ 256 ];
  match_to_string( &match, match_string, sizeof( match_string ) );
  debug( "Deleting flow entry ( datapath_id = %#" PRIx64 ", match = [%s], priority = %u ).",
        datapath_id, match_string, priority );
  flow_entry_private *deleted = delete_hash_entry( flow_entry_group_db.entry, &private );
  if ( deleted != NULL ) {
    debug( "Deleted." );
  }
}


static flow_entry_group *
lookup_flow_entry_group( uint64_t id ) {
  debug( "Looking up flow entry group ( id = %#" PRIx64 " ).", id );
  return lookup_hash_entry( flow_entry_group_db.id, &id );
}


static void
add_flow_entry_group( flow_entry_group *group ) {
  assert( group != NULL );

  debug( "Adding flow entry group ( id = %#" PRIx64 " ).", group->id );

  if ( lookup_flow_entry_group( group->id ) != NULL ) {
    error( "Duplicated flow entry group id ( id = %#" PRIx64 " ).", group->id );
    return;
  }
  insert_hash_entry( flow_entry_group_db.id, &group->id, group );
  list_element *element = group->entries;
  while ( element != NULL ) {
    flow_entry_private *entry = element->data;
    if ( entry != NULL ) {
      add_flow_entry( entry );
    }
    element = element->next;
  }
}


static flow_entry_group *
alloc_flow_entry_group( void ) {
  flow_entry_group *group = xmalloc( sizeof( flow_entry_group ) );
  memset( group, 0, sizeof( flow_entry_group ) );
  create_list( &group->entries );

  return group;
}


static void
free_flow_entry_group( flow_entry_group *group ) {
  assert( group != NULL );

  list_element *element = group->entries;
  while ( element != NULL ) {
    flow_entry_private *entry = element->data;
    xfree( entry->public );
    xfree( entry );
    element = element->next;
  }
  if ( group->context_handle != NULL ) {
    free_messenger_context_handle( group->context_handle );
  }
  if ( group->entries != NULL ) {
    delete_list( group->entries );
  }
  xfree( group );
}


static void
delete_flow_entry_group( uint64_t id ) {
  debug( "Deleting flow entry group ( id = %#" PRIx64 " ).", id );

  flow_entry_group *deleted = delete_hash_entry( flow_entry_group_db.id, &id );
  if ( deleted == NULL ) {
    return;
  }

  list_element *element = deleted->entries;
  while ( element != NULL ) {
    flow_entry_private *entry = element->data;
    if ( entry != NULL ) {
      if ( entry->flow_mod_xid != 0 ) {
        delete_flow_mod_transaction( entry->flow_mod_xid );
      }
      if ( entry->barrier_xid != 0 ) {
        delete_barrier_transaction( entry->barrier_xid );
      }
      delete_flow_entry( entry->public->datapath_id, entry->public->match,
                         entry->public->priority );
    }
    element = element->next;
  }

  free_flow_entry_group( deleted );
}


static void
delete_flow_entry_group_db( void ) {
  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( flow_entry_group_db.id, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    flow_entry_group *group = e->value;
    delete_flow_entry_group( group->id );
  }
  delete_hash( flow_entry_group_db.id );
  flow_entry_group_db.id = NULL;
  delete_hash( flow_entry_group_db.entry );
  flow_entry_group_db.entry = NULL;
  delete_hash( flow_entry_group_db.entry_flow_mod_xid );
  flow_entry_group_db.entry_flow_mod_xid = NULL;
  delete_hash( flow_entry_group_db.entry_barrier_xid );
  flow_entry_group_db.entry_barrier_xid = NULL;
}


static const char *
state_to_string( int state ) {
  switch ( state ) {
    case INIT:
      return "INIT";
    case INSTALL_IN_PROGRESS:
      return "INSTALL_IN_PROGRESS";
    case INSTALL_CONFIRMED:
      return "INSTALL_CONFIRMED";
    case INSTALL_FAILED:
      return "INSTALL_FAILED";
    case REMOVE_IN_PROGRESS:
      return "REMOVE_IN_PROGRESS";
    case REMOVE_CONFIRMED:
      return "REMOVE_CONFIRMED";
    case REMOVE_FAILED:
      return "REMOVE_FAILED";
    default:
      break;
  }

  return "UNDEFINED";
}


static bool
update_flow_entry_state( flow_entry_private *entry, int new_state ) {
  assert( entry != NULL );

  debug( "Upadting flow entry state ( %s -> %s ).",
         state_to_string( entry->state ), state_to_string( new_state ) );

  switch ( entry->state ) {
    case INIT:
    {
      switch ( new_state ) {
        case INSTALL_IN_PROGRESS:
        case INSTALL_FAILED:
          entry->state = new_state;
          break;
        default:
          error( "Invalid flow entry state transition ( %s -> %s ).",
                 state_to_string( entry->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case INSTALL_IN_PROGRESS:
    {
      switch ( new_state ) {
        case INSTALL_CONFIRMED:
        case INSTALL_FAILED:
        case REMOVE_IN_PROGRESS:
        case REMOVE_CONFIRMED:
        case REMOVE_FAILED:
          entry->state = new_state;
          break;
        default:
          error( "Invalid flow entry state transition ( %s -> %s ).",
                 state_to_string( entry->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case INSTALL_CONFIRMED:
    {
      switch ( new_state ) {
        case REMOVE_IN_PROGRESS:
        case REMOVE_CONFIRMED:
        case REMOVE_FAILED:
          entry->state = new_state;
          break;
        default:
          error( "Invalid flow entry state transition ( %s -> %s ).",
                 state_to_string( entry->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case INSTALL_FAILED:
    {
      error( "Invalid flow entry state transition ( %s -> %s ).",
             state_to_string( entry->state ), state_to_string( new_state ) );
      return false;
    }
    break;
    case REMOVE_IN_PROGRESS:
    {
      switch ( new_state ) {
        case REMOVE_CONFIRMED:
        case REMOVE_FAILED:
          entry->state = new_state;
          break;
        default:
          error( "Invalid flow entry state transition ( %s -> %s ).",
                 state_to_string( entry->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case REMOVE_CONFIRMED:
    {
      error( "Invalid flow entry state transition ( %s -> %s ).",
             state_to_string( entry->state ), state_to_string( new_state ) );
      return false;
    }
    break;
    case REMOVE_FAILED:
    {
      error( "Invalid flow entry state transition ( %s -> %s ).",
             state_to_string( entry->state ), state_to_string( new_state ) );
      return false;
    }
    default:
    {
      error( "Undefined flow entry state ( %d ).", entry->state );
      return false;
    }
    break;
  }

  return true;
}


static bool
update_flow_entry_group_state( flow_entry_group *group, int new_state ) {
  assert( group != NULL );

  debug( "Upadting flow entry group state ( id = %#" PRIx64 ", state = %s -> %s ).",
         group->id, state_to_string( group->state ), state_to_string( new_state ) );

  switch ( group->state ) {
    case INIT:
    {
      switch ( new_state ) {
        case INSTALL_IN_PROGRESS:
        case INSTALL_FAILED:
          group->state = new_state;
          break;
        default:
          error( "Invalid flow entry group state transition ( %s -> %s ).",
                 state_to_string( group->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case INSTALL_IN_PROGRESS:
    {
      switch ( new_state ) {
        case INSTALL_CONFIRMED:
        case INSTALL_FAILED:
        case REMOVE_IN_PROGRESS:
          group->state = new_state;
          break;
        default:
          error( "Invalid flow entry group state transition ( %s -> %s ).",
                 state_to_string( group->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case INSTALL_CONFIRMED:
    {
      switch ( new_state ) {
        case REMOVE_IN_PROGRESS:
        case REMOVE_FAILED:
          group->state = new_state;
          break;
        default:
          error( "Invalid flow entry group state transition ( %s -> %s ).",
                 state_to_string( group->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case INSTALL_FAILED:
    {
      switch ( new_state ) {
        case REMOVE_IN_PROGRESS:
        case REMOVE_FAILED:
          group->state = new_state;
          break;
        default:
          error( "Invalid flow entry group state transition ( %s -> %s ).",
                 state_to_string( group->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case REMOVE_IN_PROGRESS:
    {
      switch ( new_state ) {
        case REMOVE_CONFIRMED:
        case REMOVE_FAILED:
          group->state = new_state;
          break;
        default:
          error( "Invalid flow entry group state transition ( %s -> %s ).",
                 state_to_string( group->state ), state_to_string( new_state ) );
          return false;
      }
    }
    break;
    case REMOVE_CONFIRMED:
    {
      error( "Invalid flow entry group state transition ( %s -> %s ).",
             state_to_string( group->state ), state_to_string( new_state ) );
      return false;
    }
    break;
    case REMOVE_FAILED:
    {
      error( "Invalid flow entry group state transition ( %s -> %s ).",
             state_to_string( group->state ), state_to_string( new_state ) );
      return false;
    }
    default:
    {
      error( "Undefined flow entry group state ( %d ).", group->state );
      return false;
    }
    break;
  }

  return true;
}


static bool
install_flow_entry_to_switch( flow_entry_private *entry ) {
  assert( entry != NULL );
  info("***** install_flow_entry_to_switch *****");

  entry->flow_mod_xid = get_transaction_id();

  openflow_actions *actions = create_actions();
  size_t offset = 0;
  struct ofp_action_header *ah = entry->public->actions;
  while ( offset < entry->public->actions_length ) {
    void *action = xmalloc( ah->len );
    memcpy( action, ah, ah->len );
    append_to_tail( &actions->list, action );
    actions->n_actions++;
    offset += ah->len;
    ah = ( struct ofp_action_header * ) ( ( char * ) entry->public->actions + offset );
  }

  struct ofp_match match = entry->public->match;
  normalize_match( &match );

  char match_string[ 256 ];
  match_to_string( &match, match_string, sizeof( match_string ) );
  debug( "Installing flow entry to switch ( datapath_id = %#" PRIx64 ", match = [%s], "
         "priority = %u, cookie = %#" PRIx64 " ).",
         entry->public->datapath_id, match_string, entry->public->priority, entry->group_id );

  buffer *flow_mod = create_flow_mod( entry->flow_mod_xid, match, entry->group_id,
                                      OFPFC_ADD, entry->public->idle_timeout, entry->public->hard_timeout,
                                      entry->public->priority, UINT32_MAX, OFPP_NONE, OFPFF_SEND_FLOW_REM, actions );
  bool ret = send_openflow_message( entry->public->datapath_id, flow_mod );
  delete_actions( actions );
  free_buffer( flow_mod );
  if ( ret == false ) {
    update_flow_entry_state( entry, INSTALL_FAILED );
    return false;
  }
  add_flow_mod_transaction( entry );

  entry->barrier_xid = get_transaction_id();

  buffer *barrier = create_barrier_request( entry->barrier_xid );
  ret = send_openflow_message( entry->public->datapath_id, barrier );
  free_buffer( barrier );
  if ( ret == false ) {
    update_flow_entry_state( entry, INSTALL_FAILED );
    delete_flow_mod_transaction( entry->flow_mod_xid );
    return false;
  }
  update_flow_entry_state( entry, INSTALL_IN_PROGRESS );
  add_barrier_transaction( entry );

  return true;
}


static bool
install_flow_entries_to_switches( flow_entry_group *group ) {
  assert( group != NULL );

  if ( group->entries == NULL ) {
    return false;
  }

  debug( "Installing flow entries to switches ( id = %#" PRIx64 " ).", group->id );

  int count = 0;
  bool ret = true;
  list_element *element = group->entries;
  while ( element != NULL && count < group->n_entries ) {
    flow_entry_private *entry = element->data;
    if ( install_flow_entry_to_switch( entry ) ) {
      group->n_barriers++;
    }
    else {
      ret = false;
    }
    element = element->next;
    count++;
  }

  if ( ret ) {
    update_flow_entry_group_state( group, INSTALL_IN_PROGRESS );
  }
  else {
    update_flow_entry_group_state( group, INSTALL_FAILED );
  }

  return ret;
}


static bool
check_conflicted_group_id( flow_entry_group_setup_request *request ) {
  assert( request != NULL );

  flow_entry_group *group = lookup_flow_entry_group( request->id );
  if ( group != NULL ) {
      return false;
  }

  return true;
}


static bool
check_conflicted_flow_entry( flow_entry_group_setup_request *request ) {
  assert( request != NULL );

  for ( int i = 0; i < request->n_entries; i++ ) {
    flow_entry_private *entry = lookup_flow_entry( request->entries[ i ].datapath_id,
                                                   request->entries[ i ].match,
                                                   request->entries[ i ].priority );
    if ( entry != NULL ) {
      return false;
    }
  }

  return true;
}


static flow_entry *
alloc_flow_entry_from( flow_entry *entry ) {
  assert( entry != NULL );

  size_t length = offsetof( flow_entry, actions ) + entry->actions_length;
  flow_entry *new_entry = xmalloc( length );
  memcpy( new_entry, entry, length );

  return new_entry;
}


static bool
append_flow_entry_to_group( flow_entry_group *group, flow_entry *entry ) {
  assert( group != NULL );
  assert( entry != NULL );

  flow_entry_private *private = xmalloc( sizeof( flow_entry_private ) );
  memset( private, 0, sizeof( flow_entry_private ) );
  private->group_id = group->id;
  private->public = alloc_flow_entry_from( entry );
  private->state = INIT;
  append_to_tail( &group->entries, private );

  return true;
}


static flow_entry_group *
create_flow_entry_group( flow_entry_group_setup_request *request ) {
  assert( request != NULL );

  flow_entry_group *group = alloc_flow_entry_group();

  group->id = request->id;
  memcpy( group->owner, request->owner, sizeof( group->owner ) );
  group->owner[ sizeof( group->owner ) - 1 ] = '\0';
  group->state = INIT;
  group->n_entries = request->n_entries;
  group->n_active_entries = 0;
  group->n_barriers = 0;
  create_list( &group->entries );
  group->expires_at.tv_sec = 0;
  group->expires_at.tv_nsec = 0;

  int count = 0;
  size_t offset = 0;
  while ( offset < request->entries_length && count < request->n_entries ) {
    flow_entry *entry = ( flow_entry * ) ( ( char * ) request->entries + offset );
    append_flow_entry_to_group( group, entry );
    offset = offset + offsetof( flow_entry, actions ) + entry->actions_length;
    count++;
  }

  return group;
}


static bool
remove_flow_entry_from_switch( flow_entry_private *entry ) {
  assert( entry != NULL );

  struct ofp_match match = entry->public->match;
  normalize_match( &match );

  entry->flow_mod_xid = get_transaction_id();
  buffer *flow_mod = create_flow_mod( entry->flow_mod_xid, match, get_cookie(),
                                      OFPFC_DELETE_STRICT,
                                      entry->public->idle_timeout, entry->public->hard_timeout,
                                      entry->public->priority, UINT32_MAX, OFPP_NONE,
                                      OFPFF_SEND_FLOW_REM, NULL );
  bool ret = send_openflow_message( entry->public->datapath_id, flow_mod );
  free_buffer( flow_mod );
  if ( ret == false ) {
    update_flow_entry_state( entry, REMOVE_FAILED );
    return false;
  }
  add_flow_mod_transaction( entry );

  entry->barrier_xid = get_transaction_id();
  buffer *barrier = create_barrier_request( entry->barrier_xid );
  ret = send_openflow_message( entry->public->datapath_id, barrier );
  free_buffer( barrier );
  if ( ret == false ) {
    update_flow_entry_state( entry, REMOVE_FAILED );
    return false;
  }
  add_barrier_transaction( entry );
  update_flow_entry_state( entry, REMOVE_IN_PROGRESS );

  return true;
}


static bool
remove_flow_entries_from_switches( flow_entry_group *group ) {
  assert( group != NULL );

  if ( group->entries == NULL ) {
    return false;
  }

  bool ret = true;
  list_element *element = group->entries;
  while ( element != NULL ) {
    flow_entry_private *entry = element->data;
    if ( entry->state == INSTALL_IN_PROGRESS || entry->state == INSTALL_CONFIRMED ) {
      if ( remove_flow_entry_from_switch( entry ) ) {
        group->n_barriers++;
      }
      else {
        ret = false;
      }
    }
    element = element->next;
  }

  if ( ret ) {
    update_flow_entry_group_state( group, REMOVE_IN_PROGRESS );
  }
  else {
    update_flow_entry_group_state( group, REMOVE_FAILED );
  }

  return ret;
}


static void
handle_setup_request( const messenger_context_handle *handle,
                      flow_entry_group_setup_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );

  debug( "Handling a setup request( handle = %p, request = %p ).", handle, request );

  bool ret = check_conflicted_group_id( request );
  if ( ret == false ) {
    error( "Conflicated group id found ( id = %#" PRIx64 " ).", request->id );
    buffer *reply = create_flow_entry_group_setup_reply( request->id, CONFLICTED_ID );
    send_reply_message( handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                        reply->data, reply->length );
    free_buffer( reply );
    return;
  }
  ret = check_conflicted_flow_entry( request );
  if ( ret == false ) {
    error( "Conflicted flow entry found." );
    buffer *reply = create_flow_entry_group_setup_reply( request->id, CONFLICTED_ENTRY );
    send_reply_message( handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                        reply->data, reply->length );
    free_buffer( reply );
    return;
  }

  flow_entry_group *group = create_flow_entry_group( request );
  ret = install_flow_entries_to_switches( group );
  if ( ret == false ) {
    error( "Failed to install flow entries to switches." );
    buffer *reply = create_flow_entry_group_setup_reply( group->id, SWITCH_ERROR );
    send_reply_message( handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                        reply->data, reply->length );
    remove_flow_entries_from_switches( group );
    free_buffer( reply );
    free_flow_entry_group( group );
    return;
  }
  clock_gettime( CLOCK_MONOTONIC, &group->expires_at );
  ADD_TIMESPEC( &group->expires_at, &SETUP_TRANSACTION_TIMEOUT, &group->expires_at );

  group->context_handle = copy_messenger_context_handle( handle );
  add_flow_entry_group( group );
}


static void
handle_teardown_request( const messenger_context_handle *handle,
                         flow_entry_group_teardown_request *request ) {
  assert( handle != NULL );
  assert( request != NULL );

  debug( "Handling a teardown request( handle = %p, request = %p ).", handle, request );

  flow_entry_group *group = lookup_flow_entry_group( request->id );
  if ( group == NULL ) {
    buffer *reply = create_flow_entry_group_teardown_reply( request->id, NO_ID_FOUND );
    send_reply_message( handle, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
                        reply->data, reply->length );
    free_buffer( reply );
    return;
  }

  if ( group->state != INSTALL_CONFIRMED ) {
    error( "Flow entry group is being installed/removed or already removed. ( state = %s ).",
           state_to_string( group->state ) );
    return;
  }

  bool ret = remove_flow_entries_from_switches( group );
  if ( ret == false ) {
    error( "Failed to remove flow entries to switches." );
    buffer *reply = create_flow_entry_group_setup_reply( group->id, SWITCH_ERROR );
    send_reply_message( handle, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
                        reply->data, reply->length );
    free_buffer( reply );
    free_flow_entry_group( group );
    return;
  }
  clock_gettime( CLOCK_MONOTONIC, &group->expires_at );
  ADD_TIMESPEC( &group->expires_at, &TEARDOWN_TRANSACTION_TIMEOUT, &group->expires_at );

  group->context_handle = copy_messenger_context_handle( handle );
}


static void
age_transaction_entries( void *user_data ) {
  UNUSED( user_data );
  struct timespec now;
  clock_gettime( CLOCK_MONOTONIC, &now );

  hash_iterator iter;
  hash_entry *e;
  init_hash_iterator( flow_entry_group_db.id, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    flow_entry_group *group = e->value;
    if ( group->state != INSTALL_IN_PROGRESS && group->state != REMOVE_IN_PROGRESS ) {
      continue;
    }
    if ( TIMESPEC_GREATER_THAN( &group->expires_at, &now ) ) {
      continue;
    }
    if ( group->state == INSTALL_IN_PROGRESS ) {
      update_flow_entry_group_state( group, INSTALL_FAILED );
      if ( group->context_handle != NULL ) {
        buffer *reply = create_flow_entry_group_setup_reply( group->id, SWITCH_ERROR );
        send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                            reply->data, reply->length );
        free_buffer( reply );
        free_messenger_context_handle( group->context_handle );
        group->context_handle = NULL;
      }
      remove_flow_entries_from_switches( group );
    }
    else { // group->state == REMOVE_IN_PROGRESS
      update_flow_entry_group_state( group, REMOVE_FAILED );
      if ( group->context_handle != NULL ) {
        buffer *reply = create_flow_entry_group_teardown_reply( group->id, SWITCH_ERROR );
        send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
                            reply->data, reply->length );
        free_buffer( reply );
        free_messenger_context_handle( group->context_handle );
        group->context_handle = NULL;
      }
      else {
        buffer *notification = create_flow_entry_group_teardown( group->id, TIMEOUT );
        send_message( group->owner, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN,
                      notification->data, notification->length );
        free_buffer( notification );
      }
    }
    delete_flow_entry_group( group->id );
  }
}


static void
handle_request( const messenger_context_handle *handle, uint16_t tag, void *data, size_t length ) {
  assert( handle != NULL );

  info( "***Handling a request ( handle = %p, tag = %#x, data = %p, length = %u ).",
         handle, tag, data, length );

  switch ( tag ) {
    case MESSENGER_FLOW_ENTRY_GROUP_SETUP_REQUEST:
    {
      if ( length < sizeof( flow_entry_group_setup_request ) ) {
        error( "Too short setup request message ( length = %u ).", length );
        return;
      }
      flow_entry_group_setup_request *request = data;
      handle_setup_request( handle, request );
    }
    break;
    case MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REQUEST:
    {
      if ( length != sizeof( flow_entry_group_teardown_request ) ) {
        error( "Invalid teardown request message ( length = %u ).", length );
        return;
      }
      flow_entry_group_teardown_request *request = data;
      handle_teardown_request( handle, request );
    }
    break;
    default:
    {
      error( "Undefined request tag ( tag = %#x ).", tag );
    }
    break;
  }
}


static void
handle_barrier_reply( uint64_t datapath_id, uint32_t transaction_id, void *user_data ) {

  debug( "Handling Barrier Reply ( datapath_id = %#" PRIx64 ", transaction_id = %#x, user_data = %p ).",
         datapath_id, transaction_id, user_data );

  flow_entry_private *entry = lookup_flow_entry_by_barrier_xid( transaction_id );
  if ( entry == NULL ) {
    error( "No flow entry found ( barrier_reply, transaction_id = %#x, datapath_id = %#" PRIx64 " ).", transaction_id, datapath_id );
    return;
  }

  flow_entry_group *group = lookup_flow_entry_group( entry->group_id );
  if ( group == NULL ) {
    error( "No flow entry group found ( barrier_reply, id = %#" PRIx64 ", datapath_id = %#" PRIx64 " ).", entry->group_id, datapath_id );
    return;
  }

  switch ( entry->state ) {
    case INSTALL_IN_PROGRESS:
    {
      update_flow_entry_state( entry, INSTALL_CONFIRMED );
      delete_barrier_transaction( entry->barrier_xid );
      entry->barrier_xid = 0;
      delete_flow_mod_transaction( entry->flow_mod_xid );
      entry->flow_mod_xid = 0;

      group->n_barriers--;
      group->n_active_entries++;
      if ( group->n_active_entries == group->n_entries ) {
        buffer *reply = create_flow_entry_group_setup_reply( group->id, SUCCEEDED );
        send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                            reply->data, reply->length );
        free_buffer( reply );
        free_messenger_context_handle( group->context_handle );
        group->context_handle = NULL;
        update_flow_entry_group_state( group, INSTALL_CONFIRMED );
      }
    }
    break;
    case REMOVE_IN_PROGRESS:
    case REMOVE_CONFIRMED:
    {
      delete_barrier_transaction( entry->barrier_xid );
      entry->barrier_xid = 0;
      delete_flow_mod_transaction( entry->flow_mod_xid );
      entry->flow_mod_xid = 0;

      group->n_barriers--;
      if ( group->n_barriers <= 0 ) {
        buffer *reply = create_flow_entry_group_teardown_reply( group->id, SUCCEEDED );
        send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
                            reply->data, reply->length );
        free_buffer( reply );
        free_messenger_context_handle( group->context_handle );
        group->context_handle = NULL;

        if ( group->n_active_entries <= 0 ) {
          delete_flow_entry_group( group->id );
        }
      }
    }
    break;
    default:
    {
      error( "Invalid flow entry state ( state = %s, group state = %s ).",
             state_to_string( entry->state ), state_to_string( group->state ) );
    }
    break;
  }
}


static void
handle_error( uint64_t datapath_id, uint32_t transaction_id, uint16_t type, uint16_t code,
              const buffer *data, void *user_data ) {
  warn( "An error message received ( datapath_id = %#" PRIx64 ", transaction_id = %#x, "
        "type = %#x, code = %#x, data = %p, user_data = %p ).",
        datapath_id, transaction_id, type, code, data, user_data );

  flow_entry_private *entry = lookup_flow_entry_by_flow_mod_xid( transaction_id );
  if ( entry == NULL ) {
    entry = lookup_flow_entry_by_barrier_xid( transaction_id );
    if ( entry != NULL ) {
      delete_barrier_transaction( entry->barrier_xid );
      delete_flow_mod_transaction( entry->flow_mod_xid );
      flow_entry_group *group = lookup_flow_entry_group( entry->group_id );
      if ( group != NULL ) {
        if ( group->state == INSTALL_IN_PROGRESS ) {
          update_flow_entry_group_state( group, INSTALL_FAILED );
          if ( group->context_handle != NULL ) {
            buffer *reply = create_flow_entry_group_setup_reply( group->id, SWITCH_ERROR );
            send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                                reply->data, reply->length );
            free_buffer( reply );
          }
          remove_flow_entries_from_switches( group );
        }
        else if ( group->state == REMOVE_IN_PROGRESS ) {
          update_flow_entry_state( entry, REMOVE_FAILED );
          update_flow_entry_group_state( group, REMOVE_FAILED );
          if ( group->context_handle != NULL ) {
            buffer *reply = create_flow_entry_group_teardown_reply( group->id, SWITCH_ERROR );
            send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
                                reply->data, reply->length );
            free_buffer( reply );
          }
        }

        if ( group->context_handle != NULL ) {
          free_messenger_context_handle( group->context_handle );
          group->context_handle = NULL;
        }

        delete_flow_entry_group( group->id );
      }
    }
    return;
  }

  flow_entry_group *group = lookup_flow_entry_group( entry->group_id );
  if ( group == NULL ) {
    error( "No flow entry group found ( error, id = %#" PRIx64 ", datapath_id = %#" PRIx64 " ).", entry->group_id, datapath_id );
    return;
  }

  switch ( entry->state ) {
    case INSTALL_IN_PROGRESS:
    {
      update_flow_entry_state( entry, INSTALL_FAILED );
      delete_barrier_transaction( entry->barrier_xid );
      entry->barrier_xid = 0;
      delete_flow_mod_transaction( entry->flow_mod_xid );
      entry->flow_mod_xid = 0;

      buffer *reply = create_flow_entry_group_setup_reply( group->id, SWITCH_ERROR );
      send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_SETUP_REPLY,
                          reply->data, reply->length );
      free_buffer( reply );
      free_messenger_context_handle( group->context_handle );
      group->context_handle = NULL;

      remove_flow_entries_from_switches( group );
      update_flow_entry_group_state( group, INSTALL_FAILED );
    }
    break;
    case REMOVE_IN_PROGRESS:
    {
      update_flow_entry_state( entry, REMOVE_FAILED );
      delete_barrier_transaction( entry->barrier_xid );
      entry->barrier_xid = 0;
      delete_flow_mod_transaction( entry->flow_mod_xid );
      entry->flow_mod_xid = 0;

      buffer *reply = create_flow_entry_group_teardown_reply( group->id, SWITCH_ERROR );
      send_reply_message( group->context_handle, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN_REPLY,
                          reply->data, reply->length );
      free_buffer( reply );
      free_messenger_context_handle( group->context_handle );
      group->context_handle = NULL;

      update_flow_entry_group_state( group, REMOVE_FAILED );
    }
    break;
    default:
    {
      error( "Invalid flow entry state ( state = %s, group state = %s ).",
             state_to_string( entry->state ), state_to_string( group->state ) );
    }
    break;
  }

  free_messenger_context_handle( group->context_handle );
  group->context_handle = NULL;
  delete_flow_entry_group( group->id );
}


static void
handle_flow_removed( uint64_t datapath_id, uint32_t transaction_id, struct ofp_match match, uint64_t cookie,
                     uint16_t priority, uint8_t reason, uint32_t duration_sec, uint32_t duration_nsec,
                     uint16_t idle_timeout, uint64_t packet_count, uint64_t byte_count, void *user_data ) {
  char match_string[ 256 ];
  match_to_string( &match, match_string, sizeof( match_string ) );
  debug( "Handleing Flow Removed ( datapath_id %#" PRIx64 ", transaction_id = %#x, match = [%s], "
         "cookie = %#" PRIx64 ", priority = %u, reason = %#x, duration_sec = %u, duration_nsec = %u, "
         "idle_timeout = %u, packet_count = %#" PRIx64 ", byte_count = %#" PRIx64 ", user_data = %p ).",
         datapath_id, transaction_id, match_string, cookie, priority, reason, duration_sec, duration_nsec,
         idle_timeout, packet_count, byte_count, user_data );

  flow_entry_group *group = lookup_flow_entry_group( cookie );
  if ( group == NULL ) {
    error( "No flow entry group found ( flow_removed, id = %#" PRIx64 ", datapath_id = %#" PRIx64 " ).", cookie, datapath_id );
    return;
  }

  flow_entry_private *entry = lookup_flow_entry( datapath_id, match, priority );
  if ( entry == NULL ) {
    error( "No flow entry found ( flow_removed, transaction_id = %#x, datapath_id = %#" PRIx64 " ).", transaction_id, datapath_id );
    return;
  }

  update_flow_entry_state( entry, REMOVE_CONFIRMED );
  if ( group->state != REMOVE_IN_PROGRESS ) {
    update_flow_entry_group_state( group, REMOVE_IN_PROGRESS );
    clock_gettime( CLOCK_MONOTONIC, &group->expires_at );
    ADD_TIMESPEC( &group->expires_at, &FLOW_REMOVED_TRANSACTION_TIMEOUT, &group->expires_at );
  }

  group->n_active_entries--;
  if ( group->n_active_entries <= 0 ) {
    buffer *notification;
    if ( reason == OFPRR_DELETE ) {
      notification = create_flow_entry_group_teardown( group->id, MANUALLY_REQUESTED );
    }
    else {
      notification = create_flow_entry_group_teardown( group->id, TIMEOUT );
    }

    send_message( group->owner, MESSENGER_FLOW_ENTRY_GROUP_TEARDOWN,
                  notification->data, notification->length );
    free_buffer( notification );

    update_flow_entry_group_state( group, REMOVE_CONFIRMED );
    if ( group->n_barriers <= 0 ) {
      delete_flow_entry_group( group->id );
    }
  }
}


int
main( int argc, char *argv[] ) {
  // Initialize Trema world
  openlog("Flow_manager", LOG_CONS | LOG_PID,  LOG_USER );
  syslog(LOG_NOTICE, "start flow_manager standalone");
  closelog();

  init_trema( &argc, &argv );

  // Create a database for managing flow entries
  create_flow_entry_group_db();

  // Set callback for handling timer event
  add_periodic_event_callback( 1, age_transaction_entries, NULL );

  // Set callbacks for handling OpenFlow events
  set_barrier_reply_handler( handle_barrier_reply, NULL );
  set_error_handler( handle_error, NULL );
  set_flow_removed_handler( handle_flow_removed, NULL );

  // Add a callback for handling requests from peers
  add_message_requested_callback( FLOW_MANAGEMENT_SERVICE, handle_request );

  // Main loop
  start_trema();

  // Cleanup
  delete_flow_entry_group_db();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
