/*
 * Ruby wrapper class of OpenFlow packet_in message.
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


#ifndef PACKET_IN_ARP_H
#define PACKET_IN_ARP_H


#include "ruby.h"
#include "trema.h"


extern VALUE cPacketInArp;


void Init_packet_in_arp( void );


#endif // PACKET_IN_ARP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
