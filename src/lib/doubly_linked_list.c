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
 * @file doubly_linked_list.c
 * Source file containing functions for doubly linked list implementation. It
 * has functions for insertion, deletion and search of elements.
 * @code
 *     // Create a doubly linked list
 *     dlistelement_p = create_dlist();
 *     ...
 *     // Add element "ABCD" at the HEAD of the created list
 *     dlistelement_p = insert_before_dlist( get_first_element( dlistelement_p ), "ABCD" );
 *     //Add another element "EFGH" after the previously inserted element "ABCD"
 *     dlistelement_p = insert_after_dlist( dlistelement_p, "EFGH" );
 *     ...
 *     // Doubly list looks like: (pointer from EFGH)->ABCD<->EFGH<-(Pointer from ABCD)
 *     ...
 *     // Find element "ABCD" from the list, where dlistelement_p can be pointing to any element of
 *     // list.
 *     dlistelement_p1 = find_element( dlistelement_p, "ABCD" );
 *     ...
 *     // Delete element "EFGH" from the list. dlistelement_p, in this case, points to the element to be deleted
 *     delete_dlist_element( dlistelement_p );
 *     ...
 *     // Delete entire list
 *     delete_dlist( dlistelement_p );
 * @endcode
 */

#include <assert.h>
#include <pthread.h>
#include "doubly_linked_list.h"
#include "wrapper.h"


/**
 * Defines an internal structure to be used within the
 * doubly_linked_list.c file for handling doubly linked list allocation and
 * management which when used by external functions
 * (through dlist_element type)are assigned to member element and support
 * for locking is provided on it through this type. Design of this type is
 * such to embed the externally visible doubly linked list into a management
 * layer. For applications, this type is never directly accessed.
 * @see dlist_element
 */
typedef struct private_dlist_element {
  dlist_element public;
  pthread_mutex_t *mutex;
} private_dlist_element;


/**
 * Allocates memory to structure of type private_dlist_element, elements of
 * which are initialized to NULL apart from mutex which is assigned the value
 * pointed by mutex pointer passed to this function.
 * @param mutex Pointer of type pthread_mutex_t
 * @return dlist_element* Pointer to dlist_element type which holds the allocated area with its members appropriately initialized
 */
static dlist_element *
create_dlist_with_mutex( pthread_mutex_t *mutex ) {
  private_dlist_element *element = xmalloc( sizeof( private_dlist_element ) );

  element->public.data = NULL;
  element->public.prev = NULL;
  element->public.next = NULL;
  element->mutex = mutex;

  return ( dlist_element * ) element;
}


/**
 * Allocates memory to empty dlist_element type structure which can be used for
 * representing allocated area. It does so by wrapping dlist_element type into
 * private_dlist_element type. It would also initialize the mutex element of
 * private_dlist_element type structure.
 * @param None
 * @return dlist_element*  Pointer to dlist_element type, which is embedded into private_dlist_element type
 */
dlist_element *
create_dlist() {
  private_dlist_element *element = ( private_dlist_element * ) create_dlist_with_mutex( NULL );

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  element->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( element->mutex, &attr );

  return ( dlist_element * ) element;
}


/**
 * Inserts a new element (of type dlist_element) before the specified element
 * (which has been passed as first argument) in the doubly linked list .
 * @param element Pointer to element in the doubly linked list before which new element is to be inserted
 * @param data Pointer to data to be inserted in doubly linked list
 * @return dlist_element* Pointer to newly inserted node in the doubly linked list
 */
dlist_element *
insert_before_dlist( dlist_element *element, void *data ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );
  pthread_mutex_t *mutex = ( ( private_dlist_element * ) element )->mutex;

  dlist_element *new_prev = create_dlist_with_mutex( mutex );

  if ( element->prev ) {
    dlist_element *old_prev = element->prev;
    new_prev->prev = old_prev;
    old_prev->next = new_prev;
  }

  element->prev = new_prev;
  new_prev->next = element;
  new_prev->data = data;

  pthread_mutex_unlock( mutex );

  return new_prev;
}


/**
 * Inserts a new element (of type dlist_element) after the specified element
 * (which has been passed as first argument) in the doubly linked list.
 * @param element Pointer to element in the doubly linked list after which new element is to be inserted
 * @param data Pointer to data to be inserted in doubly linked list
 * @return dlist_element* Pointer to newly inserted node in the doubly linked list
 */
dlist_element *
insert_after_dlist( dlist_element *element, void *data ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );
  pthread_mutex_t *mutex = ( ( private_dlist_element * ) element )->mutex;

  dlist_element *new_next = create_dlist_with_mutex( mutex );

  if ( element->next ) {
    dlist_element *old_next = element->next;
    new_next->next = old_next;
    old_next->prev = new_next;
  }

  element->next = new_next;
  new_next->prev = element;
  new_next->data = data;

  pthread_mutex_unlock( mutex );

  return new_next;
}


/**
 * Gets the first element in the doubly linked list.
 * @param element Pointer to any of the element (of type dlist_element) of doubly linked list whose first element is needed
 * @return dlist_element* Pointer to the first element in doubly linked list
 */
dlist_element *
get_first_element( dlist_element *element ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );

  while ( element->prev != NULL ) {
    element = element->prev;
  }

  pthread_mutex_unlock( ( ( private_dlist_element * ) element )->mutex );

  return element;
}


/**
 * Gets the last element in the doubly linked list.
 * @param element Pointer to any of the element (of type dlist_element) of doubly linked list whose last element is needed
 * @return dlist_element* Pointer to the last element in doubly linked list
 */
dlist_element *
get_last_element( dlist_element *element ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );

  while ( element->next != NULL ) {
    element = element->next;
  }

  pthread_mutex_unlock( ( ( private_dlist_element * ) element )->mutex );

  return element;
}


/**
 * Given a pointer to any element of the doubly linked list, finds the element
 * which matches to the data passed as second argument. Calling functions are
 * not required to find head for searches.
 * @param element Pointer to any of the node (of type dlist_element)of doubly linked list in which element needs to be searched
 * @param data Pointer to data corresponding to which element is to be searched in doubly linked list
 * @return dlist_element* Pointer to the element in doubly linked list, Null if element with corresponding data is not found
 */
dlist_element *
find_element( dlist_element *element, const void *data ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );

  dlist_element *e = NULL;

  for ( e = element; e; e = e->next ) {
    if ( e->data == data ) {
      pthread_mutex_unlock( ( ( private_dlist_element * ) element )->mutex );
      return e;
    }
  }

  pthread_mutex_unlock( ( ( private_dlist_element * ) element )->mutex );

  return NULL;
}


/**
 * Deletes node (can be HEAD or TAIL as well) from the doubly linked list.
 * @param element Pointer to node(of type dlist_element) of doubly linked list which needs to be deleted
 * @return bool True if element is deleted from the doubly linked list
 */
bool
delete_dlist_element( dlist_element *element ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );
  pthread_mutex_t *mutex = ( ( private_dlist_element * ) element )->mutex;

  if ( element->prev != NULL ) {
    element->prev->next = element->next;
  }
  if ( element->next != NULL ) {
    element->next->prev = element->prev;
  }
  xfree( ( private_dlist_element * ) element );

  pthread_mutex_unlock( mutex );

  return true;
}


/**
 * Deletes whole of the doubly linked list.
 * @param element Pointer to any of the node (of type dlist_element)of doubly linked list
 * @return bool True if doubly linked list is deleted
 */
bool
delete_dlist( dlist_element *element ) {
  if ( element == NULL ) {
    die( "element must not be NULL" );
  }

  pthread_mutex_lock( ( ( private_dlist_element * ) element )->mutex );
  pthread_mutex_t *mutex = ( ( private_dlist_element * ) element )->mutex;

  dlist_element *e = NULL;

  while ( element->prev != NULL ) {
    element = element->prev;
  }

  for ( e = element; e != NULL; ) {
    dlist_element *delete_me = e;
    e = e->next;
    xfree( ( private_dlist_element * ) delete_me );
  }

  pthread_mutex_unlock( mutex );
  xfree( mutex );

  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
