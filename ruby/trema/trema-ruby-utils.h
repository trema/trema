/*
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


#ifndef TREMA_RUBY_UTILS_H
#define TREMA_RUBY_UTILS_H


#include <inttypes.h>
#include "ruby.h"
#include "buffer.h"


#if HAVE_RUBY_VERSION_H == 1
#include "ruby/version.h"
#endif


void set_xid( const buffer *openflow_message, uint32_t xid );
VALUE get_xid( VALUE self );
void set_length( const buffer *openflow_message, uint16_t length );
uint16_t get_length( const buffer *openflow_message );
void validate_xid( VALUE xid );


// Define Ruby 1.9 compatible wrappers for struct manipulation.
#if RUBY_VERSION_MAJOR == 1 && RUBY_VERSION_MINOR < 9


#ifndef RFLOAT_VALUE
#define RFLOAT_VALUE( v ) ( RFLOAT( v )->value )
#endif
#ifndef RARRAY_LEN
#define RARRAY_LEN( v ) ( RARRAY( v )->len )
#endif
#ifndef RARRAY_PTR
#define RARRAY_PTR( v ) ( RARRAY( v )->ptr )
#endif
#define RB_OBJ_IS_KIND_OF( v, t ) ( rb_obj_is_kind_of( v, t ) == Qtrue )
#define RB_OBJ_IS_INSTANCE_OF( v, t ) ( rb_obj_is_instance_of( v, t ) == Qtrue )
#define RB_RESPOND_TO( v, t ) ( rb_respond_to( v, t ) == Qtrue )


#else


#ifndef STR2CSTR
#define STR2CSTR( v ) ( StringValuePtr( v ) )
#endif
#define RB_OBJ_IS_KIND_OF( v, t ) ( rb_obj_is_kind_of( v, t ) )
#define RB_OBJ_IS_INSTANCE_OF( v, t ) ( rb_obj_is_instance_of( v, t ) )
#define RB_RESPOND_TO( v, t ) ( rb_respond_to( v, t ) )
#endif


#endif // TREMA_RUBY_UTILS_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
