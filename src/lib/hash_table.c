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


/**
 * Default compare function which is used for creating a
 * hash_table. In case a custom function for matching keys is
 * required, same should be passed to create_hash function.
 * @param x A void type pointer to constant identifier
 * @param y A void type pointer to constant identifier
 * @return bool True if equal, else False
 * @see create_hash
 */
bool
compare_atom( const void *x, const void *y ) {
  return x == y;
}


/**
 * Default hash function which is used for creating a hash_table. In
 * case a custom function for creating hashes is required, same should
 * be passed to create_hash function.
 * @param key Pointer to a address, for which hash key is generated
 * @return unsigned int Key value after right shifting by 2 bits
 * @see create_hash
 */
unsigned int
hash_atom( const void *key ) {
  return ( unsigned int ) ( ( unsigned long ) key >> 2 );
}


/**
 * Creates a hash table and initialize it to NULL.
 * @param compare Function pointer to compare_function
 * @param hash Function pointer to hash_function
 * @return hash_table* Pointer to created hash table
 */
hash_table *
create_hash( const compare_function compare, const hash_function hash ) {
  private_hash_table *table = xmalloc( sizeof( private_hash_table ) );

  table->public.number_of_buckets = default_hash_size;
  table->public.compare = compare ? compare : compare_atom;
  table->public.hash = hash ? hash : hash_atom;
  table->public.length = 0;
  table->public.buckets = xmalloc( sizeof( dlist_element * ) * default_hash_size );
  unsigned int i;
  for ( i = 0; i < table->public.number_of_buckets; i++ ) {
    table->public.buckets[ i ] = create_dlist();
    assert( table->public.buckets[ i ] != NULL );
  }
  table->public.nonempty_bucket_index = create_dlist();
  assert( table->public.nonempty_bucket_index != NULL );

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  table->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( table->mutex, &attr );

  return ( hash_table * ) table;
}


/**
 * Returns the index of hash bucket pointed to be the key in the
 * hash_table table.
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
 * Searches for an element in the bucket pointed by the key.
 * @param table Pointer to hash table in which element is to be searched
 * @param key Pointer to constant key identifier
 * @return void* Pointer to identified element in the bucket
 */
static void *
find_list_element_from_buckets( const hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  dlist_element *e = NULL;
  unsigned int i = get_bucket_index( table, key );
  for ( e = table->buckets[ i ]->next; e; e = e->next ) {
    if ( ( *table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      break;
    }
  }
  return e;
}


/**
 * Inserts a new element into an existing hash table. In case the hash
 * entry matches an existing entry, pointer to it is returned, else
 * NULL is returned.
 * @param table Pointer to hash table in which element is to be inserted
 * @param key Pointer to new element's key
 * @param value Pointer to associated data
 * @return void* Pointer to data associated with a matching key-value pair, else NULL if no match
 */
void *
insert_hash_entry( hash_table *table, void *key, void *value ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
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


/**
 * Performs lookup for value associated with the key in the hash
 * table.
 * @param table Pointer to hash table
 * @param key Pointer to key
 * @return void* Pointer to value associated with the key, else NULL
 */
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


/**
 * Deletes an entry referred to by the key in the hash table.
 * @param table Pointer to hash table from which element is to be deleted
 * @param key Pointer to element's key which is to be deleted
 * @return void* Pointer to data that was associated with the key, else NULL
 */
void *
delete_hash_entry( hash_table *table, const void *key ) {
  assert( table != NULL );
  assert( key != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  dlist_element *e = find_list_element_from_buckets( table, key );
  if ( e != NULL ) {
    hash_entry *delete_me = e->data;
    void *deleted = delete_me->value;
    delete_dlist_element( e );
    xfree( delete_me );
    if ( table->buckets[ i ]->next == NULL ) {
      delete_dlist_element( table->buckets[ i ]->data );
      table->buckets[ i ]->data = NULL;
    }
    table->length--;
    pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
    return deleted;
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
  return NULL;
}


/**
 * Searches (maps) for the entry in hash_table corresponding to the
 * key passed as argument. A user defined function is called if the
 * search is successful. The user defiend function is passed as a
 * function pointer. The definition of the function pointer is:
 * @code
 *      void <name of function> ( void *value, void *user_data);
 *      // value is the data being represented by hash key found
 *      // user_data is the 3rd argument of map_hash, which is passed directly to this function
 * @endcode
 * @param table Pointer to hash table in which element is to be searched
 * @param key Pointer to key for which function is to be called
 * @param function Pointer to funtion to be called if a matching entry is found
 * @param user_data A void pointer to user data
 * @return None
 */
void
map_hash( hash_table *table, const void *key, void function( void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  unsigned int i = get_bucket_index( table, key );
  dlist_element *e = NULL;
  for ( e = table->buckets[ i ]->next; e; e = e->next ) {
    if ( ( table->compare )( key, ( ( hash_entry * ) e->data )->key ) ) {
      function( ( ( hash_entry * ) e->data )->value, user_data );
    }
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
}


/**
 * Iterates over each hash entry in the hash_Table. Takes as argument
 * a function pointer which is called once for each bucket of the
 * table being pointed to by passed argument. It is used as an
 * iterator over each hash entry belonging to each bucket.
 * Example:
 * @code
 *     // Create a Hash Table
 *     hashtable_p = create_hash(<compare function>, <hash function>)
 *     ...
 *     // add entries to the hash table
 *     insert_hash_entry(key, value); // Multiple such inserts take place
 *     ...
 *     // Iterating over ALL hash entries so far in table
 *     // Second argument is pointer to a function to call for each entry
 *     // argument to this function
 *     foreach_hash(table, <function which is to be executed for each entry>);
 * @endcode
 * @param table Pointer to hash table
 * @param function The action function
 * @param user_data A void pointer to user data
 * @return None
 */
void
foreach_hash( hash_table *table, void function( void *key, void *value, void *user_data ), void *user_data ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );

  dlist_element *nonempty = NULL;
  for ( nonempty = table->nonempty_bucket_index->next; nonempty; ) {
    int i = ( int ) ( unsigned long ) nonempty->data;
    nonempty = nonempty->next;
    dlist_element *e = NULL;
    for ( e = table->buckets[ i ]->next; e; ) {
      hash_entry *he = e->data;
      e = e->next;
      function( he->key, he->value, user_data );
    }
  }

  pthread_mutex_unlock( ( ( private_hash_table * ) table )->mutex );
}


/**
 * Initializes an iterator over the hash_table. This iterator would
 * then be used with function iterate_hash_next to move over each hash
 * entry one at a time. This can be used for getting data associated
 * with each hash entry.
 * @param table Pointer to hash table to iterate over
 * @param iter Pointer to hash_iterator that needs to be initialized
 * @return None
 * @see iterate_hash_next
 */
void
init_hash_iterator( hash_table *table, hash_iterator *iter ) {
  assert( table != NULL );
  assert( iter != NULL );

  iter->buckets = table->buckets;
  if ( table->nonempty_bucket_index->next ) {
    iter->bucket_index = table->nonempty_bucket_index->next;
    iter->next_bucket_index = iter->bucket_index->next;
    iter->element = iter->buckets[ ( int ) ( unsigned long ) iter->bucket_index->data ]->next;
  }
  else {
    iter->bucket_index = NULL;
    iter->element = NULL;
  }
}


/**
 * Moves the hash iterator forward to next hash entry. It uses the
 * iterator initialized by init_hash_iterator. It can be used to fetch
 * the data associated with each hash entry.
 * Example:
 * @code
 *     // Assuming a hash table pointed to by hashtable_p
 *     // Initializing the hash iterator
 *     hash_iterator iter;
 *     hash_entry *p = NULL;
 *     init_hash_iterator(hashtable_p, &iter);
 *     // Looping over the hash entries
 *     while ( ( p = iterate_hash_next( &iter ) ) != NULL ) {
 *         // Perform some action on the hash_entry.
 *         // For example, dump data pointed to be hash_entry on terminal
 *         dump(p->value);
 *         ...
 *     }
 * @endcode        
 * @param iter Pointer to hash_iterator to move forward
 * @return hash_entry* Pointer to valid hash entry, else NULL
 * @see init_hash_iterator
 */
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


/**
 * Releases all the memory held by the hash table.
 * @param hash_table Pointer to hash table which needs to be deleted
 * @return None
 */
void
delete_hash( hash_table *table ) {
  assert( table != NULL );

  pthread_mutex_lock( ( ( private_hash_table * ) table )->mutex );
  pthread_mutex_t *mutex = ( ( private_hash_table * ) table )->mutex;

  bool dlist_deleted = false;

  unsigned int i;
  for ( i = 0; i < table->number_of_buckets; i++ ) {
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
