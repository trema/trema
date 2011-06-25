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
#include "linked_list.h"
#include "wrapper.h"


bool
create_list( list_element **list ) {
  if ( list == NULL ) {
    die( "list must not be NULL" );
  }

  *list = NULL;
  return true;
}


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


unsigned int
list_length_of( const list_element *head ) {
  if ( head == NULL ) {
    return 0;
  }
  unsigned int length = 1;
  for ( list_element *e = head->next; e; e = e->next, length++ );
  return length;
}


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
