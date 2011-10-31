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

#ifndef PACKET_IN_H
#define PACKET_IN_H


#define PACKET_SHARED_RETURN_MAC(packet_member)                         \
  VALUE ret = ULL2NUM( mac_to_uint64( get_packet_shared_info( self )->packet_member ) ); \
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );


#define PACKET_SHARED_SET_MAC(packet_member)                            \
  uint64_to_mac( NUM2ULL( value ), get_packet_shared_info( self )->packet_member );


#define PACKET_SHARED_RETURN_IP(packet_member)                          \
  VALUE ret = ULONG2NUM( get_packet_shared_info( self )->arp_spa );     \
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, ret );


#define PACKET_SHARED_SET_IP(packet_member)                             \
  get_packet_shared_info( self )->packet_member = ( uint32_t )NUM2UINT( value );


#endif // PACKET_IN_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
