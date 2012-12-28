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


#ifndef TOPOLOGY_OPTION_PARSER_H_
#define TOPOLOGY_OPTION_PARSER_H_

#include "service_management.h"
#include "discovery_management.h"


typedef struct {
  service_management_options service;
  discovery_management_options discovery;
} topology_options;


void
parse_options( topology_options *options, int *argc, char **argv[] );


#endif /* TOPOLOGY_OPTION_PARSER_H_ */
