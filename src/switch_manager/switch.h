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


#ifndef SWITCH_MANAGER_H
#define SWITCH_MANAGER_H


#include "switchinfo.h"
#include "trema.h"


#define MESSENGER_SERVICE_TYPE_STATE      1
#define MESSENGER_SERVICE_TYPE_SUBSCRIBE  2
#define MESSENGER_SERVICE_TYPE_NOTIFY     3

#define SWITCH_STATE_TIMEOUT_HELLO 5          // in seconds
#define SWITCH_STATE_TIMEOUT_FEATURES_REPLY 5 // in seconds

#define SWITCH_MANAGER_PREFIX "switch."
#define SWITCH_MANAGER_PREFIX_STR_LEN sizeof( SWITCH_MANAGER_PREFIX )
#define SWITCH_MANAGER_DPID_STR_LEN sizeof( "1234567812345678" )

int switch_event_connected( struct switch_info *switch_info );
int switch_event_disconnected( struct switch_info *switch_info );
int switch_event_recv_hello( struct switch_info *switch_info );
int switch_event_recv_echoreply( struct switch_info *switch_info, buffer *buf );
int switch_event_recv_featuresreply( struct switch_info *switch_info, uint64_t *datapath_id );
int switch_event_recv_from_application( uint64_t *datapath_id, char *application_service_name, buffer *buf );
int switch_event_disconnect_request( uint64_t *datapath_id );
int switch_event_recv_error( struct switch_info *sw_info );


#endif // SWITCH_MANAGER_H
