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


#ifndef TOPOLOGY_SERVICE_INTERFACE_OPTION_PARSER_H
#define TOPOLOGY_SERVICE_INTERFACE_OPTION_PARSER_H


void topology_service_interface_usage( const char *progname, const char *description, const char *additional_options );
void init_topology_service_interface_options( int *argc, char **argv[] );
void finalize_topology_service_interface_options( void );
const char *get_topology_service_interface_name( void );


#endif // TOPOLOGY_SERVICE_INTERFACE_OPTION_PARSER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
