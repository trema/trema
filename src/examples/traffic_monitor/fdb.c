/*
 * Traffic counter.
 *
 * Author: Kazushi SUGYO
 *
 * Copyright (C) 2008-2012 NEC Corporation
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
#include "fdb.h"


hash_table *
create_fdb() {
  return create_hash( compare_mac, hash_mac );
}


uint16_t
lookup_fdb( hash_table *db, uint8_t *mac ) {
  fdb *fdb = lookup_hash_entry( db, mac );
  return ( uint16_t ) ( fdb == NULL ? ENTRY_NOT_FOUND_IN_FDB : fdb->port_number );

}


void
learn_fdb( hash_table *db, uint8_t *mac, uint16_t port_number ) {
  fdb *fdb = lookup_hash_entry( db, mac );
  if ( fdb == NULL ) {
    fdb = xmalloc( sizeof( fdb ) );
    memcpy( fdb->mac, mac, ETH_ADDRLEN );
    fdb->port_number = port_number;
    insert_hash_entry( db, fdb->mac, fdb );
  }
  else {
    fdb->port_number = port_number;
  }
}


static void
free_each_value( void *key, void *value, void *user_data ) {
  UNUSED( key );
  UNUSED( user_data );
  xfree( value );
}


void
delete_fdb( hash_table *db ) {
  foreach_hash( db, free_each_value, NULL );
  delete_hash( db );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
