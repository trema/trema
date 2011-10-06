/*
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


#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <openflow.h>
#include "checks.h"
#include "match.h"
#include "match_table.h"
#include "log.h"
#include "wrapper.h"


#define VLAN_VID_MASK 0x0fff // 12 bits
#define VLAN_PCP_MASK 0x07 // 3 bits
#define NW_TOS_MASK 0x3f // 6 bits

#define MATCH_STRING_LENGTH 1024


typedef struct {
  struct ofp_match match; // match data. host byte order
  uint16_t priority;
  void *data;
} match_entry;


typedef struct {
  hash_table *exact_table; // no wildcards are set
  list_element *wildcards_table; // wildcards flags are set
  pthread_mutex_t *mutex;
} match_table;


match_table *_match_table_head = NULL; // non-static variable for use in unit testing


static inline bool
exact_match( struct ofp_match *match ) {
  assert( match != NULL );

  return ( ( match->wildcards & OFPFW_ALL ) == 0 );
}


static match_entry *
allocate_match_entry( struct ofp_match *match, uint16_t priority, void *data ) {
  match_entry *new_entry = xmalloc( sizeof( match_entry ) );
  new_entry->match = *match;
  new_entry->priority = priority;
  new_entry->data = data;

  return new_entry;
}


static void
free_match_entry( match_entry *free_entry ) {
  assert( free_entry != NULL );

  xfree( free_entry );
}


static bool
compare_exact_match_entry( const void *x, const void *y ) {
  assert( x != NULL );
  assert( y != NULL );

  const struct ofp_match *xofp_match = x;
  const struct ofp_match *yofp_match = y;
  return compare_match_strict( xofp_match, yofp_match );
}


static unsigned int
hash_exact_match_entry( const void *key ) {
  assert( key != NULL );
  const struct ofp_match *match = key;

  uint16_t dl_vlan = match->dl_vlan;
  if ( dl_vlan != UINT16_MAX ) {
    dl_vlan = ( uint16_t ) ( dl_vlan & VLAN_VID_MASK );
  }
  uint8_t dl_vlan_pcp = ( uint8_t ) ( match->dl_vlan_pcp & VLAN_PCP_MASK );
  uint8_t nw_tos = ( uint8_t ) ( match->nw_tos & NW_TOS_MASK );

  unsigned int hash = 0;
  hash ^= ( unsigned int ) ( match->in_port << 16 );
  hash ^= ( unsigned int ) hash_mac( match->dl_src );
  hash ^= ( unsigned int ) hash_mac( match->dl_dst );
  hash ^= dl_vlan;
  hash ^= ( unsigned int ) ( dl_vlan_pcp << 24 );
  hash ^= ( unsigned int ) ( match->dl_type << 8 );
  hash ^= nw_tos;
  hash ^= ( unsigned int ) ( match->nw_proto << 24 );
  hash ^= match->nw_src;
  hash ^= match->nw_dst;
  hash ^= ( unsigned int ) ( match->tp_src << 16 );
  hash ^= match->tp_dst;

  return hash;
}


static void
init_exact_match_table( hash_table **exact_table ) {
  assert( exact_table != NULL );

  *exact_table = create_hash( compare_exact_match_entry, hash_exact_match_entry );
}


static void
free_exact_match_entry( void *key, void *value, void *user_data ) {
  UNUSED( key );
  assert( value != NULL );
  UNUSED( user_data );

  match_entry *entry = value;

  free_match_entry( entry );
}


static void
finalize_exact_match_table( hash_table *exact_table ) {
  assert( exact_table != NULL );

  foreach_hash( exact_table, free_exact_match_entry, NULL );
  delete_hash( exact_table );
}


static bool
insert_exact_match_entry( hash_table *exact_table, struct ofp_match *match, void *data ) {
  assert( exact_table != NULL );
  assert( match != NULL );

  match_entry *entry = lookup_hash_entry( exact_table, match );
  if ( entry != NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "exact match entry already exists ( match = [%s] )", match_string );
    return false;
  }
  match_entry *new_entry = allocate_match_entry( match, 0 /* dummy priority */, data );
  match_entry *conflict_entry = insert_hash_entry( exact_table, &new_entry->match, new_entry );
  assert( conflict_entry == NULL );
  return true;
}


static match_entry *
lookup_exact_match_strict_entry( hash_table *exact_table, struct ofp_match *match ) {
  assert( exact_table != NULL );
  assert( match != NULL );

  return lookup_hash_entry( exact_table, match );
}


#define lookup_exact_match_entry( table, match ) lookup_exact_match_strict_entry( table, match)


static bool
update_exact_match_entry( hash_table *exact_table, struct ofp_match *match, void *data ) {
  assert( exact_table != NULL );
  assert( match != NULL );

  match_entry *entry = lookup_exact_match_strict_entry( exact_table, match );
  if ( entry == NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "exact match entry not found ( match = [%s] )",
          match_string );
    return false;
  }
  entry->data = data;
  return true;
}


static void *
delete_exact_match_entry( hash_table *exact_table, struct ofp_match *match ) {
  assert( exact_table != NULL );
  assert( match != NULL );

  match_entry *entry = lookup_hash_entry( exact_table, match );
  if ( entry == NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "exact match entry not found ( match = [%s] )",
          match_string );
    return NULL;
  }
  void *data = entry->data;
  delete_hash_entry( exact_table, match );
  free_match_entry( entry );
  return data;
}


static void
exact_match_table_walker( void *key, void *value, void *user_data ) {
  UNUSED( key );
  assert( value != NULL );
  assert( user_data != NULL );

  void ( * function )( struct ofp_match, uint16_t, void * ) = user_data;
  match_entry *entry = value;

  function( entry->match, entry->priority, entry->data );
}


static void
foreach_exact_match_table( hash_table *exact_table, void function( struct ofp_match, uint16_t, void * ) ) {
  assert( exact_table != NULL );
  assert( function != NULL );
  foreach_hash( exact_table, exact_match_table_walker, function );
}


static void
init_wildcards_match_table( list_element **wildcards_table ) {
  assert( wildcards_table != NULL);

  create_list( wildcards_table );
}


static void
finalize_wildcards_match_table( list_element *wildcards_table ) {
  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    free_match_entry( element->data );
    element->data = NULL;
  }
  delete_list( wildcards_table );
}


static bool
insert_wildcards_match_entry( list_element **wildcards_table, struct ofp_match *match, uint16_t priority, void *data ) {
  assert( match != NULL );

  list_element *element;
  for ( element = *wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    if ( entry->priority < priority ) {
      break;
    }
    assert( entry != NULL );
    if ( entry->priority == priority && compare_match_strict( &entry->match, match ) ) {
      char match_string[ MATCH_STRING_LENGTH ];
      match_to_string( match, match_string, sizeof( match_string ) );
      warn( "wildcards match entry already exists ( match = [%s], priority = %u )",
            match_string, priority );
      return false;
    }
  }
  match_entry *new_entry = allocate_match_entry( match, priority, data );
  if ( element == NULL ) {
    // tail
    append_to_tail( wildcards_table, new_entry );
  }
  else if ( element == *wildcards_table ) {
    // head
    insert_in_front( wildcards_table, new_entry );
  }
  else {
    // insert before
    insert_before( wildcards_table, element->data, new_entry );
  }
  return true;
}


static match_entry *
lookup_wildcards_match_strict_entry( list_element *wildcards_table, struct ofp_match *match, uint16_t priority ) {
  assert( match != NULL );

  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    if ( entry->priority < priority ) {
      break;
    }
    if ( entry->priority == priority && compare_match_strict( &entry->match, match ) ) {
      return entry;
    }
  }
  return NULL;
}


static match_entry *
lookup_wildcards_match_entry( list_element *wildcards_table, struct ofp_match *match ) {
  assert( match != NULL );

  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    if ( compare_match( &entry->match, match ) ) {
      return entry;
    }
  }
  return NULL;
}


static bool
update_wildcards_match_entry( list_element *wildcards_table, struct ofp_match *match, uint16_t priority, void *data ) {
  assert( match != NULL );

  match_entry *entry = lookup_wildcards_match_strict_entry( wildcards_table, match, priority );
  if ( entry == NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "wildcards match entry not found ( match = [%s], priority = %u )",
          match_string, priority );
    return false;
  }
  entry->data = data;
  return true;
}


static void *
delete_wildcards_match_entry( list_element **wildcards_table, struct ofp_match *match, uint16_t priority ) {
  assert( match != NULL );

  match_entry *entry = lookup_wildcards_match_strict_entry( *wildcards_table, match, priority );
  if ( entry == NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "wildcards match entry not found ( match = [%s], priority = %u )",
          match_string, priority );
    return NULL;
  }
  void *data = entry->data;
  delete_element( wildcards_table, entry );
  free_match_entry( entry );
  return data;
}


static void
foreach_wildcards_match_table( list_element *wildcards_table, void function( struct ofp_match, uint16_t, void * ) ) {
  assert( function != NULL );

  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    function( entry->match, entry->priority, entry->data );
  }
}


void
init_match_table( void ) {
  if ( _match_table_head != NULL ) {
    die( "match table is already initialized." );
  }

  match_table *table = xmalloc( sizeof( match_table ) );
  init_exact_match_table( &table->exact_table );
  init_wildcards_match_table( &table->wildcards_table );
  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  table->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( table->mutex, &attr );

  _match_table_head = table;
}


void
finalize_match_table( void ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_t *mutex = _match_table_head->mutex;

  pthread_mutex_lock( mutex );
  finalize_exact_match_table( _match_table_head->exact_table );
  finalize_wildcards_match_table( _match_table_head->wildcards_table );
  xfree( _match_table_head );
  _match_table_head = NULL;
  pthread_mutex_unlock( mutex );
  pthread_mutex_destroy( mutex );
  xfree( mutex );
}


bool
insert_match_entry( struct ofp_match match, uint16_t priority, void *data ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  bool result;
  if ( exact_match( &match ) ) {
    result = insert_exact_match_entry( _match_table_head->exact_table, &match, data );
  }
  else {
    // wildcards flags are set
    result = insert_wildcards_match_entry( &_match_table_head->wildcards_table, &match, priority, data );
  }
  pthread_mutex_unlock( _match_table_head->mutex );
  return result;
}


void *
lookup_match_strict_entry( struct ofp_match match, uint16_t priority ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  match_entry *entry;
  if ( exact_match( &match ) ) {
    entry = lookup_exact_match_strict_entry( _match_table_head->exact_table, &match );
  }
  else {
    entry = lookup_wildcards_match_strict_entry( _match_table_head->wildcards_table, &match, priority );
  }
  void *data = ( entry != NULL ? entry->data : NULL );
  pthread_mutex_unlock( _match_table_head->mutex );
  return data;
}


void *
lookup_match_entry( struct ofp_match match ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  match_entry *entry = lookup_exact_match_entry( _match_table_head->exact_table, &match );
  if ( entry == NULL ) {
    entry = lookup_wildcards_match_entry( _match_table_head->wildcards_table, &match );
  }
  void *data = ( entry != NULL ? entry->data : NULL );
  pthread_mutex_unlock( _match_table_head->mutex );
  return data;
}


bool
update_match_entry( struct ofp_match match, uint16_t priority, void *data ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  bool result;
  if ( exact_match( &match ) ) {
    result = update_exact_match_entry( _match_table_head->exact_table, &match, data );
  }
  else {
    // wildcards flags are set
    result = update_wildcards_match_entry( _match_table_head->wildcards_table, &match, priority, data );
  }
  pthread_mutex_unlock( _match_table_head->mutex );
  return result;
}


void *
delete_match_entry( struct ofp_match match, uint16_t priority ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  void *data = NULL;
  if ( exact_match( &match ) ) {
    data = delete_exact_match_entry( _match_table_head->exact_table, &match );
  }
  else {
    // wildcards flags are set
    data = delete_wildcards_match_entry( &_match_table_head->wildcards_table, &match, priority );
  }
  pthread_mutex_unlock( _match_table_head->mutex );
  return data;
}


void
foreach_match_table( void function( struct ofp_match, uint16_t, void * ) ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }
  if ( function == NULL ) {
    die( "function must not be NULL" );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  foreach_exact_match_table( _match_table_head->exact_table, function );
  foreach_wildcards_match_table( _match_table_head->wildcards_table, function );
  pthread_mutex_unlock( _match_table_head->mutex );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
