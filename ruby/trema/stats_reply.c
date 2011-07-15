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
VALUE cStatsReply;
VALUE cQueueStatsReply;
VALUE cVendorStatsReply;

static VALUE
stats_reply_init( VALUE self, VALUE attribute ) {
  rb_iv_set( self, "@attribute", attribute );
  return self;
}

static VALUE
stats_reply_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}

static VALUE
stats_reply_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}

static VALUE
stats_reply_type( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "type" ) ) );
}

static VALUE
stats_reply_flags( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "flags" ) ) );
}

static VALUE
stats_reply_stats( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "stats" ) ) );
}

void
Init_stats_reply( ) {
  rb_require( "trema/flow-stats-reply" );
  rb_require( "trema/aggregate-stats-reply" );
  rb_require( "trema/table-stats-reply" );
  rb_require( "trema/port-stats-reply" );
  rb_require( "trema/queue-stats-reply" );
  cStatsReply = rb_define_class_under( mTrema, "StatsReply", rb_cObject );
  cVendorStatsReply = rb_define_class_under( mTrema, "VendorStatsReply", cStatsReply );
  rb_define_method( cStatsReply, "initialize", stats_reply_init, 1 );
  rb_define_method( cStatsReply, "datapath_id", stats_reply_datapath_id, 0 );
  rb_define_method( cStatsReply, "transaction_id", stats_reply_transaction_id, 0 );
  rb_define_method( cStatsReply, "type", stats_reply_type, 0 );
  rb_define_method( cStatsReply, "flags", stats_reply_flags, 0 );
  rb_define_method( cStatsReply, "stats", stats_reply_stats, 0 );
}

void
handle_stats_reply(
        uint64_t datapath_id,
        uint32_t transaction_id,
        uint16_t type,
        uint16_t flags,
        const buffer *body,
        void *controller
        ) {
  if ( body == NULL ) {
    return;
  }
  if ( !body->length ) {
    return;
  }
  VALUE attributes = rb_hash_new( );
  uint16_t body_length;

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "type" ) ), UINT2NUM( type ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "flags" ) ), UINT2NUM( flags ) );

  body_length = ( uint16_t ) body->length;
  switch ( type ) {
    case OFPST_FLOW:
    {
      struct ofp_flow_stats *flow_stats;
      VALUE flow_stats_arr = rb_ary_new( );
      VALUE flow_stats_reply;
      VALUE match_obj;
      VALUE options = rb_hash_new( );

      flow_stats = ( struct ofp_flow_stats * ) body->data;

      while ( body_length > 0 ) {

        match_obj = rb_funcall( rb_eval_string( "Match.new" ), rb_intern( "replace" ), 1, Data_Wrap_Struct( cStatsReply, NULL, NULL, &flow_stats->match ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "length" ) ), UINT2NUM( flow_stats->length ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "table_id" ) ), UINT2NUM( flow_stats->table_id ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "match" ) ), match_obj );
        rb_hash_aset( options, ID2SYM( rb_intern( "duration_sec" ) ), UINT2NUM( flow_stats->duration_sec ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "duration_nsec" ) ), UINT2NUM( flow_stats->duration_nsec ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "priority" ) ), UINT2NUM( flow_stats->priority ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "idle_timeout" ) ), UINT2NUM( flow_stats->idle_timeout ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "hard_timeout" ) ), UINT2NUM( flow_stats->hard_timeout ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "cookie" ) ), ULL2NUM( flow_stats->cookie ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "packet_count" ) ), ULL2NUM( flow_stats->packet_count ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "byte_count" ) ), ULL2NUM( flow_stats->byte_count ) );
        flow_stats_reply = rb_funcall( rb_eval_string( "Trema::FlowStatsReply" ), rb_intern( "new" ), 1, options );
        rb_ary_push( flow_stats_arr, flow_stats_reply );

        // here create flow_stats object and insert into array 
        body_length = ( uint16_t ) ( body_length - flow_stats->length );

        if ( body_length ) {
          // move pointer to next entry
          flow_stats = ( struct ofp_flow_stats * ) ( ( char * ) flow_stats + flow_stats->length );
        }
      }
      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), flow_stats_arr );
    }
      break;
    case OFPST_AGGREGATE:
    {
      struct ofp_aggregate_stats_reply *aggregate_stats = ( struct ofp_aggregate_stats_reply * ) body->data;
      VALUE options = rb_hash_new( );
      VALUE aggregate_stats_reply;
      VALUE aggregate_stats_arr = rb_ary_new( );

      rb_hash_aset( options, ID2SYM( rb_intern( "packet_count" ) ), ULL2NUM( aggregate_stats->packet_count ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "byte_count" ) ), ULL2NUM( aggregate_stats->byte_count ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "flow_count" ) ), UINT2NUM( aggregate_stats->flow_count ) );
      aggregate_stats_reply = rb_funcall( rb_eval_string( " Trema::AggregateStatsReply" ), rb_intern( "new" ), 1, options );
      rb_ary_push( aggregate_stats_arr, aggregate_stats_reply );
      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), aggregate_stats_arr );
    }
      break;
    case OFPST_TABLE:
    {
      struct ofp_table_stats *table_stats;
      VALUE table_stats_arr = rb_ary_new( );
      VALUE options = rb_hash_new( );
      VALUE table_stats_reply;

      table_stats = ( struct ofp_table_stats * ) body->data;
      while ( body_length > 0 ) {

        rb_hash_aset( options, ID2SYM( rb_intern( "table_id" ) ), UINT2NUM( table_stats->table_id ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "name" ) ), rb_str_new2( table_stats->name ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "wildcards" ) ), UINT2NUM( table_stats->wildcards ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "max_entries" ) ), UINT2NUM( table_stats->max_entries ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "active_count" ) ), UINT2NUM( table_stats->active_count ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "lookup_count" ) ), ULL2NUM( table_stats->active_count ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "matched_count" ) ), ULL2NUM( table_stats->matched_count ) );

        table_stats_reply = rb_funcall( rb_eval_string( "Trema::TableStatsReply" ), rb_intern( "new" ), 1, options );

        rb_ary_push( table_stats_arr, table_stats_reply );
        body_length = ( uint16_t ) ( body_length - sizeof ( struct ofp_table_stats ) );
        if ( body_length ) {
          table_stats++;
        }
      }
      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), table_stats_arr );
    }
      break;
    case OFPST_PORT:
    {
      struct ofp_port_stats *port_stats;
      VALUE port_stats_arr = rb_ary_new( );
      VALUE options = rb_hash_new( );
      VALUE port_stats_reply;

      port_stats = ( struct ofp_port_stats * ) body->data;
      while ( body_length > 0 ) {
        rb_hash_aset( options, ID2SYM( rb_intern( "port_no" ) ), UINT2NUM( port_stats->port_no ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_packets" ) ), ULL2NUM( port_stats->rx_packets ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "tx_packets" ) ), ULL2NUM( port_stats->tx_packets ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_bytes" ) ), ULL2NUM( port_stats->rx_bytes ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "tx_bytes" ) ), ULL2NUM( port_stats->tx_bytes ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_dropped" ) ), ULL2NUM( port_stats->rx_dropped ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "tx_dropped" ) ), ULL2NUM( port_stats->tx_dropped ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_errors" ) ), ULL2NUM( port_stats->rx_errors ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "tx_errors" ) ), ULL2NUM( port_stats->tx_errors ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_frame_err" ) ), ULL2NUM( port_stats->rx_frame_err ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_over_err" ) ), ULL2NUM( port_stats->rx_over_err ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "rx_crc_err" ) ), ULL2NUM( port_stats->rx_crc_err ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "collisions" ) ), ULL2NUM( port_stats->collisions ) );

        port_stats_reply = rb_funcall( rb_eval_string( "Trema::PortStatsReply" ), rb_intern( "new" ), 1, options );

        rb_ary_push( port_stats_arr, port_stats_reply );
        body_length = ( uint16_t ) ( body_length - sizeof ( struct ofp_port_stats ) );
        if ( body_length ) {
          port_stats++;
        }
      }
      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), port_stats_arr );
    }
      break;
    case OFPST_QUEUE:
    {
      struct ofp_queue_stats *queue_stats;
      VALUE queue_stats_arr = rb_ary_new( );
      VALUE options = rb_hash_new( );
      VALUE queue_stats_reply;

      queue_stats = ( struct ofp_queue_stats * ) body->data;
      while ( body_length > 0 ) {
        rb_hash_aset( options, ID2SYM( rb_intern( "port_no" ) ), UINT2NUM( queue_stats->port_no ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "queue_id" ) ), UINT2NUM( queue_stats->queue_id ) );

        queue_stats_reply = rb_funcall( rb_eval_string( "Trema::QueueStatsReply" ), rb_intern( "new" ), 1, options );

        rb_ary_push( queue_stats_arr, queue_stats_reply );
        body_length = ( uint16_t ) ( body_length - sizeof ( struct ofp_queue_stats ) );
        if ( body_length ) {
          queue_stats++;
        }
      }
      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), queue_stats_arr );
    }
      break;
    case OFPST_VENDOR:
    {
      uint32_t *vendor_id;
      VALUE options = rb_hash_new( );
      VALUE vendor_stats_arr = rb_ary_new2( 1 );
      VALUE vendor_stats_reply;

      vendor_id = ( uint32_t * ) body->data;
      rb_hash_aset( options, ID2SYM( rb_intern( "vendor_id" ) ), UINT2NUM( *vendor_id ) );
      vendor_stats_reply = rb_funcall( rb_eval_string( "Trema::VendorStatsReply" ), rb_intern( "new" ), 1, options );
      rb_ary_push( vendor_stats_arr, vendor_stats_reply );

      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), vendor_stats_arr );
    }
      break;
    default:
      critical( "Unhandled stats type ( type = %u ),", type );
      break;
  }

  VALUE r_stats_reply = rb_funcall( cStatsReply, rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "stats_reply" ), 1, r_stats_reply );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

