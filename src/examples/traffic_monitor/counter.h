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


#ifndef COUNTER_H
#define COUNTER_H


typedef struct {
  uint8_t mac[ ETH_ADDRLEN ];
  uint64_t packet_count;
  uint64_t byte_count;
} counter;


hash_table *create_counter( void );
void add_counter( hash_table *db, uint8_t *mac, uint64_t packet_count, uint64_t byte_count );
void foreach_counter( hash_table *db, void function( uint8_t *mac, counter *counter, void *user_data ), void *user_data );
void delete_counter( hash_table *db );


#endif // COUNTER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
