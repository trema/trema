/*
 * Daemon utility functions.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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

/**
 * @file
 *
 * @brief Daemonization supporting functions for Trema Applications
 *
 * File contains function declarations for daemonization of a Trema binary and
 * controlling it (using PID). This currently being internally used by the trema
 * binary in case 'd' command line argument is passed to it.
 * @code
 * ...
 * // Send the current instance to background (daemon) by closing any terminal I/O streams
 * daemonize( get_trema_home() );
 * // Writes Process ID to file in Trema temporary directory
 * write_pid( get_trema_tmp(), get_trema_name() );
 * ...
 * // Unlinks Process ID from file in Trema temporary directory
 * unlink_pid( get_trema_tmp(), get_trema_name() );
 * ...
 * // Gets Process ID from name
 * return read_pid( get_trema_tmp(), name );
 * ...
 * @endcode
 *
 */

#ifndef DAEMON_H
#define DAEMON_H


void daemonize( const char *home );
void write_pid( const char *directory, const char *name );
void unlink_pid( const char *directory, const char *name );
pid_t read_pid( const char *directory, const char *name );
void rename_pid( const char *directory, const char *old, const char *new );


#endif // DAEMON_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
