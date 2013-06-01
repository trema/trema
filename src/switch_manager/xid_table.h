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


#ifndef XID_TABLE_H
#define XID_TABLE_H


#include "trema.h"


typedef struct xid_entry {
  uint32_t xid;
  uint32_t original_xid;
  char *service_name;
  int index;
} xid_entry_t;


uint32_t generate_xid( void );
void init_xid_table( void );
void finalize_xid_table( void );
uint32_t insert_xid_entry( uint32_t original_xid, char *service_name );
void delete_xid_entry( xid_entry_t *entry );
xid_entry_t *lookup_xid_entry( uint32_t xid );
void dump_xid_table( void );


#endif // XID_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
