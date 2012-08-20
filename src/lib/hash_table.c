/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


/**
 * Compares two pointer arguments and returns true if they are
 * equal. It can be passed to create_hash() as the key_equal_func
 * parameter, when using pointers as keys in a hash_table.
 *
 * @param x a key.
 * @param y a key to compare with x.
 * @return true if the two keys match.
 */
bool
compare_atom( const void *x, const void *y ) {
  return x == y;
}


/**
 * Converts a pointer to a hash value. It can be passed to
 * create_hash() as the hash parameter, when using pointers as keys in
 * a hash_table.
 *
 * @param key a pointer key.
 * @return a hash value corresponding to the key.
 */
unsigned int
hash_atom( const void *key ) {
  return ( unsigned int ) ( ( unsigned long ) key >> 2 );
}


/**
 * Creates a new hash_table.
 *
 * @param compare a function to check two keys for equality. This is
 *        used when looking up keys in the hash_table. If compare is
 *        NULL, keys are compared by compare_atom().
 * @param hash a function to create a hash value from a key. Hash
 *        values are used to determine where keys are stored within
 *        the hash_table data structure. If hash_func is NULL,
 *        hash_atom() is used.
 * @return a new hash_table.
 */
hash_table *
create_hash( const compare_function compare, const hash_function hash ) {
  return create_hash_with_size( compare, hash, default_hash_size );
}


/**
 * Creates a new hash_table by specifying its bucket size.
 *
 * @param compare a function to check two keys for equality. This is
 *        used when looking up keys in the hash_table. If compare is
 *        NULL, keys are compared by compare_atom().
 * @param hash a function to create a hash value from a key. Hash
 *        values are used to determine where keys are stored within
 *        the hash_table data structure. If hash_func is NULL,
 *        hash_atom() is used.
 * @param size the number of hash buckets.
 * @return a new hash_table.
 */
hash_table *
create_hash_with_size( const compare_function compare, const hash_function hash, unsigned int size ) {
  private_hash_table *table = xmalloc( sizeof( private_hash_table ) );

  table->public.number_of_buckets = size;
  table->public.compare = compare ? compare : compare_atom;
  table->public.hash = hash ? hash : hash_atom;
  table->public.length = 0;
  table->public.buckets = xmalloc( sizeof( dlist_element * ) * table->public.number_of_buckets );
  memset( table->public.buckets, 0, sizeof( dlist_element * ) * table->public.number_of_buckets );
  table->public.nonempty_bucket_index = create_dlist();

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


static void
new_bucket( hash_table *table, unsigned int bucket_index ) {
  table->buckets[ bucket_index ] = create_dlist();
  table->buckets[ bucket_index ]->data = insert_after_dlist( table->nonempty_bucket_index, ( void * ) ( unsigned long ) bucket_index );
}


#define MUTEX_LOCK( table ) pthread_mutex_lock( ( ( private_hash_table * ) ( table ) )->mutex )
#define MUTEX_UNLOCK( table ) pthread_mutex_unlock( ( ( private_hash_table * ) ( table ) )->mutex )


/**
 * Inserts a new key and value into a hash_table. If the key already
 * exists in the hash_table its current value is replaced with the new
 * value.
 *
 * @param table a hash_table.
 * @param key a key to insert.
 * @param value the value to associate with the key.
 * @return the old value associated with the key.
 */
void *
insert_hash_entry( hash_table *table, void *key, void *value ) {
  assert( table != NULL );
  assert( key != NULL );

  MUTEX_LOCK( table );

  void *old_value = NULL;
  unsigned int i = get_bucket_index( table, key );

  if ( table->buckets[ i ] != NULL ) {
    dlist_element *old_element = NULL;
    for ( old_element = table->buckets[ i ]->next; old_element; old_element = old_element->next ) {
      if ( ( *table->compare )( key, ( ( hash_entry * ) old_element->data )->key ) ) {
        break;
      }
    }
    if ( old_element != NULL ) {
      old_value = ( ( hash_entry * ) old_element->data )->value;
      delete_hash_entry( table, key );
    }
  }

  if ( table->buckets[ i ] == NULL ) {
    new_bucket( table, i );
  }

  hash_entry *new_entry = xmalloc( sizeof( hash_entry ) );
  new_entry->key = key;
  new_entry->value = value;
  insert_after_dlist( table->buckets[ i ], new_entry );
  table->length++;

  MUTEX_UNLOCK( table );

  return old_value;
}


/**
 * Looks up a key in a hash_table.
 *
 * @param table a hash_table.
 * @param key the key to look up.
 * @return the associated value, or NULL if the key is not found.
 */
void *
lookup_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  MUTEX_LOCK( table );

  void *value = NULL;
  unsigned int i = get_bucket_index( table, key );
  if ( table->buckets[ i ] != NULL ) {
    for ( dlist_element *e = table->buckets[ i ]->next; e; e = e->next ) {
      if ( ( *table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
        value = ( ( hash_entry * ) e->data )->value;
        break;
      }
    }
  }

  MUTEX_UNLOCK( table );

  return value;
}


static void
delete_bucket( hash_table *table, unsigned int bucket_index ) {
  if ( table->buckets[ bucket_index ]->data != NULL ) {
    delete_dlist_element( table->buckets[ bucket_index ]->data );
    table->buckets[ bucket_index ]->data = NULL;
  }

  delete_dlist( table->buckets[ bucket_index ] );
  table->buckets[ bucket_index ] = NULL;
}


/**
 * Deletes a key and its associated value from a hash_table.
 *
 * @param table a hash_table.
 * @param key the key to remove
 * @return the value deleted from the hash_table.
 */
void *
delete_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  MUTEX_LOCK( table );

  unsigned int i = get_bucket_index( table, key );

  if ( table->buckets[ i ] == NULL ) {
    MUTEX_UNLOCK( table );
    return NULL;
  }

  void *deleted = NULL;
  dlist_element *e = NULL;
  for ( e = table->buckets[ i ]->next; e; e = e->next ) {
    if ( ( *table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      hash_entry *delete_me = e->data;
      deleted = delete_me->value;
      delete_dlist_element( e );
      xfree( delete_me );
      table->length--;
      break;
    }
  }

  if ( table->buckets[ i ]->next == NULL ) {
    delete_bucket( table, i );
  }

  MUTEX_UNLOCK( table );

  return deleted;
}


/**
 * Calls the given function for each of the key/value pairs in the
 * hash_table. The function is passed the key and value of each pair,
 * and the given user_data parameter.
 *
 * @param table a hash_table.
 * @param function the function to call for each key/value pair.
 * @param user_data user data to pass to the function.
 */
void
foreach_hash( hash_table *table, void function( void *key, void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  MUTEX_LOCK( table );

  for ( dlist_element *nonempty = table->nonempty_bucket_index->next; nonempty; ) {
    int i = ( int ) ( unsigned long ) nonempty->data;
    nonempty = nonempty->next;

    if ( table->buckets[ i ] == NULL ) {
      continue;
    }

    for ( dlist_element *e = table->buckets[ i ]->next; e != NULL; ) {
      hash_entry *h = e->data;
      e = e->next;
      function( h->key, h->value, user_data );
    }
  }

  MUTEX_UNLOCK( table );
}


/**
 * Initializes a key/value pair iterator and associates it with
 * hash_table. Modifying the hash table after calling this function
 * invalidates the returned iterator.
 *
 * @param table a hash_table.
 * @param iterator an uninitialized hash_iterator
 */
void
init_hash_iterator( hash_table *table, hash_iterator *iterator ) {
  assert( table != NULL );
  assert( iterator != NULL );

  iterator->buckets = table->buckets;
  if ( table->nonempty_bucket_index->next ) {
    iterator->bucket_index = table->nonempty_bucket_index->next;
    iterator->next_bucket_index = iterator->bucket_index->next;
    assert( iterator->buckets[ ( int ) ( unsigned long ) iterator->bucket_index->data ] != NULL );
    iterator->element = iterator->buckets[ ( int ) ( unsigned long ) iterator->bucket_index->data ]->next;
  }
  else {
    iterator->bucket_index = NULL;
    iterator->element = NULL;
  }
}


/**
 * Advances iterator and retrieves the hash_entry that are now pointed
 * to as a result of this advancement. If NULL is returned, the
 * iterator becomes invalid.
 *
 * @param iterator a hash_iterator.
 * @return a hash_entry.
 */
hash_entry *
iterate_hash_next( hash_iterator *iterator ) {
  assert( iterator != NULL );

  for ( ;; ) {
    if ( iterator->bucket_index == NULL ) {
      return NULL;
    }

    dlist_element *e = iterator->element;
    if ( e == NULL ) {
      if ( iterator->next_bucket_index != NULL ) {
        iterator->bucket_index = iterator->next_bucket_index;
        iterator->next_bucket_index = iterator->next_bucket_index->next;
        int bucket_index = ( int ) ( unsigned long ) iterator->bucket_index->data;
        assert( iterator->buckets[ bucket_index ] != NULL );
        iterator->element = iterator->buckets[ bucket_index ]->next;
      }
      else {
        return NULL;
      }
    }
    else {
      iterator->element = e->next;
      return e->data;
    }
  }
}


/**
 * Deletes a hash_table.
 *
 * @param table a hash_table.
 */
void
delete_hash( hash_table *table ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );
  pthread_mutex_t *mutex = ( ( private_hash_table * ) table )->mutex;

  for ( dlist_element *nonempty = table->nonempty_bucket_index->next; nonempty; ) {
    unsigned int i = ( unsigned int ) ( unsigned long ) nonempty->data;
    nonempty = nonempty->next;
    if ( table->buckets[ i ] == NULL ) {
      continue;
    }

    for ( dlist_element *e = table->buckets[ i ]->next; e != NULL; ) {
      dlist_element *delete_me = e;
      e = e->next;
      xfree( delete_me->data );
    }

    delete_bucket( table, i );
  }
  xfree( table->buckets );

  delete_dlist( table->nonempty_bucket_index );

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
