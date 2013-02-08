/*
 * Chibach common functions.
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


#ifndef CHIBACH_H
#define CHIBACH_H


#include "bool.h"
#include "buffer.h"
#include "byteorder.h"
#include "checks.h"
#include "doubly_linked_list.h"
#include "ether.h"
#include "event_handler.h"
#include "hash_table.h"
#include "linked_list.h"
#include "log.h"
#include "match.h"
#include "match_table.h"
#include "message_queue.h"
#include "messenger.h"
#include "openflow_service_interface.h"
#include "openflow_switch_interface.h"
#include "openflow_message.h"
#include "packet_info.h"
#include "stat.h"
#include "timer.h"
#include "utility.h"
#include "wrapper.h"


void init_chibach( int *argc, char ***argv );
void start_chibach( void );
void stop_chibach( void );
void flush( void );
const char *get_chibach_home( void );
const char *get_chibach_tmp( void );
const char *get_chibach_name( void );
void set_chibach_name( const char *name );
__attribute__( ( weak ) ) void usage( void );
uint64_t get_datapath_id( void );


#endif // CHIBACH_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
