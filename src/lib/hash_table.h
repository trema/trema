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


/**
 * @file
 *
 * @brief Associations between keys and values so that given a key the
 * value can be found quickly.
 *
 * @code
 * // Create hash table.
 * table = create_hash( compare_string, hash_string );
 *
 * // Insert three key/value pairs.
 * insert_hash_entry( table, "alpha", &object_a );
 * insert_hash_entry( table, "bravo", &object_b );
 * insert_hash_entry( table, "charlie", &object_c );
 *
 * // Look up by a key = "alpha".
 * lookup_hash_entry( table, "alpha" ); // => object_a
 *
 * // Delete entire hash table.
 * delete_hash( table );
 * @endcode
 */


#ifndef HASH_TABLE_H
#define HASH_TABLE_H


#include "doubly_linked_list.h"


/**
 * The function is passed a key and should return a unsigned int hash
 * value.
 *
 * The hash values should be evenly distributed over a fairly large
 * range. The modulus is taken with the hash table size (a prime
 * number) to find the 'bucket' to place each key into. The function
 * should also be very fast, since it is called for each key lookup.
 *
 * @param key a key.
 * @return the hash value corresponding to the key.
 */
typedef unsigned int ( *hash_function )( const void *key );

/**
 * Specifies the type of a function used to test two values for
 * equality. The function should return true if both values are equal
 * and false otherwise.
 *
 * @param x a value.
 * @param y a value to compare with.
 * @return true if a = b; false otherwise.
 */
typedef bool ( *compare_function )( const void *x, const void *y );


/**
 * The hash_entry struct is an opaque data structure to represent a
 * key/value pair of hash_table.
 */
typedef struct {
  void *key;
  void *value;
} hash_entry;


/**
 * The hash_table struct is an opaque data structure to represent a
 * hash_table. It should only be accessed via the following functions.
 */
typedef struct {
  unsigned int number_of_buckets;
  compare_function compare;
  hash_function hash;
  unsigned int length;
  dlist_element **buckets;
  dlist_element *nonempty_bucket_index;
} hash_table;


/**
 * A hash_iterator structure represents an iterator that can be used
 * to iterate over the elements of a hash_table. hash_iterator
 * structures are typically allocated on the stack and then
 * initialized with init_hash_iterator().
 */
typedef struct {
  dlist_element **buckets;
  dlist_element *bucket_index;
  dlist_element *next_bucket_index;
  dlist_element *element;
} hash_iterator;


hash_table *create_hash( const compare_function compare, const hash_function hash );
hash_table *create_hash_with_size( const compare_function compare, const hash_function hash, unsigned int size );
void *insert_hash_entry( hash_table *table, void *key, void *value );
void *lookup_hash_entry( hash_table *table, const void *key );
void *delete_hash_entry( hash_table *table, const void *key );
void foreach_hash( hash_table * table, void function( void *key, void *value, void *user_data ), void *user_data );
void init_hash_iterator( hash_table *table, hash_iterator *iter );
hash_entry *iterate_hash_next( hash_iterator *iter );
void delete_hash( hash_table *table );

bool compare_atom( const void *x, const void *y );
unsigned int hash_atom( const void *key );


#endif // HASH_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
