/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#ifndef TOPOLOGY_SERVICE_INTERFACE_H
#define TOPOLOGY_SERVICE_INTERFACE_H

#include "ether.h"
/*
 * definitions for messager-style API
 */
enum topology_message_type {

  // MSGTYPE for request messages
  NOT_USED_TD_MSGTYPE_REQUEST_BEGIN = 0x8000,
  TD_MSGTYPE_SUBSCRIBE_REQUEST,
  TD_MSGTYPE_UNSUBSCRIBE_REQUEST,

  TD_MSGTYPE_QUERY_LINK_STATUS_REQUEST,
  TD_MSGTYPE_QUERY_PORT_STATUS_REQUEST,
  TD_MSGTYPE_QUERY_SWITCH_STATUS_REQUEST,

  TD_MSGTYPE_UPDATE_LINK_STATUS_REQUEST,

  TD_MSGTYPE_PING_REQUEST,

  TD_MSGTYPE_ENABLE_DISCOVERY_REQUEST,
  TD_MSGTYPE_DISABLE_DISCOVERY_REQUEST,


  // MSGTYPE for response messages
  NOT_USED_TD_MSGTYPE_RESPONSE_BEGIN = 0x9000,
  TD_MSGTYPE_SUBSCRIBE_RESPONSE,
  TD_MSGTYPE_UNSUBSCRIBE_RESPONSE,

  TD_MSGTYPE_QUERY_LINK_STATUS_RESPONSE,
  TD_MSGTYPE_QUERY_PORT_STATUS_RESPONSE,
  TD_MSGTYPE_QUERY_SWITCH_STATUS_RESPONSE,

  TD_MSGTYPE_UPDATE_LINK_STATUS_RESPONSE,

  TD_MSGTYPE_PING_RESPONSE,

  TD_MSGTYPE_ENABLE_DISCOVERY_RESPONSE,
  TD_MSGTYPE_DISABLE_DISCOVERY_RESPONSE,


  // MSGTYPE for notification message
  NOT_USED_TD_MSGTYPE_NOTIFICATION_BEGIN = 0xA000,
  TD_MSGTYPE_LINK_STATUS_NOTIFICATION,
  TD_MSGTYPE_PORT_STATUS_NOTIFICATION,
  TD_MSGTYPE_SWITCH_STATUS_NOTIFICATION,

//  TD_MSGTYPE_CONFIG_DISCOVERY,
};

// subscribe, unsubscribe, ping, discovery
typedef struct topology_request {
  char name[ 0 ];               /* (un)subscriber's name */
} __attribute__( ( packed ) ) topology_request;

// update, delete link status
typedef struct topology_update_link_status {
  uint64_t from_dpid;
  uint64_t to_dpid;
  uint16_t from_portno;
  uint16_t to_portno;
  uint8_t status;       // enum topology_link_status_type
  uint8_t pad[ 3 ];
} __attribute__( ( packed ) ) topology_update_link_status;

// response
typedef struct topology_response {
  uint8_t status;       // enum topology_status_type
  uint8_t pad[ 3 ];
} __attribute__( ( packed ) ) topology_response;

enum topology_status_type {
  TD_RESPONSE_OK = 0,
  TD_RESPONSE_ALREADY_SUBSCRIBED,
  TD_RESPONSE_NO_SUCH_SUBSCRIBER,
  TD_RESPONSE_INVALID,
};

typedef struct topology_ping_response {
  char name[ 0 ];
} __attribute__( ( packed ) ) topology_ping_response;

// link status
// length is specified in callback function
typedef struct topology_link_status {
  uint64_t from_dpid;
  uint64_t to_dpid;
  uint16_t from_portno;
  uint16_t to_portno;
  uint8_t status;       // enum topology_link_status_type
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
  uint8_t pad[ 6 ];     // Force 8 byte alignment. 6 = 8 - (2+16+6+1+1)%8
  char name[ OFP_MAX_PORT_NAME_LEN ];
  uint8_t mac[ ETH_ADDRLEN ];
  uint8_t external;     // enum topology_port_external_type
  uint8_t status;       // enum topology_port_status_type
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
  uint8_t status;       // enum topology_switch_status_type
  uint8_t pad[7];       // 7 = 8 - (1)%8
} __attribute__( ( packed ) ) topology_switch_status;

enum topology_switch_status_type {
  TD_SWITCH_DOWN = 0,
  TD_SWITCH_UP,
};



#endif // TOPOLOGY_SERVICE_INTERFACE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
