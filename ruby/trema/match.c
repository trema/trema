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
#include "action-common.h"


extern VALUE mTrema;
VALUE cMatch;


static VALUE
match_alloc( VALUE klass ) {
  struct ofp_match *match = xmalloc( sizeof( struct ofp_match ) );
  return Data_Wrap_Struct( klass, NULL, xfree, match );
}


static struct 
ofp_match *get_match( VALUE self ) {
  struct ofp_match *match;

  Data_Get_Struct( self, struct ofp_match, match );
  return match;
}


static VALUE
match_from( int argc, VALUE *argv, VALUE self ) {
  VALUE message, obj, wildcard_id, options;
  struct ofp_match *match;
  packet_in *packet;
  int i;
  uint32_t wildcards = 0;

  if ( rb_scan_args( argc, argv, "1*", &message, &options ) >= 1 ) {
    obj = rb_funcall( self, rb_intern( "new" ), 0 );
    match = get_match( obj );
    Data_Get_Struct( message, packet_in, packet );
    for ( i = 0; i < RARRAY_LEN( options ); i++ ) {
      wildcard_id = SYM2ID( RARRAY_PTR( options )[ i ] );
      if ( rb_intern( "inport" ) == wildcard_id ) {
        wildcards |= OFPFW_IN_PORT;
      }
      if ( rb_intern( "dl_src" ) == wildcard_id ) {
        wildcards |= OFPFW_DL_SRC;
      }
      if ( rb_intern( "dl_dst" ) == wildcard_id ) {
        wildcards |= OFPFW_DL_DST;
      }
      if ( rb_intern( "dl_vlan" ) == wildcard_id ) {
        wildcards |= OFPFW_DL_VLAN;
      }
      if ( rb_intern( "dl_vlan_pcp" ) == wildcard_id ) {
        wildcards |= OFPFW_DL_VLAN_PCP;
      }
      if ( rb_intern( "dl_type" ) == wildcard_id ) {
        wildcards |= OFPFW_DL_TYPE;
      }
      if ( rb_intern( "nw_tos" ) == wildcard_id ) {
        wildcards |= OFPFW_NW_TOS;
      }
      if ( rb_intern( "nw_proto" ) == wildcard_id ) {
        wildcards |= OFPFW_NW_PROTO;
      }
      if ( rb_intern( "nw_src" ) == wildcard_id ) {
        wildcards |= OFPFW_NW_SRC_ALL;
      }
      if ( rb_intern( "nw_dst" ) == wildcard_id ) {
        wildcards |= OFPFW_NW_DST_ALL;
      }
      if ( rb_intern( "tp_src" ) == wildcard_id ) {
        wildcards |= OFPFW_TP_SRC;
      }
      if ( rb_intern( "tp_dst" ) == wildcard_id ) {
        wildcards |= OFPFW_TP_DST;
      }
    }
  }
  set_match_from_packet( match, packet->in_port, wildcards, packet->data );
  return obj;
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


static VALUE
match_init( int argc, VALUE *argv, VALUE self ) {
  struct ofp_match *match;
  VALUE options;

  if ( rb_scan_args( argc, argv, "01", &options ) >= 1 ) {
    if ( options != Qnil ) {
      Data_Get_Struct( self, struct ofp_match, match );
      memset( match, 0, sizeof ( *match ) );

      match->wildcards = ( OFPFW_ALL & ~( OFPFW_NW_SRC_MASK | OFPFW_NW_DST_MASK ) )
              | OFPFW_NW_SRC_ALL | OFPFW_NW_DST_ALL;
      VALUE in_port = rb_hash_aref( options, ID2SYM( rb_intern( "in_port" ) ) );
      if ( in_port != Qnil ) {
        match->in_port = ( uint16_t ) NUM2UINT( in_port );
        match->wildcards &= ( uint32_t ) ~OFPFW_IN_PORT;
      }

      VALUE dl_src = rb_hash_aref( options, ID2SYM( rb_intern( "dl_src" ) ) );
      if ( dl_src != Qnil ) {
        VALUE dl_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, dl_src );
        dl_addr_short( dl_addr, match->dl_src );
        match->wildcards &= ( uint32_t ) ~OFPFW_DL_SRC;
      }

      VALUE dl_dst = rb_hash_aref( options, ID2SYM( rb_intern( "dl_dst" ) ) );
      if ( dl_dst != Qnil ) {
        VALUE dl_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, dl_dst );
        dl_addr_short( dl_addr, match->dl_dst );
        match->wildcards &= ( uint32_t ) ~OFPFW_DL_DST;
      }

      VALUE dl_type = rb_hash_aref( options, ID2SYM( rb_intern( "dl_type" ) ) );
      if ( dl_type != Qnil ) {
        match->dl_type = ( uint16_t ) NUM2UINT( dl_type );
        match->wildcards &= ( uint32_t ) ~OFPFW_DL_TYPE;
      }

      VALUE dl_vlan = rb_hash_aref( options, ID2SYM( rb_intern( "dl_vlan" ) ) );
      if ( dl_vlan != Qnil ) {
        match->dl_vlan = ( uint16_t ) NUM2UINT( dl_vlan );
        match->wildcards &= ( uint32_t ) ~OFPFW_DL_VLAN;
      }

      VALUE dl_vlan_pcp = rb_hash_aref( options, ID2SYM( rb_intern( "dl_vlan_pcp" ) ) );
      if ( dl_vlan_pcp != Qnil ) {
        match->dl_vlan_pcp = ( uint8_t ) NUM2UINT( dl_vlan_pcp );
        match->wildcards &= ( uint32_t ) ~OFPFW_DL_VLAN_PCP;
      }

      VALUE nw_tos = rb_hash_aref( options, ID2SYM( rb_intern( "nw_tos" ) ) );
      if ( nw_tos != Qnil ) {
        match->nw_tos = ( uint8_t ) NUM2UINT( nw_tos );
        match->wildcards &= ( uint32_t ) ~OFPFW_NW_TOS;
      }

      VALUE nw_proto = rb_hash_aref( options, ID2SYM( rb_intern( "nw_proto" ) ) );
      if ( nw_proto != Qnil ) {
        match->nw_proto = ( uint8_t ) NUM2UINT( nw_proto );
        match->wildcards &= ( uint32_t ) ~OFPFW_NW_PROTO;
      }

      VALUE nw_src = rb_hash_aref( options, ID2SYM( rb_intern( "nw_src" ) ) );
      if ( nw_src != Qnil ) {
        VALUE nw_addr = rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, nw_src );
        match->nw_src = nw_addr_to_i( nw_addr );
        match->wildcards &= ( uint32_t ) ~OFPFW_NW_SRC_MASK;
      }

      VALUE nw_dst = rb_hash_aref( options, ID2SYM( rb_intern( "nw_dst" ) ) );
      if ( nw_dst != Qnil ) {
        VALUE nw_addr = rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, nw_dst );
        match->nw_dst = nw_addr_to_i( nw_addr );
        match->wildcards &= ( uint32_t ) ~OFPFW_NW_DST_MASK;
      }

      VALUE tp_src = rb_hash_aref( options, ID2SYM( rb_intern( "tp_src" ) ) );
      if ( tp_src != Qnil ) {
        match->tp_src = ( uint16_t ) NUM2UINT( tp_src );
        match->wildcards &= ( uint32_t ) ~OFPFW_TP_SRC;
      }

      VALUE tp_dst = rb_hash_aref( options, ID2SYM( rb_intern( "tp_dst" ) ) );
      if ( tp_dst != Qnil ) {
        match->tp_dst = ( uint16_t ) NUM2UINT( tp_dst );
        match->wildcards &= ( uint32_t ) ~OFPFW_TP_DST;
      }
    }
  }
  return self;
}


void
Init_match() {
  cMatch = rb_define_class_under( mTrema, "Match", rb_cObject );
  rb_define_alloc_func( cMatch, match_alloc );
  rb_define_method( cMatch, "initialize", match_init, -1 );
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
  rb_define_singleton_method( cMatch, "from", match_from, -1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
