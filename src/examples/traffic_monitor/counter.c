/*
 * Traffic counter.
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


#include "trema.h"
#include "counter.h"


hash_table *
create_counter() {
  return create_hash( compare_mac, hash_mac );
}


void
add_counter( hash_table *db, uint8_t *mac, uint64_t packet_count, uint64_t byte_count ) {
  counter *entry = lookup_hash_entry( db, mac );
  if ( entry == NULL ) {
    entry = xmalloc( sizeof( counter ) );
    memcpy( entry->mac, mac, ETH_ADDRLEN );
    entry->packet_count = 0;
    entry->byte_count = 0;
    insert_hash_entry( db, entry->mac, entry );
  }
  entry->packet_count += packet_count;
  entry->byte_count += byte_count;
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
