/*
 * A simple application for "cbench" performance measurements.
 *
 * Author: Yasunobu Chiba
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trema.h"


static struct option long_options[] = {
  { "datapath_id", required_argument, NULL, 'i' },
  { NULL, 0, NULL, 0  },
};

static char short_options[] = "i:";


void
usage() {
  printf(
    "A simple application for \"cbench\" performance measurements.\n"
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -n, --name=SERVICE_NAME     service name\n"
    "  -i, --datapath_id=ID        datapath ID\n"
    "  -d, --daemonize             run in the background\n"
    "  -l, --logging_level=LEVEL   set logging level\n"
    "  -h, --help                  display this help and exit\n"
    , get_executable_name()
  );
}


static void
handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                  uint32_t buffer_id, uint16_t total_len,
                  uint16_t in_port, uint8_t reason, const buffer *data,
                  void *managed_datapath_id ) {
  debug( "packet_in received (datapath_id = %#llx, transaction_id = %#lx, "
         "buffer_id = %#lx, total_len = %u, in_port = %u, reason = %#x, length = %u).",
         datapath_id, transaction_id, buffer_id, total_len, in_port, reason, data->length );

  if ( datapath_id != *( ( uint64_t * ) managed_datapath_id ) ) {
    error( "packet_in message from unmanaged switch (dpid = %#llx).", datapath_id );
    return;
  }

  openflow_actions_t *actions = create_actions();
  append_action_output( actions, ( uint16_t ) ( in_port + 1 ), UINT16_MAX );

  struct ofp_match match;
  set_match_from_packet( &match, in_port, 0, data );

  buffer *buffer = create_flow_mod( get_transaction_id(), match, get_cookie(),
                                    OFPFC_ADD, 0, 0, UINT16_MAX, buffer_id,
                                    OFPP_NONE, OFPFF_SEND_FLOW_REM, actions );

  if ( buffer_id == UINT32_MAX ) {
    send_openflow_message( datapath_id, buffer );
    free_buffer( buffer );
    
    buffer = create_packet_out( get_transaction_id(), buffer_id, in_port, actions, data );
  }

  send_openflow_message( datapath_id, buffer );

  free_buffer( buffer );
  delete_actions( actions );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  char *datapath_id = NULL;
  int c;

  while ( ( c = getopt_long( argc, argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'i':
        datapath_id = optarg;
        break;
      default:
        usage();
        exit( EXIT_FAILURE );
    }
  }
  if ( datapath_id == NULL ) {
    usage();
    exit( EXIT_FAILURE );
  }

  uint64_t managed_datapath_id = 0;
  string_to_datapath_id( datapath_id, &managed_datapath_id );

  set_packet_in_handler( handle_packet_in, &managed_datapath_id );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
