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
match_from( VALUE klass, VALUE message ) {
  VALUE obj;
  struct ofp_match *match;
  packet_in *packet;

  obj = rb_funcall( klass, rb_intern( "new" ), 0 );
  Data_Get_Struct( obj, struct ofp_match, match );
  Data_Get_Struct( message, packet_in, packet );
  set_match_from_packet( match, packet->in_port, 0, packet->data );

  return obj;
}

static struct ofp_match*
get_match( VALUE self ) {
  struct ofp_match *match;

  Data_Get_Struct( self, struct ofp_match, match );
  return match;
}

static VALUE
match_replace( VALUE self, VALUE other ) {
  memcpy( get_match( self ), get_match( other ), sizeof ( struct ofp_match ) );
  return self;
}

static VALUE
match_to_s( VALUE self ) {
  char match_str[ 1024 ];

  match_to_string( get_match( self ), match_str, sizeof ( match_str ) );
  return rb_str_new2( match_str );
}

static VALUE
match_wildcards( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->wildcards );
}

static VALUE
match_in_port( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->in_port );
}

static VALUE
match_dl( VALUE self, uint8_t which ) {
  struct ofp_match *match;
  uint8_t *dl_addr;

  match = get_match( self );
  if ( which ) {
    dl_addr = match->dl_src;
  } else {
    dl_addr = match->dl_dst;
  }
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( dl_addr ) ) );
}

static VALUE
match_dl_src( VALUE self ) {

  return match_dl( self, 1 );
}

static VALUE
match_dl_dst( VALUE self ) {
  return match_dl( self, 0 );
}

static VALUE
match_dl_vlan( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->dl_vlan );
}

static VALUE
match_dl_vlan_pcp( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->dl_vlan_pcp );
}

static VALUE
match_dl_type( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->dl_type );
}

static VALUE
match_nw_tos( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->nw_tos );
}

static VALUE
match_nw_proto( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->nw_proto );
}

static VALUE
match_nw_src( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->nw_src );
}

static VALUE
match_nw_dst( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->nw_dst );
}

static VALUE
match_tp_src( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->tp_src );
}

static VALUE
match_tp_dst( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->tp_dst );
}

void
Init_match() {
  cMatch = rb_define_class_under( mTrema, "Match", rb_cObject );
  rb_define_alloc_func( cMatch, match_alloc );
  rb_define_method( cMatch, "replace", match_replace, 1 );
  rb_define_method( cMatch, "to_s", match_to_s, 0 );
  rb_define_method( cMatch, "wildcards", match_wildcards, 0 );
  rb_define_method( cMatch, "in_port", match_in_port, 0 );
  rb_define_method( cMatch, "dl_src", match_dl_src, 0 );
  rb_define_method( cMatch, "dl_dst", match_dl_dst, 0 );
  rb_define_method( cMatch, "dl_vlan", match_dl_vlan, 0 );
  rb_define_method( cMatch, "dl_vlan_pcp", match_dl_vlan_pcp, 0 );
  rb_define_method( cMatch, "dl_type", match_dl_type, 0 );
  rb_define_method( cMatch, "nw_tos", match_nw_tos, 0 );
  rb_define_method( cMatch, "nw_proto", match_nw_proto, 0 );
  rb_define_method( cMatch, "nw_src", match_nw_src, 0 );
  rb_define_method( cMatch, "nw_dst", match_nw_dst, 0 );
  rb_define_method( cMatch, "tp_src", match_tp_src, 0 );
  rb_define_method( cMatch, "tp_dst", match_tp_dst, 0 );
  rb_define_singleton_method( cMatch, "from", match_from, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

