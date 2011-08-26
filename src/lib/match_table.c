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

/**
 * @file
 *
 * @brief Flow Table implementation
 *
 * File containing functions for handling flow tables in an OpenFlow Switch.
 * @code
 * // Initialize match table
 * init_match_table();
 * ...
 * // Insert match entry
 * insert_match_entry( &ofp_match, priority, service_name, entry_name );
 * ...
 * // Lookup match entry
 * match_entry *match_entry = lookup_match_entry( &ofp_match );
 * ...
 * // Delete match entry
 * delete_match_entry( struct ofp_match *ofp_match );
 * ...
 * // Finalize match table
 * finalize_match_table();
 * @endcode
 */

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <openflow.h>
#include "checks.h"
#include "match_table.h"
#include "match.h"
#include "log.h"
#include "wrapper.h"


#ifdef UNIT_TESTING
#define static
#endif // UNIT_TESTING


typedef struct match_table {
  hash_table *exact_table; // no wildcards are set
  list_element *wildcard_table; // wildcard flags are set
  pthread_mutex_t *mutex;
} match_table;


static match_table match_table_head;


/**
 * Compare function for compairing structures of type ofp_match which contains
 * fields to match against flows. It is wrapped around by init_match_table.
 * @param x A void type pointer to constant identifier
 * @param y A void type pointer to constant identifier
 * @return bool True if equal, else False
 * @see init_match_table
 */
static bool
compare_match_entry( const void *x, const void *y ) {
  const struct ofp_match *xofp_match = x;
  const struct ofp_match *yofp_match = y;

  return compare_match( xofp_match, yofp_match );
}


/**
 * Generates hash for flow table. It is wrapped around by init_match_table.
 * @param key Pointer to constant key identifier
 * @return unsigned int hash
 * @see init_match_table
 */
static unsigned int
hash_match_entry( const void *key ) {
  const struct ofp_match *ofp_match = key;
  unsigned int hash = 0;

  assert( ofp_match->wildcards == 0 );

  hash ^= ( unsigned int ) ( ofp_match->in_port << 16 );
  hash ^= ( unsigned int ) hash_mac( ofp_match->dl_src );
  hash ^= ( unsigned int ) hash_mac( ofp_match->dl_dst );
  hash ^= ofp_match->dl_vlan;
  hash ^= ( unsigned int ) ( ofp_match->dl_vlan_pcp << 24 );
  hash ^= ( unsigned int ) ( ofp_match->dl_type << 8 );
  hash ^= ofp_match->nw_tos;
  hash ^= ( unsigned int ) ( ofp_match->nw_proto << 24 );
  hash ^= ofp_match->nw_src;
  hash ^= ofp_match->nw_dst;
  hash ^= ( unsigned int ) ( ofp_match->tp_src << 16 );
  hash ^= ofp_match->tp_dst;

  return hash;
}


/**
 * Allocates space to structure of type match entry. Members of the structure are initialized to
 * values passed as arguments to the function.
 * @param ofp_match Pointer to structure containing fields to match against flows
 * @param priority Priority order
 * @param service_name Pointer to application service name of messenger
 * @param entry_name Pointer to name of match entry
 * @return match_entry* Pointer to newly allocated match entry
 */
static match_entry *
allocate_match_entry( struct ofp_match *ofp_match, uint16_t priority, const char *service_name, const char *entry_name ) {
  match_entry *new_entry;

  new_entry = xmalloc( sizeof( match_entry ) );
  new_entry->ofp_match = *ofp_match;
  new_entry->priority = priority;
  new_entry->service_name = xstrdup( service_name );
  new_entry->entry_name = xstrdup( entry_name );

  return new_entry;
}


/**
 * Releases the memory allocated to the match entry. It is wrapped around by
 * free_match_table_walker.
 * @param match_entry Pointer to structure (of type match entry), memory allocated to which needs to be freed
 * @return None
 * @see free_match_table_walker
 */
static void
free_match_entry( match_entry *free_entry ) {
  assert( free_entry != NULL );
  xfree( free_entry->service_name );
  xfree( free_entry->entry_name );
  xfree( free_entry );
}


/**
 * Releases the memory allocated to each match entry. It is wrapped around by
 * finalize_match_table.
 * @param key Pointer to constant key identifier
 * @param value Pointer to associated data
 * @param user_data A void pointer to user data
 * @return None
 * @see finalize_match_table
 */
static void
free_match_table_walker( void *key, void *value, void *user_data ) {
  match_entry *entry = value;

  UNUSED( key );
  UNUSED( user_data );

  free_match_entry( entry );
}


/**
 * Initializes match_table_head (of type match_table) i.e, creates an exact table,
 * wildcard table and initialize all there members to NULL. It would also
 * initialize the mutex element of match_table_head.
 * @param None
 * @return None
 */
void
init_match_table( void ) {
  match_table_head.exact_table = create_hash( compare_match_entry, hash_match_entry );
  create_list( &match_table_head.wildcard_table );

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  match_table_head.mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( match_table_head.mutex, &attr );
}


/**
 * Finalizes match table (of type match_table) i.e, it frees all the memory
 * allocated to the associated match_table_head structure (of type match_table).
 * @param None
 * @return None
 */
void
finalize_match_table( void ) {
  list_element *list;

  pthread_mutex_lock( match_table_head.mutex );

  foreach_hash( match_table_head.exact_table, free_match_table_walker, NULL );
  delete_hash( match_table_head.exact_table );
  match_table_head.exact_table = NULL;

  for ( list = match_table_head.wildcard_table; list != NULL; list = list->next ) {
    free_match_entry( list->data );
  }
  delete_list( match_table_head.wildcard_table );
  match_table_head.wildcard_table = NULL;

  pthread_mutex_unlock( match_table_head.mutex );
  pthread_mutex_destroy( match_table_head.mutex );
  xfree( match_table_head.mutex );
  match_table_head.mutex = NULL;
}


/**
 * Inserts a new match entry (of type match_entry)in a flow table.
 * @param ofp_match Pointer to structure containing fields to match against flows
 * @param priority Priority order
 * @param service_name Pointer to application service name of messenger
 * @param entry_name Pointer to name of match entry
 * @return None
 */
void
insert_match_entry( struct ofp_match *ofp_match, uint16_t priority, const char *service_name, const char *entry_name ) {
  match_entry *new_entry, *entry;
  list_element *list;

  pthread_mutex_lock( match_table_head.mutex );

  new_entry = allocate_match_entry( ofp_match, priority, service_name, entry_name );

  if ( !ofp_match->wildcards ) {
    entry = lookup_hash_entry( match_table_head.exact_table, ofp_match );
    if ( entry != NULL ) {
      warn( "insert entry exits" );
      free_match_entry( new_entry );
      pthread_mutex_unlock( match_table_head.mutex );
      return;
    }
    insert_hash_entry( match_table_head.exact_table, &new_entry->ofp_match, new_entry );
    pthread_mutex_unlock( match_table_head.mutex );
    return;
  }

  // wildcard flags are set
  for ( list = match_table_head.wildcard_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( entry->priority <= new_entry->priority ) {
      break;
    }
  }
  if ( list == NULL ) {
    // wildcard_table is null or tail
    append_to_tail( &match_table_head.wildcard_table, new_entry );
    pthread_mutex_unlock( match_table_head.mutex );
    return;
  }
  if ( list == match_table_head.wildcard_table ) {
    // head
    insert_in_front( &match_table_head.wildcard_table, new_entry );
    pthread_mutex_unlock( match_table_head.mutex );
    return;
  }

  // insert brefore
  insert_before( &match_table_head.wildcard_table, list->data, new_entry );
  pthread_mutex_unlock( match_table_head.mutex );
}


/**
 * Deletes a match entry referred by the ofp_match in the flow table.
 * @param ofp_match Pointer to structure containing fields to match against flows
 * @return None
 */
void
delete_match_entry( struct ofp_match *ofp_match ) {
  match_entry *delete_entry;
  list_element *list;

  pthread_mutex_lock( match_table_head.mutex );

  assert( ofp_match != NULL );
  if ( !ofp_match->wildcards ) {
    delete_entry = delete_hash_entry( match_table_head.exact_table, ofp_match );
    if ( delete_entry == NULL ) {
      pthread_mutex_unlock( match_table_head.mutex );
      return;
    }
  }
  else {
    for ( list = match_table_head.wildcard_table; list != NULL; list = list->next ) {
      delete_entry = list->data;
      if ( ( ( ( delete_entry->ofp_match.wildcards ^ ofp_match->wildcards ) & OFPFW_ALL ) == 0 )
          && compare_match( &delete_entry->ofp_match, ofp_match ) ) {
        break;
      }
    }
    if ( list == NULL ) {
      pthread_mutex_unlock( match_table_head.mutex );
      return;
    }
    delete_element( &match_table_head.wildcard_table, delete_entry );
  }
  free_match_entry( delete_entry );
  pthread_mutex_unlock( match_table_head.mutex );
}


/**
 * Performs lookup for a value associated with match entry (of type match_entry)
 * in flow table.
 * @param ofp_match Pointer to structure containing fields to match against flows
 * @return match_entry* Pointer to found match entry else NULL
 */
match_entry *
lookup_match_entry( struct ofp_match *ofp_match ) {
  match_entry *entry;
  list_element *list;

  pthread_mutex_lock( match_table_head.mutex );

  entry = lookup_hash_entry( match_table_head.exact_table, ofp_match );
  if ( entry != NULL ) {
    pthread_mutex_unlock( match_table_head.mutex );
    return entry;
  }

  for ( list = match_table_head.wildcard_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( compare_match( &entry->ofp_match, ofp_match ) ) {
      pthread_mutex_unlock( match_table_head.mutex );
      return entry;
    }
  }

  pthread_mutex_unlock( match_table_head.mutex );

  return NULL;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
