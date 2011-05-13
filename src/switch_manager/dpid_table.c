/*
 * Author: Kazusi Sugyo
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
#include <string.h>
#include "trema.h"
#include "dpid_table.h"


static hash_table *dpid_table;


void
init_dpid_table( void ) {
  dpid_table = create_hash( compare_datapath_id, hash_datapath_id );
}


static void
free_dpid_table_walker( void *key, void *value, void *user_data ) {
  UNUSED( key );
  UNUSED( user_data );
  xfree( value );
}


void
finalize_dpid_table( void ) {
  foreach_hash( dpid_table, free_dpid_table_walker, NULL );
  delete_hash( dpid_table );
  dpid_table = NULL;
}


void
insert_dpid_entry( uint64_t *dpid ) {
  uint64_t *new_entry = xmalloc( sizeof ( uint64_t ) );
  *new_entry = *dpid;
  insert_hash_entry( dpid_table, dpid, new_entry );
}


void
delete_dpid_entry( uint64_t *dpid ) {
  uint64_t *delete_entry = delete_hash_entry( dpid_table, dpid );
  if ( delete_entry != NULL ) {
    xfree( delete_entry );
  }
}


buffer *
get_switches( void ) {
  buffer *buf = alloc_buffer_with_length( sizeof( uint64_t ) );

  hash_iterator iter;
  hash_entry *entry;
  init_hash_iterator( dpid_table, &iter );
  while ( ( entry = iterate_hash_next( &iter ) ) != NULL ) {
    uint64_t *dpid = append_back_buffer( buf, sizeof( uint64_t ) );
    *dpid = htonll( *( uint64_t * )( entry->value ) );
  }

  return buf;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
