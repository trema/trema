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


#include "trema.h"


typedef struct {
  struct ofp_match match;
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  bool strict;
} handler_data;


void timeout( void *user_data );
void add_filter_completed( int status, void *user_data );
void delete_filter_completed( int status, int n_deleted, void *user_data );
void dump_filters( int status, int n_entries, packetin_filter_entry *entries, void *user_data );


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
