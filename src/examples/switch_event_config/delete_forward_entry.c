/*
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


#include <getopt.h>
#include "trema.h"


static bool operate_on_all = true;
static bool sw_manager = true;
static uint64_t dpid;
static enum efi_event_type type;
static const char *service_name = NULL;

static struct option long_options[] = {
  { "manager", 0, NULL, 'm' },
  { "switch", 1, NULL, 's' },
  { "type", 1, NULL, 't' },
  { NULL, 0, NULL, 0  },
};

static char short_options[] = "ms:t:";


void
usage() {
  printf(
    "Delete OpenFlow Switch Manager/Daemon event forward entry.\n"
    " Both Switch Mgr/Daemon: %s -t EVENT_TYPE service_name\n"
    " Only Switch Manager   : %s -m -t EVENT_TYPE service_name\n"
    " Only Switch Daemon    : %s -s SWITCH_DPID -t EVENT_TYPE service_name\n"
    "\n"
    " EVENT_TYPE:\n"
    "  -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.\n"
    " TREMA COMMON:\n"
    "  -n, --name=SERVICE_NAME         service name\n"
    "  -d, --daemonize                 run in the background\n"
    "  -l, --logging_level=LEVEL       set logging level\n"
    "  -g, --syslog                    output log messages to syslog\n"
    "  -f, --logging_facility=FACILITY set syslog facility\n"
    "  -h, --help                      display this help and exit\n"
    , get_executable_name()
    , get_executable_name()
    , get_executable_name()
  );
}


static bool
parse_argument( int argc, char *argv[] ) {

  bool type_specified = false;

  int c;
  while ( ( c = getopt_long( argc, argv, short_options, long_options, NULL ) ) != -1 ) {
    switch ( c ) {
      case 'm':
        sw_manager = true;
        operate_on_all = false;
        break;

      case 's':
        sw_manager = false;
        operate_on_all = false;
        if ( !string_to_datapath_id( optarg, &dpid ) ) {
          error( "Invalid dpid '%s' specified. ", optarg );
          usage();
          exit( EXIT_SUCCESS );
          return false;
        }
        break;

      case 't': // add
        type_specified = true;
        if ( false ) {
        }
        else if ( strcasecmp( "vendor", optarg ) == 0 ) {
          type = EVENT_FORWARD_TYPE_VENDOR;
        }
        else if ( strcasecmp( "packet_in", optarg ) == 0 ) {
          type = EVENT_FORWARD_TYPE_PACKET_IN;
        }
        else if ( strcasecmp( "port_status", optarg ) == 0 ) {
          type = EVENT_FORWARD_TYPE_PORT_STATUS;
        }
        else if ( strcasecmp( "state_notify", optarg ) == 0 ) {
          type = EVENT_FORWARD_TYPE_STATE_NOTIFY;
        }
        else {
          error( "Invalid type '%s' specified. Must e one of vendor, packet_in, port_status, or state_notify\n", optarg );
          usage();
          exit( EXIT_SUCCESS );
          return false;
        }
        break;


      default:
        error( "Encountered unknown option." );
        usage();
        exit( EXIT_SUCCESS );
        return false;
        break;
    }
  }

  if ( !type_specified ) {
    error( "Event Type was not specified with -t option.\n" );
    usage();
    exit( EXIT_SUCCESS );
    return false;
  }

  if ( optind >= argc ) {
    error( "Service name was not specified.\n" );
    usage();
    exit( EXIT_FAILURE );
    return false;
  }

  service_name = argv[ optind ];

  return true;
}


static void
timeout( void *user_data ) {
  UNUSED( user_data );

  error( "Request timed out." );
  stop_trema();
  exit( EXIT_FAILURE );
}


static void
update_result_all_callback( enum efi_result result, void *user_data ) {
  UNUSED( user_data );
  if ( result == EFI_OPERATION_SUCCEEDED ) {
    info( "Operation Succeeded." );
    stop_trema();
  }
  else {
    error( "Operation Failed." );
    stop_trema();
    exit( EXIT_FAILURE );
  }
}


static void
update_result_callback( event_forward_operation_result result, void *user_data) {
  UNUSED( user_data );

  if ( result.result != EFI_OPERATION_SUCCEEDED ) {
    error( "Operation Failed." );
    stop_trema();
    exit( EXIT_FAILURE );
  }
  else {
    if ( result.n_services == 0 ) {
      info( "Updated service name list is empty.");
    }
    else {
      info( "Updated service name list:" );
      unsigned i;
      for ( i = 0; i < result.n_services; ++i ) {
        info( "  %s", result.services[ i ] );
      }
    }
    stop_trema();
  }
}


static void
send_efi_request( void ) {
  info( "Deleting '%s'... ", service_name );
  if ( operate_on_all ) {
    delete_event_forward_entry_to_all_switches( type, service_name,
                                                update_result_all_callback, NULL );
  }
  else {
    if ( sw_manager ) {
      delete_switch_manager_event_forward_entry( type, service_name,
                                                 update_result_callback, NULL );
    } else {
      delete_switch_event_forward_entry( dpid, type, service_name,
                                         update_result_callback, NULL );
    }
  }
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  parse_argument( argc, argv );

  init_event_forward_interface();

  send_efi_request();

  add_periodic_event_callback( 30, timeout, NULL );

  start_trema();

  finalize_event_forward_interface();

  return 0;
}
