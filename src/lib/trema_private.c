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
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "log.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#define static

#endif // UNIT_TESTING


static const char TREMA_HOME[] = "TREMA_HOME";
static const char TREMA_TMP[] = "TREMA_TMP";
static char *trema_home = NULL;
static char *trema_tmp = NULL;


static bool
expand( const char *path, char *absolute_path ) {
  assert( path != NULL );
  assert( absolute_path != NULL );

  char buf[ 256 ];
  char *result = realpath( path, absolute_path );
  if ( result == NULL ) {
    trema_fprintf( stderr, "Could not get the absolute path of %s: %s.\n", path, strerror_r( errno, buf, sizeof( buf ) ) );
    return false;
  }

  return true;
}


/**
 * Sets trema home directory for your trema session.
 */
void
set_trema_home( void ) {
  if ( getenv( TREMA_HOME ) == NULL ) {
    setenv( TREMA_HOME, "/", 1 );
    trema_home = xstrdup( "/" );
  }
  else {
    char absolute_path[ PATH_MAX ];
    if ( !expand( getenv( TREMA_HOME ), absolute_path ) ) {
      trema_fprintf( stderr, "Falling back TREMA_HOME to \"/\".\n" );
      strncpy( absolute_path, "/", 2 );
    }
    setenv( TREMA_HOME, absolute_path, 1 );
    trema_home = xstrdup( absolute_path );
  }
}


/**
 * Returns trema home directory used in your trema session.
 */
const char *
get_trema_home( void ) {
  if ( trema_home == NULL ) {
    set_trema_home();
  }
  return trema_home;
}


/**
 * Unsets trema home directory used in your trema session.
 */
void unset_trema_home( void ) {
  if ( trema_home != NULL ) {
    xfree( trema_home );
    trema_home = NULL;
  }
}


/**
 * Sets temporary directory for your Trema session.
 */
void
set_trema_tmp( void ) {
  char path[ PATH_MAX ];

  if ( getenv( TREMA_TMP ) == NULL ) {
    const char *trema_home = get_trema_home();
    if ( trema_home[ strlen( trema_home ) - 1 ] == '/' ) {
      snprintf( path, PATH_MAX, "%stmp", trema_home );
    }
    else {
      snprintf( path, PATH_MAX, "%s/tmp", trema_home );
    }
    path[ PATH_MAX - 1 ] = '\0';
  }
  else {
    if ( !expand( getenv( TREMA_TMP ), path ) ) {
      trema_fprintf( stderr, "Falling back TREMA_TMP to \"/tmp\".\n" );
      strncpy( path, "/tmp", 5 );
    }
  }

  trema_tmp = xstrdup( path );
  setenv( TREMA_TMP, trema_tmp, 1 );
}


/**
 * Returns temporary directory used in your Trema session.
 */
const char *
get_trema_tmp( void ) {
  if ( trema_tmp == NULL ) {
    set_trema_tmp();
  }
  return trema_tmp;
}


/**
 * Unsets temporary directory used in your Trema session.
 */
void unset_trema_tmp( void ) {
  if ( trema_tmp != NULL ) {
    xfree( trema_tmp );
    trema_tmp = NULL;
  }
}


const char *
_get_trema_home( void ) {
  return trema_home;
}


const char *
_get_trema_tmp( void ) {
  return trema_tmp;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
