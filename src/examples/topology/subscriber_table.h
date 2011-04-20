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


#ifndef SUBSCRIBER_TABLE_H
#define SUBSCRIBER_TABLE_H


#include "trema.h"


typedef struct subscriber_entry {
  char *name;
} subscriber_entry;


void init_subscriber_table( void );
void finalize_subscriber_table( void );
bool insert_subscriber_entry( const char *name );
void delete_subscriber_entry( subscriber_entry *entry );
subscriber_entry *lookup_subscriber_entry( const char *name );
void foreach_subscriber( void function( subscriber_entry *entry, void *user_data ), void *user_data );


#endif // SUBSCRIBER_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
