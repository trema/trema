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
#include <openflow.h>
#include "trema.h"
#include "ruby.h"

extern VALUE mTrema;
VALUE cSetConfig;

static VALUE
set_config_alloc( VALUE klass ) {
  buffer *set_config = create_set_config( get_transaction_id( ), OFPC_FRAG_NORMAL, OFP_DEFAULT_MISS_SEND_LEN );
  return Data_Wrap_Struct( klass, NULL, free_buffer, set_config );
}

static VALUE
set_config_init( int argc, VALUE *argv, VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );

  VALUE xid_ruby, flags_ruby, miss_send_len_ruby;
  uint32_t xid;
  uint16_t flags, miss_send_len;

  if ( rb_scan_args( argc, argv, "03", &xid_ruby, &flags_ruby, &miss_send_len_ruby ) == 3 ) {
    xid = NUM2UINT( xid_ruby );
    flags = ( uint16_t ) NUM2UINT( flags_ruby );
    miss_send_len = ( uint16_t ) NUM2UINT( miss_send_len_ruby );
  } else {
    xid = get_transaction_id( );
    flags = OFPC_FRAG_NORMAL;
    miss_send_len = OFP_DEFAULT_MISS_SEND_LEN;
  }
  ( ( struct ofp_header * ) ( set_config->data ) )->xid = htonl( xid );
  ( ( struct ofp_switch_config * ) ( set_config->data ) )->flags = htons( flags );
  ( ( struct ofp_switch_config * ) ( set_config->data ) )->miss_send_len = htons( miss_send_len );
  return self;
}

static VALUE
set_config_transaction_id( VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );
  uint32_t xid = ntohl( ( ( struct ofp_header * ) ( set_config->data ) )->xid );
  return UINT2NUM( xid );
}

static VALUE
set_config_flags( VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );
  uint16_t flags = ntohs( ( ( struct ofp_switch_config * ) ( set_config->data ) )->flags );
  return UINT2NUM( flags );
}

static VALUE
set_config_miss_send_len( VALUE self ) {
  buffer *set_config;
  Data_Get_Struct( self, buffer, set_config );
  uint16_t miss_send_len = ntohs( ( ( struct ofp_switch_config * ) ( set_config->data ) )->miss_send_len );
  return UINT2NUM( miss_send_len );
}

void
Init_set_config( ) {
  cSetConfig = rb_define_class_under( mTrema, "SetConfig", rb_cObject );
  rb_define_alloc_func( cSetConfig, set_config_alloc );
  rb_define_method( cSetConfig, "initialize", set_config_init, -1 );
  rb_define_method( cSetConfig, "transaction_id", set_config_transaction_id, 0 );
  rb_define_method( cSetConfig, "flags", set_config_flags, 0 );
  rb_define_method( cSetConfig, "miss_send_len", set_config_miss_send_len, 0 );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
