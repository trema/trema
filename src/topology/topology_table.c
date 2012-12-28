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


#include <assert.h>
#include "trema.h"
#include "topology_table.h"


static list_element *sw_table;


link_to *
update_link_to( port_entry *port, uint64_t *datapath_id, uint16_t port_no, bool up ) {
  if ( port->link_to != NULL ) {
    xfree( port->link_to );
    port->link_to = NULL;
  }

  port->link_to = xmalloc( sizeof( link_to ) );
  port->link_to->datapath_id = *datapath_id;
  port->link_to->port_no = port_no;
  port->link_to->up = up;

  return port->link_to;
}


void
delete_link_to( port_entry *port ) {
  if ( port->link_to == NULL ) {
    return;
  }
  xfree( port->link_to );
  port->link_to = NULL;
}


static port_entry *
allocate_port_entry( sw_entry *sw, uint16_t port_no, const char *name ) {
  port_entry *new_entry;

  new_entry = xmalloc( sizeof( port_entry ) );
  new_entry->sw = sw;
  new_entry->port_no = port_no;
  strncpy( new_entry->name, name, sizeof( new_entry->name ) );
  new_entry->name[ OFP_MAX_PORT_NAME_LEN - 1] = '\0';
  new_entry->up = false;
  new_entry->external = false;
  new_entry->link_to = NULL;

  return new_entry;
}


static void
free_port_entry( port_entry *free_entry ) {
  if ( free_entry->link_to != NULL ) {
    delete_link_to( free_entry );
  }
  xfree( free_entry );
}


port_entry *
update_port_entry( sw_entry *sw, uint16_t port_no, const char *name ) {
  port_entry *entry;

  entry = lookup_port_entry( sw, port_no, name );
  if ( entry != NULL ) {
      return entry;
  }
  entry = allocate_port_entry( sw, port_no, name );
  insert_in_front( &( sw->port_table ), entry );

  return entry;
}


void
delete_port_entry( sw_entry *sw, port_entry *port ) {
  assert( port->link_to == NULL );
  delete_element( &( sw->port_table ), port );
  free_port_entry( port );
}


port_entry *
lookup_port_entry( sw_entry *sw, uint16_t port_no, const char *name ) {
  port_entry *store = NULL, *entry;
  list_element *list;

  for ( list = sw->port_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( name == NULL ) {
      if ( entry->port_no == port_no ) {
        return entry;
      }
    } else {
      if ( strcmp( entry->name, name ) == 0 ) {
        return entry;
      }
      if ( entry->port_no == port_no ) {
        store = entry;
      }
    }
  }
  if ( store != NULL ) {
    return store;
  }

  return NULL;
}


static void
foreach_port_entry_in_sw( sw_entry *sw,
                          void function( port_entry *entry, void *user_data ),
                          void *user_data ) {
  list_element *list, *next;

  for ( list = sw->port_table; list != NULL; list = next ) {
    next = list->next;
    function( list->data, user_data );
  }
}

void
foreach_port_entry( void function( port_entry *entry, void *user_data ),
                    void *user_data ) {
  list_element *list, *next;

  for ( list = sw_table; list != NULL; list = next ) {
    next = list->next;
    foreach_port_entry_in_sw( list->data, function, user_data );
  }
}


static sw_entry *
allocate_sw_entry( uint64_t *datapath_id ) {
  sw_entry *new_entry;

  new_entry = xmalloc( sizeof( sw_entry ) );
  new_entry->datapath_id = *datapath_id;
  new_entry->id = get_transaction_id();
  create_list( &( new_entry->port_table ) );

  return new_entry;
}


static void
free_sw_entry( sw_entry *sw ) {
  list_element *list;

  for ( list = sw->port_table; list != NULL; list = list->next ) {
    free_port_entry( list->data );
  }
  delete_list( sw->port_table );
  xfree( sw );
}


sw_entry *
update_sw_entry( uint64_t *datapath_id ) {
  sw_entry *entry;

  entry = lookup_sw_entry( datapath_id );
  if ( entry != NULL ) {
      return entry;
  }
  entry = allocate_sw_entry( datapath_id );
  insert_in_front( &sw_table, entry );

  return entry;
}


void
delete_sw_entry( sw_entry *sw ) {
  assert( sw->port_table == NULL );
  delete_element( &sw_table, sw );
  free_sw_entry( sw );
}


sw_entry *
lookup_sw_entry( uint64_t *datapath_id ) {
  sw_entry *entry;
  list_element *list;

  for ( list = sw_table; list != NULL; list = list->next ) {
    entry = list->data;
    if ( entry->datapath_id == *datapath_id ) {
      // found
      return entry;
    }
  }

  return NULL;
}


void
foreach_sw_entry( void function( sw_entry *entry, void *user_data ),
                  void *user_data ) {
  list_element *list, *next;

  for ( list = sw_table; list != NULL; list = next ) {
    next = list->next;
    function( list->data, user_data );
  }
}


void
init_topology_table( void ) {
  create_list( &sw_table );
}


void
finalize_topology_table( void ) {
  list_element *list;

  for ( list = sw_table; list != NULL; list = list->next ) {
    free_sw_entry( list->data );
  }
  delete_list( sw_table );
  sw_table = NULL;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
