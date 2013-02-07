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
 * @brief Linked lists containing pointers to data, limited to
 * iterating over the list in one direction.
 *
 * Each element in the list contains a piece of data, together with a
 * pointer which links to the next element in the list. Using this
 * pointer it is possible to move through the list in one direction
 * only (unlike the Doubly-Linked Lists which allow movement in both
 * directions).
 *
 * @code
 * // Create a linked list ("alpha" => "bravo" => "charlie")
 * list_element *list;
 * create_list( &list );
 * append_to_tail( &list, "alpha" );
 * append_to_tail( &list, "bravo" );
 * append_to_tail( &list, "charlie" );
 *
 * // Delete element "bravo" from the list
 * delete_element( &list, "bravo" ); // => true
 *
 * // Delete entire list
 * delete_list( list );
 * @endcode
 */


#ifndef LINKED_LIST_H
#define LINKED_LIST_H


#include "bool.h"


/**
 * The list_element struct is used for each element in the linked
 * list.
 */
typedef struct list_element {
  struct list_element *next; /**< Contains the link to the next element in the list. */
  void *data; /**< Holds the element's data, which can be a pointer to any kind of data. */
} list_element;


bool create_list( list_element **list );
bool insert_in_front( list_element **head, void *data );
bool insert_before( list_element **head, const void *sibling, void *data );
bool append_to_tail( list_element **head, void *data );
unsigned int list_length_of( const list_element *head );
void iterate_list( list_element *head, void function( void *data, void *user_data ), void *user_data );
void *find_list_custom( list_element *head, bool function( void *data, void *user_data ), void *user_data );
bool delete_element( list_element **head, const void *data );
bool delete_list( list_element *head );


#endif // LINKED_LIST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
