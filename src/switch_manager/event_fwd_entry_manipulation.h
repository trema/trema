/*
 * Copyright (C) 2013 NEC Corporation
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

#ifndef EVENT_FWD_ENTRY_MANIPULATION_H_
#define EVENT_FWD_ENTRY_MANIPULATION_H_

#include "event_fwd_interface.h"

void management_event_fwd_entry_add( list_element** service_list,
                             const event_fwd_op_request* request, size_t request_len );
void management_event_fwd_entry_delete( list_element** service_list,
                             const event_fwd_op_request* request, size_t request_len );
void management_event_fwd_entries_set( list_element** service_list,
                             const event_fwd_op_request* request, size_t request_len );


#endif /* EVENT_FWD_ENTRY_MANIPULATION_H_ */
