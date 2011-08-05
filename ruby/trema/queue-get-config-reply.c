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
VALUE cQueueGetConfigReply;


static VALUE
queue_get_config_reply_init( VALUE self, VALUE attribute ) {
  rb_iv_set( self, "@attribute", attribute );
  return self;
}


static VALUE
queue_get_config_reply_datapath_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "datapath_id" ) ) );
}


static VALUE
queue_get_config_reply_transaction_id( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "transaction_id" ) ) );
}


static VALUE
queue_get_config_reply_port( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "port" ) ) );
}


static VALUE
queue_get_config_reply_queues( VALUE self ) {
  return rb_hash_aref( rb_iv_get( self, "@attribute" ), ID2SYM( rb_intern( "queues" ) ) );
}


void
Init_queue_get_config_reply( ) {
  rb_require( "trema/packet-queue" );
  cQueueGetConfigReply = rb_define_class_under( mTrema, "QueueGetConfigReply", rb_cObject );
  rb_define_method( cQueueGetConfigReply, "initialize", queue_get_config_reply_init, 1 );
  rb_define_method( cQueueGetConfigReply, "datapath_id", queue_get_config_reply_datapath_id, 0 );
  rb_define_method( cQueueGetConfigReply, "transaction_id", queue_get_config_reply_transaction_id, 0 );
  rb_define_method( cQueueGetConfigReply, "port", queue_get_config_reply_port, 0 );
  rb_define_method( cQueueGetConfigReply, "queues", queue_get_config_reply_queues, 0 );
}


static void
get_property( struct ofp_packet_queue *pq, VALUE packet_queue ) {
  size_t offset;
  uint16_t properties_length;
  struct ofp_queue_prop_header *qph;
  struct ofp_queue_prop_min_rate *qpmr;

  offset = offsetof( struct ofp_packet_queue, properties );
  qph = ( struct ofp_queue_prop_header * ) ( ( char * ) pq + offset );

  properties_length = ( uint16_t ) ( pq->len - offset );
  while ( properties_length > 0 ) {
    if ( qph->property == OFPQT_MIN_RATE ) {
      qpmr = ( struct ofp_queue_prop_min_rate * ) qph;
      rb_funcall( rb_eval_string( "Trema::MinRateQueue" ), rb_intern( "new" ), 4,
              UINT2NUM( qph->property ),
              UINT2NUM( qph->len ),
              UINT2NUM( qpmr->rate ),
              packet_queue );
    }
    properties_length = ( uint16_t ) ( properties_length - pq->len );
    if ( properties_length > 0 ) {
      qph = ( struct ofp_queue_prop_header * ) ( ( char * ) qph + qph->len );
    }
  }
}


void
handle_queue_get_config_reply(
        uint64_t datapath_id,
        uint32_t transaction_id,
        uint16_t port,
        list_element *queues,
        void *user_data
        ) {
  VALUE controller = ( VALUE ) user_data;
  if ( rb_respond_to( controller, rb_intern( "queue_get_config_reply" ) ) == Qfalse ) {
    return;
  }

  VALUE attributes = rb_hash_new( );

  list_element *queue = NULL;
  struct ofp_packet_queue *pq;
  VALUE pq_attributes = rb_hash_new( );
  VALUE packet_queue;


  rb_hash_aset( attributes, ID2SYM( rb_intern( "datapath_id" ) ), ULL2NUM( datapath_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "transaction_id" ) ), UINT2NUM( transaction_id ) );
  rb_hash_aset( attributes, ID2SYM( rb_intern( "port" ) ), UINT2NUM( port ) );

  if ( queues != NULL ) {
    queue = queues;
    while ( queue != NULL ) {
      pq = ( struct ofp_packet_queue * ) queue->data;

      rb_hash_aset( pq_attributes, ID2SYM( rb_intern( "queue_id" ) ), UINT2NUM( pq->queue_id ) );
      rb_hash_aset( pq_attributes, ID2SYM( rb_intern( "len" ) ), UINT2NUM( pq->len ) );
      packet_queue = rb_funcall( rb_eval_string( "Trema::PacketQueue" ), rb_intern( "new" ), 1, pq_attributes );

      get_property( pq, packet_queue );
      queue = queue->next;
    }
    rb_hash_aset( attributes, ID2SYM( rb_intern( "queues" ) ), rb_eval_string( "PacketQueue.queues" ) );
  }
  VALUE queue_get_config_reply = rb_funcall( cQueueGetConfigReply, rb_intern( "new" ), 1, attributes );
  rb_funcall( controller, rb_intern( "queue_get_config_reply" ), 1, queue_get_config_reply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */