/*
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


#include "ruby.h"


void
validate_xid( VALUE xid ) {
  if ( rb_obj_is_kind_of( xid, rb_cInteger ) != Qtrue ) {
    rb_raise( rb_eTypeError, "Transaction ID must be an integer" );
  }
  if ( rb_funcall( xid, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
    rb_raise( rb_eArgError, "Transaction ID must be an unsigned 32-bit integer" );
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
