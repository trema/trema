/*
 * Author: Yasunobu Chiba
 *
 * Copyright (C) 2012 NEC Corporation
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
#include "bool.h"
#include "checks.h"
#include "log.h"
#include "management_service_interface.h"
#include "messenger.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "utility.h"
#include "wrapper.h"


static char management_service_name[ MESSENGER_SERVICE_NAME_LENGTH ];


static const char *
_get_management_service_name( const char *service_name ) {
  if ( service_name == NULL ) {
    die( "Service name must not be NULL." );
  }
  if ( strlen( service_name ) == 0 ) {
    die( "Service name length must not be zero." );
  }
  if ( strlen( service_name ) >= ( MESSENGER_SERVICE_NAME_LENGTH - 2 ) ) {
    die( "Too long service name ( %s ).", service_name );
  }

  memset( management_service_name, '\0', MESSENGER_SERVICE_NAME_LENGTH );
  snprintf( management_service_name, MESSENGER_SERVICE_NAME_LENGTH, "%s.m", service_name );

  return management_service_name;
}
const char *( *get_management_service_name )( const char *service_name ) = _get_management_service_name;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
