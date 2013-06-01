/*
 * OpenFlow Switch Manager
 *
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


#ifndef COOKIE_TABLE_H
#define COOKIE_TABLE_H


#include <limits.h>
#include <time.h>
#include "trema.h"


#define RESERVED_COOKIE 0


typedef struct application_entry {
  uint64_t cookie;
  char service_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  uint16_t flags;
} application_entry_t;

typedef struct cookie_entry {
  uint64_t cookie;
  application_entry_t application;
  int reference_count;
  time_t expire_at;
} cookie_entry_t;

typedef struct cookie_table {
  hash_table *global;
  hash_table *application;
} cookie_table_t;


void init_cookie_table( void );
void finalize_cookie_table( void );
uint64_t *insert_cookie_entry( uint64_t *original_cookie, char *service_name, uint16_t flags );
void delete_cookie_entry( cookie_entry_t *entry );
cookie_entry_t *lookup_cookie_entry_by_cookie( uint64_t *cookie );
cookie_entry_t *lookup_cookie_entry_by_application( uint64_t *cookie, char *service_name );
void age_cookie_table( void *user_data );
void dump_cookie_table( void );


#endif // COOKIE_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
