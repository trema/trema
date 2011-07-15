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


#include "trema.h"
#include "ruby.h"

extern VALUE mTrema;
VALUE cFlowRemoved;

static VALUE
flow_removed_init( VALUE self, VALUE attribute ) {
  rb_iv_set( self, "@attribute", attribute );
  return self;
}

static VALUE
flow_removed_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}

static VALUE
flow_removed_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}

static VALUE
flow_removed_match( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "match" ) ) );
}

static VALUE
flow_removed_cookie( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "cookie" ) ) );
}

static VALUE
flow_removed_priority( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "priority" ) ) );
}

static VALUE
flow_removed_reason( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "reason" ) ) );
}

static VALUE
flow_removed_duration_sec( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "duration_sec" ) ) );
}

static VALUE
flow_removed_duration_nsec( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "duration_nsec" ) ) );
}

static VALUE
flow_removed_idle_timeout( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "idle_timeout" ) ) );
}

static VALUE
flow_removed_packet_count( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "packet_count" ) ) );
}

static VALUE
flow_removed_byte_count( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "byte_count" ) ) );
}

void
Init_flow_removed( ) {
  cFlowRemoved = rb_define_class_under( mTrema, "FlowRemoved", rb_cObject );
  rb_define_method( cFlowRemoved, "initialize", flow_removed_init, 1 );
  rb_define_method( cFlowRemoved, "datapath_id", flow_removed_datapath_id, 0 );
  rb_define_method( cFlowRemoved, "transaction_id", flow_removed_transaction_id, 0 );
  rb_define_method( cFlowRemoved, "match", flow_removed_match, 0 );
  rb_define_method( cFlowRemoved, "cookie", flow_removed_cookie, 0 );
  rb_define_method( cFlowRemoved, "priority", flow_removed_priority, 0 );
  rb_define_method( cFlowRemoved, "reason", flow_removed_reason, 0 );
  rb_define_method( cFlowRemoved, "duration_sec", flow_removed_duration_sec, 0 );
  rb_define_method( cFlowRemoved, "duration_nsec", flow_removed_duration_nsec, 0 );
  rb_define_method( cFlowRemoved, "idle_timeout", flow_removed_idle_timeout, 0 );
  rb_define_method( cFlowRemoved, "packet_count", flow_removed_packet_count, 0 );
  rb_define_method( cFlowRemoved, "byte_count", flow_removed_byte_count, 0 );
}

void
handle_flow_removed(
        uint64_t datapath_id,
        uint32_t transaction_id,
        struct ofp_match match,
        uint64_t cookie,
        uint16_t priority,
        uint8_t reason,
        uint32_t duration_sec,
        uint32_t duration_nsec,
        uint16_t idle_timeout,
        uint64_t packet_count,
        uint64_t byte_count,
        void *controller
        ) {
  VALUE attributes = rb_hash_new( );
  VALUE match_obj;

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );

  match_obj = rb_eval_string( "Match.new" );
  rb_funcall( match_obj, rb_intern( "replace" ), 1, Data_Wrap_Struct( cFlowRemoved, NULL, NULL, &match ) );

  rb_hash_aset( attributes, ID2SYM( rb_intern( "match" ) ), match_obj );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "cookie" ) ), ULL2NUM( cookie ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "priority" ) ), UINT2NUM( priority ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "reason" ) ), UINT2NUM( reason ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "duration_sec" ) ), UINT2NUM( duration_sec ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "duration_nsec" ) ), UINT2NUM( duration_nsec ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "idle_timeout" ) ), UINT2NUM( idle_timeout ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "packet_count" ) ), ULL2NUM( packet_count ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "byte_count" ) ), ULL2NUM( byte_count ) );

  VALUE flow_removed = rb_funcall( cFlowRemoved, rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "flow_removed" ), 1, flow_removed );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

