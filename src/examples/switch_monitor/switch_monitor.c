/*
 * Monitor switch on/off
 *
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


#include <inttypes.h>
#include "trema.h"


static void
join( char *result, const list_element *switches ) {
  const char comma[] = ", ";
  const list_element *element;
  for ( element = switches; element != NULL; element = element->next ) {
    char tmp[ 19 ]; // "0x" + 64bits in hex + '\0'
    snprintf( tmp, sizeof( tmp ), "%#" PRIx64, *( uint64_t * ) element->data );
    strcat( result, tmp );
    strcat( result, comma );
  }
  result[ strlen( result ) - strlen( comma ) ] = '\0';
}


static void
create_switches( list_element **switches ) {
  create_list( switches );
}


static void
delete_switches( list_element *switches ) {
  list_element *element;
  for ( element = switches; element != NULL; element = element->next ) {
    if ( element->data != NULL ) {
      xfree( element->data );
    }
  }
  delete_list( switches );
}


static void
insert_datapath_id( list_element **switches, uint64_t datapath_id ) {
  list_element *element = NULL;
  for ( element = *switches; element != NULL; element = element->next ) {
    if ( *( ( uint64_t * ) element->data ) > datapath_id ) {
      break;
    }
    if ( *( ( uint64_t * ) element->data ) == datapath_id ) {
      // already exists
      return;
    }
  }
  uint64_t *new = xmalloc( sizeof( uint64_t ) );
  *new = datapath_id;
  if ( element == NULL ) {
    append_to_tail( switches, new );
  }
  else if ( element == *switches ) {
    insert_in_front( switches, new );
  }
  else {
    insert_before( switches, element->data, new );
  }
}


static void
delete_datapath_id( list_element **switches, uint64_t datapath_id ) {
  list_element *element = NULL;
  for ( element = *switches; element != NULL; element = element->next ) {
    if ( *( ( uint64_t * ) element->data ) == datapath_id ) {
      void *data = element->data;
      delete_element( switches, data );
      xfree( data );
      return;
    }
  }
  // not found
}


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  list_element **switches = user_data;
  insert_datapath_id( switches, datapath_id );
  info( "Switch %#" PRIx64 " is UP", datapath_id );
}


static void
handle_switch_disconnected( uint64_t datapath_id, void *user_data ) {
  list_element **switches = user_data;
  delete_datapath_id( switches, datapath_id );
  info( "Switch %#" PRIx64 " is DOWN", datapath_id );
}


static void
show_switches( void *user_data ) {
  list_element **switches = user_data;
  unsigned int num_switch = list_length_of( *switches );

  char *list = xmalloc( 20 * num_switch + 1 ); // 20 = dpid string (18 chars) + ", "
  list[ 0 ] = '\0';
  join( list, *switches );
  info( "All switches = %s", list );
  xfree( list );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  list_element *switches;
  create_switches( &switches );

  add_periodic_event_callback( 10, show_switches, &switches );

  set_switch_ready_handler( handle_switch_ready, &switches );
  set_switch_disconnected_handler( handle_switch_disconnected, &switches );

  start_trema();

  delete_switches( switches );

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
