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
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "checks.h"
#include "log.h"
#include "persistent_storage.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "wrapper.h"


static const size_t MAX_KEY_LENGTH = 256;
static const size_t MAX_VALUE_LENGTH = 256;
static const char DEFAULT_DB_FILE[] = ".trema.db";
static char *db_file = NULL;
static sqlite3 *db_handle = NULL;
static bool backend_initialized = false;


static bool
finalize_backend( bool delete_file ) {
  assert( db_handle != NULL );
  assert( db_file != NULL );

  int ret = trema_sqlite3_close( db_handle );
  if ( ret != SQLITE_OK ) {
    error( "Failed to destroy a sqlite3 object ( %s ).", trema_sqlite3_errmsg( db_handle ) );
    goto error;
  }
  db_handle = NULL;

  if ( delete_file ) {
    int ret = trema_unlink( db_file );
    if ( ret < 0 ) {
      char buf[ 256 ];
      error( "Failed to delete database file. ( db_file = %s, errno = %s [%d] ).",
             db_file, strerror_r( errno, buf, sizeof( buf ) ), errno );
      goto error;
    }
  }

  xfree( db_file );
  db_file = NULL;

  return true;

error:
  db_handle = NULL;
  if ( db_file != NULL ) {
    xfree( db_file );
    db_file = NULL;
  }
  return false;
}


static bool
init_backend() {
  assert( db_file == NULL );
  assert( db_handle == NULL );

  db_file = xmalloc( PATH_MAX );
  memset( db_file, '\0', PATH_MAX );
  sprintf( db_file, "%s/%s", get_trema_tmp(), DEFAULT_DB_FILE );

  int ret = trema_sqlite3_open( db_file, &db_handle );
  if ( ret != SQLITE_OK ) {
    error( "Failed to open backend database ( %s ).", trema_sqlite3_errmsg( db_handle ) );
    finalize_backend( false );
    return false;
  }

  char *err = NULL;
  char *statement = sqlite3_mprintf( "CREATE TABLE IF NOT EXISTS trema ( key TEXT, value TEXT, CONSTRAINT key_unique UNIQUE (key) ON CONFLICT FAIL )" );
  ret = trema_sqlite3_exec( db_handle, statement, NULL, NULL, &err );
  if ( ret != SQLITE_OK ) {
    error( "Failed to create a table ( error = %s ).", err );
    xfree( db_file );
    db_file = NULL;
    trema_sqlite3_close( db_handle );
    db_handle = NULL;
    trema_sqlite3_free( err );
    trema_sqlite3_free( statement );
    return false;
  }
  trema_sqlite3_free( statement );

  return true;
}


static int
save_value( void *value, int n_columns, char **columns, char **column_names ) {
  UNUSED( column_names );

  assert( n_columns == 1 );
  assert( columns != NULL );
  assert( columns[ 0 ] != NULL );
  assert( value != NULL );

  size_t length = strlen( columns[ 0 ] ) + 1;
  *( char ** ) value = xmalloc( length );
  strncpy( *( char ** ) value, columns[ 0 ], length );

  return 0;
}


static char *
get_value_from_backend( const char *key ) {
  assert( db_handle != NULL );
  assert( key != NULL );

  char *value = NULL;
  char *err = NULL;
  char *statement = sqlite3_mprintf( "SELECT value FROM trema WHERE key = '%q'", key );
  int ret = trema_sqlite3_exec( db_handle, statement, save_value, &value, &err );
  if ( ret != SQLITE_OK ) {
    error( "Failed to execute a SQL statement ( statement = %s, error = %s ).", statement, err );
    trema_sqlite3_free( statement );
    trema_sqlite3_free( err );
    return NULL;
  }
  trema_sqlite3_free( statement );

  return value;
}


static bool
save_value_to_backend( const char *key, const char *value ) {
  assert( db_handle != NULL );
  assert( key != NULL );

  char *err = NULL;
  char *statement = sqlite3_mprintf( "INSERT INTO trema (key,value) VALUES ('%q','%q')", key, value );
  int ret = trema_sqlite3_exec( db_handle, statement, NULL, NULL, &err );
  if ( ret != SQLITE_OK ) {
    error( "Failed to execute a SQL statement ( statement = %s, error = %s ).", statement, err );
    trema_sqlite3_free( statement );
    trema_sqlite3_free( err );
    return false;
  }
  trema_sqlite3_free( statement );

  int n_changes = trema_sqlite3_changes( db_handle );
  if ( n_changes != 1 ) {
    if ( n_changes > 1 ) {
      error( "Multiple entries are created ( n_changes = %d ).", n_changes );
    }
    return false;
  }

  return true;
}


static bool
update_value_on_backend( const char *key, const char *value ) {
  assert( db_handle != NULL );
  assert( key != NULL );

  char *err = NULL;
  char *statement = sqlite3_mprintf( "UPDATE trema SET value = '%q' WHERE key = '%q'", value, key );
  int ret = trema_sqlite3_exec( db_handle, statement, NULL, NULL, &err );
  if ( ret != SQLITE_OK ) {
    error( "Failed to execute a SQL statement ( statement = %s, error = %s ).", statement, err );
    trema_sqlite3_free( statement );
    trema_sqlite3_free( err );
    return false;
  }
  trema_sqlite3_free( statement );

  int n_changes = trema_sqlite3_changes( db_handle );
  if ( n_changes != 1 ) {
    if ( n_changes > 1 ) {
      error( "Multiple entries are updated ( n_changes = %d ).", n_changes );
    }
    return false;
  }

  return true;
}


static bool
delete_key_value_from_backend( const char *key ) {
  assert( db_handle != NULL );
  assert( key != NULL );

  char *err = NULL;
  char *statement = sqlite3_mprintf( "DELETE FROM trema WHERE key = '%q'", key );
  int ret = trema_sqlite3_exec( db_handle, statement, NULL, NULL, &err );
  if ( ret != SQLITE_OK ) {
    error( "Failed to execute a SQL statement ( statement = %s, error = %s ).", statement, err );
    trema_sqlite3_free( statement );
    trema_sqlite3_free( err );
    return false;
  }
  trema_sqlite3_free( statement );

  int n_changes = trema_sqlite3_changes( db_handle );
  if ( n_changes != 1 ) {
    if ( n_changes > 1 ) {
      error( "Multiple entries are deleted ( n_changes = %d ).", n_changes );
    }
    return false;
  }

  return true;
}


bool
init_persistent_storage() {
  if ( backend_initialized ) {
    error( "Backend is already initialized." );
    return false;
  }

  backend_initialized = init_backend();

  return backend_initialized;
}


bool
finalize_persistent_storage() {
  if ( !backend_initialized ) {
    error( "Backend is not initialized yet." );
    return false;
  }

  backend_initialized = false;

  return finalize_backend( false );
}


bool
clear_persistent_storage() {
  if ( !backend_initialized ) {
    error( "Backend is not initialized yet." );
    return false;
  }

  bool ret = finalize_backend( true );
  if ( ret == false ) {
    backend_initialized = false;
    return false;
  }

  backend_initialized = init_backend();

  return backend_initialized;
}


static bool
key_exists( const char *key ) {
  assert( key != NULL );

  char *retrieved = get_value_from_backend( key );
  if ( retrieved == NULL ) {
    return false;
  }

  xfree( retrieved );

  return true;
}


static bool
backend_ready( void ) {
  if ( !backend_initialized ) {
    error( "Backend is not initialized yet." );
  }

  return backend_initialized;
}


static bool
ascii_string( const char *string ) {
  assert( string != NULL );

  int i = 0;
  while ( string[ i ] != '\0' ) {
    if ( isprint( string[ i ] ) == 0 ) {
      return false;
    }
    i++;
  }

  return true;
}


static bool
valid_key( const char *key ) {
  if ( key == NULL ) {
    error( "'key' must be specified." );
    return false;
  }

  if ( !ascii_string( key ) ) {
    error( "'key' must only have printable ASCII characters." );
    return false;
  }

  size_t length = strlen( key );
  if ( length > MAX_KEY_LENGTH ) {
    error( "Too long key ( length = %zu ) specified. Maximum length is %zu.", length, MAX_KEY_LENGTH );
    return false;
  }

  return true;
}


static bool
valid_value( const char *value ) {
  if ( value == NULL ) {
    error( "'value' must be specified." );
    return false;
  }

  if ( !ascii_string( value ) ) {
    error( "'value' must only have printable ASCII characters." );
    return false;
  }

  size_t length = strlen( value );
  if ( length > MAX_KEY_LENGTH ) {
    error( "Too long value ( length = %zu ) specified. Maximum length is %zu.", length, MAX_VALUE_LENGTH );
    return false;
  }

  return true;
}



bool
set_value( const char *key, const char *value ) {
  if ( !backend_ready() ) {
    return false;
  }
  if ( !valid_key( key ) ) {
    return false;
  }
  if ( !valid_value( value ) ) {
    return false;
  }

  if ( key_exists( key ) ) {
    return update_value_on_backend( key, value );
  }

  return save_value_to_backend( key, value );
}


bool
get_value( const char *key, char *value, const size_t length ) {
  if ( !backend_ready() ) {
    return false;
  }
  if ( !valid_key( key ) ) {
    return false;
  }
  if ( value == NULL ) {
    error( "'value' must be specified." );
    return false;
  }
  if ( length == 0 ) {
    error( "'length' must be greater than zero." );
    return false;
  }

  char *retrieved = get_value_from_backend( key );
  if ( retrieved == NULL ) {
    error( "Failed to retrieve a value for '%s'.", key );
    return false;
  }

  size_t required_length = strlen( retrieved ) + 1;
  if ( required_length > length ) {
    error( "Insufficient buffer space ( %zu [bytes] > %zu [bytes] ).",
           required_length, length );
    return false;
  }

  strncpy( value, retrieved, required_length );
  xfree( retrieved );

  return true;
}


bool
delete_key_value( const char *key ) {
  if ( !backend_ready() ) {
    return false;
  }
  if ( !valid_key( key ) ) {
    return false;
  }

  if ( !key_exists( key ) ) {
    error( "An entry for '%s' does not exist.", key );
    return false;
  }

  return delete_key_value_from_backend( key );
}


const char *
_get_db_file( void ) {
  return db_file;
}


const sqlite3 *
_get_db_handle( void ) {
  return db_handle;
}


bool
_get_backend_initialized( void ) {
  return backend_initialized;
}


size_t
_get_max_key_length( void ) {
  return MAX_KEY_LENGTH;
}


size_t
_get_max_value_length( void ) {
  return MAX_VALUE_LENGTH;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
