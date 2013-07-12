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


/*
 * Creates a {Match} instance from packet_in's data, the method accepts an
 * additional list of symbols to wildcard set and ignore while matching flow entries.
 *
 * @overload match_from(message, *options)
 *
 *   @example
 *     def packet_in datapath_id, message
 *       send_flow_mod(
 *         datapath_id,
 *         :match => Match.from( message, :dl_type, :nw_proto ),
 *         :actions => Trema::ActionOutput.new( 2 )
 *       )
 *     end
 *
 *   @param [PacketIn] message
 *     the {PacketIn}'s message content.
 *
 *   @param [optional, list] options
 *     If supplied a comma-separated list of symbol ids indicating fields to be wildcarded.
 *
 *     [:in_port]
 *       the physical port number to wildcard.
 *
 *     [:dl_src]
 *       the source Ethernet address to wildcard.
 *
 *     [:dl_dst]
 *       the destination Ethernet address to wildcard.
 *
 *     [:dl_vlan]
 *       the IEEE 802.1q virtual VLAN tag to wildcard.
 *
 *     [:dl_vlan_pcp]
 *       the IEEE 802.1q priority code point to wildcard.
 *
 *     [:dl_type]
 *       the Ethernet protocol type to wildcard.
 *
 *     [:nw_tos]
 *       the IP ToS /DSCP field to wildcard.
 *
 *     [:nw_proto]
 *       the IP protocol type to wildcard.
 *
 *     [:nw_src]
 *       the IPv4 source address to wildcard.
 *
 *     [:nw_dst]
 *       the IPv4 destination address to wildcard.
 *
 *     [:tp_src]
 *       the source TCP/UDP port number to wildcard.
 *
 *     [:tp_dst]
 *       the destination TCP/UDP port number to wildcard.
 *
 * @return [Match] self
 *   the modified or exact match from packet depending on whether the options
 *   argument supplied or not.
 */
static VALUE
match_from( int argc, VALUE *argv, VALUE self ) {
  VALUE message, obj, wildcard_id, options;
  struct ofp_match *match = NULL;
  packet_in *packet = NULL;
  uint32_t wildcards = 0;

  if ( rb_scan_args( argc, argv, "1*", &message, &options ) >= 1 ) {
    obj = rb_funcall( self, rb_intern( "new" ), 0 );
    match = get_match( obj );
    Data_Get_Struct( message, packet_in, packet );
    int i;
    for ( i = 0; i < RARRAY_LEN( options ); i++ ) {
      wildcard_id = SYM2ID( RARRAY_PTR( options )[ i ] );
      if ( rb_intern( "in_port" ) == wildcard_id ) {
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
    set_match_from_packet( match, packet->in_port, wildcards, packet->data );
  }
  else {
    rb_raise( rb_eArgError, "Message is a mandatory option" );
  }
  return obj;
}


/*
 * Compare context of {Match} self with {Match} other.
 *
 * @example
 *   def packet_in datapath_id, message
 *     match = Match.new( :dl_type => 0x0800, :nw_src => "192.168.0.1" )
 *     if match.compare( ExactMatch.from( message ) )
 *       info "Received packet from 192.168.0.1"
 *     end
 *   end
 *
 * @return [Boolean] true if the {Match} match
 */
static VALUE
match_compare( VALUE self, VALUE other ) {
  return compare_match( get_match( self ), get_match( other ) ) ? Qtrue : Qfalse;
}


/*
 * Replaces context of {Match} self with {Match} other.
 *
 * @return [Match] self
 *   the modified object instance.
 */
static VALUE
match_replace( VALUE self, VALUE other ) {
  memcpy( get_match( self ), get_match( other ), sizeof( struct ofp_match ) );
  return self;
}


/*
 * (see ActionEnqueue#to_s)
 */
static VALUE
match_to_s( VALUE self ) {
  char match_str[ 1024 ];

  match_to_string( get_match( self ), match_str, sizeof( match_str ) );
  return rb_str_new2( match_str );
}


/*
 * The wildcard field expressed as a 32-bit bitmap,
 *
 * @return [Number] the value of wildcards.
 */
static VALUE
match_wildcards( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->wildcards );
}


/*
 * @return [Number] the value of in_port.
 */
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
  }
  else {
    dl_addr = match->dl_dst;
  }
  return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( dl_addr ) ) );
}


/*
 * @return [Mac] the value of dl_src.
 */
static VALUE
match_dl_src( VALUE self ) {
  return match_dl( self, 1 );
}


/*
 * @return [Mac] the value of dl_dst.
 */
static VALUE
match_dl_dst( VALUE self ) {
  return match_dl( self, 0 );
}


/*
 * @return [Number] the value of dl_vlan.
 */
static VALUE
match_dl_vlan( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->dl_vlan );
}


/*
 * @return [Number] the value of dl_vlan_pcp.
 */
static VALUE
match_dl_vlan_pcp( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->dl_vlan_pcp );
}


/*
 * @return [Number] the value of dl_type.
 */
static VALUE
match_dl_type( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->dl_type );
}


/*
 * @return [Number] the value of nw_tos.
 */
static VALUE
match_nw_tos( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->nw_tos );
}


/*
 * @return [Number] the value of nw_proto.
 */
static VALUE
match_nw_proto( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->nw_proto );
}


static VALUE
match_nw( VALUE self, uint8_t which ) {
  struct ofp_match *match;
  uint32_t nw_addr;
  uint32_t masklen;

  match = get_match( self );
  if ( which ) {
    nw_addr = match->nw_src;
    masklen = ( match->wildcards & OFPFW_NW_SRC_MASK ) >> OFPFW_NW_SRC_SHIFT;
  }
  else {
    nw_addr = match->nw_dst;
    masklen = ( match->wildcards & OFPFW_NW_DST_MASK ) >> OFPFW_NW_DST_SHIFT;
  }
  uint32_t prefixlen = masklen > 32 ? 0 : 32 - masklen;
  return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 2, UINT2NUM( nw_addr ), UINT2NUM( prefixlen ) );
}


/*
 * An IPv4 source address in its numeric representation.
 *
 * @return [IP] the value of nw_src.
 */
static VALUE
match_nw_src( VALUE self ) {
  return match_nw( self, 1 );
}


/*
 * An IPv4 destination address in its numeric representation.
 *
 * @return [IP] the value of nw_dst.
 */
static VALUE
match_nw_dst( VALUE self ) {
  return match_nw( self, 0 );
}


/*
 * @return [Number] the value of tp_src.
 */
static VALUE
match_tp_src( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->tp_src );
}


/*
 * @return [Number] the value of tp_dst.
 */
static VALUE
match_tp_dst( VALUE self ) {
  return UINT2NUM( ( get_match( self ) )->tp_dst );
}


/*
 * Creates a {Match} instance which describe fields such as MAC addresses, IP
 * addresses, TCP/UDP ports of a flow to match against. An exact match
 * flow would match on all fields whereas don't care bits are wildcarded and
 * ignored.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     Match.new(
 *       :in_port => port_no,
 *       :dl_src => "xx:xx:xx;xx:xx:xx",
 *       :dl_dst => "xx:xx:xx:xx:xx:xx",
 *       :dl_type => ethertype,
 *       :dl_vlan => vlan,
 *       :dl_vlan_pcp => priority,
 *       :nw_tos => tos,
 *       :nw_proto => proto,
 *       :nw_src => ip_address/netmask,
 *       :nw_dst => ip_address/netmask,
 *       :tp_src => port,
 *       :tp_dst => port,
 *     )
 *
 *   @param [Hash] options the options hash.
 *
 *   @option options [Number] :in_port
 *     the physical port number to match.
 *
 *   @option options [String,Number,Trema::Mac] :dl_src
 *     the source ethernet address to match specified either as 6
 *     pairs of hexadecimal digits delimited by colon or as a
 *     hexadecimal number or as a Trema::Mac object.
 *     (eg. "00:11:22:33:44:55" or 0x001122334455 or Mac.new("00:11:22:33:44:55")).
 *
 *   @option options [String,Number] :dl_dst
 *     the destination ethernet address to match specified either as a
 *     6 pairs of hexadecimal digits delimited by colon or as a
 *     hexadecimal number or as a Trema::Mac object.
 *     (eg. "00:11:22:33:44:55" or 0x001122334455 or Mac.new("00:11:22:33:44:55")).
 *
 *   @option options [Number] :dl_type
 *     the Ethernet protocol type to match. Can be specified either as a decimal
 *     or hexadecimal number. (eg 0x0800 to match IP packets, 0x08006 to match
 *     ARP packets, 0x88cc for LLDP packets).
 *
 *   @option options [Number] :dl_vlan
 *     the IEEE 802.1q virtual VLAN tag to match specified as a 12-bit number
 *     0 to 4095 inclusive.
 *
 *   @option options [Number] :dl_vlan_pcp
 *     the IEEE 802.1q Priority Code Point (PCP) to match specified as a value of
 *     0 to 7 inclusive. A higher value indicates a higher priority frame.
 *
 *   @option options [Number] :nw_tos
 *     the IP ToS/DSCP field to match specified as a decimal number between 0 and
 *     255 inclusive.
 *
 *   @option options [Number] :nw_proto
 *     Depending on the dl_type the IP protocol type to match. (eg if dl_type
 *     equals 0x0800 UDP packets can be match by setting nw_proto to 17.)
 *     to match TCP packets). When dl_type = 0x0806 is set to arp it matches the
 *     lower 8 bits of the ARP opcode.
 *
 *   @option options [String] :nw_src
 *     the IPv4 source address to match if dl_type is set to 0x0800.
 *
 *   @option options [String] :nw_dst
 *     the IPv4 destination address to match if dl_type is set to 0x0800.
 *
 *   @option options [Number] :tp_src
 *     the source TCP/UDP port number to match specified as a decimal number
 *     between 0 and 65535 inclusive. The value dl_type and nw_proto must be set
 *     to specify TCP or UDP.
 *
 *   @option options [Number] :tp_dst
 *     the destination TCP/UDP port number to match specified as a decimal number
 *     between 0 and 65535 inclusive.
 *
 *   @return [Match] self an object that encapsulates and wraps the +struct ofp_match+
 */
static VALUE
match_init( int argc, VALUE *argv, VALUE self ) {
  struct ofp_match *match;
  VALUE options;

  // Always clear the memory as the unused memory locations are
  // exposed to both the user and the OpenFlow controller.
  Data_Get_Struct( self, struct ofp_match, match );
  memset( match, 0, sizeof( *match ) );

  // Default matches all packets.
  match->wildcards = ( OFPFW_ALL & ~( OFPFW_NW_SRC_MASK | OFPFW_NW_DST_MASK ) )
    | OFPFW_NW_SRC_ALL | OFPFW_NW_DST_ALL;

  if ( rb_scan_args( argc, argv, "01", &options ) >= 1 ) {
    if ( options != Qnil ) {
      VALUE in_port = rb_hash_aref( options, ID2SYM( rb_intern( "in_port" ) ) );
      if ( in_port != Qnil ) {
        match->in_port = ( uint16_t ) NUM2UINT( in_port );
        match->wildcards &= ( uint32_t ) ~OFPFW_IN_PORT;
      }

      VALUE dl_src = rb_hash_aref( options, ID2SYM( rb_intern( "dl_src" ) ) );
      if ( dl_src != Qnil ) {
        VALUE dl_addr;
        if ( rb_obj_is_kind_of( dl_src, rb_eval_string( "Trema::Mac" ) ) ) {
          dl_addr = dl_src;
        }
        else {
          dl_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, dl_src );
        }
        dl_addr_to_a( dl_addr, match->dl_src );
        match->wildcards &= ( uint32_t ) ~OFPFW_DL_SRC;
      }

      VALUE dl_dst = rb_hash_aref( options, ID2SYM( rb_intern( "dl_dst" ) ) );
      if ( dl_dst != Qnil ) {
        VALUE dl_addr;
        if ( rb_obj_is_kind_of( dl_dst, rb_eval_string( "Trema::Mac" ) ) ) {
          dl_addr = dl_dst;
        }
        else {
          dl_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, dl_dst );
        }
        dl_addr_to_a( dl_addr, match->dl_dst );
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
        uint32_t prefixlen = ( uint32_t ) NUM2UINT( rb_funcall( nw_addr, rb_intern( "prefixlen" ), 0 ) );
        if ( prefixlen > 0 ) {
          match->nw_src = nw_addr_to_i( nw_addr );
          match->wildcards &= ( uint32_t ) ~OFPFW_NW_SRC_MASK;
          match->wildcards |= ( uint32_t ) ( ( 32 - prefixlen ) << OFPFW_NW_SRC_SHIFT );
        }
      }

      VALUE nw_dst = rb_hash_aref( options, ID2SYM( rb_intern( "nw_dst" ) ) );
      if ( nw_dst != Qnil ) {
        VALUE nw_addr = rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, nw_dst );
        uint32_t prefixlen = ( uint32_t ) NUM2UINT( rb_funcall( nw_addr, rb_intern( "prefixlen" ), 0 ) );
        if ( prefixlen > 0 ) {
          match->nw_dst = nw_addr_to_i( nw_addr );
          match->wildcards &= ( uint32_t ) ~OFPFW_NW_DST_MASK;
          match->wildcards |= ( uint32_t ) ( ( 32 - prefixlen ) << OFPFW_NW_DST_SHIFT );
        }
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


/*
 * Document-class: Trema::Match
 */
void
Init_match() {
  mTrema = rb_eval_string( "Trema" );
  cMatch = rb_define_class_under( mTrema, "Match", rb_cObject );
  rb_define_alloc_func( cMatch, match_alloc );
  rb_define_const( cMatch, "OFPFW_IN_PORT", INT2NUM( OFPFW_IN_PORT ) );
  rb_define_const( cMatch, "OFPFW_DL_VLAN", INT2NUM( OFPFW_DL_VLAN ) );
  rb_define_const( cMatch, "OFPFW_DL_SRC", INT2NUM( OFPFW_DL_SRC ) );
  rb_define_const( cMatch, "OFPFW_DL_DST", INT2NUM( OFPFW_DL_DST ) );
  rb_define_const( cMatch, "OFPFW_DL_TYPE", INT2NUM( OFPFW_DL_TYPE ) );
  rb_define_const( cMatch, "OFPFW_NW_PROTO", INT2NUM( OFPFW_NW_PROTO ) );
  rb_define_const( cMatch, "OFPFW_TP_SRC", INT2NUM( OFPFW_TP_SRC ) );
  rb_define_const( cMatch, "OFPFW_TP_DST", INT2NUM( OFPFW_TP_DST ) );
  rb_define_const( cMatch, "OFPFW_NW_SRC_SHIFT", INT2NUM( OFPFW_NW_SRC_SHIFT ) );
  rb_define_const( cMatch, "OFPFW_NW_SRC_BITS", INT2NUM( OFPFW_NW_SRC_BITS ) );
  rb_define_const( cMatch, "OFPFW_NW_SRC_MASK", INT2NUM( OFPFW_NW_SRC_MASK ) );
  rb_define_const( cMatch, "OFPFW_NW_SRC_ALL", INT2NUM( OFPFW_NW_SRC_ALL ) );
  rb_define_const( cMatch, "OFPFW_NW_DST_SHIFT", INT2NUM( OFPFW_NW_DST_SHIFT ) );
  rb_define_const( cMatch, "OFPFW_NW_DST_BITS", INT2NUM( OFPFW_NW_DST_BITS ) );
  rb_define_const( cMatch, "OFPFW_NW_DST_MASK", INT2NUM( OFPFW_NW_DST_MASK ) );
  rb_define_const( cMatch, "OFPFW_NW_DST_ALL", INT2NUM( OFPFW_NW_DST_ALL ) );
  rb_define_const( cMatch, "OFPFW_DL_VLAN_PCP", INT2NUM( OFPFW_DL_VLAN_PCP ) );
  rb_define_const( cMatch, "OFPFW_NW_TOS", INT2NUM( OFPFW_NW_TOS ) );
  rb_define_const( cMatch, "OFPFW_ALL", INT2NUM( OFPFW_ALL ) );
  rb_define_method( cMatch, "initialize", match_init, -1 );
  rb_define_method( cMatch, "compare", match_compare, 1 );
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
