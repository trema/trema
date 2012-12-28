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
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include "trema.h"
#include "libtopology.h"
#include "show_topology.h"
#include "topology_service_interface_option_parser.h"


typedef struct show_topology_options {
  enum output_format_type {
    dsl = 0,
    graph_easy,
    csv,
  } output_format;
} show_topology_options;


static char option_description[] =
  "  -D, --dsl                       print dsl format\n"
  "  -G, --graph-easy                print graph-easy format\n"
  "  -C, --csv                       print csv format\n";
static char short_options[] = "DGC";
static struct option long_options[] = {
  { "dsl", 0, NULL, 'D' },
  { "graph-easy", 0, NULL, 'G' },
  { "csv", 0, NULL, 'C' },
  { NULL, 0, NULL, 0  },
};


static void
reset_getopt() {
  optind = 0;
  opterr = 1;
}


void
usage() {
  topology_service_interface_usage( get_executable_name(), "show topology", option_description );
}


static void
init_show_topology_options( show_topology_options *options, int *argc, char **argv[] ) {
  assert( options != NULL );
  assert( argc != NULL );
  assert( *argc >= 0 );
  assert( argv != NULL );

  // set default values
  options->output_format = dsl;

  int argc_tmp = *argc;
  char *new_argv[ *argc ];

  int i;
  for ( i = 0; i <= *argc; ++i ) {
    new_argv[ i ] = ( *argv )[ i ];
  }

  int c;
  while ( ( c = getopt_long( *argc, *argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
    case 'D':
      options->output_format = dsl;
      break;

    case 'G':
      options->output_format = graph_easy;
      break;

    case 'C':
      options->output_format = csv;
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

  int j;
  for ( i = 0, j = 0; i < *argc; ++i ) {
    if ( new_argv[ i ] != NULL ) {
      ( *argv )[ j ] = new_argv[ i ];
      j++;
    }
  }

  ( *argv )[ *argc ] = NULL;
  *argc = argc_tmp;

  reset_getopt();
}


static void
timed_out( void *user_data ) {
  UNUSED( user_data );

  error( "timed out." );

  stop_trema();
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  show_topology_options options;
  init_show_topology_options( &options, &argc, &argv );
  init_topology_service_interface_options( &argc, &argv );
  init_libtopology( get_topology_service_interface_name() );

  switch ( options.output_format ) {
  case dsl:
    get_all_link_status( print_with_dsl_format, NULL );
    break;

  case graph_easy:
    get_all_link_status( print_with_graph_easy_format, NULL );
    break;

  case csv:
    get_all_link_status( print_with_csv_format, NULL );
    break;

  default:
    printf( "not supported\n" );
    break;
  }
  add_periodic_event_callback( 10, timed_out, NULL );

  start_trema();

  finalize_libtopology();
  finalize_topology_service_interface_options();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
