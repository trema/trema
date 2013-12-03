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


#include "trema.h"
#include "trema-ruby-utils.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE cStatsRequest;
VALUE cDescStatsRequest;
VALUE cFlowStatsRequest;
VALUE cAggregateStatsRequest;
VALUE cTableStatsRequest;
VALUE cPortStatsRequest;
VALUE cQueueStatsRequest;
VALUE cVendorStatsRequest;


static const struct ofp_match MATCH = { 0, 1,
                                      { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 },
                                      { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d },
                                       1, 1, { 0 }, 0x800, 0xfc, 0x6, { 0, 0 },
                                       0x0a090807, 0x0a090807, 1024, 2048 };
static const uint32_t MY_TRANSACTION_ID = 1;
static const uint32_t VENDOR_ID = 0x00004cff;
static const uint16_t NO_FLAGS = 0;
static const uint16_t OUT_PORT = 1;
static const uint16_t VENDOR_STATS_FLAG = 0xaabb;
static const uint8_t  TABLE_ID = 0xff;


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


static VALUE
desc_stats_request_alloc( VALUE klass ) {
  buffer *desc_stats_request = create_desc_stats_request( MY_TRANSACTION_ID, NO_FLAGS );
  return Data_Wrap_Struct( klass, NULL, free_buffer, desc_stats_request );
}


static VALUE
flow_stats_request_alloc( VALUE klass ) {
  buffer *flow_stats_request = create_flow_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, TABLE_ID, OUT_PORT );
  return Data_Wrap_Struct( klass, NULL, free_buffer, flow_stats_request );
}


static VALUE
aggregate_stats_request_alloc( VALUE klass ) {
  buffer *aggregate_stats_request = create_aggregate_stats_request( MY_TRANSACTION_ID, NO_FLAGS, MATCH, TABLE_ID, OUT_PORT );
  return Data_Wrap_Struct( klass, NULL, free_buffer, aggregate_stats_request );
}


static VALUE
table_stats_request_alloc( VALUE klass ) {
  buffer *table_stats_request = create_table_stats_request( MY_TRANSACTION_ID, NO_FLAGS );
  return Data_Wrap_Struct( klass, NULL, free_buffer, table_stats_request );
}


static VALUE
port_stats_request_alloc( VALUE klass ) {
  uint16_t port_no = 1;
  buffer *port_stats_request = create_port_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no );
  return Data_Wrap_Struct( klass, NULL, free_buffer, port_stats_request );
}


static VALUE
queue_stats_request_alloc( VALUE klass ) {
  uint32_t queue_id = 10;
  uint16_t port_no = 1;
  buffer *queue_stats_request = create_queue_stats_request( MY_TRANSACTION_ID, NO_FLAGS, port_no, queue_id );
  return Data_Wrap_Struct( klass, NULL, free_buffer, queue_stats_request );
}


static VALUE
vendor_stats_request_alloc( VALUE klass ) {
  uint16_t length = 128;
  buffer *body = alloc_buffer_with_length( length );
  void *p = append_back_buffer( body, length );
  memset( p, 0xaf, length );
  buffer *vendor_stats_request = create_vendor_stats_request( MY_TRANSACTION_ID, VENDOR_STATS_FLAG, VENDOR_ID, body );
  return Data_Wrap_Struct( klass, NULL, free_buffer, vendor_stats_request );
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


/*
 * Vendor specific data payload.
 *
 * @return [Array] an array of data payload bytes.
 * @return [nil] vendor specific data not found.
 */
static VALUE
stats_vendor_data( VALUE self ) {
  return rb_iv_get( self, "@data" );
}


uint32_t
get_stats_request_num2uint( VALUE self, const char *field ) {
  return ( uint32_t ) NUM2UINT( rb_iv_get( self, field ) );
}


uint16_t
get_stats_request_num2uint16( VALUE self, const char *field ) {
  return ( uint16_t ) NUM2UINT( rb_iv_get( self, field ) );
}


uint8_t
get_stats_request_table_id( VALUE self ) {
  return ( uint8_t ) NUM2UINT( rb_iv_get( self, "@table_id" ) );
}


static VALUE
parse_common_arguments( int argc, VALUE *argv, VALUE self ) {
  VALUE options;
  if ( !rb_scan_args( argc, argv, "01", &options ) ) {
    options = rb_hash_new();
  }
  rb_call_super( 1, &options );
  buffer *message;
  Data_Get_Struct( self, buffer, message );
  ( ( struct ofp_header * ) ( message->data ) )->xid = htonl( get_stats_request_num2uint( self, "@transaction_id" ) );
  struct ofp_stats_request *stats_request;
  stats_request = ( struct ofp_stats_request * ) message->data;
  stats_request->flags = htons ( get_stats_request_num2uint16( self, "@flags" ) );
  return self;
}


/*
 * A {DescStatsRequest} object instance to request descriptive information of
 * OpenFlow switch. Such information includes switch manufacturer, hardware
 * revision and serial number
 *
 * @overload initialize(options={})
 *
 *   @example
 *     DescStatsRequest.new
 *     DescStatsRequest.new( :transaction_id => 1234 )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     set the transaction_id as specified or auto-generate it.
 *
 *   @return [DescStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_DESC)+ OpenFlow
 *     message.
 */
static VALUE
desc_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  return parse_common_arguments( argc, argv, self );
}


/*
 * A {FlowStatsRequest} object instance to request flow statistics.
 *
 * @overload initialize(options={})
 *   @example
 *     FlowStatsRequest.new( :match => Match )
 *     FlowStatsRequest.new( :match => Match, :table_id => 1 )
 *     FlowStatsRequest.new( :match => Match, :table_id => 1, :out_port => 2 )
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
 *   @raise [ArgumentError] if option match is not specified.
 *   @raise [TypeError] if option match is not a Trema::Match object.
 *
 *   @return [FlowStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_FLOW)+ OpenFlow message.
 */
static VALUE
flow_stats_request_init( VALUE self, VALUE options ) {
  buffer *message;
  Data_Get_Struct( self, buffer, message );

  subclass_stats_request_init( self, options );

  ( ( struct ofp_header * ) ( message->data ) )->xid = htonl( get_stats_request_num2uint( self, "@transaction_id" ) );

  struct ofp_stats_request *stats_request;
  stats_request = ( struct ofp_stats_request * ) message->data;
  struct ofp_flow_stats_request *flow_stats_request;
  flow_stats_request = ( struct ofp_flow_stats_request * ) stats_request->body;

  stats_request->flags = htons ( get_stats_request_num2uint16( self, "@flags" ) );

  const struct ofp_match *match;
  Data_Get_Struct( rb_iv_get( self, "@match" ), struct ofp_match, match );
  hton_match( &flow_stats_request->match, match );
  flow_stats_request->table_id = get_stats_request_table_id( self );
  flow_stats_request->out_port = htons( get_stats_request_num2uint16( self, "@out_port" ) );
  return self;
}


/*
 * A {AggregateStatsRequest} object instance to request aggregate statistics.
 * @overload initialize(options={})
 *   @example
 *     AggregateStatsRequest.new( :match => Match )
 *     AggregateStatsRequest.new( :match => Match, :table_id => 1, :out_port => 2 )
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
 *   @raise [ArgumentError] if option match is not specified.
 *   @raise [TypeError] if option match is not a Trema::Match object.
 *
 *   @return [AggregateStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_AGGREGATE)+ OpenFlow message.
 */
static VALUE
aggregate_stats_request_init( VALUE self, VALUE options ) {
  buffer *message;
  Data_Get_Struct( self, buffer, message );

  subclass_stats_request_init( self, options );

  ( ( struct ofp_header * ) ( message->data ) )->xid = htonl( get_stats_request_num2uint( self, "@transaction_id" ) );

  struct ofp_stats_request *stats_request;
  stats_request = ( struct ofp_stats_request * ) message->data;
  struct ofp_aggregate_stats_request *aggregate_stats_request;
  aggregate_stats_request = ( struct ofp_aggregate_stats_request * ) stats_request->body;

  stats_request->flags = htons ( get_stats_request_num2uint16( self, "@flags" ) );

  const struct ofp_match *match;
  Data_Get_Struct( rb_iv_get( self, "@match" ), struct ofp_match, match );
  hton_match( &aggregate_stats_request->match, match );
  aggregate_stats_request->table_id = get_stats_request_table_id( self );
  aggregate_stats_request->out_port = htons( get_stats_request_num2uint16( self, "@out_port" ) );
  return self;
}


/*
 * A {TableStatsRequest} object instance to request table statistics.
 * Request table statistics. The table stats. request does not contain any data
 * in the body.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     TableStatsRequest.new
 *     TableStatsRequest.new( :transaction_id => 1234 )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :transaction_id
 *     set the transaction_id as specified or auto-generate it.
 *
 *   @return [TableStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_TABLE)+ OpenFlow
 *     message.
 */
static VALUE
table_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  return parse_common_arguments( argc, argv, self );
}


/*
 * A {PortStatsRequest} object instance to request port statistics.
 * Request port statistics.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     PortStatsRequest.new
 *     PortStatsRequest.new( :port_no => 1 )
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
  if ( !rb_scan_args( argc, argv, "01", &options ) ) {
    options = rb_hash_new();
  }
  rb_call_super( 1, &options );
  VALUE port_no = rb_hash_aref( options, ID2SYM( rb_intern( "port_no" ) ) );
  if ( port_no == Qnil ) {
    port_no = UINT2NUM( OFPP_NONE );
  }
  rb_iv_set( self, "@port_no", port_no );

  buffer *message;
  Data_Get_Struct( self, buffer, message );
  ( ( struct ofp_header * ) ( message->data ) )->xid = htonl( get_stats_request_num2uint( self, "@transaction_id" ) );
  struct ofp_stats_request *stats_request;
  stats_request = ( struct ofp_stats_request * ) message->data;
  stats_request->flags = htons ( get_stats_request_num2uint16( self, "@flags" ) );
  struct ofp_port_stats_request *port_stats_request = ( struct ofp_port_stats_request * ) stats_request->body;
  port_stats_request->port_no = htons( get_stats_request_num2uint16( self, "@port_no" ) );
  return self;
}


/*
 * A {QueueStatsRequest} object instance to request queue statistics.
 * Request queue statistics.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     QueueStatsRequest.new
 *     QueueStatsRequest.new( :port_no => 1, :queue_id => 123 )
 *     QueueStatsRequest.new( :port_no => 1 )
 *     QueueStatsRequest.new( :queue_id => 123 )
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
  if ( !rb_scan_args( argc, argv, "01", &options ) ) {
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


  buffer *message;
  Data_Get_Struct( self, buffer, message );
  ( ( struct ofp_header * ) ( message->data ) )->xid = htonl( get_stats_request_num2uint( self, "@transaction_id" ) );
  struct ofp_stats_request *stats_request;
  stats_request = ( struct ofp_stats_request * ) message->data;
  stats_request->flags = htons ( get_stats_request_num2uint16( self, "@flags" ) );

  stats_request = ( struct ofp_stats_request * ) message->data;
  struct ofp_queue_stats_request *queue_stats_request;
  queue_stats_request = ( struct ofp_queue_stats_request * ) stats_request->body;
  queue_stats_request->port_no = htons( get_stats_request_num2uint16( self, "@port_no" ) );
  queue_stats_request->queue_id = htonl( get_stats_request_num2uint( self, "@queue_id" ) );
  return self;
}


/*
 * A {VendorStatsRequest} object instance to request vendor statistics.
 * Request vendor specific statistics.
 *
 * @overload initialize(options={})
 *
 *   @example
 *     VendorStatsRequest.new
 *     VendorStatsRequest.new(
 *       :vendor_id => 123,
 *       :data => "deadbeef".unpack( "C*" )
 *     )
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *   @option options [Number] :vendor_id
 *     request statistics for a specific vendor_id, otherwise set vendor_id
 *     to a default value of 0x00004cff.
 *
 *   @option options [Array] :data
 *     a String that holds vendor's defined arbitrary length data.
 *
 *   @return [VendorStatsRequest]
 *     an object that encapsulates the +OFPT_STATS_REQUEST(OFPST_VENDOR)+ OpenFlow
 *     message.
 */
static VALUE
vendor_stats_request_init( int argc, VALUE *argv, VALUE self ) {
  VALUE options;

  if ( !rb_scan_args( argc, argv, "01", &options ) ) {
    options = rb_hash_new();
  }
  rb_call_super( 1, &options );
  VALUE vendor_id = rb_hash_aref( options, ID2SYM( rb_intern( "vendor_id" ) ) );
  if ( vendor_id == Qnil ) {
    vendor_id = UINT2NUM( 0x00004cff );
  }
  rb_iv_set( self, "@vendor_id", vendor_id );
  buffer *message;
  Data_Get_Struct( self, buffer, message );
  ( ( struct ofp_header * ) ( message->data ) )->xid = htonl( get_stats_request_num2uint( self, "@transaction_id" ) );
  struct ofp_stats_request *stats_request;
  stats_request = ( struct ofp_stats_request * ) message->data;
  stats_request->flags = htons ( get_stats_request_num2uint16( self, "@flags" ) );
  uint32_t *vendor;
  vendor = ( uint32_t * ) stats_request->body;
  *vendor = htonl( get_stats_request_num2uint( self, "@vendor_id" ) );

  VALUE ary = rb_hash_aref( options, ID2SYM( rb_intern( "data" ) ) );
  message->length = offsetof( struct ofp_stats_request, body ) + sizeof( uint32_t );
  if ( ary != Qnil ) {
    rb_iv_set( self, "@data", ary );
    Check_Type( ary, T_ARRAY );
    uint16_t ary_len = ( uint16_t ) RARRAY_LEN( ary );
    uint8_t *data = append_back_buffer( message, ary_len );
    for ( int i = 0; i < ary_len; i++ ) {
      data[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( ary )[ i ] );
    }
  }
  set_length( message, ( uint16_t ) message->length );
  return self;
}


/*
 * Document-class: Trema::StatsRequest
 * Document-class: Trema::DescStatsRequest
 * Document-class: Trema::FlowStatsRequest
 * Document-class: Trema::AggregateStatsRequest
 * Document-class: Trema::TableStatsRequest
 * Document-class: Trema::PortStatsRequest
 * Document-class: Trema::QueueStatsRequest
 * Document-class: Trema::VendorStatsRequest
 */
void
Init_stats_request() {
  mTrema = rb_eval_string( "Trema" );
  cStatsRequest = rb_define_class_under( mTrema, "StatsRequest", rb_cObject );

  cDescStatsRequest = rb_define_class_under( mTrema, "DescStatsRequest", cStatsRequest );
  rb_define_alloc_func( cDescStatsRequest, desc_stats_request_alloc );
  rb_define_method( cDescStatsRequest, "initialize", desc_stats_request_init, -1 );

  cFlowStatsRequest = rb_define_class_under( mTrema, "FlowStatsRequest", cStatsRequest );
  rb_define_method( cStatsRequest, "initialize", stats_request_init, 1 );
  rb_define_method( cStatsRequest, "transaction_id", stats_transaction_id, 0 );
  rb_alias( cStatsRequest, rb_intern( "xid" ), rb_intern( "transaction_id" ) );
  rb_define_method( cStatsRequest, "flags", stats_flags, 0 );

  rb_define_alloc_func( cFlowStatsRequest, flow_stats_request_alloc );
  rb_define_method( cFlowStatsRequest, "initialize", flow_stats_request_init, 1 );
  rb_define_method( cFlowStatsRequest, "match", stats_match, 0 );
  rb_define_method( cFlowStatsRequest, "table_id", stats_table_id, 0 );
  rb_define_method( cFlowStatsRequest, "out_port", stats_out_port, 0 );

  cAggregateStatsRequest = rb_define_class_under( mTrema, "AggregateStatsRequest", cStatsRequest );
  rb_define_alloc_func( cAggregateStatsRequest, aggregate_stats_request_alloc );
  rb_define_method( cAggregateStatsRequest, "initialize", aggregate_stats_request_init, 1 );
  rb_define_method( cAggregateStatsRequest, "match", stats_match, 0 );
  rb_define_method( cAggregateStatsRequest, "table_id", stats_table_id, 0 );
  rb_define_method( cAggregateStatsRequest, "out_port", stats_out_port, 0 );

  cTableStatsRequest = rb_define_class_under( mTrema, "TableStatsRequest", cStatsRequest );
  rb_define_alloc_func( cTableStatsRequest, table_stats_request_alloc );
  rb_define_method( cTableStatsRequest, "initialize", table_stats_request_init, -1 );

  cPortStatsRequest = rb_define_class_under( mTrema, "PortStatsRequest", cStatsRequest );
  rb_define_alloc_func( cPortStatsRequest, port_stats_request_alloc );
  rb_define_method( cPortStatsRequest, "initialize", port_stats_request_init, -1 );
  rb_define_method( cPortStatsRequest, "port_no", stats_port_no, 0 );

  cQueueStatsRequest = rb_define_class_under( mTrema, "QueueStatsRequest", cStatsRequest );
  rb_define_alloc_func( cQueueStatsRequest, queue_stats_request_alloc );
  rb_define_method( cQueueStatsRequest, "initialize", queue_stats_request_init, -1 );
  rb_define_method( cQueueStatsRequest, "port_no", stats_port_no, 0 );
  rb_define_method( cQueueStatsRequest, "queue_id", stats_queue_id, 0 );

  cVendorStatsRequest = rb_define_class_under( mTrema, "VendorStatsRequest", cStatsRequest );
  rb_define_alloc_func( cVendorStatsRequest, vendor_stats_request_alloc );
  rb_define_method( cVendorStatsRequest, "initialize", vendor_stats_request_init, -1 );
  rb_define_method( cVendorStatsRequest, "vendor_id", stats_vendor_id, 0 );
  rb_define_method( cVendorStatsRequest, "data", stats_vendor_data, 0 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
