/*
 * Utility functions.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


#ifndef UTILITY_H
#define UTILITY_H


#include <stdint.h>
#include <stdio.h>
#include "bool.h"
#include "openflow.h"


#define NO_RETURN __attribute__( ( __noreturn__ ) )


// If this is being built for a unit test.
#ifdef UNIT_TESTING

#include <stdio.h>
#include <stdlib.h>

#define die( _fmt, ... )                         \
  do {                                           \
    fprintf( stderr, ( _fmt ), ## __VA_ARGS__ ); \
    abort();                                     \
  }                                              \
  while ( 0 )

#else // UNIT_TESTING

void NO_RETURN die( const char *format, ... );

#endif // UNIT_TESTING

bool compare_string( const void *x, const void *y );
unsigned int hash_string( const void *key );

bool compare_mac( const void *x, const void *y );
unsigned int hash_mac( const void *mac );

bool compare_uint32( const void *x, const void *y );
unsigned int hash_uint32( const void *key );

bool compare_datapath_id( const void *x, const void *y );
unsigned int hash_datapath_id( const void *key );

bool string_to_datapath_id( const char *str, uint64_t *datapath_id );

bool match_to_string( const struct ofp_match *match, char *str, size_t size );
bool phy_port_to_string( const struct ofp_phy_port *phy_port, char *str, size_t size );

#endif // UTILITY_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
