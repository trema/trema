/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cPort;


static VALUE
port_init( VALUE self, VALUE attributes ) {
  VALUE number = rb_hash_aref( attributes, ID2SYM( rb_intern( "number" ) ) );
  rb_iv_set( self, "@number", number );
  return self;
}


static VALUE
port_number( VALUE self ) {
  return rb_iv_get( self, "@number" );
}


void
Init_port() {
  cPort = rb_define_class_under( mTrema, "Port", rb_cObject );
  rb_define_method( cPort, "initialize", port_init, 1 );
  rb_define_method( cPort, "number", port_number, 0 );
}


VALUE
port_from( const struct ofp_phy_port *phy_port ) {
  VALUE attributes = rb_hash_new();
  rb_hash_aset( attributes, ID2SYM( rb_intern( "number" ) ), UINT2NUM( phy_port->port_no ) );
  return rb_funcall( cPort, rb_intern( "new" ), 1, attributes );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
