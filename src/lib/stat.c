/*
 * Copyright (C) 2008-2013 NEC Corporation
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

static hash_table *stats = NULL;
static pthread_mutex_t stats_table_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


static void
create_stats_table() {
  assert( stats == NULL );
  stats = create_hash( compare_string, hash_string );
  assert( stats != NULL );
}


static void
delete_stats_table() {
  hash_iterator iter;
  hash_entry *e;

  assert( stats != NULL );

  init_hash_iterator( stats, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    void *value = delete_hash_entry( stats, e->key );
    if ( value != NULL ) {
      xfree( value );
    }
  }
  delete_hash( stats );
  stats = NULL;
}


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


bool
finalize_stat() {
  debug( "Finalizing statistics collector." );

  assert( stats != NULL );

  pthread_mutex_lock( &stats_table_mutex );
  delete_stats_table();
  pthread_mutex_unlock( &stats_table_mutex );

  return true;
}


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


void
reset_stats() {
  assert( stats != NULL );

  pthread_mutex_lock( &stats_table_mutex );

  hash_entry *e = NULL;
  hash_iterator iter;
  init_hash_iterator( stats, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    stat_entry *st = e->value;
    if ( st != NULL ) {
      void *deleted = delete_hash_entry( stats, st->key );
      if ( deleted != NULL ) {
        xfree( deleted );
      }
    }
  }

  pthread_mutex_unlock( &stats_table_mutex );
}


void
foreach_stat( void function( const char *key, const uint64_t value, void *user_data ), void *user_data ) {
  assert( stats != NULL );
  assert( function != NULL );

  pthread_mutex_lock( &stats_table_mutex );

  hash_entry *e = NULL;
  hash_iterator iter;
  init_hash_iterator( stats, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    stat_entry *st = e->value;
    if ( st != NULL ) {
      function( st->key, st->value, user_data );
    }
  }

  pthread_mutex_unlock( &stats_table_mutex );
}


static void
print_stat( const char *key, const uint64_t value, void *user_data ) {
  assert( key != NULL );
  assert( user_data != NULL );

  info( "%s: %" PRIu64, key, value );
  ( *( int * ) user_data )++;
}


void
dump_stats() {
  assert( stats != NULL );

  info( "Statistics:" );

  int n_stats = 0;
  foreach_stat( print_stat, &n_stats );

  if ( n_stats == 0 ) {
    info( "No statistics found." );
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
