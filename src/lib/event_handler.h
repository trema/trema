/*
 * Trema event handler library.
 *
 * Author: <addme>
 *
 * Copyright (C) <addme>
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

typedef void ( *event_fd_callback )( int, void* data );

void init_event_handler();
void finalize_event_handler();

void add_fd_event( int fd, event_fd_callback read_callback, event_fd_callback write_callback );
void delete_fd_event( int fd );

void notify_readable_event( int fd, bool state );
void notify_writable_event( int fd, bool state );

void is_notifying_readable_event( int fd, bool state );
void is_notifying_writable_event( int fd, bool state );


#endif // EVENT_HANDLER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
