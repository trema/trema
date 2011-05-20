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


#include <string.h>
#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cMatch;


static VALUE
match_alloc( VALUE klass ) {
  struct ofp_match *match = malloc( sizeof( struct ofp_match ) );
  return Data_Wrap_Struct( klass, NULL, free, match );
}


static VALUE
match_from( VALUE klass, VALUE rpacket ) {
  VALUE obj;
  struct ofp_match *match;
  packet_in *packet;

  obj = rb_funcall( klass, rb_intern( "new" ), 0 );
  Data_Get_Struct( obj, struct ofp_match, match );
  Data_Get_Struct( rpacket, packet_in, packet );
  set_match_from_packet( match, packet->in_port, 0, packet->data );

  return obj;
}


void
Init_match() {
  cMatch = rb_define_class_under( mTrema, "Match", rb_cObject );
  rb_define_alloc_func( cMatch, match_alloc );
  rb_define_singleton_method( cMatch, "from", match_from, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

