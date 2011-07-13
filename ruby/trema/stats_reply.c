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
VALUE cFlowStatsReply;
VALUE cTableStatsReply;
VALUE cPortStatsReply;
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
  cStatsReply = rb_define_class_under( mTrema, "StatsReply", rb_cObject );
	cTableStatsReply = rb_define_class_under( mTrema, "TableStatsReply", cStatsReply );
	cPortStatsReply = rb_define_class_under( mTrema, "PortStatsReply", cStatsReply );
	cQueueStatsReply = rb_define_class_under( mTrema, "QueueStatsReply", cStatsReply );
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
  VALUE attributes = rb_hash_new( );
	uint16_t body_length;

  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "type" ) ), UINT2NUM( type ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "flags" ) ), UINT2NUM( flags ) );

	body_length = body->length;
	switch ( type ) {
		case OFPST_FLOW:
			{
				struct ofp_flow_stats *flow_stats;
				VALUE flow_stats_arr = rb_ary_new();
				VALUE flow_stats_reply;
			
				flow_stats  = ( struct ofp_flow_stats * ) body->data;

				while ( body_length > 0 ) {
					flow_stats_reply = rb_funcall( rb_eval_string( "Trema::FlowStatsReply" ), rb_intern( "new" ), 1, Data_Wrap_Struct( cStatsReply, NULL, NULL, flow_stats ) );
					rb_ary_push( flow_stats_arr, flow_stats_reply );
					
					// here create flow_stats object and insert into array 
					body_length = ( uint16_t ) ( body_length - flow_stats->length );
				
					if ( body_length ) {
						// move pointer to next entry
						flow_stats = ( struct ofp_flow_stats * ) ( ( char * )flow_stats + flow_stats->length );
					}
				}
  			rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), flow_stats_arr );
			}
			break;
		case OFPST_AGGREGATE:
			{
				struct ofp_aggregate_stats_reply *aggregate_stats = ( struct ofp_aggregate_stats_reply * ) body->data;
				rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), Data_Wrap_Struct( cStatsReply, NULL, NULL, aggregate_stats ) );
			}
			break;
		case OFPST_TABLE:
			{
				struct ofp_table_stats *table_stats;
				VALUE table_stats_arr = rb_ary_new();

				table_stats = ( struct ofp_table_stats * ) body->data;
				while ( body_length > 0 ) {
					rb_ary_push( table_stats_arr, Data_Wrap_Struct( cStatsReply, NULL, NULL, table_stats ) );
					body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_table_stats ) );
					if ( body_length ) {
						table_stats++;
					}
				}
  			rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), table_stats_arr );
			}
		case OFPST_PORT:
			{
				struct ofp_port_stats *port_stats;
				VALUE port_stats_arr = rb_ary_new();
					
				port_stats = ( struct ofp_port_stats * ) body->data;
				while ( body_length > 0 ) {
					rb_ary_push( port_stats_arr, Data_Wrap_Struct( cStatsReply, NULL, NULL, port_stats ) );
					body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_port_stats ) );
					if ( body_length ) {
						port_stats++;
					}
				}
  			rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), port_stats_arr );
			}
		case OFPST_QUEUE:
			{
				struct ofp_queue_stats *queue_stats;
				VALUE queue_stats_arr = rb_ary_new();

				queue_stats = ( struct ofp_queue_stats * ) body->data;
				while ( body_length > 0 ) {
					rb_ary_push( queue_stats_arr, Data_Wrap_Struct( cStatsReply, NULL, NULL, queue_stats) );
					body_length = ( uint16_t ) ( body_length - sizeof( struct ofp_queue_stats ) );
					if ( body_length ) {
						queue_stats++;
					}
				}
  			rb_hash_aset( attributes, ID2SYM( rb_intern( "stats" ) ), queue_stats_arr );
			}
		case OFPST_VENDOR:
			{
				uint32_t *vendor_id;
				VALUE vendor_stats_arr = rb_ary_new2( 1 );
				
				vendor_id = ( uint32_t * ) body->data;
				rb_ary_push( vendor_stats_arr, UINT2NUM( *vendor_id ) );
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

