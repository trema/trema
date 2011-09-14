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
 * @file 
 *
 * @brief Linked List Implementation
 *
 * File containing various functions for Single Linked List implemenation.
 * Helper functions for insertion, deletion and modification of elements are provided.
 *
 * @code 
 * // Initialize a Linked List HEAD
 * list_element **HEAD;
 * bool ret = create_list( HEAD );
 * // On failure to create, FALSE (0) is returned
 *
 * // Insert elements into the linked list
 * insert_in_front( HEAD, "Apple" );      // For inserting in front (like a Stack)
 * append_to_tail( HEAD, "Ball" );        // Insert at the end (like a Queue)
 * insert_before( HEAD, "Ball", "Cat" );  // Insert before a specified Node
 *
 * // Linked list looks like Apple->Cat->Ball
 *
 * // Delete a node
 * delete_element( HEAD, "Ball" );
 * // Delete the complete list
 * delete_list( *HEAD );
 * @endcode
 */

#include <assert.h>
#include "linked_list.h"
#include "wrapper.h"


/**
 * Creates list and initializes the value of its HEAD to NULL.
 * @param list Double pointer to list_element
 * @return bool True on successful completion, else False
 */
bool
create_list( list_element **list ) {
  if ( list == NULL ) {
    die( "list must not be NULL" );
  }

  *list = NULL;
  return true;
}


/** 
 * Inserts the element (of type list_element) at head of the list. Error is
 * reported if passed HEAD is NULL.
 * @param head Double pointer to head of the list
 * @param data Pointer to data to be inserted in the list
 * @return bool True on successful completion, else False
 */
bool
insert_in_front( list_element **head, void *data ) {
  if ( head == NULL ) {
    die( "head must not be NULL" );
  }

  list_element *old_head = *head;
  list_element *new_head = xmalloc( sizeof( list_element ) );

  new_head->data = data;
  *head = new_head;
  ( *head )->next = old_head;
  return true;
}


/** 
 * Inserts new element(of type list_element) before element which has data as sibling.
 * @param head Double pointer to head of the list
 * @param sibling Pointer to data before which new element needs to be inserted
 * @param data Pointer to data which is to be added in the new element inserted
 * @return bool True if element is inserted successfully, else False
 */
bool
insert_before( list_element **head, const void *sibling, void *data ) {
  if ( head == NULL ) {
    die( "head must not be NULL" );
  }

  list_element *e;
  list_element *new_element;

  for ( e = *head; e->next != NULL; e = e->next ) {
    if ( e->next->data == sibling ) {
      new_element = xmalloc( sizeof( list_element ) );
      new_element->next = e->next;
      new_element->data = data;
      e->next = new_element;
      return true;
    }
  }

  return false;
}


/** 
 * Inserts an element (of type list_element) at the end of list.
 * @param head Double pointer to head of the list
 * @param data Pointer to the data which needs to be added to the list
 * @return bool True if element is inserted successfully, else False
 */
bool
append_to_tail( list_element **head, void *data ) {
  if ( head == NULL ) {
    die( "head must not be NULL" );
  }

  list_element *e;
  list_element *new_tail = xmalloc( sizeof( list_element ) );

  new_tail->data = data;
  new_tail->next = NULL;

  if ( *head == NULL ) {
    *head = new_tail;
    return true;
  }

  for ( e = *head; e->next != NULL; e = e->next );
  e->next = new_tail;
  return true;
}


/** 
 * Calculates the total number of elements in the list.
 * @param head Pointer to head of the list
 * @return unsigned int Length of list
 */
unsigned int
list_length_of( const list_element *head ) {
  if ( head == NULL ) {
    return 0;
  }
  unsigned int length = 1;
  for ( list_element *e = head->next; e; e = e->next, length++ );
  return length;
}


/** 
 * Releases the element which has data element same as data parameter
 * of the function.
 * @param head Double pointer to head of the list
 * @param data Pointer to data which needs to be freed from the list
 * @return bool True if data is freed successfuly, else False
 */
bool
delete_element( list_element **head, const void *data ) {
  if ( head == NULL ) {
    die( "head must not be NULL" );
  }

  list_element *e = *head;
  list_element *delete_me;

  if ( e->data == data ) {
    *head = e->next;
    xfree( e );
    return true;
  }

  for ( ; e->next != NULL; e = e->next ) {
    if ( e->next->data == data ) {
      delete_me = e->next;
      e->next = e->next->next;
      xfree( delete_me );
      return true;
    }
  }

  return false;
}


/** 
 * Releases memory allocated to each entry of type list_element from the list.
 * @param head Pointer to head of the list
 * @return bool True if deletion of list is successful, else False
 */
bool
delete_list( list_element *head ) {
  list_element *e;
  list_element *delete_me;

  for ( e = head; e != NULL; ) {
    delete_me = e;
    e = e->next;
    xfree( delete_me );
  }
  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
