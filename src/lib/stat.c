/*
 * Author: Yasunobu Chiba
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
 * @brief Trema Statistics Management layer
 *
 * This file provides various functions for creating a generic statistics management service, 
 * which can be used by Trema application to manage stats for arbitrary parameters.
 *
 * @code
 * // Initialize the Statistics layer
 * init_stat();
 * // Add an arbitrary parameter to track its statistics
 * add_stat_entry( "count_of_apples" );
 * ...
 * // Increment the number of apples we have by 1
 * increment_stat( "count_of_apples" );
 * ...
 * // Dump all the current parameters with their stats
 * dump_stats();
 * // Which would output the following
 * count_of_apples: 1
 * ...
 * // Finish stats parameter recording or tracking
 * finalize_stat();
 */
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include "bool.h"
#include "hash_table.h"
#include "log.h"
#include "stat.h"
#include "utility.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

// Allow static functions to be called from unit tests.
#define static

#ifdef debug
#undef debug
#endif
#define debug mock_debug
void mock_debug( const char *format, ... );

#ifdef info
#undef info
#endif
#define info mock_info
void mock_info( const char *format, ... );

#ifdef warn
#undef warn
#endif
#define warn mock_warn
void mock_warn( const char *format, ... );

#ifdef error
#undef error
#endif
#define error mock_error
void mock_error( const char *format, ... );

#endif // UNIT_TESTING

/**
 * Global Hash table which would store the Stats Parameters and their values
 */
static hash_table *stats = NULL;
static pthread_mutex_t stats_table_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


/**
 * Type that stores each entry of hash table representing stats of a parameter
 */
typedef struct {
  char key[ STAT_KEY_LENGTH ];
  uint64_t value;
} stat_entry;


/**
 * Initialize the global hash_table which would store the stats of parameters.
 * @param None
 * @return None
 */
static void
create_stats_table() {
  assert( stats == NULL );
  stats = create_hash( compare_string, hash_string );
  assert( stats != NULL );
}


/**
 * Delete the global hash_table which stored the stats of parameters.
 * @param None
 * @return None
 */
static void
delete_stats_table() {
  hash_iterator iter;
  hash_entry *e;

  assert( stats != NULL );

  init_hash_iterator( stats, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    void *value = delete_hash_entry( stats, e->key );
    xfree( value );
  }
  delete_hash( stats );
  stats = NULL;
}


/**
 * Primiary routine which initializes a Statistics holding database/table (of hash_table type).
 * @param None
 * @return bool Always returns True
 */
bool
init_stat() {
  debug( "Initializing statistics collector." );

  if ( stats != NULL ) {
    warn( "Statistics collector is already initialized. Reinitializing." );
    finalize_stat();
  }

  pthread_mutex_lock( &stats_table_mutex );
  create_stats_table();
  pthread_mutex_unlock( &stats_table_mutex );

  return true;
}


/**
 * Cleans up the Statistics database.
 * @param None
 * @return bool Always returns True
 */
bool
finalize_stat() {
  debug( "Finalizing statistics collector." );

  assert( stats != NULL );

  pthread_mutex_lock( &stats_table_mutex );
  delete_stats_table();
  pthread_mutex_unlock( &stats_table_mutex );

  return true;
}


/**
 * Adds a parameter to the Statistics database/table, stats of which are to be tracked.
 * @param key Identifier for the parameter of which stats are required
 * @return bool True if the entry was successfully added, else False in case entry already exists
 */
bool
add_stat_entry( const char *key ) {
  assert( key != NULL );

  pthread_mutex_lock( &stats_table_mutex );

  stat_entry *entry = lookup_hash_entry( stats, key );

  if ( entry != NULL ) {
    error( "Statistic entry for %s already exists.", key );
    pthread_mutex_unlock( &stats_table_mutex );
    return false;
  }

  entry = xmalloc( sizeof( stat_entry ) );
  entry->value = 0;
  strncpy( entry->key, key, STAT_KEY_LENGTH );
  entry->key[ STAT_KEY_LENGTH - 1 ] = '\0';

  insert_hash_entry( stats, entry->key, entry );

  pthread_mutex_unlock( &stats_table_mutex );

  return true;
}


/**
 * Increment the stat counter for the specified Parameter by 1
 * @param key Identifier for parameter
 * @return None
 */
void
increment_stat( const char *key ) {
  assert( key != NULL );
  assert( stats != NULL );

  pthread_mutex_lock( &stats_table_mutex );

  stat_entry *entry = lookup_hash_entry( stats, key );
  if ( entry == NULL ) {
    if ( add_stat_entry( key ) == false ) {
      pthread_mutex_unlock( &stats_table_mutex );
      return;
    }
    entry = lookup_hash_entry( stats, key );
  }

  assert( entry != NULL );

  ( entry->value )++;

  pthread_mutex_unlock( &stats_table_mutex );
}


/**
 * Dump the statistics onto screen (or stream specified by info function)
 * @param None
 * @return None
 */
void
dump_stats() {
  assert( stats != NULL );

  int n_stats = 0;
  hash_iterator iter;
  hash_entry *e;

  pthread_mutex_lock( &stats_table_mutex );

  info( "Statistics:" );

  init_hash_iterator( stats, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    stat_entry *st = e->value;
    info( "%s: %" PRIu64, st->key, st->value );
    n_stats++;
  }

  if ( n_stats == 0 ) {
    info( "No statistics found." );
  }

  pthread_mutex_unlock( &stats_table_mutex );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

