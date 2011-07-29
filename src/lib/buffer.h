/*
 * Buffer library.
 *
 * Author: Shin-ya Zenke
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


/**
 * @file buffer.h
 * This file contains function and type declarations for functions defined in buffer.c file
 * @see buffer.c
 */

#ifndef BUFFER_H
#define BUFFER_H


#include <stddef.h>


/**
 * This type is used for representing an allocated space. It holds the pointer to the space on which
 * data can be stored, and has members to define the length of data. It can be used either in local
 * context (data allocation and usage) or for User's data.
 */
typedef struct buffer {
  void *data;
  size_t length;
  void *user_data;
} buffer;


buffer *alloc_buffer( void );
buffer *alloc_buffer_with_length( size_t length );
void free_buffer( buffer *buf );
void *append_front_buffer( buffer *buf, size_t length );
void *remove_front_buffer( buffer *buf, size_t length );
void *append_back_buffer( buffer *buf, size_t length );
buffer *duplicate_buffer( const buffer *buf );
void dump_buffer( const buffer *buf, void dump_function( const char *format, ... ) );


#endif // BUFFER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
