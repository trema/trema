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

/**
 * @file hash_table.c
 * This source file contain functions for hash table implementation
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


/**
 * This is default compare function
 * @param x A void type pointer to constant identifier
 * @param y A void type pointer to constant identifier
 * @return bool True if equal, else False
 */
bool
compare_atom( const void *x, const void *y ) {
  return x == y;
}


/**
 * This is a default hash function
 * @param key Pointer to a address, for which hash key is generated
 * @return unsigned int Key value after right shifting by 2 bits
 */
unsigned int
hash_atom( const void *key ) {
  return ( unsigned int ) ( ( unsigned long ) key >> 2 );
}


/**
 * This function creates the hash table and initialize it to NULL
 * @param compare Function Pointer to compare_function
 * @param hash Function Pointer to hash_function
 * @return hash_table Pointer to created hash table
 */
hash_table *
create_hash( const compare_function compare, const hash_function hash ) {
  private_hash_table *table = xmalloc( sizeof( private_hash_table ) );

  table->public.number_of_buckets = default_hash_size;
  table->public.compare = compare ? compare : compare_atom;
  table->public.hash = hash ? hash : hash_atom;
  table->public.length = 0;
  table->public.buckets = xmalloc( sizeof( list_element * ) * default_hash_size );
  unsigned int i;
  bool list_created = false;
  for ( i = 0; i < table->public.number_of_buckets; i++ ) {
    list_created = create_list( &table->public.buckets[ i ] );
    assert( list_created == true );
  }
  list_created = create_list( &table->public.nonempty_bucket_index );
  assert( list_created == true );

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  table->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( table->mutex, &attr );

  return ( hash_table * ) table;
}


/**
 * This function returns the index of hash bucket
 * @param table Pointer to hash table for which bucket index is needed
 * @param key Pointer to constant key identifier
 * @return unsigned int Index of hash bucket
 */
static unsigned int
get_bucket_index( const hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  return ( *table->hash )( key ) % table->number_of_buckets;
}


/**
 * This function searches for an element in the bucket pointed by the key
 * @param table Pointer to hash table in which element is to be searched
 * @param key Pointer to constant key identifier
 * @return void Pointer to identified element in the bucket
 */
static void *
find_list_element_from_buckets( const hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  list_element *e = NULL;
  unsigned int i = get_bucket_index( table, key );
  for ( e = table->buckets[ i ]; e; e = e->next ) {
    if ( ( *table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      break;
    }
  }
  return e;
}


/**
 * This function inserts new element into an existing hash table
 * @param table Pointer to hash table in which element is to be inserted
 * @param key Pointer to new element's key
 * @param value Pointer to associated data
 * @return void Pointer to data associated with the key, else NULL
 */
void *
insert_hash_entry( hash_table *table, void *key, void *value ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  if ( table->buckets[ i ] == NULL ) {
    insert_in_front( &table->nonempty_bucket_index, ( void * ) ( unsigned long ) i );
  }

  list_element *old_elem = find_list_element_from_buckets( table, key );

  hash_entry *new_entry = xmalloc( sizeof( hash_entry ) );
  new_entry->key = key;
  new_entry->value = value;
  insert_in_front( &table->buckets[ i ], new_entry );
  table->length++;

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );

  if ( old_elem == NULL ) {
    return NULL;
  }
  else {
    return ( ( hash_entry * ) old_elem->data )->value;
  }
}


/**
 * This function performs lookup for value associated with the key in the hash table
 * @param table Pointer to hash table
 * @param key Pointer to key
 * @return void Pointer to value associated with the key, else NULL
 */
void *
lookup_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  list_element *e = find_list_element_from_buckets( table, key );
  if ( e != NULL ) {
    pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
    return ( ( hash_entry * ) e->data )->value;
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
  return NULL;
}


/**
 * This function deletes an entry from the hash table
 * @param table Pointer to hash table from which element is to be deleted
 * @param key Pointer to element's key which is to be deleted
 * @return void Pointer to data that was associated with the key, else NULL
 */
void *
delete_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  list_element *e = find_list_element_from_buckets( table, key );
  if ( e != NULL ) {
    hash_entry *delete_me = e->data;
    void *deleted = delete_me->value;
    delete_element( &table->buckets[ i ], delete_me );
    xfree( delete_me );
    if ( table->buckets[ i ] == NULL ) {
      delete_element( &table->nonempty_bucket_index, ( void * ) ( unsigned long ) i );
    }
    table->length--;
    pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
    return deleted;
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
  return NULL;
}


/**
 * This function takes as argument a function pointer which it calls in
 * case the key passed matches to a key in the hash_table
 * @param table Pointer to hash table in which element is to be searched
 * @param key Pointer to key for which function is to be called
 * @param function The action function
 * @param user_data A void pointer to user data
 * @return None
 */
void
map_hash( hash_table *table, const void *key, void function( void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  list_element *e = NULL;
  for ( e = table->buckets[ i ]; e; e = e->next ) {
    if ( ( table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      function( ( ( hash_entry * ) e->data )->value, user_data );
    }
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
}


/**
 * This function takes as argument a function pointer which is called
 * once for each bucket of the table being pointed to by passed argument
 * @param table Pointer to hash table
 * @param function The action function
 * @param user_data A void pointer to user data
 * @return None
 */
void
foreach_hash( hash_table *table, void function( void *key, void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i;
  for ( i = 0; i < table->number_of_buckets; i++ ) {
    list_element *e = NULL;
    for ( e = table->buckets[ i ]; e; ) {
      hash_entry *he = e->data;
      e = e->next;
      function( he->key, he->value, user_data );
    }
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
}


/**
 * This function initializes iteration over hash_table
 * @param table Pointer to hash table to iterate over
 * @param iter Pointer to hash_iterator that needs to be initialized
 * @return None
 */
void
init_hash_iterator( hash_table *table, hash_iterator *iter ) {
  assert( table != NULL );
  assert( iter != NULL );

  iter->buckets = table->buckets;
  if ( table->nonempty_bucket_index ) {
    iter->bucket_index = table->nonempty_bucket_index;
    iter->next_bucket_index = table->nonempty_bucket_index->next;
    iter->element = iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ];
  }
  else {
    iter->bucket_index = NULL;
    iter->element = NULL;
  }
}


/**
 * This function moves the hash iterator forward to next hash entry
 * @param iter Pointer to hash_iterator to move forward
 * @return hash_entry Pointer to valid hash entry, else NULL
 */
hash_entry *
iterate_hash_next( hash_iterator *iter ) {
  assert( iter != NULL );

  for ( ;; ) {
    if ( iter->bucket_index == NULL ) {
      return NULL;
    }

    list_element *e = iter->element;
    if ( e == NULL ) { 
      if ( iter->next_bucket_index != NULL ) {
        iter->bucket_index = iter->next_bucket_index;
        iter->next_bucket_index = iter->next_bucket_index->next;
        iter->element = iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ];
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


/**
 * This function releases all the memory held by the hash table
 * @param hash_table Pointer to hash table which needs to be deleted
 * @return None
 */
void
delete_hash( hash_table *table ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );
  pthread_mutex_t *mutex = ( ( private_hash_table * ) table )->mutex;

  bool list_deleted = false;

  if ( table->length > 0 ) {
    unsigned int i;
    for ( i = 0; i < table->number_of_buckets; i++ ) {
      list_element *e = NULL;
      for ( e = table->buckets[ i ]; e != NULL; ) {
        list_element *delete_me = e;
        e = e->next;
        xfree( delete_me->data );
      }
      list_deleted = delete_list( table->buckets[ i ] );
      assert( list_deleted == true );
    }
  }
  xfree( table->buckets );

  list_deleted = delete_list( table->nonempty_bucket_index );
  assert( list_deleted == true );

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
