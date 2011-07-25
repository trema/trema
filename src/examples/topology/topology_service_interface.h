/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#ifndef TOPOLOGY_SERVICE_INTERFACE_H
#define TOPOLOGY_SERVICE_INTERFACE_H


/*
 * definitions for messager-style API
 */
enum topology_message_type {
  // response message for subscribe, unsubscribe
  TD_MSGTYPE_RESPONSE = 0x8001,
  // request
  TD_MSGTYPE_SUBSCRIBE,
  TD_MSGTYPE_UNSUBSCRIBE,
  TD_MSGTYPE_QUERY_LINK_STATUS,
  TD_MSGTYPE_QUERY_PORT_STATUS,
  TD_MSGTYPE_QUERY_SWITCH_STATUS,
  TD_MSGTYPE_UPDATE_LINK_STATUS,
  // notify and response message for query
  TD_MSGTYPE_LINK_STATUS,
  TD_MSGTYPE_PORT_STATUS,
  TD_MSGTYPE_SWITCH_STATUS,
};

// subscribe, unsubscribe
typedef struct topology_request {
  char name[ 0 ];               /* (un)subscriber's name */
} __attribute__( ( packed ) ) topology_request;

// update, delete link status
typedef struct topology_update_link_status {
  uint64_t from_dpid;
  uint64_t to_dpid;
  uint16_t from_portno;
  uint16_t to_portno;
  uint8_t status;
  uint8_t pad[ 3 ];
} __attribute__( ( packed ) ) topology_update_link_status;

// response
typedef struct topology_response {
  uint8_t status;
  uint8_t pad[ 3 ];
} __attribute__( ( packed ) ) topology_response;

enum topology_status_type {
  TD_RESPONSE_OK = 0,
  TD_RESPONSE_ALREADY_SUBSCRIBED,
  TD_RESPONSE_NO_SUCH_SUBSCRIBER,
};

// link status
// length is specified in callback function
typedef struct topology_link_status {
  uint64_t from_dpid;
  uint64_t to_dpid;
  uint16_t from_portno;
  uint16_t to_portno;
  uint8_t status;
  uint8_t pad[ 3 ];
} __attribute__( ( packed ) ) topology_link_status;

enum topology_link_status_type {
  TD_LINK_DOWN = 0,
  TD_LINK_UP,
  TD_LINK_UNSTABLE,
};

// port status
// length is specified in callback function
typedef struct topology_port_status {
  uint64_t dpid;
  uint16_t port_no;
  char name[ OFP_MAX_PORT_NAME_LEN ];
  uint8_t mac[ ETH_ADDRLEN ];
  uint8_t external;
  uint8_t status;
  uint8_t pad[ 2 ];
} __attribute__( ( packed ) ) topology_port_status;

enum topology_port_status_type {
  TD_PORT_DOWN = 0,
  TD_PORT_UP,
};

enum topology_port_external_type {
  TD_PORT_INACTIVE = 0,
  TD_PORT_EXTERNAL,
};

// openflow switch status
// length is specified in callback function
typedef struct topology_switch_status {
  uint64_t dpid;
} __attribute__( ( packed ) ) topology_switch_status;


#endif // TOPOLOGY_SERVICE_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
