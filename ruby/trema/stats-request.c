/*
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


/*
 * @overload initialize(options={})
 *   A +OPPT_STATS_REQUEST+ message is sent to collect statistics for a
 *   type element from the datapath. This type element can be a flow
 *   port, queue, or vendor. All stats. request messages share a common header
 *   followed by additional fields that further describe the request type.
 *   There is a derived class for each associated type. All stats. requests
 *   encapsulate their instances as a buffer object that can be converted
 *   to a packet to be used for transmission.
 *
 *   @see FlowStatsRequest
 *   @see AggregateStatsRequest
 *   @see TableStatsRequest
 *   @see PortStatsRequest
 *   @see QueueStatsRequest
 *   @see VendorStatsRequest
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     transaction_id for this request or auto-generated if not specified.
 *
 *   @option options [Number] :flags
 *     flags not defined yet should be set to zero.
 *
 *   @raise [ArgumentError] if supplied transaction_id is not an unsigned 32-bit integer.
 *
 *   @return [StatsRequest] 
 *     an object that encapsulates an +OFPT_STATS_REQUEST+ OpenFlow message.
 */
static VALUE
stats_request_init( VALUE self, VALUE options ) {
  VALUE transaction_id;
  if ( ( transaction_id = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) ) ) != Qnil ) {
    if ( rb_funcall( transaction_id, rb_intern( "unsigned_32bit?" ), 0 ) == Qfalse ) {
      rb_raise( rb_eArgError, "Transaction ID must be an unsigned 32-bit integer" );
    }
  }
  else {
    transaction_id = UINT2NUM( get_transaction_id() );
  }
  rb_iv_set( self, "@transaction_id", transaction_id );
  VALUE flags = rb_hash_aref( options, ID2SYM( rb_intern( "flags" ) ) );
  if ( flags == Qnil ) {
    flags = UINT2NUM( 0 );
  }
  rb_iv_set( self, "@flags", flags );
  return self;
}


static VALUE
subclass_stats_request_init( VALUE self, VALUE options ) {
  rb_call_super( 1, &options );
  VALUE match = rb_hash_aref( options, ID2SYM( rb_intern( "match" ) ) );
  if ( match == Qnil ) {
    rb_raise( rb_eArgError, "match option must be specified" );
  }
  rb_iv_set( self, "@match", match );
  VALUE table_id = rb_hash_aref( options, ID2SYM( rb_intern( "table_id" ) ) );
  if ( table_id == Qnil ) {
    table_id = UINT2NUM( 0xff );
  }
  rb_iv_set( self, "@table_id", table_id );
  VALUE out_port = rb_hash_aref( options, ID2SYM( rb_intern( "out_port" ) ) );
  if ( out_port == Qnil ) {
    out_port = UINT2NUM( OFPP_NONE );
  }
  rb_iv_set( self, "@out_port", out_port );
  return self;
}


/*
 * A {FlowStatsRequest} object instance to request flow statistics.
 *
 * @overload initialize(options={})
 *   @example
 *     FlowStatsRequest.new(
 *       :match => Match
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Match] :match
 *     a {Match} object to match flow fields with this request.
 *     This option is mandatory.
 *
 *   @option options [Number] :table_id
 *     a table id to match and restrict returned results.
 *     A value of 0xff would return all tables and is set to if not specified.
 *
 *   @option options [Number] :out_port
 *     a value of +OFPP_NONE+ would match all flow entries and is set to if not
 *     specified.
 *
 *   @raise [ArgumentError] if option[:match] is not specified.
 *
 *   @return [FlowStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_FLOW)+ OpenFlow message.
 */
static VALUE
flow_stats_request_init( VALUE self, VALUE options ) {
  return subclass_stats_request_init( self, options );
}


/*
 * A {AggregateStatsRequest} object instance to request aggregate statistics.
 * @overload initialize(options={})
 *   @example
 *     AggregateStatsRequest.new(
 *       :match => Match
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Match] :match
 *     a {Match} object to match flow fields with this request.
 *     This option is mandatory.
 *
 *   @option options [Number] :table_id
 *     a table id to match and restrict returned results.
 *     A value of 0xff would return all tables and is set to if not specified.
 *
 *   @option options [Number] :out_port
 *     a value of +OFPP_NONE+ would match all flow entries and is set to if not
 *     specified.
 *
 *   @raise [ArgumentError] if option[:match] is not specified.
 *
 *   @return [AggregateStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_AGGREGATE)+ OpenFlow message.
 */
static VALUE
aggregate_stats_request_init( VALUE self, VALUE options ) {
  return subclass_stats_request_init( self, options );
}


/*
 * Transaction ids, message sequence numbers matching requests to replies.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
stats_transaction_id( VALUE self ) {
  return rb_iv_get( self, "@transaction_id" );
}


/*
 * Not yet defined. Set to zero.
 *
 * @return [Number] the value of flags.
 */
static VALUE
stats_flags( VALUE self ) {
  return rb_iv_get( self, "@flags" );
}


/*
 * Detailed description of each flow field.
 *
 * @return [Match] the value of match.
 */
static VALUE
stats_match( VALUE self ) {
  return rb_iv_get( self, "@match" );
}


/*
 * An index into array of tables. 0xff for all tables.
 *
 * @return [Number] the value of table_id.
 */
static VALUE
stats_table_id( VALUE self ) {
  return rb_iv_get( self, "@table_id" );
}


/*
 * Requires flow matching if defined.
 *
 * @return [Number] the value of out_port.
 */
static VALUE
stats_out_port( VALUE self ) {
  return rb_iv_get( self, "@out_port" );
}


/*
 * Restrict port statistics to a specific port_no or to all ports.
 *
 * @return [Number] the value of port_no.
 */
static VALUE
stats_port_no( VALUE self ) {
  return rb_iv_get( self, "@port_no" );
}


/*
 * Restrict queue statistics to a specific queue_id or to all queues.
 *
 * @return [Number] the value of queue_id.
 */
static VALUE
stats_queue_id( VALUE self ) {
  return rb_iv_get( self, "@queue_id" );
}


/*
 * Vendor id uniquely assigned for each vendor.
 *
 * @return [Number] the value of vendor_id.
 */
static VALUE
stats_vendor_id( VALUE self ) {
  return rb_iv_get( self, "@vendor_id" );
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


/*
 * Creates an opaque pointer to +OFPT_STATS_REQUEST(OFPST_FLOW)+ message
 * saved into its +buffer+ attribute.
 *
 * @return [FlowStatsRequest] self
 */
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


/*
 * Creates an opaque pointer to +OFPT_STATS_REQUEST(OFPST_AGGREAGATE)+ message
 * saved into its +buffer+ attribute.
 *
 * @return [AggregateStatsRequest] self
 */
static VALUE
aggregate_stats_request_to_packet( VALUE self ) {
  buffer *aggregate_stats_request;
  aggregate_stats_request = create_aggregate_stats_request(
    get_stats_request_num2uint( self, "@transaction_id" ),
    get_stats_request_num2uint( self, "@flags" ),
    get_stats_request_match( self ),
    get_stats_request_table_id( self ),
    get_stats_request_num2uint( self, "@out_port" ) );
  stats_request_buffer_set( self, aggregate_stats_request );
  return self;
}


/*
 * @return [buffer] the stats. request type object.
 */
static VALUE
stats_request_buffer( VALUE self ) {
  return rb_iv_get( self, "@buffer" );
}


/*
 * A {TableStatsRequest} object instance to request table statistics.
 * Request table statistics. The table stats. request does not contain any data
 * in the body.
 *
 * @overload initialize(options={})
 *
 *   @example 
 *     TableStatsRequest.new(
 *       :transaction_id => 1234
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     set the transaction_id as specified or auto-generate it.
 *
 *   @return [TableStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_TABLE)+ openflow
 *     message.
 */
static VALUE
table_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  UNUSED(self);
  VALUE options;
  if ( !rb_scan_args( argc, argv, "01", &options )) {
    options = rb_hash_new();
  }
  return rb_call_super( 1, &options );
}


/*
 * Creates an opaque pointer to +OFPT_STATS_REQUEST(OFPST_TABLE)+ message
 * saved into its +buffer+ attribute.
 *
 * @return [TableStatsRequest] self
 */
static VALUE
table_stats_request_to_packet( VALUE self ) {
  buffer *table_stats_request;
  table_stats_request = create_table_stats_request(
    get_stats_request_num2uint( self, "@transaction_id" ),
    get_stats_request_num2uint( self, "@flags" ) );
  stats_request_buffer_set( self, table_stats_request );
  return self;
}


/*
 * A {PortStatsRequest} object instance to request port statistics.
 * Request port statistics.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     PortStatsRequest.new(
 *       :port_no => port_no
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :port_no
 *     request statistics for a specific port if specified, otherwise set port_no
 *     to +OFPP_NONE+ for all ports.
 *
 *   @return [PortStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_PORT)+ OpenFlow 
 *     message.
 */
static VALUE
port_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;
  if ( !rb_scan_args( argc, argv, "01", &options )) {
    options = rb_hash_new();
  }
  rb_call_super( 1, &options );
  VALUE port_no = rb_hash_aref( options, ID2SYM( rb_intern( "port_no" ) ) );
  if ( port_no == Qnil ) {
    port_no = UINT2NUM( OFPP_NONE );
  }
  rb_iv_set( self, "@port_no", port_no );
  return self;
}


/*
 * Creates an opaque pointer to +OFPT_STATS_REQUEST(OFPST_PORT)+ message
 * saved into its +buffer+ attribute.
 *
 * @return [PortStatsRequest] self
 */
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


/*
 * A {QueueStatsRequest} object instance to request queue statistics.
 * Request queue statistics.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     QueueStatsRequest.new(
 *       :port_no => port_no,
 *       :queue_id => queue_id
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :port_no
 *     request statistics for a specific port if specified, otherwise set port_no
 *     to +OFPP_ALL+ for all ports.
 *
 *   @option options [Number] :queue_id
 *     request statistics for a specific queue_id or set queue_id to +OFPQ_ALL+
 *     for all queues.
 *
 *   @return [QueueStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_QUEUE)+ OpenFlow
 *     message.
 */
static VALUE
queue_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;
  if ( !rb_scan_args( argc, argv, "01", &options )) {
    options = rb_hash_new();
  }
  rb_call_super( 1, &options );
  VALUE port_no = rb_hash_aref( options, ID2SYM( rb_intern( "port_no" ) ) );
  if ( port_no == Qnil ) {
    port_no = UINT2NUM( OFPP_ALL );
  }
  rb_iv_set( self, "@port_no", port_no );
  VALUE queue_id = rb_hash_aref( options, ID2SYM( rb_intern( "queue_id" ) ) );
  if ( queue_id == Qnil ) {
    queue_id = UINT2NUM( OFPQ_ALL );
  }
  rb_iv_set( self, "@queue_id", queue_id );
  return self;
}


/*
 * Creates an opaque pointer to the +OFPT_STATS_REQUEST(OFPST_QUEUE)+ message
 * saved into its +buffer+ attribute.
 *
 * @return [QueueStatsRequest] self
 */
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


/*
 * A {VendorStatsRequest} object instance to request vendor statistics.
 * Request vendor specific statistics.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     VendorStatsRequeset.new(
 *       :vendor_id => vendor_id
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :vendor_id
 *     request statistics for a specific vendor_id, otherwise set vendor_id
 *     to a default value of 0x00004cff.
 *
 *   @return [VendorStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_VENDOR)+ openflow
 *     message.
 */
static VALUE
vendor_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( !rb_scan_args( argc, argv, "01", &options )) {
    options = rb_hash_new();
  }
  rb_call_super( 1, &options );
  VALUE vendor_id = rb_hash_aref( options, ID2SYM( rb_intern( "vendor_id" ) ) );
  if ( vendor_id == Qnil ) {
    vendor_id = UINT2NUM( 0x00004cff );
  }
  rb_iv_set( self, "@vendor_id", vendor_id );
  return self;
}


/*
 * Serializes this object instance and store the result into its buffer attribute.
 *
 * @return [VendorStatsRequest] self
 */
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
Init_stats_request(){
  cStatsRequest = rb_define_class_under( mTrema, "StatsRequest", rb_cObject );
  cFlowStatsRequest = rb_define_class_under( mTrema, "FlowStatsRequest", cStatsRequest );
  rb_define_method( cStatsRequest, "initialize", stats_request_init, 1 );
  rb_define_method( cStatsRequest, "transaction_id", stats_transaction_id, 0 );
  rb_define_method( cStatsRequest, "flags", stats_flags, 0 );
  
  rb_define_method( cFlowStatsRequest, "initialize", flow_stats_request_init, 1 );
  rb_define_method( cFlowStatsRequest, "match", stats_match, 0 );
  rb_define_method( cFlowStatsRequest, "table_id", stats_table_id, 0 );
  rb_define_method( cFlowStatsRequest, "out_port", stats_out_port, 0 );
  rb_define_method( cFlowStatsRequest, "to_packet", flow_stats_request_to_packet, 0 );

  cAggregateStatsRequest = rb_define_class_under( mTrema, "AggregateStatsRequest", cStatsRequest );
  rb_define_method( cAggregateStatsRequest, "initialize", aggregate_stats_request_init, 1 );
  rb_define_method( cAggregateStatsRequest, "match", stats_match, 0 );
  rb_define_method( cAggregateStatsRequest, "table_id", stats_table_id, 0 );
  rb_define_method( cAggregateStatsRequest, "out_port", stats_out_port, 0 );
  rb_define_method( cAggregateStatsRequest, "to_packet", aggregate_stats_request_to_packet, 0 );

  cTableStatsRequest = rb_define_class_under( mTrema, "TableStatsRequest", cStatsRequest );
  rb_define_method( cTableStatsRequest, "initialize", table_stats_request_init, -1 );
  rb_define_method( cTableStatsRequest, "to_packet", table_stats_request_to_packet, 0 );

  cPortStatsRequest = rb_define_class_under( mTrema, "PortStatsRequest", cStatsRequest );
  rb_define_method( cPortStatsRequest, "initialize", port_stats_request_init, -1 );
  rb_define_method( cPortStatsRequest, "port_no", stats_port_no, 0 );
  rb_define_method( cPortStatsRequest, "to_packet", port_stats_request_to_packet, 0 );

  cQueueStatsRequest = rb_define_class_under( mTrema, "QueueStatsRequest", cStatsRequest );
  rb_define_method( cQueueStatsRequest, "initialize", queue_stats_request_init, -1 );
  rb_define_method( cQueueStatsRequest, "port_no", stats_port_no, 0 );
  rb_define_method( cQueueStatsRequest, "queue_id", stats_queue_id, 0 );
  rb_define_method( cQueueStatsRequest, "to_packet", queue_stats_request_to_packet, 0 );

  cVendorStatsRequest = rb_define_class_under( mTrema, "VendorStatsRequest", cStatsRequest );
  rb_define_method( cVendorStatsRequest, "initialize", vendor_stats_request_init, -1 );
  rb_define_method( cVendorStatsRequest, "vendor_id", stats_vendor_id, 0 );
  rb_define_method( cVendorStatsRequest, "to_packet", vendor_stats_request_to_packet, 0 );

  rb_define_method( cStatsRequest, "buffer", stats_request_buffer, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
