/*
 * Trema event handler library.
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


#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H


#include <sys/types.h>
#include "bool.h"


typedef void ( *event_fd_callback )( int, void *data );
typedef void ( *external_callback_t )( void );

extern void ( *init_event_handler )();
extern void ( *finalize_event_handler )();

extern bool ( *start_event_handler )();
extern void ( *stop_event_handler )();

extern bool ( *run_event_handler_once )( int timeout_usec );

extern void ( *set_fd_handler )( int fd, event_fd_callback read_callback, void *read_data, event_fd_callback write_callback, void *write_data );
extern void ( *delete_fd_handler )( int fd );

extern void ( *set_readable )( int fd, bool state );
extern void ( *set_writable )( int fd, bool state );

extern bool ( *readable )( int fd );
extern bool ( *writable )( int fd );

// Optional functions for event handlers to implement, must be signal
// safe. Leave as NULL if not supported.
extern bool ( *set_external_callback )( external_callback_t callback );


#endif // EVENT_HANDLER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
