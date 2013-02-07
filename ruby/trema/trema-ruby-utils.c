/*
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


#include "ruby.h"
#include "trema.h"


void
set_xid( const buffer *openflow_message, uint32_t xid ) {
  ( ( struct ofp_header * ) ( openflow_message->data ) )->xid = htonl( xid );
}


VALUE
get_xid( VALUE self ) {
  buffer *openflow_message;
  Data_Get_Struct( self, buffer, openflow_message );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( openflow_message->data ) )->xid );
  return UINT2NUM( xid );
}


void
set_length( const buffer *openflow_message, uint16_t length ) {
  ( ( struct ofp_header * ) ( openflow_message->data ) )->length = htons( length );
}


uint16_t
get_length( const buffer *openflow_message ) {
  return ( uint16_t ) ( openflow_message->length - sizeof( struct ofp_vendor_header ) );
}


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
