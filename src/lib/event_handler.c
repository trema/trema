/*
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

#include "event_handler.h"

#include <stddef.h>

void (*init_event_handler)() = NULL;
void (*finalize_event_handler)() = NULL;

bool (*start_event_handler)() = NULL;
void (*stop_event_handler)() = NULL;

bool (*run_event_handler_once)() = NULL;

void (*add_fd_event)( int fd, event_fd_callback read_callback, void* read_data, event_fd_callback write_callback, void* write_data ) = NULL;
void (*delete_fd_event)( int fd ) = NULL;

void (*notify_readable_event)( int fd, bool state ) = NULL;
void (*notify_writable_event)( int fd, bool state ) = NULL;

bool (*is_notifying_readable_event)( int fd ) = NULL;
bool (*is_notifying_writable_event)( int fd ) = NULL;
