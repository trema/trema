/*
 * A statistics collector library.
 *
 * Author: Yasunobu Chiba
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

/**
 * @file
 *
 * @brief Function declaraions for Trema Statistics Management 
 *
 * File containing various functions for creating a generic statistics management service,
 * which can be used by Trema application to manage stats for arbitrary parameters.
 * @code
 * // Initialize the Statistics layer
 * init_stat();
 * // Add an arbitrary parameter to track its statistics
 * add_stat_entry( "count_of_apples" );
 * ...
 * // Increment the number of apples we have by 1
 * increment_stat( "count_of_apples" );
 * ...
 * // Dump all the current parameters with their stats
 * dump_stats();
 * // Which would output the following
 * count_of_apples: 1
 * ...
 * // Finish stats parameter recording or tracking
 * finalize_stat();
 * @endcode
 */

#ifndef STAT_H


#define STAT_KEY_LENGTH 256


bool init_stat( void );
bool finalize_stat( void );
bool add_stat_entry( const char *key );
void increment_stat( const char *key );
void dump_stats();


#endif // STAT_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
