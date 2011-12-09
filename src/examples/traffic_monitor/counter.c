/*
 * Traffic counter.
 *
 * Author: Kazushi SUGYO
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
#include "counter.h"


hash_table *
create_counter() {
  return create_hash( compare_mac, hash_mac );
}


void
add_counter( hash_table *db, uint8_t *mac, uint64_t packet_count, uint64_t byte_count ) {
  counter *counter = lookup_hash_entry( db, mac );
  if ( counter == NULL ) {
    counter = xmalloc( sizeof( counter ) );
    memcpy( counter->mac, mac, ETH_ADDRLEN );
    counter->packet_count = 0;
    counter->byte_count = 0;
    insert_hash_entry( db, counter->mac, counter );
  }
  counter->packet_count += packet_count;
  counter->byte_count += byte_count;
}


void
foreach_counter( hash_table *db, void function( uint8_t *, counter *, void * ), void *user_data ) {
  foreach_hash( db, ( void ( * )( void *, void *, void * ) ) function, user_data );
}


static void
free_each_value( void *key, void *value, void *user_data ) {
  UNUSED( key );
  UNUSED( user_data );
  xfree( value );
}


void
delete_counter( hash_table *db ) {
  foreach_hash( db, free_each_value, NULL );
  delete_hash( db );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
