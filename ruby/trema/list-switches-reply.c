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


#include "trema.h"
#include "ruby.h"


void
handle_list_switches_reply( const list_element *switches, void *controller ) {
  if ( rb_respond_to( ( VALUE ) controller, rb_intern( "list_switches_reply" ) ) == Qfalse ) {
   return;
  }
  VALUE dpids = rb_ary_new();

  const list_element *element = NULL;
  for ( element = switches; element != NULL; element = element->next ) {
    rb_ary_push( dpids, ULL2NUM( *( uint64_t * ) element->data ) );
  }

  rb_funcall( ( VALUE ) controller, rb_intern( "list_switches_reply" ), 1, dpids );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
