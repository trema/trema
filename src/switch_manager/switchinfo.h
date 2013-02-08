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


#ifndef SWITCHINFO_H
#define SWITCHINFO_H


#include "message_queue.h"


#define SWITCH_STATE_CONNECTED           0
#define SWITCH_STATE_WAIT_HELLO          1
#define SWITCH_STATE_WAIT_FEATURES_REPLY 2
#define SWITCH_STATE_COMPLETED           3
#define SWITCH_STATE_DISCONNECTED        4


struct switch_info {
  list_element *vendor_service_name_list;     // vender manager service
  list_element *packetin_service_name_list;   // packetin manager service
  list_element *portstatus_service_name_list; // portstatus manager service
  list_element *state_service_name_list;      // switch state manager service

  char *dpid_service_name;      // service name of messenger
  struct notify_info *notify_info;

  int secure_channel_fd;        // socket file descriptor of secure channel

  bool flow_cleanup;
  bool cookie_translation;
  bool deny_packet_in_on_startup;

  int state;                    // state of switch secure channel
  uint64_t datapath_id;

  uint16_t config_flags;        // OFPC_* flags
  uint16_t miss_send_len;       /* Max bytes of new flow that datapath should
                                   send to the controller. */

  buffer *fragment_buf;         /* openflow message fragmentation buffer of
                                   secure channel receiver */

  message_queue *send_queue;
  message_queue *recv_queue;

  bool running_timer;

  uint32_t echo_request_xid;
};


#endif // SWITCHINFO_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
