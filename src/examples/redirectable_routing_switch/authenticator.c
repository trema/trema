/*
 * Author: Yasunobu Chiba <y-chiba@bq.jp.nec.com>
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


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sqlite3.h>
#include "authenticator.h"
#include "redirector.h"


#define AUTHORIZED_HOST_DB_UPDATE_INTERVAL 60
#define AUTHORIZED_HOST_DB_FILE "./examples/redirectable_routing_switch/authorized_host.db"


typedef struct authorized_host_db_entry {
  uint8_t mac[ ETH_ADDRLEN ];
} authorized_host_entry;

static hash_table *authorized_host_db = NULL;
static time_t last_authorized_host_db_mtime = 0;


static authorized_host_entry*
lookup_authorized_host( const uint8_t *mac ) {
  authorized_host_entry *entry;

  debug( "Looking up an authorized host (mac = %02x:%02x:%02x:%02x:%02x:%02x).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

  entry = lookup_hash_entry( authorized_host_db, mac );

  if ( entry == NULL ) {
    debug( "Entry not found." );
    return NULL;
  }

  debug( "A host entry found (mac = %02x:%02x:%02x:%02x:%02x:%02x).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

  return entry;
}


static void
add_authorized_host( const uint8_t *mac ) {
  authorized_host_entry *entry;

  entry = lookup_authorized_host( mac );

  if ( entry != NULL ) {
    debug( "An authorized host entry is already registered "
           "(mac = %02x:%02x:%02x:%02x:%02x:%02x).",
           mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
    return;
  }

  debug( "Adding an authorized host entry (mac = %02x:%02x:%02x:%02x:%02x:%02x).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

  entry = xmalloc( sizeof( authorized_host_entry ) );

  memcpy( entry->mac, mac, ETH_ADDRLEN );

  insert_hash_entry( authorized_host_db, entry->mac, entry );
}


static bool
create_authorized_host_db() {
  authorized_host_db = create_hash( compare_mac, hash_mac );

  if ( authorized_host_db == NULL ) {
    return false;
  }

  return true;
}


static bool
delete_authorized_host_db() {
  hash_iterator iter;
  hash_entry *entry;

  if ( authorized_host_db == NULL ) {
    return false;
  }

  init_hash_iterator( authorized_host_db, &iter );
  while ( ( entry = iterate_hash_next( &iter ) ) != NULL ) {
      xfree( entry->value );
  }
  delete_hash( authorized_host_db );

  authorized_host_db = NULL;

  return true;
}


static uint64_t
string_to_uint64( const char *str ) {
  uint64_t u64 = 0;

  if ( !string_to_datapath_id( str, &u64 ) ) {
    error( "Failed to translate a string (%s) to uint64_t.", str );
  }

  return u64;
}


static int
add_authorized_host_from_sqlite(void *NotUsed, int argc, char **argv, char **column) {
  UNUSED( NotUsed );
  UNUSED( argc );
  UNUSED( column );

  uint8_t mac[ ETH_ADDRLEN ];
  uint64_t mac_u64;

  mac_u64 = string_to_uint64( argv[ 0 ] );

  mac[ 0 ] = ( uint8_t ) ( ( mac_u64 >> 40 ) & 0xff );
  mac[ 1 ] = ( uint8_t ) ( ( mac_u64 >> 32 ) & 0xff );
  mac[ 2 ] = ( uint8_t ) ( ( mac_u64 >> 24 ) & 0xff );
  mac[ 3 ] = ( uint8_t ) ( ( mac_u64 >> 16 ) & 0xff );
  mac[ 4 ] = ( uint8_t ) ( ( mac_u64 >> 8 ) & 0xff );
  mac[ 5 ] = ( uint8_t ) ( mac_u64  & 0xff );
  
  add_authorized_host( mac );

  return 0;
}


static void
load_authorized_host_db_from_sqlite( void *user_data ) {
  UNUSED( user_data );

  char *err;
  int ret;
  struct stat st;
  sqlite3 *db;

  memset( &st, 0, sizeof( struct stat ) );

  stat( AUTHORIZED_HOST_DB_FILE, &st );
  if ( st.st_mtime == last_authorized_host_db_mtime ) {
    debug( "Authorized host database is not changed." );
    return;
  }

  last_authorized_host_db_mtime = st.st_mtime;

  delete_authorized_host_db();
  create_authorized_host_db();

  ret = sqlite3_open( AUTHORIZED_HOST_DB_FILE, &db );
  if ( ret ) {
    error( "Failed to load authorized host database (%s).", sqlite3_errmsg( db ) );
    sqlite3_close( db );
    return;
  }

  ret = sqlite3_exec( db, "select mac from authorized_host",
                      add_authorized_host_from_sqlite, 0, &err );
  if ( ret != SQLITE_OK ) {
    error( "Failed to execute a SQL statement (%s).", sqlite3_errmsg( db ) );
    sqlite3_close( db );
    return;
  }

  sqlite3_close( db );

  return;
}


bool
init_authenticator() {
  if ( authorized_host_db != NULL ) {
    error( "Authorized host database is already created." );
    return false;
  }

  load_authorized_host_db_from_sqlite( NULL );

  add_periodic_event_callback( AUTHORIZED_HOST_DB_UPDATE_INTERVAL,
                               load_authorized_host_db_from_sqlite,
                               NULL );

  return true;
}


bool
finalize_authenticator() {
  return delete_authorized_host_db();
}


bool
authenticate( const uint8_t *mac ) {
  if ( lookup_authorized_host( mac ) == NULL ) {
    return false;
  }

  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
