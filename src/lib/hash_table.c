/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
#include <string.h>
#include "hash_table.h"
#include "wrapper.h"


static const unsigned int default_hash_size = 65521;


typedef struct {
  hash_table public;
  pthread_mutex_t *mutex;
} private_hash_table;


bool
compare_atom( const void *x, const void *y ) {
  return x == y;
}


unsigned int
hash_atom( const void *key ) {
  return ( unsigned int ) ( ( unsigned long ) key >> 2 );
}


hash_table *
create_hash( const compare_function compare, const hash_function hash ) {
  private_hash_table *table = xmalloc( sizeof( private_hash_table ) );

  table->public.number_of_buckets = default_hash_size;
  table->public.compare = compare ? compare : compare_atom;
  table->public.hash = hash ? hash : hash_atom;
  table->public.length = 0;
  table->public.buckets = xmalloc( sizeof( dlist_element * ) * default_hash_size );
  memset( table->public.buckets, 0, sizeof( dlist_element * ) * default_hash_size );
  table->public.nonempty_bucket_index = create_dlist();
  assert( table->public.nonempty_bucket_index != NULL );

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  table->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( table->mutex, &attr );

  return ( hash_table * ) table;
}


static unsigned int
get_bucket_index( const hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  return ( *table->hash )( key ) % table->number_of_buckets;
}


static void *
find_list_element_from_buckets( const hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  dlist_element *e = NULL;
  unsigned int i = get_bucket_index( table, key );
  if ( table->buckets[ i ] == NULL ) {
    return NULL;
  }
  for ( e = table->buckets[ i ]->next; e; e = e->next ) {
    if ( ( *table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      break;
    }
  }
  return e;
}


void *
insert_hash_entry( hash_table *table, void *key, void *value ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  if ( table->buckets[ i ] == NULL ) {
    table->buckets[ i ] = create_dlist();
  }
  if ( table->buckets[ i ]->next == NULL ) {
    table->buckets[ i ]->data = insert_after_dlist( table->nonempty_bucket_index, ( void * ) ( unsigned long ) i );
  }

  dlist_element *old_elem = find_list_element_from_buckets( table, key );

  hash_entry *new_entry = xmalloc( sizeof( hash_entry ) );
  new_entry->key = key;
  new_entry->value = value;
  insert_after_dlist( table->buckets[ i ], new_entry );
  table->length++;

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );

  if ( old_elem == NULL ) {
    return NULL;
  }
  else {
    return ( ( hash_entry * ) old_elem->data )->value;
  }
}


void *
lookup_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  dlist_element *e = find_list_element_from_buckets( table, key );
  if ( e != NULL ) {
    pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
    return ( ( hash_entry * ) e->data )->value;
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
  return NULL;
}


void *
delete_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  dlist_element *e = find_list_element_from_buckets( table, key );
  if ( e == NULL ) {
    pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
    return NULL;
  }

  hash_entry *delete_me = e->data;
  void *deleted = delete_me->value;
  delete_dlist_element( e );
  xfree( delete_me );
  unsigned int i = get_bucket_index( table, key );
  assert( table->buckets[ i ] != NULL );
  if ( table->buckets[ i ]->next == NULL ) {
    delete_dlist_element( table->buckets[ i ]->data );
    table->buckets[ i ]->data = NULL;
  }
  table->length--;
  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
  return deleted;
}


void
map_hash( hash_table *table, const void *key, void function( void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  if ( table->buckets[ i ] == NULL ) {
    pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
    return;
  }
  dlist_element *e = NULL;
  for ( e = table->buckets[ i ]->next; e; e = e->next ) {
    if ( ( table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      function( ( ( hash_entry * ) e->data )->value, user_data );
    }
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
}


void
foreach_hash( hash_table *table, void function( void *key, void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  dlist_element *nonempty = NULL;
  for ( nonempty = table->nonempty_bucket_index->next; nonempty; ) {
    int i = ( int ) ( unsigned long ) nonempty->data;
    nonempty = nonempty->next;
    dlist_element *e = NULL;
    if ( table->buckets[ i ] == NULL ) {
      continue;
    }
    for ( e = table->buckets[ i ]->next; e; ) {
      hash_entry *he = e->data;
      e = e->next;
      function( he->key, he->value, user_data );
    }
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
}


void
init_hash_iterator( hash_table *table, hash_iterator *iter ) {
  assert( table != NULL );
  assert( iter != NULL );

  iter->buckets = table->buckets;
  if ( table->nonempty_bucket_index->next ) {
    iter->bucket_index = table->nonempty_bucket_index->next;
    iter->next_bucket_index = iter->bucket_index->next;
    assert( iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ] != NULL );
    iter->element = iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ]->next;
  }
  else {
    iter->bucket_index = NULL;
    iter->element = NULL;
  }
}


hash_entry *
iterate_hash_next( hash_iterator *iter ) {
  assert( iter != NULL );

  for ( ;; ) {
    if ( iter->bucket_index == NULL ) {
      return NULL;
    }

    dlist_element *e = iter->element;
    if ( e == NULL ) { 
      if ( iter->next_bucket_index != NULL ) {
        iter->bucket_index = iter->next_bucket_index;
        iter->next_bucket_index = iter->next_bucket_index->next;
        assert( iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ] != NULL );
        iter->element = iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ]->next;
      }
      else {
        return NULL;
      }
    }
    else {
      iter->element = e->next;
      return e->data;
    }
  }
}


void
delete_hash( hash_table *table ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );
  pthread_mutex_t *mutex = ( ( private_hash_table * ) table )->mutex;

  bool dlist_deleted = false;

  unsigned int i;
  for ( i = 0; i < table->number_of_buckets; i++ ) {
    if ( table->buckets[ i ] == NULL ) {
      continue;
    }
    dlist_element *e = NULL;
    for ( e = table->buckets[ i ]->next; e != NULL; ) {
      dlist_element *delete_me = e;
      e = e->next;
      xfree( delete_me->data );
    }
    if ( table->buckets[ i ]->data != NULL ) {
      dlist_deleted = delete_dlist_element( table->buckets[ i ]->data );
      assert( dlist_deleted == true );
    }
    table->buckets[ i ]->data = NULL;
    dlist_deleted = delete_dlist( table->buckets[ i ] );
    assert( dlist_deleted == true );
  }
  xfree( table->buckets );

  dlist_deleted = delete_dlist( table->nonempty_bucket_index );
  assert( dlist_deleted == true );

  xfree( table );

  pthread_mutex_unlock( mutex );
  xfree( mutex );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
