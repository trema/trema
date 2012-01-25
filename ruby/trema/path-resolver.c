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


#include "ruby.h"
#include "trema.h"
#include "libpathresolver.h"


extern VALUE mTrema;
VALUE mPathResolver;
VALUE cPathResolverHop;
pathresolver *handle;


static VALUE
init_path_resolver( VALUE self ) {
  handle = create_pathresolver();
  return self;
}


#if 0
static VALUE
pathresolver_hop_init( VALUE kclass ) {
  pathresolver_hop *_pathresolver_hop = xmalloc( sizeof( pathresolver_hop ) );
  return Data_Wrap_Struct( klass, 0, xfree, _pathresolver_hop );
}
#endif


static VALUE
pathresolver_hop_alloc( VALUE kclass ) {
  pathresolver_hop *_pathresolver_hop = xmalloc( sizeof( pathresolver_hop ) );
  return Data_Wrap_Struct( kclass, 0, xfree, _pathresolver_hop );
}


static pathresolver_hop *
get_pathresolver_hop( VALUE self ) {
  pathresolver_hop *_pathresolver_hop;
  Data_Get_Struct( self, pathresolver_hop, _pathresolver_hop );
  return _pathresolver_hop;
}


static VALUE
pathresolver_hop_dpid( VALUE self ) {
  return ULL2NUM( get_pathresolver_hop( self )->dpid );
}


static VALUE
pathresolver_hop_in_port_no( VALUE self ) {
  return UINT2NUM( get_pathresolver_hop( self )->in_port_no );
}


static VALUE
pathresolver_hop_out_port_no( VALUE self ) {
  return UINT2NUM( get_pathresolver_hop( self )->out_port_no );
}


static VALUE
path_resolve( VALUE self, VALUE in_dpid, VALUE in_port, VALUE out_dpid_id, VALUE out_port  ) {
  UNUSED( self );

  if ( handle != NULL ) {
    dlist_element *hops = resolve_path( handle, NUM2ULL( in_dpid ), ( uint16_t ) NUM2UINT( in_port ), NUM2ULL( out_dpid_id ), ( uint16_t ) NUM2UINT( out_port ) );
    if ( hops != NULL ) {
      VALUE pathresolver_hops_arr = rb_ary_new();
      for ( dlist_element *e = hops; e != NULL; e = e->next ) {
        VALUE message = rb_funcall( cPathResolverHop, rb_intern( "new" ), 0 );
        pathresolver_hop *tmp = NULL;
        Data_Get_Struct( message, pathresolver_hop, tmp );
        memcpy( tmp, e->data,  sizeof( pathresolver_hop ) );
        rb_ary_push( pathresolver_hops_arr, message );
      }
      return pathresolver_hops_arr;
    }
    else {
      return Qnil;
    }
  }
  return Qnil;
}


#ifdef TEST
static VALUE
update_path( VALUE self, VALUE from_dpid, VALUE to_dpid, VALUE from_portno, VALUE to_portno, VALUE status ) {
  topology_link_status *link_status = xmalloc( sizeof( topology_link_status ) );

  link_status->from_dpid = NUM2ULL( from_dpid );
  link_status->to_dpid = NUM2ULL( to_dpid );
  link_status->from_portno = ( uint16_t ) NUM2UINT( from_portno );
  link_status->to_portno = ( uint16_t ) NUM2UINT( to_portno );
  link_status->status = ( uint8_t ) NUM2UINT( status );
  update_topology( handle, link_status );
  xfree( link_status );
  return self;
}
#endif


static VALUE
update_path( VALUE self, VALUE message ) {
  topology_link_status *_topology_link_status;
  Data_Get_Struct( message, topology_link_status, _topology_link_status );
  update_topology( handle, _topology_link_status );
  return self;
}


void
Init_path_resolver() {
  mPathResolver = rb_define_module_under( mTrema, "PathResolver" );
  cPathResolverHop = rb_define_class_under( mPathResolver, "HopInfo", rb_cObject );
  rb_define_alloc_func( cPathResolverHop, pathresolver_hop_alloc );
#if 0
  rb_define_method( cPathResolverHop "initialize", pathresolver_hop_init, 0 );
#endif
  rb_define_method( cPathResolverHop, "dpid", pathresolver_hop_dpid, 0 );
  rb_define_method( cPathResolverHop, "in_port_no", pathresolver_hop_in_port_no, 0 );
  rb_define_method( cPathResolverHop, "out_port_no", pathresolver_hop_out_port_no, 0 );

  rb_define_method( mPathResolver, "init_path_resolver", init_path_resolver, 0 );
  rb_define_method( mPathResolver, "path_resolve", path_resolve, 4 );
  rb_define_method( mPathResolver, "update_path", update_path, 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
