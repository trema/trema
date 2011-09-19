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
#include "packet_info.h"
#include "wrapper.h"


/**
 * Releases the memory allocated to structure of type packet_header_info.
 * @param buf Pointer to buffer type structure
 * @return None
 */
static void
free_packet_header_info( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->user_data != NULL );

  xfree( buf->user_data );
  buf->user_data = NULL;
  buf->user_data_free_function = NULL;
}


/**
 * Allocates memory to structure of type packet_header_info.
 * @param buf Pointer to buffer type structure
 * @return None
 */
void
alloc_packet( buffer *buf ) {
  assert( buf != NULL );

  packet_header_info *header_info = xcalloc( 1, sizeof( packet_header_info ) );
  assert( header_info != NULL );

  header_info->ethtype = 0;
  header_info->nvtags = 0;
  header_info->ipproto = 0;
  header_info->l2_data.l2 = NULL;
  header_info->vtag = NULL;
  header_info->l3_data.l3 = NULL;
  header_info->l4_data.l4 = NULL;
  buf->user_data = header_info;
  buf->user_data_free_function = free_packet_header_info;
}


/**
 * This function is deprecated.
 *
 * Releases the memory allocated to structure of type buffer.
 * @param buf Pointer to buffer type structure
 * @return None
 */
void
free_packet( buffer *buf ) {
  assert( buf != NULL );

  free_packet_header_info( buf );
  free_buffer( buf );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
