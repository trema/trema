/*
 * Trema common functions.
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


#ifndef TREMA_H
#define TREMA_H


#include "bool.h"
#include "buffer.h"
#include "byteorder.h"
#include "checks.h"
#include "doubly_linked_list.h"
#include "etherip.h"
#include "event_forward_interface.h"
#include "event_handler.h"
#include "hash_table.h"
#include "linked_list.h"
#include "log.h"
#include "management_service_interface.h"
#include "match.h"
#include "match_table.h"
#include "message_queue.h"
#include "messenger.h"
#include "openflow_application_interface.h"
#include "openflow_message.h"
#include "packet_info.h"
#include "packetin_filter_interface.h"
#include "persistent_storage.h"
#include "stat.h"
#include "timer.h"
#include "topology.h"
#include "topology_service_interface_option_parser.h"
#include "utility.h"
#include "wrapper.h"


static const char DEFAULT_DUMP_SERVICE_NAME[] = "dump_service";


void init_trema( int *argc, char ***argv );
void start_trema( void );
void start_trema_up();
void start_trema_down();
void stop_trema( void );
void flush( void );
const char *get_trema_home( void );
const char *get_trema_tmp( void );
void set_trema_name( const char *name );
const char *get_trema_name( void );
const char *get_executable_name( void );
pid_t get_trema_process_from_name( const char *name );
bool terminate_trema_process( pid_t pid );
__attribute__( ( weak ) ) void usage( void );


#endif // TREMA_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
