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
allocate_match_entry( struct ofp_match *ofp_match, uint16_t priority ) {
  match_entry *new_entry;

  new_entry = xmalloc( sizeof( match_entry ) );
  new_entry->ofp_match = *ofp_match;
  new_entry->priority = priority;
  create_list( &new_entry->services_name );

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
  list_element *element;
  for ( element = free_entry->services_name; element != NULL; element = element->next ) {
    xfree( element->data );
  }
  delete_list( free_entry->services_name );
  xfree( free_entry );
}


static bool
add_service_name( match_entry *entry, const char *service_name ) {
  char *copy = xstrdup( service_name );
  return append_to_tail( &entry->services_name, copy );
}


static void
delete_service_name( match_entry *entry, const char *service_name ) {
  list_element *element;
  for ( element = entry->services_name; element != NULL; element = element->next ) {
    if ( strcmp( ( char * ) element->data, service_name ) == 0 ) {
      char *delete_data = element->data;
      delete_element( &entry->services_name, element->data );
      xfree( delete_data );
      return;
    }
  }
}


static unsigned int
services_name_length_of( match_entry *entry ) {
  return list_length_of( entry->services_name );
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
insert_match_entry( struct ofp_match *ofp_match, uint16_t priority, const char *service_name ) {
  assert( ofp_match != NULL );
  assert( service_name != NULL );


  pthread_mutex_lock( match_table_head.mutex );
  match_entry *entry = NULL;
  if ( !ofp_match->wildcards ) {
    entry = lookup_hash_entry( match_table_head.exact_table, ofp_match );
    if ( entry == NULL ) {
      entry = allocate_match_entry( ofp_match, 0 );
      insert_hash_entry( match_table_head.exact_table, &entry->ofp_match, entry );
    }
  }
  else {
    // wildcard flags are set
    list_element *element;
    for ( element = match_table_head.wildcard_table; element != NULL; element = element->next ) {
      entry = element->data;
      if ( entry->priority == priority
           && ( ( entry->ofp_match.wildcards ^ ofp_match->wildcards ) & OFPFW_ALL ) == 0
           && compare_match( &entry->ofp_match, ofp_match ) ) {
        break;
      }
      if ( entry->priority < priority ) {
        entry = NULL;
        break;
      }
    }
    if ( entry == NULL || element == NULL ) {
      entry = allocate_match_entry( ofp_match, priority );
      if ( element == NULL ) {
        // tail
        append_to_tail( &match_table_head.wildcard_table, entry );
      }
      else if ( element == match_table_head.wildcard_table ) {
        // head
        insert_in_front( &match_table_head.wildcard_table, entry );
      }
      else {
        // insert brefore
        insert_before( &match_table_head.wildcard_table, element->data, entry );
      }
    }
  }
  add_service_name( entry, service_name );
  pthread_mutex_unlock( match_table_head.mutex );
}


/**
 * Deletes a match entry referred by the ofp_match in the flow table.
 * @param ofp_match Pointer to structure containing fields to match against flows
 * @return None
 */
void
delete_match_entry( struct ofp_match *ofp_match, uint16_t priority, const char *service_name ) {
  assert( ofp_match != NULL );
  assert( service_name != NULL );

  pthread_mutex_lock( match_table_head.mutex );
  match_entry *entry = NULL;
  if ( !ofp_match->wildcards ) {
    entry = lookup_hash_entry( match_table_head.exact_table, ofp_match );
    if ( entry == NULL ) {
      pthread_mutex_unlock( match_table_head.mutex );
      return;
    }
  }
  else {
    // wildcard flags are set
    list_element *element;
    for ( element = match_table_head.wildcard_table; element != NULL; element = element->next ) {
      entry = element->data;
      if ( entry->priority == priority 
           && ( ( entry->ofp_match.wildcards ^ ofp_match->wildcards ) & OFPFW_ALL ) == 0
           && compare_match( &entry->ofp_match, ofp_match ) ) {
        break;
      }
    }
    if ( element == NULL ) {
      pthread_mutex_unlock( match_table_head.mutex );
      return;
    }
  }
  delete_service_name( entry, service_name );
  if ( services_name_length_of( entry ) == 0 ) {
    if ( !ofp_match->wildcards ) {
      delete_hash_entry( match_table_head.exact_table, ofp_match );
    }
    else {
      delete_element( &match_table_head.wildcard_table, entry );
    }
    free_match_entry( entry );
  }
  pthread_mutex_unlock( match_table_head.mutex );
  return;
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
