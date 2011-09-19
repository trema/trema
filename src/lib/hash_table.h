/*
 * Hash table library.
 *
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
 * @file 
 *
 * @brief Hash table implementation
 *
 * Function declarations and type definitions of Hash table implementation
 * @code
 * // Create a hash table with default hash and compare function
 * hash_table *table = create_hash( NULL, NULL );
 *
 * // Insert key:value pairs into the created hash table
 * insert_hash_entry( table, "A", "Apple" );
 * insert_hash_entry( table, "B", "Bat" );
 * insert_hash_entry( table, "C", "Cat" );
 *
 * // Find some data
 * void *apple = lookup_hash_entry( table, "A" );
 * void *bat = lookup_hash_entry( table, "B" );
 * void *cat = lookup_hash_entry( table, "C" );
 *
 * // Delete the table
 * delete_hash( table );
 * @endcode
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H


#include "doubly_linked_list.h"


typedef unsigned int ( *hash_function )( const void *key );
typedef bool ( *compare_function )( const void *x, const void *y );


/**
 * Individual entry in hash table
 */
typedef struct {
  void *key;
  void *value;
} hash_entry;


/**
 * Parameters associated with a hash table
 */
typedef struct {
  unsigned int number_of_buckets; /*!<Total number of buckets allocated in hash table*/
  compare_function compare; /*!<Function pointer to compare items*/
  hash_function hash; /*!<Pointer to hash function*/
  unsigned int length; /*!<Total number of entries in hash table*/
  dlist_element **buckets; /*!<Pointer to buckets in hash table*/
  dlist_element *nonempty_bucket_index; /*!<List of non-empty buckets in hash table*/
} hash_table;


/**
 * Parameters used to iterate over hash table
 */
typedef struct {
  dlist_element **buckets; /*!<Pointer to buckets in hash table*/
  dlist_element *bucket_index; /*!<Pointer to non-empty bucket index */
  dlist_element *next_bucket_index; /*!<Pointer to next non-empty bucket index*/
  dlist_element *element; /*!<Pointer to hash entries in a bucket*/
} hash_iterator;


hash_table *create_hash( const compare_function compare, const hash_function hash );
void *insert_hash_entry( hash_table *table, void *key, void *value );
void *lookup_hash_entry( hash_table *table, const void *key );
void *delete_hash_entry( hash_table *table, const void *key );
void map_hash( hash_table * table, const void *key, void function( void *value, void *user_data ), void *user_data );
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
