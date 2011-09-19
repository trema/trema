/*
 * OpenFlow flow matching library
 *
 * Author: Kazushi SUGYO
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
 * @brief Match Table declarations
 *
 * File contains functions declaration for handling flow tables in an OpenFlow Switch.
 * @code
 * // Initialize match table
 * init_match_table();
 * ...
 * // Insert match entry
 * insert_match_entry( &ofp_match, priority, service_name, entry_name );
 * ...
 * // Lookup match entry
 * match_entry *match_entry = lookup_match_entry( &ofp_match );
 * ...
 * // Delete match entry
 * delete_match_entry( struct ofp_match *ofp_match );
 * ...
 * // Finalize match table
 * finalize_match_table();
 * @endcode
 */

#ifndef MATCH_TABLE_H
#define MATCH_TABLE_H


#include <openflow.h>
#include "hash_table.h"
#include "linked_list.h"


/**
 * Specifies element of flow table
 */
typedef struct match_entry {
  struct ofp_match ofp_match; /*!< match data. host byte order */
  uint16_t priority;
  list_element *services_name; // application service name of messenger
} match_entry;


void init_match_table( void );
void finalize_match_table( void );
void insert_match_entry( struct ofp_match *ofp_match, uint16_t priority, const char *service_name );
void delete_match_entry( struct ofp_match *ofp_match, uint16_t priority, const char *service_name );
match_entry *lookup_match_entry( struct ofp_match *match );


#endif // MATCH_TABLE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
