/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include "trema.h"
#include "subscriber_table.h"


#ifdef UNIT_TESTING

#define static

#endif // UNIT_TESTING


static list_element *subscriber_table;


static subscriber_entry *
allocate_subscriber_entry( const char *name ) {
  subscriber_entry *new_entry;

  new_entry = xmalloc( sizeof( subscriber_entry ) );
  new_entry->name = xstrdup( name );

  return new_entry;
}


static void
free_subscriber_entry( subscriber_entry *entry ) {
  xfree( entry->name );
  xfree( entry );
}


bool
insert_subscriber_entry( const char *name ) {
  subscriber_entry *entry = lookup_subscriber_entry( name );
  if ( entry != NULL ) {
    debug( "subscriber '%s' already subscribed", name );
    return false;
  }
  entry = allocate_subscriber_entry( name );
  insert_in_front( &subscriber_table, entry );

  return true;
}


void
delete_subscriber_entry( subscriber_entry *entry ) {
  delete_element( &subscriber_table, entry );
  free_subscriber_entry( entry );
}


subscriber_entry *
lookup_subscriber_entry( const char *name ) {
  subscriber_entry *entry;
  list_element *list;

  for ( list = subscriber_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( strcmp( entry->name, name ) == 0 ) {
      // found
      return entry;
    }
  }

  return NULL;
}


void
foreach_subscriber( void function( subscriber_entry *entry, void *user_data ),
                    void *user_data ) {
  list_element *list, *next;

  for ( list = subscriber_table; list != NULL; list = next ) {
    next = list->next;
    function( list->data, user_data );
  }
}


void
init_subscriber_table( void ) {
  create_list( &subscriber_table );
}


void
finalize_subscriber_table( void ) {
  list_element *list, *next;

  for ( list = subscriber_table; list != NULL; list = next ) {
    next = list->next;
    free_subscriber_entry( list->data );
  }
  delete_list( subscriber_table );
  subscriber_table = NULL;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
