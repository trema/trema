/*
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


#include <assert.h>
#include "log.h"
#include "packet_info.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#ifdef debug
#undef debug
#endif
#define debug mock_debug
void mock_debug( const char *format, ... );

#endif // UNIT_TESTING


uint16_t
fill_ether_padding( buffer *buf ) {
  assert( buf != NULL );
  size_t padding_length = 0;

  if ( buf->length + ETH_FCS_LENGTH < ETH_MINIMUM_LENGTH ) {
    padding_length = ETH_MINIMUM_LENGTH - buf->length - ETH_FCS_LENGTH;
    debug( "Adding %u octets padding ( original frame length = %u ).", buf->length, padding_length );
    append_back_buffer( buf, padding_length );
  }
  return ( uint16_t ) padding_length;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
