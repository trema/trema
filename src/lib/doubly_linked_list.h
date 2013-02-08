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
 * @brief linked lists containing pointers to data, with the ability
 * to iterate over the list in both directions.
 *
 * Each element in the list contains a piece of data, together with
 * pointers which link to the previous and next elements in the
 * list. Using these pointers it is possible to move through the list
 * in both directions (unlike the linked-list which only allows
 * movement through the list in the forward direction).
 *
 * @code
 * // Create a doubly linked list ("alpha" <=> "bravo" <=> "charlie")
 * dlist_element *alpha = create_dlist();
 * dlist_element *bravo = insert_after_dlist( alpha, "bravo" );
 * dlist_element *charlie = insert_after_dlist( bravo, "charlie" );
 *
 * // Find element "charlie" from the list
 * find_element( alpha, "charlie" ); // => charlie
 * find_element( alpha, "delta" ); // => NULL
 *
 * // Delete element "bravo" from the list
 * delete_dlist_element( bravo ); // => true
 *
 * // Delete entire list
 * delete_dlist( alpha );
 * @endcode
 */


#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H


#include "bool.h"


/**
 * The dlist_element struct is used for each element in a
 * doubly-linked list.
 */
typedef struct dlist_element {
  void *data; /**< Holds the element's data, which can be a pointer to any kind of data. */
  struct dlist_element *prev; /**< Contains the link to the previous element in the list. */
  struct dlist_element *next; /**< Contains the link to the next element in the list. */
} dlist_element;


dlist_element *create_dlist( void );
dlist_element *insert_before_dlist( dlist_element *element, void *data );
dlist_element *insert_after_dlist( dlist_element *element, void *data );
dlist_element *get_first_element( dlist_element *element );
dlist_element *get_last_element( dlist_element *element );
dlist_element *find_element( dlist_element *element, const void *data );
bool delete_dlist_element( dlist_element *element );
bool delete_dlist( dlist_element *element );


#endif // DOUBLY_LINKED_LIST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
