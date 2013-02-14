/*
 * Author: Yasunobu Chiba
 *
 * Copyright (C) 2011 NEC Corporation
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


#include <sys/types.h>
#include <unistd.h>
#include "flow_manager_interface.h"
#include "trema.h"


static uint64_t flow_entry_group_index = 0;


buffer *
create_flow_entry( uint64_t datapath_id, struct ofp_match match,
                   uint16_t priority, uint16_t idle_timeout,
                   uint16_t hard_timeout, openflow_actions *actions ) {
  size_t actions_length = 0;
  if ( actions != NULL && actions->n_actions > 0 ) {
    list_element *e = actions->list;
    while ( e != NULL ) {
      struct ofp_action_header *ah = e->data;
      actions_length += ah->len;
      e = e->next;
    }
  }

  size_t length = offsetof( flow_entry, actions ) + actions_length;
  buffer *buf = alloc_buffer_with_length( length );
  flow_entry *entry = append_front_buffer( buf, length );

  entry->datapath_id = datapath_id;
  entry->match = match;
  entry->priority = priority;
  entry->idle_timeout = idle_timeout;
  entry->hard_timeout = hard_timeout;
  entry->actions_length = ( uint16_t ) actions_length;

  if ( actions != NULL ) {
    size_t offset = 0;
    list_element *e = actions->list;
    while ( e != NULL ) {
      struct ofp_action_header *ah = e->data;
      memcpy( ( char * ) entry->actions + offset, ah, ah->len );
      offset += ah->len;
      e = e->next;
    }
  }

  return buf;
}


uint64_t
get_flow_entry_group_id( void ) {
  if ( ( flow_entry_group_index & 0x0000ffffffffffffULL ) == 0x0000ffffffffffffULL ) {
    flow_entry_group_index = 0x0000000000000000ULL;
  }
  else {
    flow_entry_group_index++;
  }

  return ( flow_entry_group_index | ( ( uint64_t ) getpid() << 48 ) );
}


buffer *
create_flow_entry_group_setup_request( uint64_t id, const char *owner,
                                       uint16_t n_entries, buffer *entries ) {
  size_t length = offsetof( flow_entry_group_setup_request, entries ) + entries->length;
  buffer *buf = alloc_buffer_with_length( length );
  flow_entry_group_setup_request *request = append_front_buffer( buf, length );

  request->id = id;
  memcpy( request->owner, owner, sizeof( request->owner ) );
  request->owner[ sizeof( request->owner ) - 1 ] = '\0';
  request->n_entries = n_entries;
  request->entries_length = ( uint32_t ) entries->length;
  memcpy( request->entries, entries->data, entries->length );

  return buf;
}


buffer *
create_flow_entry_group_setup_reply( uint64_t id, uint8_t status ) {
  size_t length = sizeof( flow_entry_group_setup_reply );
  buffer *buf = alloc_buffer_with_length( length );
  flow_entry_group_teardown_reply *reply = append_front_buffer( buf, length );

  reply->id = id;
  reply->status = status;

  return buf;
}


buffer *
create_flow_entry_group_teardown_request( uint64_t id ) {
  size_t length = sizeof( flow_entry_group_teardown_request );
  buffer *buf = alloc_buffer_with_length( length );
  flow_entry_group_teardown_request *request = append_front_buffer( buf, length );

  request->id = id;

  return buf;
}


buffer *
create_flow_entry_group_teardown_reply( uint64_t id, uint8_t status ) {
  size_t length = sizeof( flow_entry_group_teardown_reply );
  buffer *buf = alloc_buffer_with_length( length );
  flow_entry_group_teardown_reply *reply = append_front_buffer( buf, length );

  reply->id = id;
  reply->status = status;

  return buf;
}


buffer *
create_flow_entry_group_teardown( uint64_t id, uint8_t reason ) {
  size_t length = sizeof( flow_entry_group_teardown );
  buffer *buf = alloc_buffer_with_length( length );
  flow_entry_group_teardown *notification = append_front_buffer( buf, length );

  notification->id = id;
  notification->reason = reason;

  return buf;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
