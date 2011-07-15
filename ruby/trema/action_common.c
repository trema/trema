/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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
#include "ruby.h"

uint32_t nw_addr_to_i( VALUE nw_addr );
uint8_t *dl_addr_short( VALUE dl_addr, uint8_t *ret_dl_addr );

uint32_t
nw_addr_to_i( VALUE nw_addr ) {
  char cmd_buf[80];

  sprintf( cmd_buf, "IPAddr.new('%s').to_i", RSTRING_PTR( nw_addr ) );
  return rb_big2uint( rb_eval_string( cmd_buf ) );
}

uint8_t *
dl_addr_short( VALUE dl_addr, uint8_t *ret_dl_addr ) {
  VALUE mac_arr;

  mac_arr = rb_str_split( dl_addr, ":" );

  VALUE *data_ptr = RARRAY_PTR( mac_arr );
  int len = RARRAY_LEN( mac_arr ), i;

  for ( i = 0; i < len; i++ ) {
    ret_dl_addr[i] = ( uint8_t ) ( FIX2INT( rb_funcall( data_ptr[i], rb_intern( "hex" ), 0 ) ) );
  }
  return ret_dl_addr;
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

