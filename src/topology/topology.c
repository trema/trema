/*
 * Author: Shuji Ishii, Kazushi SUGYO
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <openflow.h>
#include "trema.h"
#include "topology_table.h"
#include "service_management.h"
#include "topology_management.h"


static char short_options[] = "io:r:";
static struct option long_options[] = {
  { "lldp_over_ip", no_argument, NULL, 'i' },
  { "lldp_ip_src", required_argument, NULL, 'o' },
  { "lldp_ip_dst", required_argument, NULL, 'r' },
  { NULL, 0, NULL, 0  },
};


typedef struct {
  topology_management_options management;
} topology_options;


void
usage() {
  printf(
    "topology manager\n"
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -i, --lldp_over_ip              send LLDP messages over IP\n"
    "  -o, --lldp_ip_src=IP_ADDR       source IP address for sending LLDP over IP\n"
    "  -r, --lldp_ip_dst=IP_ADDR       destination IP address for sending LLDP over IP\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
    , get_executable_name()
  );
}


static void
reset_getopt() {
  optind = 0;
  opterr = 1;
}


static bool
set_ip_address_from_string( uint32_t *address, const char *string ) {
  assert( address != NULL );
  assert( string != NULL );

  struct in_addr addr;

  if ( inet_aton( string, &addr ) == 0 ) {
    error( "Invalid IP address specified." );
    return false;
  }

  *address = ntohl( addr.s_addr );

  return true;
}


static void
parse_options( topology_options *options, int *argc, char **argv[] ) {
  assert( options != NULL );
  assert( argc != NULL );
  assert( *argc >= 0 );
  assert( argv != NULL );

  int argc_tmp = *argc;
  char *new_argv[ *argc ];

  for ( int i = 0; i <= *argc; ++i ) {
    new_argv[ i ] = ( *argv )[ i ];
  }

  options->management.lldp_over_ip = false;
  options->management.lldp_ip_src = 0;
  options->management.lldp_ip_dst = 0;

  int c;
  while ( ( c = getopt_long( *argc, *argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'i':
        debug( "Enabling LLDP over IP" );
        options->management.lldp_over_ip = true;
        break;

      case 'o':
        if ( set_ip_address_from_string( &options->management.lldp_ip_src, optarg ) == false ) {
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        info( "%s ( %#x ) is used as source address for sending LLDP over IP.", optarg, options->management.lldp_ip_src );
        break;
      case 'r':
        if ( set_ip_address_from_string( &options->management.lldp_ip_dst, optarg ) == false ) {
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        info( "%s ( %#x ) is used as destination address for sending LLDP over IP.", optarg, options->management.lldp_ip_dst );
        break;

      default:
        continue;
    }

    if ( optarg == 0 || strchr( new_argv[ optind - 1 ], '=' ) != NULL ) {
      argc_tmp -= 1;
      new_argv[ optind - 1 ] = NULL;
    }
    else {
      argc_tmp -= 2;
      new_argv[ optind - 1 ] = NULL;
      new_argv[ optind - 2 ] = NULL;
    }
  }

  if ( options->management.lldp_over_ip == true && ( options->management.lldp_ip_src == 0 || options->management.lldp_ip_dst == 0 ) ) {
    printf( "-o and -r options are mandatory." );
    usage();
    exit( EXIT_FAILURE );
    return;
  }

  for ( int i = 0, j = 0; i < *argc; ++i ) {
    if ( new_argv[ i ] != NULL ) {
      ( *argv )[ j ] = new_argv[ i ];
      j++;
    }
  }

  ( *argv )[ *argc ] = NULL;
  *argc = argc_tmp;

  reset_getopt();
}


int
main( int argc, char *argv[] ) {
  topology_options options;

  init_trema( &argc, &argv );
  parse_options( &options, &argc, &argv );
  init_topology_table();
  init_topology_management( options.management );

  start_topology_management();
  start_service_management();

  start_trema();

  stop_service_management();
  stop_topology_management();
  finalize_topology_table();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

