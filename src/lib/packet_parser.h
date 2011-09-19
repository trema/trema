/*
 * Packet parser
 *
 * Author: Naoyoshi Tada
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
 * @brief Function declarations of Packet parsing implementation
 *
 * File containing functions declarations for handling packets i.e, parsing a packet to find
 * its type (whether IPv4, or ARP) and calculating checksum if required.
 * @code
 * // Calculates checksum
 * get_checksum( ( uint16_t * ) packet_info( buf )->l3_data.ipv4, ( uint32_t ) hdr_len )
 * ...
 * // Validates packet header information
 * bool parse_ok = parse_packet( body );
 * if ( !parse_ok ) {
 * error( "Failed to parse a packet." );
 * ...
 * @endcode
 */

#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H


#define verify_checksum get_checksum


uint16_t get_checksum( uint16_t *pos, uint32_t size );
bool parse_packet( buffer *buf );


#endif // PACKET_PARSER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
