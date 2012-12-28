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


#include <stdio.h>
#include <getopt.h>
#include "trema.h"
#include "topology_service_interface_option_parser.h"


static struct options {
  char *name;
} options;


static struct option long_options[] = {
  { "topology", 1, NULL, 't' },
  { NULL, 0, NULL, 0  },
};
static char short_options[] = "t:";


void
topology_service_interface_usage( const char *progname, const char *description, const char *additional_options ) {
  printf(
    "%s\n"
    "Usage: %s [OPTION]...\n"
    "\n"
    , description, progname
  );

  if ( additional_options != NULL ) {
    printf( "%s", additional_options );
  }

  printf(
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -t, --topology=SERVICE_NAME     topology service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
  );
}


static void
reset_getopt() {
  optind = 0;
  opterr = 1;
}


void
option_parser( int *argc, char **argv[] ) {
  int c;

  int argc_tmp = *argc;
  char *new_argv[ *argc ];

  for ( int i = 0; i <= *argc; ++i ) {
    new_argv[ i ] = ( *argv )[ i ];
  }

  while ( ( c = getopt_long( *argc, *argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 't':
        xfree( options.name );
        options.name = xstrdup( optarg );
        break;

      default:
        finalize_topology_service_interface_options();
        exit( EXIT_SUCCESS );
        return;
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

  for ( int i = 0, j = 0; i < *argc; ++i ) {
    if ( new_argv[ i ] != NULL ) {
      ( *argv )[ j ] = new_argv[ i ];
      j++;
    }
  }

  ( *argv )[ *argc ] = NULL;
  *argc = argc_tmp;

  reset_getopt();

  debug( "topology_service_interface_name=`%s'", options.name );
}


const char *
get_topology_service_interface_name( void ) {
  return options.name;
}


void
init_topology_service_interface_options( int *argc, char **argv[] ) {
  options.name = xstrdup( "topology" );
  option_parser( argc, argv );
}


void
finalize_topology_service_interface_options( void ) {
  xfree( options.name );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
