/*
 * Linked list library.
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
 * @file linked_list.h
 * This header file contains type definitions and function declarations for 
 * Linked list implementation.
 * @see linked_list.c
 */


#ifndef LINKED_LIST_H
#define LINKED_LIST_H


#include "bool.h"


/** 
 * This is the type that specifies a node for linked list
 */
typedef struct list_element {
  struct list_element *next; /*!< pointer to next list_element*/
  void *data; /*!< pointer to data*/
} list_element;


bool create_list( list_element **list );
bool insert_in_front( list_element **head, void *data );
bool insert_before( list_element **head, const void *sibling, void *data );
bool append_to_tail( list_element **head, void *data );
unsigned int list_length_of( const list_element *head );
bool delete_element( list_element **head, const void *data );
bool delete_list( list_element *head );


#endif // LINKED_LIST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
