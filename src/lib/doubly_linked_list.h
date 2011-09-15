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
 * @brief File containing function declarations and type definitions for doubly
 * linked list implementation
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
