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


static bool
compare_match_entry( const void *x, const void *y ) {
  const struct ofp_match *xofp_match = x;
  const struct ofp_match *yofp_match = y;

  return compare_match( xofp_match, yofp_match );
}


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


static match_entry *
allocate_match_entry( struct ofp_match *ofp_match, uint16_t priority ) {
  match_entry *new_entry;

  new_entry = xmalloc( sizeof( match_entry ) );
  new_entry->ofp_match = *ofp_match;
  new_entry->priority = priority;
  create_list( &new_entry->services_name );

  return new_entry;
}


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


static void
free_match_table_walker( void *key, void *value, void *user_data ) {
  match_entry *entry = value;

  UNUSED( key );
  UNUSED( user_data );

  free_match_entry( entry );
}


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
