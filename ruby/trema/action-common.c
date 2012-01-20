/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


#include <stdint.h>
#include "ruby.h"


/*
 * @return [Number] an IPv4 address in its numeric representation.
 */
uint32_t
nw_addr_to_i( VALUE nw_addr ) {
  return ( uint32_t ) NUM2UINT( rb_funcall( nw_addr, rb_intern( "to_i" ), 0 ) );
}


/*
 * @return [String] an IPv4 address in its text representation.
 */
VALUE
nw_addr_to_s( VALUE nw_addr ) {
  return rb_funcall( nw_addr, rb_intern( "to_s" ), 0 );
}


uint8_t *
dl_addr_short( VALUE dl_addr, uint8_t *ret_dl_addr ) {
  VALUE mac_arr = rb_funcall( dl_addr, rb_intern( "to_short" ), 0 );
  int i;

  for ( i = 0; i < RARRAY_LEN( mac_arr); i++ ) {
    ret_dl_addr[ i ] = ( uint8_t ) ( NUM2INT( RARRAY_PTR( mac_arr )[ i ] ) );
  }
  return ret_dl_addr;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
