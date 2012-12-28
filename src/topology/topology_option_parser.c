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


#include "topology_option_parser.h"

#include <assert.h>
#include <stdio.h>
#include <getopt.h>
#include <netinet/ether.h>

static char short_options[] = "w:e:am:io:r:";

static struct option long_options[] = {
  { "liveness_wait", required_argument, NULL, 'w'},
  { "liveness_limit", required_argument, NULL, 'e'},
  { "always_run_discovery", no_argument, NULL, 'a'},
  { "lldp_mac_dst", required_argument, NULL, 'm' },
  { "lldp_over_ip", no_argument, NULL, 'i' },
  { "lldp_ip_src", required_argument, NULL, 'o' },
  { "lldp_ip_dst", required_argument, NULL, 'r' },
  { NULL, 0, NULL, 0  },
};

static uint8_t g_lldp_default_dst[ ETH_ADDRLEN ] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e };

void
usage() {
  printf(
    "topology manager\n"
    "Usage: %s [OPTION]...\n"
    "\n"
    "  -w, --liveness_wait=SEC         subscriber liveness check interval\n"
    "  -e, --liveness_limit=COUNT      set liveness check error threshold\n"
    "  -a, --always_run_discovery      discovery will always be enabled\n"
    "  -m, --lldp_mac_dst=MAC_ADDR     destination Mac address for sending LLDP\n"
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
set_mac_address_from_string( uint8_t *address, const char *string ) {
  assert( address != NULL );
  assert( string != NULL );

  struct ether_addr *addr = ether_aton( string );
  if ( addr == NULL ) {
    error( "Invalid MAC address specified." );
    return false;
  }

  memcpy( address, addr->ether_addr_octet, ETH_ADDRLEN );

  return true;
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


void
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


  options->service.ping_interval_sec = 60;
  options->service.ping_ageout_cycles = 5;

  memcpy( options->discovery.lldp.lldp_mac_dst, g_lldp_default_dst, ETH_ADDRLEN );
  options->discovery.lldp.lldp_over_ip = false;
  options->discovery.lldp.lldp_ip_src = 0;
  options->discovery.lldp.lldp_ip_dst = 0;
  options->discovery.always_enabled = false;

  int c;
  while ( ( c = getopt_long( *argc, *argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'w':
      {
        int sec = atoi( optarg );
        if( sec <= 0 ) {
          error( "Invalid liveness wait interval." );
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        options->service.ping_interval_sec = sec;
        info( "Liveness check interval is set to %d seconds.", sec );
        break;
      }

      case 'e':
      {
        int sec = atoi( optarg );
        if( sec <= 0 ) {
          error( "Invalid liveness check error count." );
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        options->service.ping_ageout_cycles = sec;
        info( "Liveness check error threshold is set to %d.", sec );
        break;
      }

      case 'a':
        options->discovery.always_enabled = true;
        info( "Discovery will always be enabled." );
        break;

      case 'm':
        if ( set_mac_address_from_string( options->discovery.lldp.lldp_mac_dst, optarg ) == false ) {
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        info( "%s is used as destination address for sending LLDP.", ether_ntoa( ( const struct ether_addr * ) options->discovery.lldp.lldp_mac_dst ) );
        break;

      case 'i':
        debug( "Enabling LLDP over IP" );
        options->discovery.lldp.lldp_over_ip = true;
        break;

      case 'o':
        if ( set_ip_address_from_string( &options->discovery.lldp.lldp_ip_src, optarg ) == false ) {
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        info( "%s ( %#x ) is used as source address for sending LLDP over IP.", optarg, options->discovery.lldp.lldp_ip_src );
        break;
      case 'r':
        if ( set_ip_address_from_string( &options->discovery.lldp.lldp_ip_dst, optarg ) == false ) {
          usage();
          exit( EXIT_FAILURE );
          return;
        }
        info( "%s ( %#x ) is used as destination address for sending LLDP over IP.", optarg, options->discovery.lldp.lldp_ip_dst );
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

  if ( options->discovery.lldp.lldp_over_ip == true && ( options->discovery.lldp.lldp_ip_src == 0 || options->discovery.lldp.lldp_ip_dst == 0 ) ) {
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


