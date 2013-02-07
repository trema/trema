/*
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


#include <assert.h>
#include <openflow.h>
#include <string.h>
#include "trema.h"
#include "xid_table.h"


static uint32_t transaction_id = 0U;

#define XID_MAX_ENTRIES 4096

typedef struct xid_table {
  xid_entry_t *entries[ XID_MAX_ENTRIES ];
  hash_table *hash;
  int next_index;
} xid_table_t;

static xid_table_t xid_table;


uint32_t
generate_xid( void ) {
  uint32_t initial_value = ( transaction_id != UINT32_MAX ) ? ++transaction_id : 0;

  while ( lookup_xid_entry( transaction_id ) != NULL ) {
    if ( transaction_id != UINT32_MAX ) {
      transaction_id++;
    }
    else {
      transaction_id = 0;
    }
    if ( initial_value == transaction_id ) {
      error( "Failed to generate transaction id value." );
      transaction_id = 0;
      break;
    }
  }

  return transaction_id;
}


static xid_entry_t *
allocate_xid_entry( uint32_t original_xid, char *service_name, int index ) {
  xid_entry_t *new_entry;

  new_entry = xmalloc( sizeof( xid_entry_t ) );
  new_entry->xid = generate_xid();
  new_entry->original_xid = original_xid;
  new_entry->service_name = xstrdup( service_name );
  new_entry->index = index;

  return new_entry;
}


static void
free_xid_entry( xid_entry_t *free_entry ) {
  xfree( free_entry->service_name );
  xfree( free_entry );
}


void
init_xid_table( void ) {
  memset( &xid_table, 0, sizeof( xid_table_t ) );
  xid_table.hash = create_hash( compare_uint32, hash_uint32 );
  xid_table.next_index = 0;
}


void
finalize_xid_table( void ) {
  for ( int i = 0; i < XID_MAX_ENTRIES; i++ ) {
    if ( xid_table.entries[ i ] != NULL ) {
      free_xid_entry( xid_table.entries[ i ] );
      xid_table.entries[ i ] = NULL;
    }
  }
  delete_hash( xid_table.hash );
  xid_table.hash = NULL;
  xid_table.next_index = 0;
}


uint32_t
insert_xid_entry( uint32_t original_xid, char *service_name ) {
  xid_entry_t *new_entry;

  debug( "Inserting xid entry ( original_xid = %#" PRIx32 ", service_name = %s ).",
         original_xid, service_name );

  if ( xid_table.next_index >= XID_MAX_ENTRIES ) {
    xid_table.next_index = 0;
  }

  if ( xid_table.entries[ xid_table.next_index ] != NULL ) {
    delete_xid_entry( xid_table.entries[ xid_table.next_index ] );
  }

  new_entry = allocate_xid_entry( original_xid, service_name, xid_table.next_index );
  xid_entry_t *old = insert_hash_entry( xid_table.hash, &new_entry->xid, new_entry );
  if ( old != NULL ) {
    xid_table.entries[ old->index ] = NULL;
    free_xid_entry( old );
  }
  xid_table.entries[ xid_table.next_index ] = new_entry;
  xid_table.next_index++;

  return new_entry->xid;
}


void
delete_xid_entry( xid_entry_t *delete_entry ) {
  debug( "Deleting xid entry ( xid = %#" PRIx32 ", original_xid = %#" PRIx32 ", service_name = %s, index = %d ).",
         delete_entry->xid, delete_entry->original_xid, delete_entry->service_name, delete_entry->index );

  xid_entry_t *deleted = delete_hash_entry( xid_table.hash, &delete_entry->xid );

  if ( deleted == NULL ) {
    error( "Failed to delete xid entry ( xid = %#" PRIx32 " ).", delete_entry->xid );
    free_xid_entry( ( xid_entry_t * ) delete_entry );
    return;
  }

  xid_table.entries[ deleted->index ] = NULL;
  free_xid_entry( deleted );
}


xid_entry_t *
lookup_xid_entry( uint32_t xid ) {
  return lookup_hash_entry( xid_table.hash, &xid );
}


static void
dump_xid_entry( xid_entry_t *entry ) {
  info( "xid = %#" PRIx32 ", original_xid = %#" PRIx32 ", service_name = %s, index = %d",
        entry->xid, entry->original_xid, entry->service_name, entry->index );
}


void
dump_xid_table( void ) {
  hash_iterator iter;
  hash_entry *e;

  info( "#### XID TABLE ####" );
  init_hash_iterator( xid_table.hash, &iter );
  while ( ( e = iterate_hash_next( &iter ) ) != NULL ) {
    dump_xid_entry( e->value );
  }
  info( "#### END ####" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
