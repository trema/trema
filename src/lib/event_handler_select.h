/*
 * Trema event handler library.
 *
 * Author: Jari Sundell
 *
 * Copyright (C) 2011 axsh Ltd.
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


#ifndef LIB_EVENT_HANDLER_SELECT_H
#define LIB_EVENT_HANDLER_SELECT_H

#include <sys/types.h>

#include "bool.h"
#include "event_handler.h"


extern void select_init_event_handler();
extern void select_finalize_event_handler();

extern bool select_start_event_handler();
extern void select_stop_event_handler();

extern bool select_run_event_handler_once();

extern void select_add_fd_event( int fd, event_fd_callback read_callback, void *read_data, event_fd_callback write_callback, void *write_data );
extern void select_delete_fd_event( int fd );

extern void select_notify_readable_event( int fd, bool state );
extern void select_notify_writable_event( int fd, bool state );

extern bool select_is_notifying_readable_event( int fd );
extern bool select_is_notifying_writable_event( int fd );

extern bool select_set_external_callback( external_callback_t callback );


#endif // EVENT_HANDLER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
