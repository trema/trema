/*
 * A statistics collector library.
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


#ifndef STAT_H
#define STAT_H


#define STAT_KEY_LENGTH 256


typedef struct {
  char key[ STAT_KEY_LENGTH ];
  uint64_t value;
} stat_entry;


bool init_stat( void );
bool finalize_stat( void );
bool add_stat_entry( const char *key );
void increment_stat( const char *key );
void reset_stats( void );
void foreach_stat( void function( const char *key, const uint64_t value, void *user_data ), void *user_data );
void dump_stats();


#endif // STAT_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
