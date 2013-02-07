/*
 * OpenFlow Switch Listener
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


#ifndef SECURE_CANNEL_H
#define SECURE_CANNEL_H


typedef void ( *connected_handler )( void );
typedef void ( *disconnected_handler )( void );


bool init_secure_channel( uint32_t ip, uint16_t port,
                          connected_handler connected_callback, disconnected_handler disconnected_callback );
bool finalize_secure_channel();
bool send_message_to_secure_channel( buffer *message );


#endif // SECURE_CANNEL_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
