/*
 * OpenFlow service interface.
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


#ifndef OPENFLOW_SERVICE_INTERFACE_H
#define OPENFLOW_SERVICE_INTERFACE_H


/**
 * Message type definitions for sending/receiving OpenFlow messages or
 * related events via messenger.
 */
#define MESSENGER_OPENFLOW_MESSAGE 1
#define MESSENGER_OPENFLOW_CONNECTED 2
#define MESSENGER_OPENFLOW_READY 3
#define MESSENGER_OPENFLOW_DISCONNECTED 4
#define MESSENGER_OPENFLOW_DISCONNECT_REQUEST 5
#define MESSENGER_OPENFLOW_FAILD_TO_CONNECT 6


/**
 * Header for sending/receiving OpenFlow messages or events via messenger.
 * A null-terminated service name can be provided after service_name_len
 * and an OpenFlow message must be included in the rest of part in case of
 * MESSENGER_OPENFLOW_MESSAGE. service_name_length can be zero if service
 * name notification is not necessary.
 */
typedef struct openflow_service_header {
  uint64_t datapath_id;
  uint16_t service_name_length;
} __attribute__( ( packed ) ) openflow_service_header_t;


#endif // OPENFLOW_SERVICE_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
