/*
 * Traffic counter.
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


#ifndef FDB_H
#define FDB_H


typedef struct {
  uint8_t mac[ ETH_ADDRLEN ];
  uint16_t port_number;
} fdb;


#define ENTRY_NOT_FOUND_IN_FDB OFPP_NONE


hash_table *create_fdb( void );
uint16_t lookup_fdb( hash_table *db, uint8_t *mac );
void learn_fdb( hash_table *db, uint8_t *mac, uint16_t port_number );
void delete_fdb( hash_table *db );


#endif // FDB_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
