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


#ifndef SERVICE_INTERFACE_H
#define SERVICE_INTERFACE_H


#include "trema.h"
#include "openflow_service_interface.h"
#include "switchinfo.h"


void service_send_to_reply( char *service_name, uint16_t message_type, uint64_t *datapath_id, buffer *buf );
void service_send_to_application( list_element *service_name_list, uint16_t message_type, uint64_t *datapath_id, buffer *buf );
void service_recv_from_application( uint16_t message_type, buffer *buf );


#endif // SERVICE_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

