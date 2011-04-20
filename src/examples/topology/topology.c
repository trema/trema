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
#include <openflow.h>
#include "trema.h"
#include "topology_table.h"
#include "service_management.h"
#include "topology_management.h"


#ifdef UNIT_TESTING

#define static
#define main topology_main

#endif // UNIT_TESTING


void
usage() {
  printf(
	 "topology manager\n"
	 "Usage: %s [OPTION]...\n"
	 "\n"
	 "  -n, --name=SERVICE_NAME     service name\n"
	 "  -d, --daemonize             run in the background\n"
	 "  -l, --logging_level=LEVEL   set logging level\n"
	 "  -h, --help                  display this help and exit\n"
	 , get_executable_name()
	 );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );
  init_topology_table();
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

