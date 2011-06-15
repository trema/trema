/*
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


#include <getopt.h>
#include <inttypes.h>
#include <time.h>
#include "trema.h"


static void
recv_reply( uint16_t tag, void *data, size_t len, void *user_data ) {
  UNUSED( tag );
  UNUSED( user_data );

  char time_string[ 64 ];
  time_t now = time( NULL );
  ctime_r( &now, time_string );
  time_string[ strlen( time_string ) - 1 ] = '\0';
  info( "[%s]", time_string );

  uint64_t *dpid = ( uint64_t * ) data;
  size_t count = len / sizeof( uint64_t );
  unsigned int i;
  for ( i = 0; i < count; i++ ) {
    info( "%#" PRIx64, ntohll( dpid[ i ] ) );
  }
}


static void
send_request( void *user_data ) {
  UNUSED( user_data );
  send_request_message( "switch_manager", get_trema_name(), 0, NULL, 0, NULL );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  add_periodic_event_callback( 1, send_request, NULL );
  add_message_replied_callback( get_trema_name(), recv_reply );
  start_trema();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
