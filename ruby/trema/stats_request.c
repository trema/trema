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
VALUE cStatsRequest;
VALUE cFlowStatsRequest;
VALUE cAggregateStatsRequest;
VALUE cTableStatsRequest;
VALUE cPortStatsRequest;
VALUE cQueueStatsRequest;
VALUE cVendorStatsRequest;

static VALUE
stats_request_init( VALUE self, VALUE options ) {
  VALUE transaction_id;
  VALUE flags;

  transaction_id = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
  if ( transaction_id == Qnil ) {
    transaction_id = UINT2NUM( get_transaction_id( ) );
  }
  rb_iv_set( self, "@transaction_id", transaction_id );

  flags = rb_hash_aref( options, ID2SYM( rb_intern( "flags" ) ) );
  if ( flags == Qnil ) {
    flags = UINT2NUM( 0 );
  }
  rb_iv_set( self, "@flags", flags );
  return self;
}

static VALUE
subclass_stats_request_init( VALUE self, VALUE options ) {
  VALUE match;
  VALUE table_id;
  VALUE out_port;

  rb_call_super( 1, &options );
  match = rb_hash_aref( options, ID2SYM( rb_intern( "match" ) ) );
  if ( match == Qnil ) {
    rb_raise( rb_eArgError, "match option must be specified" );
  }
  rb_iv_set( self, "@match", match );
  table_id = rb_hash_aref( options, ID2SYM( rb_intern( "table_id" ) ) );
  if ( table_id == Qnil ) {
    table_id = UINT2NUM( 0xff );
  }
  rb_iv_set( self, "@table_id", table_id );
  out_port = rb_hash_aref( options, ID2SYM( rb_intern( "out_port" ) ) );
  if ( out_port == Qnil ) {
    out_port = UINT2NUM( OFPP_NONE );
  }
  rb_iv_set( self, "@out_port", out_port );
  return self;
}

uint16_t
get_stats_request_num2uint( VALUE self, const char *field ) {
  return ( uint16_t ) NUM2UINT( rb_iv_get( self, field ) );
}

uint8_t
get_stats_request_table_id( VALUE self ) {
  return ( uint8_t ) NUM2UINT( rb_iv_get( self, "@table_id" ) );
}

struct ofp_match
get_stats_request_match( VALUE self ) {
  struct ofp_match *match;

  Data_Get_Struct( rb_iv_get( self, "@match" ), struct ofp_match, match );
  return *match;
}

void
stats_request_buffer_set( VALUE self, buffer *stats_request_buffer ) {
  rb_iv_set( self,
          "@buffer",
          Data_Wrap_Struct( rb_eval_string( "Trema::StatsRequest" ), NULL, free_buffer, stats_request_buffer ) );
}

static VALUE
flow_stats_request_to_packet( VALUE self ) {
  buffer *flow_stats_request;

  flow_stats_request = create_flow_stats_request(
          get_stats_request_num2uint( self, "@transaction_id" ),
          get_stats_request_num2uint( self, "@flags" ),
          get_stats_request_match( self ),
          get_stats_request_table_id( self ),
          get_stats_request_num2uint( self, "@out_port" ) );

  stats_request_buffer_set( self, flow_stats_request );
  return self;
}

static VALUE
aggregate_stats_request_to_packet( VALUE self ) {
  buffer *aggregate_stats_request;

  aggregate_stats_request = create_aggregate_stats_request(
          get_stats_request_num2uint( self, "@transaction_id" ),
          get_stats_request_num2uint( self, "@flags" ),
          get_stats_request_match( self ),
          get_stats_request_table_id( self ),
          get_stats_request_num2uint( self, "out_port" ) );

  stats_request_buffer_set( self, aggregate_stats_request );
  return self;
}

static VALUE
stats_request_buffer( VALUE self ) {
  return rb_iv_get( self, "@buffer" );
}

static VALUE
table_stats_request_init( VALUE self, VALUE options ) {
  UNUSED( self );
  return rb_call_super( 1, &options );
}

static VALUE
table_stats_request_to_packet( VALUE self ) {
  buffer *table_stats_request;

  table_stats_request = create_table_stats_request(
          get_stats_request_num2uint( self, "@transaction_id" ),
          get_stats_request_num2uint( self, "@flags" ) );

  stats_request_buffer_set( self, table_stats_request );
  return self;
}

static VALUE
port_stats_request_init( VALUE self, VALUE options ) {
  VALUE port_no;

  rb_call_super( 1, &options );
  port_no = rb_hash_aref( options, ID2SYM( rb_intern( "port_no" ) ) );
  if ( port_no == Qnil ) {
    port_no = UINT2NUM( OFPP_NONE );
  }
  rb_iv_set( self, "@port_no", port_no );
  return self;
}

static VALUE
port_stats_request_to_packet( VALUE self ) {
  buffer *port_stats_request;

  port_stats_request = create_port_stats_request(
          get_stats_request_num2uint( self, "@transaction_id" ),
          get_stats_request_num2uint( self, "@flags" ),
          get_stats_request_num2uint( self, "@port_no" ) );

  stats_request_buffer_set( self, port_stats_request );
  return self;
}

static VALUE
queue_stats_request_init( VALUE self, VALUE options ) {
  VALUE port_no, queue_id;

  rb_call_super( 1, &options );
  port_no = rb_hash_aref( options, ID2SYM( rb_intern( "port_no" ) ) );
  if ( port_no == Qnil ) {
    port_no = UINT2NUM( OFPP_ALL );
  }
  rb_iv_set( self, "@port_no", port_no );
  queue_id = rb_hash_aref( options, ID2SYM( rb_intern( "queue_id" ) ) );
  if ( queue_id == Qnil ) {
    queue_id = UINT2NUM( OFPQ_ALL );
  }
  rb_iv_set( self, "@queue_id", queue_id );
  return self;
}

static VALUE
queue_stats_request_to_packet( VALUE self ) {
  buffer *queue_stats_request;

  queue_stats_request = create_queue_stats_request(
          get_stats_request_num2uint( self, "@transaction_id" ),
          get_stats_request_num2uint( self, "@flags" ),
          get_stats_request_num2uint( self, "@port_no" ),
          get_stats_request_num2uint( self, "@queue_id" ) );

  stats_request_buffer_set( self, queue_stats_request );
  return self;
}

static VALUE
vendor_stats_request_init( VALUE self, VALUE options ) {
  VALUE vendor_id;

  rb_call_super( 1, &options );
  vendor_id = rb_hash_aref( options, ID2SYM( rb_intern( "vendor_id" ) ) );
  if ( vendor_id == Qnil ) {
    vendor_id = UINT2NUM( 0x00004cff );
  }
  rb_iv_set( self, "@vendor_id", vendor_id );
  return self;
}

static VALUE
vendor_stats_request_to_packet( VALUE self ) {
  buffer *vendor_stats_request;

  vendor_stats_request = create_vendor_stats_request(
          get_stats_request_num2uint( self, "@transaction_id" ),
          get_stats_request_num2uint( self, "@flags" ),
          get_stats_request_num2uint( self, "@vendor_id" ),
          NULL );

  stats_request_buffer_set( self, vendor_stats_request );
  return self;
}

void
Init_stats_request( ) {
  cStatsRequest = rb_define_class_under( mTrema, "StatsRequest", rb_cObject );
  cFlowStatsRequest = rb_define_class_under( mTrema, "FlowStatsRequest", cStatsRequest );
  rb_define_method( cStatsRequest, "initialize", stats_request_init, 1 );
  rb_define_method( cFlowStatsRequest, "initialize", subclass_stats_request_init, 1 );
  rb_define_method( cFlowStatsRequest, "to_packet", flow_stats_request_to_packet, 0 );

  cAggregateStatsRequest = rb_define_class_under( mTrema, "AggregateStatsRequest", cStatsRequest );
  rb_define_method( cAggregateStatsRequest, "initialize", subclass_stats_request_init, 1 );
  rb_define_method( cAggregateStatsRequest, "to_packet", aggregate_stats_request_to_packet, 0 );

  cTableStatsRequest = rb_define_class_under( mTrema, "TableStatsRequest", cStatsRequest );
  rb_define_method( cTableStatsRequest, "initialize", table_stats_request_init, 1 );
  rb_define_method( cTableStatsRequest, "to_packet", table_stats_request_to_packet, 0 );

  cPortStatsRequest = rb_define_class_under( mTrema, "PortStatsRequest", cStatsRequest );
  rb_define_method( cPortStatsRequest, "initialize", port_stats_request_init, 1 );
  rb_define_method( cPortStatsRequest, "to_packet", port_stats_request_to_packet, 0 );

  cQueueStatsRequest = rb_define_class_under( mTrema, "QueueStatsRequest", cStatsRequest );
  rb_define_method( cQueueStatsRequest, "initialize", queue_stats_request_init, 1 );
  rb_define_method( cQueueStatsRequest, "to_packet", queue_stats_request_to_packet, 0 );

  cVendorStatsRequest = rb_define_class_under( mTrema, "VendorStatsRequest", cStatsRequest );
  rb_define_method( cVendorStatsRequest, "initialize", vendor_stats_request_init, 1 );
  rb_define_method( cVendorStatsRequest, "to_packet", vendor_stats_request_to_packet, 0 );

  rb_define_method( cStatsRequest, "buffer", stats_request_buffer, 0 );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
