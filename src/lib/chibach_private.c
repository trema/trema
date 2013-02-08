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
#include "chibach_private.h"
#include "log.h"
#include "trema_wrapper.h"
#include "wrapper.h"


static const char CHIBACH_HOME[] = "CHIBACH_HOME";
static const char CHIBACH_TMP[] = "CHIBACH_TMP";
static char *chibach_home = NULL;
static char *chibach_tmp = NULL;


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
 * Sets chibach home directory for your chibach session.
 */
void
set_chibach_home( void ) {
  if ( getenv( CHIBACH_HOME ) == NULL ) {
    setenv( CHIBACH_HOME, "/", 1 );
    chibach_home = xstrdup( "/" );
  }
  else {
    char absolute_path[ PATH_MAX ];
    if ( !expand( getenv( CHIBACH_HOME ), absolute_path ) ) {
      trema_fprintf( stderr, "Falling back CHIBACH_HOME to \"/\".\n" );
      strncpy( absolute_path, "/", 2 );
    }
    setenv( CHIBACH_HOME, absolute_path, 1 );
    chibach_home = xstrdup( absolute_path );
  }
}


/**
 * Returns chibach home directory used in your chibach session.
 */
const char *
get_chibach_home( void ) {
  if ( chibach_home == NULL ) {
    set_chibach_home();
  }
  return chibach_home;
}


/**
 * Unsets chibach home directory used in your chibach session.
 */
void unset_chibach_home( void ) {
  if ( chibach_home != NULL ) {
    xfree( chibach_home );
    chibach_home = NULL;
  }
}


/**
 * Sets temporary directory for your Chibach session.
 */
void
set_chibach_tmp( void ) {
  char path[ PATH_MAX ];

  if ( getenv( CHIBACH_TMP ) == NULL ) {
    const char *chibach_home = get_chibach_home();
    if ( chibach_home[ strlen( chibach_home ) - 1 ] == '/' ) {
      snprintf( path, PATH_MAX, "%stmp", chibach_home );
    }
    else {
      snprintf( path, PATH_MAX, "%s/tmp", chibach_home );
    }
    path[ PATH_MAX - 1 ] = '\0';
  }
  else {
    if ( !expand( getenv( CHIBACH_TMP ), path ) ) {
      trema_fprintf( stderr, "Falling back CHIBACH_TMP to \"/tmp\".\n" );
      strncpy( path, "/tmp", 5 );
    }
  }

  chibach_tmp = xstrdup( path );
  setenv( CHIBACH_TMP, chibach_tmp, 1 );
}


/**
 * Returns temporary directory used in your Chibach session.
 */
const char *
get_chibach_tmp( void ) {
  if ( chibach_tmp == NULL ) {
    set_chibach_tmp();
  }
  return chibach_tmp;
}


/**
 * Unsets temporary directory used in your Chibach session.
 */
void unset_chibach_tmp( void ) {
  if ( chibach_tmp != NULL ) {
    xfree( chibach_tmp );
    chibach_tmp = NULL;
  }
}


const char *
_get_chibach_home( void ) {
  return chibach_home;
}


const char *
_get_chibach_tmp( void ) {
  return chibach_tmp;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
