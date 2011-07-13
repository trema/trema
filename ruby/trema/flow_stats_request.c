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
VALUE cFlowStatsRequest;


static VALUE
flow_stats_request_init( VALUE self, VALUE match_obj ) {
	struct ofp_match *match;
  buffer *flow_stats_request;
	uint32_t transaction_id = get_transaction_id();
	uint16_t flags = 0;
	uint8_t table_id = 0xff;
	uint16_t out_port = OFPP_NONE;

	Data_Get_Struct( match_obj, struct ofp_match, match );
	flow_stats_request = create_flow_stats_request( transaction_id, flags, *match, table_id, out_port );
  rb_iv_set( self, "@buffer", Data_Wrap_Struct( cFlowStatsRequest, NULL, free_buffer, flow_stats_request ) );
	return self;
}


static VALUE
flow_stats_request_buffer( VALUE self ) {
	return rb_iv_get( self, "@buffer" );
}


void
Init_flow_stats_request( ) {
  cFlowStatsRequest = rb_define_class_under( mTrema, "FlowStatsRequest", rb_cObject );
  rb_define_method( cFlowStatsRequest, "initialize", flow_stats_request_init, 1 );
  rb_define_method( cFlowStatsRequest, "buffer", flow_stats_request_buffer, 0 );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
