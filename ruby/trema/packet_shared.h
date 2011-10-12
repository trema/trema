/*
 * Author: Jari Sundell
 *
 * Copyright (C) 2011 axsh Ltd.
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


#define PACKET_SHARED_DEFINE_METHODS( class_name )                      \
  rb_define_method( class_name, "macsa", packet_shared_macsa, 0 );      \
  rb_define_method( class_name, "macda", packet_shared_macda, 0 );      \
  rb_define_method( class_name, "arp?", packet_shared_is_arp, 0 );      \
  rb_define_method( class_name, "arp_sha", packet_shared_arp_sha, 0 );  \
  rb_define_method( class_name, "arp_spa", packet_shared_arp_spa, 0 );  \
  rb_define_method( class_name, "arp_tha", packet_shared_arp_tha, 0 );  \
  rb_define_method( class_name, "arp_tpa", packet_shared_arp_tpa, 0 );


/*
 * The MAC source address.
 *
 * @return [Trema::Mac] macsa MAC source address.
 */
static VALUE
packet_shared_macsa( VALUE self ) {
  VALUE macsa = ULL2NUM( mac_to_uint64( get_packet_shared_info( self )->eth_macsa ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, macsa );
}


/*
 * The MAC destination address.
 *
 * @return [Trema::Mac] macda MAC destination address.
 */
static VALUE
packet_shared_macda( VALUE self ) {
  VALUE macda = ULL2NUM( mac_to_uint64( get_packet_shared_info( self )->eth_macda ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, macda );
}


static VALUE
packet_shared_is_arp( VALUE self ) {
  if ( ( get_packet_shared_info( self )->format & NW_ARP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


static VALUE
packet_shared_arp_sha( VALUE self ) {
  VALUE value = ULL2NUM( mac_to_uint64( get_packet_shared_info( self )->arp_sha ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, value );
}


static VALUE
packet_shared_arp_spa( VALUE self ) {
  VALUE value = ULONG2NUM( get_packet_shared_info( self )->arp_spa );
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, value );
}


static VALUE
packet_shared_arp_tha( VALUE self ) {
  VALUE value = ULL2NUM( mac_to_uint64( get_packet_shared_info( self )->arp_tha ) );
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, value );
}


static VALUE
packet_shared_arp_tpa( VALUE self ) {
  VALUE value = ULONG2NUM( get_packet_shared_info( self )->arp_tpa );
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, value );
}
