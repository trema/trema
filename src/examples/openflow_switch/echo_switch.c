/*
 * A simple switch that has minimum function to test echo message.
 *
 * Copyright (C) 2013 NEC Corporation
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


#include "chibach.h"


// FIXME: Copy & Paste from hello_switch.c
static void
handle_features_request( uint32_t xid, void *user_data ) {
  UNUSED( user_data );

  uint32_t supported = ( ( 1 << OFPAT_OUTPUT ) |
                         ( 1 << OFPAT_SET_VLAN_VID ) |
                         ( 1 << OFPAT_SET_VLAN_PCP ) |
                         ( 1 << OFPAT_STRIP_VLAN ) |
                         ( 1 << OFPAT_SET_DL_SRC ) |
                         ( 1 << OFPAT_SET_DL_DST ) |
                         ( 1 << OFPAT_SET_NW_SRC ) |
                         ( 1 << OFPAT_SET_NW_DST ) |
                         ( 1 << OFPAT_SET_NW_TOS ) |
                         ( 1 << OFPAT_SET_TP_SRC ) |
                         ( 1 << OFPAT_SET_TP_DST ) );

  buffer *msg = create_features_reply( xid, get_datapath_id(), 0, 1, 0, supported, NULL );

  switch_send_openflow_message( msg );
}


// FIXME: Copy & Paste from hello_switch.c
static void
handle_hello( uint32_t xid, uint8_t version, void *user_data ) {
  UNUSED( version );
  UNUSED( user_data );

  switch_send_openflow_message( create_hello( xid ) );
}


static void
handle_echo_request( uint32_t xid, const buffer *body, void *user_data ) {
  UNUSED( body );
  UNUSED( user_data );

  info( "received: OFPT_ECHO_REQUEST" );

  switch_send_openflow_message( create_echo_reply( xid, NULL ) );
}


int
main( int argc, char **argv ) {
  init_chibach( &argc, &argv );

  set_hello_handler( handle_hello, NULL );
  set_features_request_handler( handle_features_request, NULL );
  set_echo_request_handler( handle_echo_request, NULL );

  start_chibach();

  stop_chibach();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
