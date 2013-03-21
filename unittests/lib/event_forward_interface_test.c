/*
 * Copyright (C) 2013 NEC Corporation
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


#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "event_forward_interface.h"
#include "trema.h"
#include "trema_private.h"
#include "trema_wrapper.h"
#include "cmockery_trema.h"


/********************************************************************************
 * Fwd decl for internal functions.
 ********************************************************************************/

struct event_forward_operation_to_all_request_param {
  bool add;
  enum efi_event_type type;
  char *service_name;
  event_forward_entry_to_all_callback callback;
  void *user_data;
};

typedef struct all_sw_tx {
  uint32_t txid;
  hash_table *waiting_dpid;
  enum efi_result tx_result;

  void *user_data;
} all_sw_tx;

struct txinfo {
  uint64_t dpid;
  uint32_t txid;
};

extern const char *_get_efi_queue_name( void );
extern void _get_switch_list_after_swm_succ( event_forward_operation_result result, void *user_data );
extern void _dispatch_to_all_switch( uint64_t *dpids, size_t n_dpids, void *user_data );
extern void _switch_response_handler( event_forward_operation_result result, void *user_data );
extern void _cleanup_tx_table();
extern all_sw_tx *_insert_tx( size_t n_dpids, struct event_forward_operation_to_all_request_param *param );
extern void _switch_response_timeout( void *user_data );

/********************************************************************************
 * Mock functions.
 ********************************************************************************/

struct callback_info {
  void *callback;
  void *user_data;
};


static void ( *original_error )( const char *format, ... );
static void ( *original_warn )( const char *format, ... );
static void ( *original_debug )( const char *format, ... );
static bool ( *original_add_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
static bool ( *original_add_message_replied_callback )( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) );
static bool ( *original_delete_message_replied_callback )( const char *service_name, void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) );
static bool ( *original_delete_message_requested_callback )( const char *service_name, void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) );
static const char *( *original_get_management_service_name )( const char *service_name );
static pid_t ( *original_getpid )( void );
static void ( *handle_efi_reply )( uint16_t tag, void *data, size_t length, void *user_data ) = NULL;
static bool ( *original_send_request_message )( const char *to_service_name, const char *from_service_name, const uint16_t tag, const void *data, size_t len, void *user_data );

static void
mock_error( const char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
}


static void
mock_warn( const char *format, ... ) {
  va_list args;
  va_start( args, format );
  char message[ 1000 ];
  vsprintf( message, format, args );
  va_end( args );

  check_expected( message );
}


static void
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


static bool
mock_add_message_requested_callback( const char *service_name,
                                     void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  UNUSED( callback );

  check_expected( service_name );

  return ( bool ) mock();
}


static bool
mock_add_message_replied_callback( const char *service_name,
                                   void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  check_expected( service_name );
  assert_true( callback != NULL );
  handle_efi_reply = callback;

  return ( bool ) mock();
}


static bool
mock_delete_message_replied_callback( const char *service_name,
                                      void ( *callback )( uint16_t tag, void *data, size_t len, void *user_data ) ) {
  check_expected( service_name );
  assert_true( callback != NULL );
  handle_efi_reply = NULL;

  return ( bool ) mock();
}


static bool
mock_delete_message_requested_callback( const char *service_name,
                                        void ( *callback )( const messenger_context_handle *handle, uint16_t tag, void *data, size_t len ) ) {
  UNUSED( callback );

  check_expected( service_name );

  return ( bool ) mock();
}


static const char *
mock_get_management_service_name( const char *service_name ) {
  check_expected( service_name );

  return ( const char * ) ( intptr_t ) mock();
}


static pid_t
mock_getpid( void ) {
  return 1234;
}


static bool free_user_data_member = false;


static bool
mock_send_request_message( const char *to_service_name, const char *from_service_name,
                           const uint16_t tag, const void *data, size_t len, void *user_data ) {
  uint32_t tag32 = tag;
  struct callback_info *hd = user_data;

  check_expected( to_service_name );
  check_expected( from_service_name );
  check_expected( tag32 );
  check_expected( data );
  check_expected( len );
  check_expected( hd->callback );
  check_expected( hd->user_data );

  bool sent_ok = ( bool ) mock();
  if ( sent_ok ) {
    if ( free_user_data_member ) {
      xfree( hd->user_data );
    }
    xfree( hd );
  }
  return sent_ok;
}


void mock_event_forward_entry_operation_callback( event_forward_operation_result result, void *user_data ) {
  check_expected( result.result );
  check_expected( result.n_services );
  for ( unsigned i = 0; i < result.n_services; ++i ) {
    const char *service = result.services[i];
    check_expected( service );
  }
  check_expected( user_data );
}


void mock_switch_list_request_callback( uint64_t *dpids, size_t n_dpids, void *user_data ) {
  check_expected( n_dpids );
  check_expected( dpids );
  check_expected( user_data );
}


void mock_event_forward_entry_to_all_callback( enum efi_result result, void *user_data ) {
  check_expected( result );
  check_expected( user_data );
}


/********************************************************************************
 * Setup and teardown functions.
 ********************************************************************************/

static void
setup() {
  free_user_data_member = false;

  original_error = error;
  error = mock_error;
  original_warn = warn;
  warn = mock_warn;
  original_debug = debug;
  debug = mock_debug;
  original_add_message_requested_callback= add_message_requested_callback;
  add_message_requested_callback = mock_add_message_requested_callback;
  original_delete_message_requested_callback= delete_message_requested_callback;
  delete_message_requested_callback = mock_delete_message_requested_callback;
  original_get_management_service_name = get_management_service_name;
  get_management_service_name = mock_get_management_service_name;
  original_getpid = trema_getpid;
  trema_getpid = mock_getpid;
  original_add_message_replied_callback = add_message_replied_callback;
  add_message_replied_callback = mock_add_message_replied_callback;
  original_delete_message_replied_callback = delete_message_replied_callback;
  delete_message_replied_callback = mock_delete_message_replied_callback;
  original_send_request_message = send_request_message;
  send_request_message = mock_send_request_message;
}


static void
setup_init_efi() {
  setup();

  const char service_name[] = "tetris";
  const char efi_queue_name[] = "tetris-efic-1234";
  set_trema_name( service_name );

  expect_string( mock_add_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_add_message_replied_callback, true );

  assert_true( _get_efi_queue_name() == NULL );
  assert_true( init_event_forward_interface() );
  assert_false( _get_efi_queue_name() == NULL );
}

static void
teardown() {
  bool *initialized = _get_management_interface_initialized();
  *initialized = false;

  error = original_error;
  warn = original_warn;
  debug = original_debug;
  add_message_requested_callback= original_add_message_requested_callback;
  delete_message_requested_callback= original_delete_message_requested_callback;
  get_management_service_name = original_get_management_service_name;
  trema_getpid = original_getpid;
  add_message_replied_callback = original_add_message_replied_callback;
  delete_message_replied_callback = original_delete_message_replied_callback;
  send_request_message = original_send_request_message;
}


static void
teardown_finl_efi() {
  const char efi_queue_name[] = "tetris-efic-1234";
  expect_string( mock_delete_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_delete_message_replied_callback, true );

  assert_true( finalize_event_forward_interface() );
  assert_true( _get_efi_queue_name() == NULL );

  _free_trema_name();
  teardown();
}
/********************************************************************************
 * init_event_forward_interface() tests.
 ********************************************************************************/

static void
test_init_event_forward_interface_succeeds() {
  const char service_name[] = "tetris";
  const char efi_queue_name[] = "tetris-efic-1234";

  set_trema_name( service_name );

  expect_string( mock_add_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_add_message_replied_callback, true );

  assert_true( _get_efi_queue_name() == NULL );
  assert_true( init_event_forward_interface() );
  assert_false( _get_efi_queue_name() == NULL );
  assert_string_equal( _get_efi_queue_name(), efi_queue_name );

  expect_string( mock_delete_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_delete_message_replied_callback, true );
  assert_true( finalize_event_forward_interface() );

  _free_trema_name();
}


static void
test_init_event_forward_interface_succeeds_even_with_long_name() {
  const char service_name[] = "tetris1234567890123456";
  const char efi_queue_name[] = "efic-1234";

  set_trema_name( service_name );

  expect_string( mock_add_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_add_message_replied_callback, true );

  assert_true( _get_efi_queue_name() == NULL );
  assert_true( init_event_forward_interface() );
  assert_false( _get_efi_queue_name() == NULL );
  assert_string_equal( _get_efi_queue_name(), efi_queue_name );

  expect_string( mock_delete_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_delete_message_replied_callback, true );
  assert_true( finalize_event_forward_interface() );

  _free_trema_name();
}

static void
test_init_event_forward_interface_fails_if_already_initialized() {
  const char service_name[] = "tetris";
  const char efi_queue_name[] = "tetris-efic-1234";

  set_trema_name( service_name );

  expect_string( mock_add_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_add_message_replied_callback, true );

  init_event_forward_interface();
  assert_false( _get_efi_queue_name() == NULL );
  assert_string_equal( _get_efi_queue_name(), efi_queue_name );

  expect_string( mock_warn, message, "already initialized." );

  assert_false( init_event_forward_interface() );

  assert_false( _get_efi_queue_name() == NULL );
  assert_string_equal( _get_efi_queue_name(), efi_queue_name );

  expect_string( mock_delete_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_delete_message_replied_callback, true );

  assert_true( finalize_event_forward_interface() );
  _free_trema_name();
}


/********************************************************************************
 * finalize_event_forward_interface() tests.
 ********************************************************************************/

static void
test_finalize_event_forward_interface_succeeds() {
  const char service_name[] = "tetris";
  const char efi_queue_name[] = "tetris-efic-1234";

  set_trema_name( service_name );

  expect_string( mock_add_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_add_message_replied_callback, true );
  assert_true( init_event_forward_interface() );


  expect_string( mock_delete_message_replied_callback, service_name, efi_queue_name );
  will_return( mock_delete_message_replied_callback, true );

  assert_true( finalize_event_forward_interface() );
  assert_true( _get_efi_queue_name() == NULL );

  _free_trema_name();
}


static void
test_finalize_event_forward_interface_fails_if_not_initialized() {
  expect_string( mock_warn, message, "already finalized." );

  assert_false( finalize_event_forward_interface() );
  assert_true( _get_efi_queue_name() == NULL );
}


/********************************************************************************
 * set_switch_manager_event_forward_entries() tests.
 ********************************************************************************/

static void
test_set_switch_manager_event_forward_entries_succeeds() {
  list_element *head;
  char alpha[] = "alpha-12345678901234567890";
  char bravo[] = "bravo-12345678901234567890";
  create_list( &head );
  append_to_tail( &head, alpha );
  append_to_tail( &head, bravo );

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 + 21 ];
    char bravo[ 6 + 21 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .n_services = htonl( 2 ),
      },
      .alpha = "alpha-12345678901234567890",
      .bravo = "bravo-12345678901234567890",
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( set_switch_manager_event_forward_entries( EVENT_FORWARD_TYPE_VENDOR, head, callback, user_data ) );

  delete_list( head );
}


/********************************************************************************
 * add_switch_manager_event_forward_entry() tests.
 ********************************************************************************/

static void
test_add_switch_manager_event_forward_entry_succeeds() {

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( add_switch_manager_event_forward_entry( EVENT_FORWARD_TYPE_PACKET_IN, "alpha", callback, user_data ) );
}


/********************************************************************************
 * delete_switch_manager_event_forward_entry() tests.
 ********************************************************************************/

static void
test_delete_switch_manager_event_forward_entry_succeeds() {

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_DELETE ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PORT_STATUS,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( delete_switch_manager_event_forward_entry( EVENT_FORWARD_TYPE_PORT_STATUS, "alpha", callback, user_data ) );
}


/********************************************************************************
 * dump_switch_manager_event_forward_entries() tests.
 ********************************************************************************/

static void
test_dump_switch_manager_event_forward_entries_succeeds() {

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_DUMP ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_STATE_NOTIFY,
          .n_services = htonl( 0 ),
      },
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( dump_switch_manager_event_forward_entries( EVENT_FORWARD_TYPE_STATE_NOTIFY, callback, user_data ) );
}


/********************************************************************************
 * set_switch_event_forward_entries() tests.
 ********************************************************************************/

static void
test_set_switch_event_forward_entries_succeeds() {
  list_element *head;
  char alpha[] = "alpha";
  char bravo[] = "bravo";
  create_list( &head );
  append_to_tail( &head, alpha );
  append_to_tail( &head, bravo );

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
    char bravo[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .n_services = htonl( 2 ),
      },
      .alpha = "alpha",
      .bravo = "bravo",
  };

  expect_string( mock_send_request_message, to_service_name, "switch.0xabc.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( set_switch_event_forward_entries( 0xabc, EVENT_FORWARD_TYPE_VENDOR, head, callback, user_data ) );

  delete_list( head );
}


/********************************************************************************
 * add_switch_event_forward_entry() tests.
 ********************************************************************************/

static void
test_add_switch_event_forward_entry_succeeds() {

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch.0xabc.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( add_switch_event_forward_entry( 0xabc, EVENT_FORWARD_TYPE_PACKET_IN, "alpha", callback, user_data ) );
}


/********************************************************************************
 * delete_switch_event_forward_entry() tests.
 ********************************************************************************/

static void
test_delete_switch_event_forward_entry_succeeds() {

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_DELETE ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PORT_STATUS,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch.0xabc.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( delete_switch_event_forward_entry( 0xabc, EVENT_FORWARD_TYPE_PORT_STATUS, "alpha", callback, user_data ) );
}


/********************************************************************************
 * dump_switch_event_forward_entries() tests.
 ********************************************************************************/

static void
test_dump_switch_event_forward_entries_succeeds() {

  event_forward_entry_operation_callback callback = ( event_forward_entry_operation_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_DUMP ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_STATE_NOTIFY,
          .n_services = htonl( 0 ),
      },
  };

  expect_string( mock_send_request_message, to_service_name, "switch.0xabc.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( dump_switch_event_forward_entries( 0xabc, EVENT_FORWARD_TYPE_STATE_NOTIFY, callback, user_data ) );
}


/********************************************************************************
 * handle_efi_reply() tests.
 ********************************************************************************/

static void
test_handle_efi_reply_succeeds_with_success_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
    char alpha[ 6 ];
    char bravo[ 6 ];
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = EFI_OPERATION_SUCCEEDED,
          .n_services = htonl( 2 ),
      },
      .alpha = "alpha",
      .bravo = "bravo"
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_value( mock_event_forward_entry_operation_callback, result.result, EFI_OPERATION_SUCCEEDED );
  expect_value( mock_event_forward_entry_operation_callback, result.n_services, 2  );
  expect_string( mock_event_forward_entry_operation_callback, service, "alpha" );
  expect_string( mock_event_forward_entry_operation_callback, service, "bravo" );
  expect_value( mock_event_forward_entry_operation_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


static void
test_handle_efi_reply_succeeds_with_event_forward_entry_operation_failure_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = EFI_OPERATION_FAILED,
          .n_services = htonl( 0 ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_value( mock_event_forward_entry_operation_callback, result.result, EFI_OPERATION_FAILED );
  expect_value( mock_event_forward_entry_operation_callback, result.n_services, 0  );
  expect_value( mock_event_forward_entry_operation_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


static void
test_handle_efi_reply_succeeds_with_management_failure_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_FAILED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = EFI_OPERATION_FAILED,
          .n_services = htonl( 0 ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_warn, message, "Management request failed." );

  expect_value( mock_event_forward_entry_operation_callback, result.result, EFI_OPERATION_FAILED );
  expect_value( mock_event_forward_entry_operation_callback, result.n_services, 0  );
  expect_value( mock_event_forward_entry_operation_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


static void
test_handle_efi_reply_ignores_wrong_message_tag_reply() {

  struct input_data {
    management_application_reply mgmt;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_FAILED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_error, message, "Unexpected message received tag=0x1234" );

  handle_efi_reply( 0x1234, &input_data, sizeof( struct input_data ), user_data );
  xfree( user_data );
}


static void
test_handle_efi_reply_ignores_wrong_message_length_reply() {

  struct input_data {
    management_application_reply mgmt;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_FAILED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_SET ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_error, message, "Data length too short 11. expecting >= 12" );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data )-1, user_data );
  xfree( user_data );
}


static void
test_handle_efi_reply_ignores_empty_reply() {

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_any( mock_error, message );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, NULL, 0, user_data );
  xfree( user_data );
}


static void
test_handle_efi_reply_ignores_wrong_event_forward_operation_command_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( 0x1234 ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = EFI_OPERATION_FAILED,
          .n_services = htonl( 0 ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_warn, message, "Invalid command/application_id: 0x1234" );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
  xfree( user_data );
}


static void
test_handle_efi_reply_succeeds_with_wrong_event_forward_operation_result_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = 0xFF,
          .n_services = htonl( 0 ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_warn, message, "Unknown result type 0xff. Translating as FAILED." );

  expect_value( mock_event_forward_entry_operation_callback, result.result, EFI_OPERATION_FAILED );
  expect_value( mock_event_forward_entry_operation_callback, result.n_services, 0  );
  expect_value( mock_event_forward_entry_operation_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


static void
test_handle_efi_reply_succeeds_with_more_service_found_then_expected_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
    char alpha[ 6 ];
    char bravo[ 6 ];
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = EFI_OPERATION_SUCCEEDED,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
      .bravo = "bravo"
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_warn, message, "Expected 1 name(s), but found more service name. Ignoring 'bravo'." );

  expect_value( mock_event_forward_entry_operation_callback, result.result, EFI_OPERATION_SUCCEEDED );
  expect_value( mock_event_forward_entry_operation_callback, result.n_services, 1  );
  expect_string( mock_event_forward_entry_operation_callback, service, "alpha" );
  expect_value( mock_event_forward_entry_operation_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


static void
test_handle_efi_reply_succeeds_with_empty_service_name_reply() {

  struct input_data {
    management_application_reply mgmt;
    event_forward_operation_reply efi;
    char alpha[ 6 ];
    char bravo[ 1 ];
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_VENDOR,
          .result = EFI_OPERATION_SUCCEEDED,
          .n_services = htonl( 2 ),
      },
      .alpha = "alpha",
      .bravo = ""
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_event_forward_entry_operation_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_warn, message, "Encountered empty service name." );

  expect_value( mock_event_forward_entry_operation_callback, result.result, EFI_OPERATION_SUCCEEDED );
  expect_value( mock_event_forward_entry_operation_callback, result.n_services, 1  );
  expect_string( mock_event_forward_entry_operation_callback, service, "alpha" );
  expect_value( mock_event_forward_entry_operation_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


/********************************************************************************
 * create_event_forward_op_reply() tests.
 ********************************************************************************/

static void
test_create_event_forward_operation_reply_with_null_service_list() {
  struct expected_data {
    event_forward_operation_reply efi;
  } __attribute__( ( packed ) ) expected_data = {
    .efi = {
        .type = EVENT_FORWARD_TYPE_PACKET_IN,
        .result = EFI_OPERATION_SUCCEEDED,
        .n_services = 0
    }
  };
  buffer *buf = create_event_forward_operation_reply( EVENT_FORWARD_TYPE_PACKET_IN, EFI_OPERATION_SUCCEEDED, NULL );

  assert_memory_equal( buf->data, &expected_data, sizeof( struct expected_data ) );
  free_buffer( buf );
}


static void
test_create_event_forward_operation_reply_with_empty_service_list() {
  struct expected_data {
    event_forward_operation_reply efi;
  } __attribute__( ( packed ) ) expected_data = {
    .efi = {
        .type = EVENT_FORWARD_TYPE_PACKET_IN,
        .result = EFI_OPERATION_SUCCEEDED,
        .n_services = 0
    }
  };
  list_element *head;
  create_list( &head );
  buffer *buf = create_event_forward_operation_reply( EVENT_FORWARD_TYPE_PACKET_IN, EFI_OPERATION_SUCCEEDED, head );

  assert_memory_equal( buf->data, &expected_data, sizeof( struct expected_data ) );
  free_buffer( buf );
  delete_list( head );
}


static void
test_create_event_forward_operation_reply_with_one_service_list() {
  char alpha[] = "alpha";
  struct expected_data {
    event_forward_operation_reply efi;
    const char service[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
    .efi = {
        .type = EVENT_FORWARD_TYPE_PACKET_IN,
        .result = EFI_OPERATION_SUCCEEDED,
        .n_services = htonl( 1 )
    },
    .service = "alpha"
  };
  list_element *head;
  create_list( &head );
  append_to_tail( &head, alpha );
  buffer *buf = create_event_forward_operation_reply( EVENT_FORWARD_TYPE_PACKET_IN, EFI_OPERATION_SUCCEEDED, head );

  assert_memory_equal( buf->data, &expected_data, sizeof( struct expected_data ) );
  free_buffer( buf );
  delete_list( head );
}


static void
test_create_event_forward_operation_reply_with_multi_service_list() {
  char alpha[] = "alpha-12345678901234567890";
  char bravo[] = "bravo-12345678901234567890";
  char charlie[] = "charlie-12345678901234567890";
  struct expected_data {
    event_forward_operation_reply efi;
    const char alpha[ 6 + 21 ];
    const char bravo[ 6 + 21 ];
    const char charlie[ 8 + 21 ];
  } __attribute__( ( packed ) ) expected_data = {
    .efi = {
        .type = EVENT_FORWARD_TYPE_PACKET_IN,
        .result = EFI_OPERATION_SUCCEEDED,
        .n_services = htonl( 3 )
    },
    .alpha = "alpha-12345678901234567890",
    .bravo = "bravo-12345678901234567890",
    .charlie = "charlie-12345678901234567890"
  };
  list_element *head;
  create_list( &head );
  append_to_tail( &head, alpha );
  append_to_tail( &head, bravo );
  append_to_tail( &head, charlie );
  buffer *buf = create_event_forward_operation_reply( EVENT_FORWARD_TYPE_PACKET_IN, EFI_OPERATION_SUCCEEDED, head );

  assert_memory_equal( buf->data, &expected_data, sizeof( struct expected_data ) );
  free_buffer( buf );
  delete_list( head );
}


/********************************************************************************
 * send_efi_switch_list_request() tests.
 ********************************************************************************/

static void
test_send_efi_switch_list_request_succeeds() {

  switch_list_request_callback callback = ( switch_list_request_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EFI_GET_SWLIST ),
      },
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, callback );
  expect_value( mock_send_request_message, hd->user_data, user_data );
  will_return( mock_send_request_message, true );

  assert_true( send_efi_switch_list_request( callback, user_data ) );
}


static void
test_handle_efi_reply_succeeds_with_switch_list_reply() {

  const uint64_t dpids_host[] = { 0x12345678, 0xabcdef00 };
  struct input_data {
    management_application_reply mgmt;
    uint64_t dpids[ 2 ];
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_SUCCEEDED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EFI_GET_SWLIST ),
      },
      .dpids = {
          htonll( dpids_host[ 0 ] ),
          htonll( dpids_host[ 1 ] )
      }
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_switch_list_request_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_value( mock_switch_list_request_callback, n_dpids, 2 );
  expect_memory( mock_switch_list_request_callback, dpids, dpids_host, 2 * sizeof( uint64_t ) );
  expect_value( mock_switch_list_request_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


static void
test_handle_efi_reply_succeeds_with_management_failure_on_switch_list_reply() {

  struct input_data {
    management_application_reply mgmt;
  } __attribute__( ( packed ) ) input_data = {
      .mgmt = {
          .header = {
              .type = htons( MANAGEMENT_APPLICATION_REPLY ),
              .status = MANAGEMENT_REQUEST_FAILED,
              .length = htonl( sizeof( struct input_data ) ),
          },
          .application_id = htonl( EFI_GET_SWLIST ),
      },
  };

  struct callback_info *user_data = xcalloc( 1, sizeof( struct callback_info ) );
  user_data->callback = mock_switch_list_request_callback;
  user_data->user_data = ( void * ) 0x12345678;

  expect_string( mock_warn, message, "Management request failed." );

  expect_value( mock_switch_list_request_callback, n_dpids, 0 );
  expect_value( mock_switch_list_request_callback, dpids, NULL );
  expect_value( mock_switch_list_request_callback, user_data, 0x12345678 );

  handle_efi_reply( MESSENGER_MANAGEMENT_REPLY, &input_data, sizeof( struct input_data ), user_data );
}


/********************************************************************************
 * add_event_forward_to_all_switches() tests.
 ********************************************************************************/


static void
test_add_event_forward_entry_to_all_switches_succeeds() {

  event_forward_entry_to_all_callback callback = ( event_forward_entry_to_all_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, _get_switch_list_after_swm_succ );
  expect_any( mock_send_request_message, hd->user_data );

  // using send fail pattern to avoid all_request_param leak warning
  will_return( mock_send_request_message, false );
  assert_false( add_event_forward_entry_to_all_switches( EVENT_FORWARD_TYPE_PACKET_IN, "alpha", callback, user_data ) );
}


static void
test_delete_event_forward_entry_to_all_switches_succeeds() {

  event_forward_entry_to_all_callback callback = ( event_forward_entry_to_all_callback ) 0x12345678;
  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_DELETE ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, _get_switch_list_after_swm_succ );
  expect_any( mock_send_request_message, hd->user_data );

  // using send fail pattern to avoid all_request_param leak warning
  will_return( mock_send_request_message, false );
  assert_false( delete_event_forward_entry_to_all_switches( EVENT_FORWARD_TYPE_PACKET_IN, "alpha", callback, user_data ) );
}


static void
test__get_switch_list_after_swm_succ_succeeds() {

  void *user_data = ( void * ) 0xABCDEF;

  struct expected_data {
    management_application_request mgmt;
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EFI_GET_SWLIST ),
      },
  };

  struct event_forward_operation_to_all_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_to_all_request_param ) );
  param->add = true;
  param->type = EVENT_FORWARD_TYPE_PACKET_IN;
  param->service_name = xstrdup( "alpha" );
  param->callback = mock_event_forward_entry_to_all_callback;
  param->user_data = user_data;

  event_forward_operation_result result;
  result.result = EFI_OPERATION_SUCCEEDED;
  result.n_services = 0;
  result.services = NULL;

  expect_string( mock_send_request_message, to_service_name, "switch_manager.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, _dispatch_to_all_switch );
  expect_value( mock_send_request_message, hd->user_data, param );
  will_return( mock_send_request_message, true );

  _get_switch_list_after_swm_succ( result, param );

  xfree( param->service_name );
  xfree( param );
}


static void
test__dispatch_to_all_switch_succeeds() {
  init_timer();
  uint64_t dpids[] = { 0x12345678 };
  const size_t n_dpids = 1;

  void *user_data = ( void * ) 0xABCDEF;

  struct event_forward_operation_to_all_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_to_all_request_param ) );
  param->add = true;
  param->type = EVENT_FORWARD_TYPE_PACKET_IN;
  param->service_name = xstrdup( "alpha" );
  param->callback = mock_event_forward_entry_to_all_callback;
  param->user_data = user_data;


  struct expected_data {
    management_application_request mgmt;
    event_forward_operation_request efi;
    char alpha[ 6 ];
  } __attribute__( ( packed ) ) expected_data = {
      .mgmt = {
        .header = {
            .type = htons( MANAGEMENT_APPLICATION_REQUEST ),
            .length = htonl( sizeof( struct expected_data ) ),
        },
        .application_id = htonl( EVENT_FORWARD_ENTRY_ADD ),
      },
      .efi = {
          .type = EVENT_FORWARD_TYPE_PACKET_IN,
          .n_services = htonl( 1 ),
      },
      .alpha = "alpha",
  };

  expect_string( mock_send_request_message, to_service_name, "switch.0x12345678.m" );
  expect_string( mock_send_request_message, from_service_name, "tetris-efic-1234" );
  expect_value( mock_send_request_message, tag32, MESSENGER_MANAGEMENT_REQUEST );
  expect_memory( mock_send_request_message, data, &expected_data, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, len, sizeof( struct expected_data ) );
  expect_value( mock_send_request_message, hd->callback, _switch_response_handler );
  expect_any( mock_send_request_message, hd->user_data ); // txinfo
  will_return( mock_send_request_message, true );

  free_user_data_member = true;
  _dispatch_to_all_switch( dpids, n_dpids, param );

  // free all_sw_tx, param
  expect_string( mock_warn, message, "txid:0x1 was left behind." );
  _cleanup_tx_table();

  finalize_timer();
}


static void
test__switch_response_handler_succeeds_when_last_one_standing() {

  void *user_data = ( void * ) 0x1234;

  struct event_forward_operation_to_all_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_to_all_request_param ) );
  param->add = true;
  param->type = EVENT_FORWARD_TYPE_PACKET_IN;
  param->service_name = xstrdup( "alpha" );
  param->callback = mock_event_forward_entry_to_all_callback;
  param->user_data = user_data;

  all_sw_tx *tx = _insert_tx( 1, param );
  uint64_t *dpid = xmalloc( sizeof( uint64_t ) );
  *dpid = 0x12345678;
  insert_hash_entry( tx->waiting_dpid, dpid, dpid );

  event_forward_operation_result result;
  result.result = EFI_OPERATION_SUCCEEDED;
  result.n_services = 0;
  result.services = NULL;

  struct txinfo *txinfo = xmalloc( sizeof( struct txinfo ) );
  txinfo->dpid = 0x12345678;
  txinfo->txid = tx->txid;

  expect_value( mock_event_forward_entry_to_all_callback, result, EFI_OPERATION_SUCCEEDED );
  expect_value( mock_event_forward_entry_to_all_callback, user_data, user_data );

  _switch_response_handler( result, txinfo );
}


static void
test__switch_response_timeout_then_fails() {

  void *user_data = ( void * ) 0x1234;

  struct event_forward_operation_to_all_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_to_all_request_param ) );
  param->add = true;
  param->type = EVENT_FORWARD_TYPE_PACKET_IN;
  param->service_name = xstrdup( "alpha" );
  param->callback = mock_event_forward_entry_to_all_callback;
  param->user_data = user_data;

  all_sw_tx *tx = _insert_tx( 1, param );
  uint64_t *dpid = xmalloc( sizeof( uint64_t ) );
  *dpid = 0x12345678;
  insert_hash_entry( tx->waiting_dpid, dpid, dpid );

  struct txinfo *txinfo = xmalloc( sizeof( struct txinfo ) );
  txinfo->dpid = 0x12345678;
  txinfo->txid = tx->txid;

  expect_value( mock_event_forward_entry_to_all_callback, result, EFI_OPERATION_FAILED );
  expect_value( mock_event_forward_entry_to_all_callback, user_data, user_data );

  _switch_response_timeout( txinfo );
  xfree( txinfo );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_init_event_forward_interface_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_init_event_forward_interface_succeeds_even_with_long_name,
                              setup, teardown ),
    unit_test_setup_teardown( test_init_event_forward_interface_fails_if_already_initialized,
                              setup, teardown ),

    unit_test_setup_teardown( test_finalize_event_forward_interface_succeeds,
                              setup, teardown ),
    unit_test_setup_teardown( test_finalize_event_forward_interface_fails_if_not_initialized,
                              setup, teardown ),

    unit_test_setup_teardown( test_set_switch_manager_event_forward_entries_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_add_switch_manager_event_forward_entry_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_delete_switch_manager_event_forward_entry_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_dump_switch_manager_event_forward_entries_succeeds,
                              setup_init_efi, teardown_finl_efi ),

    unit_test_setup_teardown( test_set_switch_event_forward_entries_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_add_switch_event_forward_entry_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_delete_switch_event_forward_entry_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_dump_switch_event_forward_entries_succeeds,
                              setup_init_efi, teardown_finl_efi ),

    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_success_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_event_forward_entry_operation_failure_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_management_failure_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_ignores_wrong_message_tag_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_ignores_wrong_message_length_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_ignores_empty_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_ignores_wrong_event_forward_operation_command_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_wrong_event_forward_operation_result_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_more_service_found_then_expected_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_empty_service_name_reply,
                              setup_init_efi, teardown_finl_efi ),

    unit_test_setup_teardown( test_create_event_forward_operation_reply_with_null_service_list,
                              setup, teardown ),
    unit_test_setup_teardown( test_create_event_forward_operation_reply_with_empty_service_list,
                              setup, teardown ),
    unit_test_setup_teardown( test_create_event_forward_operation_reply_with_one_service_list,
                              setup, teardown ),
    unit_test_setup_teardown( test_create_event_forward_operation_reply_with_multi_service_list,
                              setup, teardown ),

    unit_test_setup_teardown( test_send_efi_switch_list_request_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_switch_list_reply,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_handle_efi_reply_succeeds_with_management_failure_on_switch_list_reply,
                              setup_init_efi, teardown_finl_efi ),


    unit_test_setup_teardown( test_add_event_forward_entry_to_all_switches_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test_delete_event_forward_entry_to_all_switches_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test__get_switch_list_after_swm_succ_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test__dispatch_to_all_switch_succeeds,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test__switch_response_handler_succeeds_when_last_one_standing,
                              setup_init_efi, teardown_finl_efi ),
    unit_test_setup_teardown( test__switch_response_timeout_then_fails,
                              setup_init_efi, teardown_finl_efi ),
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
