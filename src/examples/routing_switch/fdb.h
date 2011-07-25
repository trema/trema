/*
 * Sample routing switch (switching HUB) application.
 *
 * This application provides a switching HUB function using multiple
 * openflow switches.
 *
 * Author: Shuji Ishii
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


#ifndef FDB_H
#define FDB_H


#include "trema.h"


hash_table *create_fdb( void );
bool is_ether_multicast( const uint8_t mac[ OFP_ETH_ALEN ] );
void delete_fdb( hash_table *fdb );
bool update_fdb( hash_table *fdb, const uint8_t mac[ OFP_ETH_ALEN ], uint64_t dpid, uint16_t port );
bool lookup_fdb( hash_table *fdb, const uint8_t mac[ OFP_ETH_ALEN ], uint64_t *dpid, uint16_t *port );
void init_age_fdb( hash_table *fdb );
void delete_fdb_entries( hash_table *fdb, uint64_t dpid, uint16_t port );


#endif // FDB_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
