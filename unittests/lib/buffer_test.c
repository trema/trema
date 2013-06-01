/*
 * Unit tests for buf functions and macros.
 *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "checks.h"
#include "cmockery_trema.h"
#include "openflow.h"
#include "packet_info.h"


typedef struct tea {
  const char *name;
  const char *origin;
} tea;

static tea CEYLON = { "Ceylon", "Sri Lanka" };
static tea DARJEELING = { "Darjeeling", "India" };


typedef struct private_buffer {
  buffer public;
  size_t real_length;
  void *top;
  pthread_mutex_t *mutex;
} private_buffer;


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_alloc_buffer_succeeds() {
  buffer *buf = alloc_buffer();

  assert_true( buf != NULL );

  free_buffer( buf );
}


static void
test_alloc_buffer_with_length_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );
  free_buffer( buf );
}


static void
test_free_buffer_succeeds() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );
  free_buffer( buf );
}


static void
test_append_front_buffer_new_alloc_succeeds() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );
  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );
  free_buffer( buf );
}


static void
test_append_front_twice_suceeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  memcpy( data_pointer, &CEYLON, sizeof( tea ) );

  data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  tea *tea_data = ( tea * ) ( ( char * ) data_pointer + sizeof( tea ) );
  assert_true( 0 == strcmp( tea_data->name, CEYLON.name ) );

  free_buffer( buf );
}


static void
test_append_front_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  free_buffer( buf );
}


static void
test_append_front_buffer_resize_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  void *data_pointer = append_front_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  free_buffer( buf );
}


static void
test_remove_front_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  void *data_pointer = append_front_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  data_pointer = remove_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  free_buffer( buf );
}


static void
test_remove_front_buffer_text_insert_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  void *data_pointer = append_front_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  memcpy( ( char * ) data_pointer + sizeof( tea ), &CEYLON, sizeof( tea ) );

  data_pointer = remove_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );
  tea *tea_data = ( tea * ) ( ( char * ) buf->data );
  assert_true( 0 == strcmp( tea_data->name, CEYLON.name ) );

  free_buffer( buf );
}


static void
test_remove_front_buffer_all_removed() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  void *data_pointer = append_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( data_pointer == buf->data );

  data_pointer = remove_front_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == 0 );

  free_buffer( buf );
}


static void
test_append_back_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 2 );
  assert_true( buf != NULL );

  void *data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  free_buffer( buf );
}


static void
test_append_back_twice_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) * 3 );
  assert_true( buf != NULL );

  void *data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  memcpy( data_pointer, &CEYLON, sizeof( tea ) );

  data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  memcpy( data_pointer, &DARJEELING, sizeof( tea ) );

  tea *tea_data = ( tea * ) ( ( char * ) buf->data );
  assert_true( 0 == strcmp( tea_data->name, CEYLON.name ) );
  tea_data = ( tea * ) ( ( char * ) buf->data + sizeof( tea ) );
  assert_true( 0 == strcmp( tea_data->name, DARJEELING.name ) );

  free_buffer( buf );
}


static void
test_append_back_buffer_resize_succeeds() {
  buffer *buf = alloc_buffer_with_length( sizeof( tea ) );
  assert_true( buf != NULL );

  void *data_pointer = append_back_buffer( buf, sizeof( tea ) * 2 );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) * 2 );

  free_buffer( buf );
}


static void
test_append_back_buffer_succeeds_if_initialize_length_is_0() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );

  void *data_pointer = append_back_buffer( buf, sizeof( tea ) );
  assert_true( data_pointer != NULL );
  assert_true( buf->length == sizeof( tea ) );

  free_buffer( buf );
}


static void
test_duplicate_buffer_succeeds() {
  buffer *buf = alloc_buffer_with_length( 1024 );
  assert_true( buf != NULL );
  unsigned char *data = append_back_buffer( buf, 1024 );
  int i;
  for ( i = 0; i < 1024; i++ ) {
    data[ i ] = ( unsigned char ) ( i % 0xff );
  }

  buffer *duplicate = duplicate_buffer( buf );
  assert_true( duplicate != NULL );
  assert_true( duplicate->user_data == buf->user_data );
  assert_true( duplicate->length == buf->length );
  assert_memory_equal( duplicate->data, buf->data, 1024 );

  free_buffer( buf );
  free_buffer( duplicate );
}


static void
test_duplicate_buffer_succeeds_if_initialize_length_is_0() {
  buffer *buf = alloc_buffer();
  assert_true( buf != NULL );
  assert_true( buf->data == NULL );

  buffer *duplicate = duplicate_buffer( buf );
  assert_true( duplicate != NULL );
  assert_true( duplicate->data == NULL );
  assert_true( duplicate->user_data == NULL );
  assert_true( duplicate->length == 0 );

  free_buffer( buf );
  free_buffer( duplicate );
}


static void
dump_function( const char *format, ... ) {
  char hex[ 1000 ];

  va_list args;
  va_start( args, format );
  vsnprintf( hex, sizeof( hex ), format, args );
  va_end( args );

  check_expected( hex );
}


static void
test_dump_buffer() {
  buffer *buf = alloc_buffer();

  void *datap = append_back_buffer( buf, ( size_t ) 1 );
  int data255 = 255;
  memcpy( datap, &data255, ( size_t ) 1 );

  expect_string( dump_function, hex, "ff" );
  dump_buffer( buf, dump_function );

  free_buffer( buf );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test( test_alloc_buffer_succeeds ),
    unit_test( test_alloc_buffer_with_length_succeeds ),

    unit_test( test_free_buffer_succeeds ),

    unit_test( test_append_front_twice_suceeds ),
    unit_test( test_append_front_buffer_succeeds ),
    unit_test( test_append_front_buffer_resize_succeeds ),
    unit_test( test_append_front_buffer_new_alloc_succeeds ),

    unit_test( test_remove_front_buffer_succeeds ),
    unit_test( test_remove_front_buffer_text_insert_succeeds ),
    unit_test( test_remove_front_buffer_all_removed ),

    unit_test( test_append_back_buffer_succeeds ),
    unit_test( test_append_back_buffer_resize_succeeds ),
    unit_test( test_append_back_buffer_succeeds_if_initialize_length_is_0 ),
    unit_test( test_append_back_twice_succeeds ),

    unit_test( test_duplicate_buffer_succeeds ),
    unit_test( test_duplicate_buffer_succeeds_if_initialize_length_is_0 ),

    unit_test( test_dump_buffer ),
  };
  setup_leak_detector();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
