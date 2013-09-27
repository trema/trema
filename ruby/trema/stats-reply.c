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


#include <arpa/inet.h>
#include "trema.h"
#include "ruby.h"
#include "action-common.h"


extern VALUE mTrema;
VALUE cStatsReply;


/*
 * A {StatsReply} instance that encapsulates the header part of the
 * +OFPT_STATS_REPLY+ message. The body of the reply message may be an array
 * of one or more specific reply objects designated by the type.
 * The user would not instantiate stats. reply objects explicitly, the stats.
 * reply handler would normally do that while parsing the message.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     StatsReply.new(
 *       :datapath_id => 0xabc,
 *       :transaction_id => 123,
 *       :type => OFPST_FLOW
 *       :flags => 0,
 *       :stats => [FlowStatsReply]
 *     )
 *
 *   @param [Hash] options the options hash.
 *
 *   @option options [Number] :datapath_id
 *     message originator identifier.
 *
 *   @option options [Number] :transaction_id
 *     transaction_id value carried over from request.
 *
 *   @option options [Number] :type
 *     type id for the reply.
 *
 *   @option options [Number] :flags
 *     if set to 1 more replies would follow, 0 for the last reply.
 *
 *   @option options [Array] :stats
 *     an array of objects associated with the reply instance.
 *
 * @return [StatsReply]
 *   an object that encapsulates the +OFPT_STATS_REPLY+ openflow message.
 */
static VALUE
stats_reply_init( VALUE self, VALUE options ) {
  rb_iv_set( self, "@attribute", options );
  return self;
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
stats_reply_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
stats_reply_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


/*
 * The type of this reply.
 *
 * @return [Number] the value of type.
 */
static VALUE
stats_reply_type( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "type" ) ) );
}


/*
 * Flag that indicates if more reply message(s) expected to follow.
 *
 * @return [Number] the value of flags.
 */
static VALUE
stats_reply_flags( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "flags" ) ) );
}


/*
 * A list of reply type objects for this message.
 *
 * @return [Array<DescStatsReply>]
 *   an array of {DescStatsReply} objects if type is +OFPST_DESC+.
 * @return [Array<FlowStatsReply>]
 *   an array of {FlowStatsReply} objects if type is +OFPST_FLOW+.
 * @return [AggregateStatsReply]
 *   a {AggregateStatsReply} object if type is +OFPST_AGGREGATE+.
 * @return [Array<TableStatsReply>]
 *   an array of {TableStatsReply} objects if type is +OFPST_TABLE+.
 * @return [Array<PortStatsReply>]
 *   an array of {PortStatsReply} objects if type is +OFPST_PORT+.
 * @return [Array<QueueStatsReply>]
 *   an array of {QueueStatsReply} objects if type is +OFPST_QUEUE+.
 * @return [VendorStatsReply]
 *   a {VendorStatsReply} object if type is +OFPST_VENDOR+.
 */
static VALUE
stats_reply_stats( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "stats" ) ) );
}


/*
 * Document-class: Trema::StatsReply
 */
void
Init_stats_reply() {
  rb_require( "trema/desc-stats-reply" );
  rb_require( "trema/flow-stats-reply" );
  rb_require( "trema/aggregate-stats-reply" );
  rb_require( "trema/table-stats-reply" );
  rb_require( "trema/port-stats-reply" );
  rb_require( "trema/queue-stats-reply" );
  rb_require( "trema/vendor-stats-reply" );
  mTrema = rb_eval_string( "Trema" );
  cStatsReply = rb_define_class_under( mTrema, "StatsReply", rb_cObject );
  rb_define_const( cStatsReply, "OFPST_DESC", INT2NUM( OFPST_DESC ) );
  rb_define_const( cStatsReply, "OFPST_FLOW", INT2NUM( OFPST_FLOW ) );
  rb_define_const( cStatsReply, "OFPST_AGGREGATE", INT2NUM( OFPST_AGGREGATE ) );
  rb_define_const( cStatsReply, "OFPST_TABLE", INT2NUM( OFPST_TABLE ) );
  rb_define_const( cStatsReply, "OFPST_PORT", INT2NUM( OFPST_PORT ) );
  rb_define_const( cStatsReply, "OFPST_QUEUE", INT2NUM( OFPST_QUEUE ) );
  rb_define_const( cStatsReply, "OFPST_VENDOR", INT2NUM( OFPST_VENDOR ) );
  rb_define_method( cStatsReply, "initialize", stats_reply_init, 1 );
  rb_define_method( cStatsReply, "datapath_id", stats_reply_datapath_id, 0 );
  rb_define_method( cStatsReply, "transaction_id", stats_reply_transaction_id, 0 );
  rb_define_method( cStatsReply, "type", stats_reply_type, 0 );
  rb_define_method( cStatsReply, "flags", stats_reply_flags, 0 );
  rb_define_method( cStatsReply, "stats", stats_reply_stats, 0 );
}


static VALUE
get_action( const struct ofp_action_header *ah ) {
  VALUE action = Qnil;

  switch ( ah->type ) {
    case OFPAT_OUTPUT:
    {
      const struct ofp_action_output *ao = ( const struct ofp_action_output * ) ah;

      VALUE options = rb_hash_new();
      rb_hash_aset( options, ID2SYM( rb_intern( "port_number" ) ), UINT2NUM( ao->port ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "max_len" ) ), UINT2NUM( ao->max_len ) );
      action = rb_funcall( rb_eval_string( "Trema::SendOutPort" ), rb_intern( "new" ), 1, options );
    }
      break;
    case OFPAT_SET_VLAN_VID:
    {
      const struct ofp_action_vlan_vid *action_vlan_vid = ( const struct ofp_action_vlan_vid * ) ah;

      VALUE vlan_id = UINT2NUM( action_vlan_vid->vlan_vid );
      action = rb_funcall( rb_eval_string( "Trema::SetVlanVid" ), rb_intern( "new" ), 1, vlan_id );
    }
      break;
    case OFPAT_SET_VLAN_PCP:
    {
      const struct ofp_action_vlan_pcp *action_vlan_pcp = ( const struct ofp_action_vlan_pcp * ) ah;

      VALUE vlan_priority =  UINT2NUM( action_vlan_pcp->vlan_pcp );
      action = rb_funcall( rb_eval_string( "Trema::SetVlanPriority" ), rb_intern( "new" ), 1, vlan_priority );
    }
      break;
    case OFPAT_STRIP_VLAN:
    {
      action = rb_funcall( rb_eval_string( "Trema::StripVlanHeader" ), rb_intern( "new" ), 0 );
    }
      break;
    case OFPAT_SET_DL_SRC:
    case OFPAT_SET_DL_DST:
    {
      VALUE mac_address;
      const struct ofp_action_dl_addr *action_dl_addr = ( const struct ofp_action_dl_addr * ) ah;

      mac_address = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( action_dl_addr->dl_addr ) ) );
      if ( ah->type == OFPAT_SET_DL_SRC ) {
        action = rb_funcall( rb_eval_string( "Trema::SetEthSrcAddr" ), rb_intern( "new" ), 1, mac_address );
      }
      else {
        action = rb_funcall( rb_eval_string( "Trema::SetEthDstAddr" ), rb_intern( "new" ), 1, mac_address );
      }
    }
      break;
    case OFPAT_SET_NW_SRC:
    case OFPAT_SET_NW_DST:
    {
      const struct ofp_action_nw_addr *action_nw_addr = ( const struct ofp_action_nw_addr * ) ah;

      VALUE ip_address = rb_funcall( rb_eval_string( "Pio::IPv4Address " ), rb_intern( "new" ), 1, UINT2NUM( action_nw_addr->nw_addr ) );
      if ( ah->type == OFPAT_SET_NW_SRC ) {
        action = rb_funcall( rb_eval_string( "Trema::SetIpSrcAddr" ), rb_intern( "new" ), 1, rb_funcall( ip_address, rb_intern( "to_s" ), 0 ) );
      }
      else {
        action = rb_funcall( rb_eval_string( "Trema::SetIpDstAddr" ), rb_intern( "new" ), 1, rb_funcall( ip_address, rb_intern( "to_s" ), 0 ) );
      }
    }
      break;
    case OFPAT_SET_NW_TOS:
    {
      const struct ofp_action_nw_tos *action_nw_tos = ( const struct ofp_action_nw_tos * ) ah;

      VALUE type_of_service = ULL2NUM( action_nw_tos->nw_tos );
      action = rb_funcall( rb_eval_string( "Trema::SetIpTos" ), rb_intern( "new" ), 1, type_of_service );
    }
      break;
    case OFPAT_SET_TP_SRC:
    {
      const struct ofp_action_tp_port *action_tp_port = ( const struct ofp_action_tp_port * ) ah;

      VALUE port_number = ULL2NUM( action_tp_port->tp_port );
      action = rb_funcall( rb_eval_string( "Trema::SetTransportSrcPort" ), rb_intern( "new" ), 1, port_number );
    }
      break;
    case OFPAT_SET_TP_DST:
    {
      const struct ofp_action_tp_port *action_tp_port = ( const struct ofp_action_tp_port * ) ah;

      VALUE port_number = ULL2NUM( action_tp_port->tp_port );
      action = rb_funcall( rb_eval_string( "Trema::SetTransportDstPort" ), rb_intern( "new" ), 1, port_number );
    }
      break;
    case OFPAT_ENQUEUE:
    {
      const struct ofp_action_enqueue *action_enqueue = ( const struct ofp_action_enqueue * ) ah;
      VALUE options = rb_hash_new();
      rb_hash_aset( options, ID2SYM( rb_intern( "port_number" ) ), UINT2NUM( action_enqueue->port ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "queue_id" ) ), ULL2NUM( action_enqueue->queue_id ) );
      action = rb_funcall( rb_eval_string( "Trema::Enqueue" ), rb_intern( "new" ), 1, options );
    }
      break;
    case OFPAT_VENDOR:
    {
      const struct ofp_action_vendor_header *vendor_header = ( const struct ofp_action_vendor_header * ) ah;

      VALUE vendor_id = ULL2NUM( vendor_header->vendor );
      if ( vendor_header->len > ( uint16_t ) sizeof( *vendor_header ) ) {
        long length = ( long ) ( vendor_header->len - sizeof( *vendor_header ) );
        VALUE data_array = rb_ary_new2( length );
        const uint8_t *data = ( const uint8_t * ) ( ( const char * ) vendor_header + sizeof( *vendor_header ) );
        long i;
        for ( i = 0; i < length; i++ ) {
          rb_ary_push( data_array, UINT2NUM( data[ i ] ) );
        }
        action = rb_funcall( rb_eval_string( "Trema::VendorAction" ), rb_intern( "new" ), 2, vendor_id, data_array );
      }
      else {
        action = rb_funcall( rb_eval_string( "Trema::VendorAction" ), rb_intern( "new" ), 1, vendor_id );
      }
    }
      break;
  }
  return action;
}


void
handle_stats_reply(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t type,
  uint16_t flags,
  const buffer *body,
  void *user_data
) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "stats_reply" ) ) == Qfalse ) {
    return;
  }
  if ( body == NULL ) {
    return;
  }
  if ( !body->length ) {
    return;
  }
  VALUE attributes = rb_hash_new();

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "type" ) ), UINT2NUM( type ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "flags" ) ), UINT2NUM( flags ) );

  uint16_t body_length = ( uint16_t ) body->length;
  switch ( type ) {
    case OFPST_DESC:
    {
      struct ofp_desc_stats *desc_stats = ( struct ofp_desc_stats * ) body->data;
      VALUE options = rb_hash_new();
      VALUE desc_stats_arr = rb_ary_new();
      VALUE desc_stats_reply;

      rb_hash_aset( options, ID2SYM( rb_intern( "mfr_desc" ) ),
        rb_str_new( desc_stats->mfr_desc, ( long ) strnlen( desc_stats->mfr_desc, DESC_STR_LEN - 1 ) ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "hw_desc" ) ),
        rb_str_new( desc_stats->hw_desc, ( long ) strnlen( desc_stats->hw_desc, DESC_STR_LEN  - 1 ) ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "sw_desc" ) ),
        rb_str_new( desc_stats->sw_desc, ( long ) strnlen( desc_stats->sw_desc, DESC_STR_LEN  - 1 ) ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "serial_num" ) ),
        rb_str_new( desc_stats->serial_num, ( long ) strnlen( desc_stats->serial_num, SERIAL_NUM_LEN  - 1 ) ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "dp_desc" ) ),
        rb_str_new( desc_stats->dp_desc, ( long ) strnlen( desc_stats->dp_desc, DESC_STR_LEN  - 1 ) ) );
      desc_stats_reply = rb_funcall( rb_eval_string( " Trema::DescStatsReply" ), rb_intern( "new" ), 1, options );
      rb_ary_push( desc_stats_arr, desc_stats_reply );
      rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), desc_stats_arr );
    }
      break;
    case OFPST_FLOW:
    {
      struct ofp_flow_stats *flow_stats;
      struct ofp_action_header *ah;
      VALUE flow_stats_arr = rb_ary_new();
      VALUE flow_stats_reply;
      VALUE match_obj;
      VALUE options = rb_hash_new();
      VALUE actions_arr;

      flow_stats = ( struct ofp_flow_stats * ) body->data;

      while ( body_length > 0 ) {
        actions_arr = rb_ary_new();

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

        uint16_t actions_length = ( uint16_t ) ( flow_stats->length - offsetof( struct ofp_flow_stats, actions ) );

        ah = ( struct ofp_action_header * ) flow_stats->actions;
        while ( actions_length > 0 ) {
          rb_ary_push( actions_arr, get_action( ah ) );
          actions_length = ( uint16_t ) ( actions_length - ah->len );
          if ( actions_length ) {
            ah = ( struct ofp_action_header * ) ( ( char * ) ah + ah->len );
          }
        }
        rb_hash_aset( options, ID2SYM( rb_intern( "actions" ) ), actions_arr );

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
      VALUE options = rb_hash_new();
      VALUE aggregate_stats_reply;
      VALUE aggregate_stats_arr = rb_ary_new();

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
      VALUE table_stats_arr = rb_ary_new();
      VALUE options = rb_hash_new();
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
        body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_table_stats ) );
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
      VALUE port_stats_arr = rb_ary_new();
      VALUE options = rb_hash_new();
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
        body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_port_stats ) );
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
      VALUE queue_stats_arr = rb_ary_new();
      VALUE options = rb_hash_new();
      VALUE queue_stats_reply;

      queue_stats = ( struct ofp_queue_stats * ) body->data;
      while ( body_length > 0 ) {
        rb_hash_aset( options, ID2SYM( rb_intern( "port_no" ) ), UINT2NUM( queue_stats->port_no ) );
        rb_hash_aset( options, ID2SYM( rb_intern( "queue_id" ) ), UINT2NUM( queue_stats->queue_id ) );

        queue_stats_reply = rb_funcall( rb_eval_string( "Trema::QueueStatsReply" ), rb_intern( "new" ), 1, options );

        rb_ary_push( queue_stats_arr, queue_stats_reply );
        body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_queue_stats ) );
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
      VALUE options = rb_hash_new();
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
  rb_funcall( controller, rb_intern( "stats_reply" ), 2, ULL2NUM( datapath_id ), r_stats_reply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
