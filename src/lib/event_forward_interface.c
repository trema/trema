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

#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include "trema.h"
#include "trema_wrapper.h"
#include "event_forward_interface.h"


static char *efi_queue_name = NULL;


// txid -> all_sw_tx;
static hash_table *efi_tx_table = NULL;


struct event_forward_operation_request_param {
  event_forward_entry_operation_callback callback;
  void *user_data;
};


struct event_forward_operation_to_all_request_param {
  bool add;
  enum efi_event_type type;
  char *service_name;
  event_forward_entry_to_all_callback callback;
  void *user_data;
};


struct sw_list_request_param {
  switch_list_request_callback callback;
  void *user_data;
};


typedef struct all_sw_tx {
  uint32_t txid;
  hash_table *waiting_dpid;
  enum efi_result tx_result;

  struct event_forward_operation_to_all_request_param *request_param;
} all_sw_tx;


struct txinfo {
  uint64_t dpid;
  uint32_t txid;
};


static void
xfree_event_forward_operation_to_all_request_param( struct event_forward_operation_to_all_request_param *param ) {
  xfree( param->service_name );
  param->service_name = NULL;
  xfree( param );
}


// foreach_hash helper function
static void
xfree_dpid( void *key, void *value, void *user_data ) {
  UNUSED( value );
  UNUSED( user_data );
  xfree( key );
}


static void
xfree_all_sw_tx( all_sw_tx *tx ) {
  struct event_forward_operation_to_all_request_param *param = tx->request_param;
  xfree_event_forward_operation_to_all_request_param( param );
  tx->request_param = NULL;

  foreach_hash( tx->waiting_dpid, xfree_dpid, NULL );
  delete_hash( tx->waiting_dpid );
  tx->waiting_dpid = NULL;
  xfree( tx );
}


const char*
_get_efi_queue_name( void ) {
  return efi_queue_name;
}


buffer*
create_event_forward_operation_request( buffer *buf, enum efi_event_type type, list_element *service_list ) {
  if ( buf == NULL ) {
    buf = alloc_buffer_with_length( sizeof( event_forward_operation_request ) + MESSENGER_SERVICE_NAME_LENGTH );
  }
  event_forward_operation_request *req = append_back_buffer( buf, sizeof( event_forward_operation_request ) );
  memset( req, 0, sizeof( event_forward_operation_request ) );

  req->type = ( uint8_t ) type;

  size_t request_head_offset = sizeof( event_forward_operation_request );
  uint32_t n_services = 0;
  if ( service_list != NULL ) {
    for ( list_element *e = service_list; e != NULL; e = e->next ) {
      const size_t len = strlen( e->data );
      char *dst = append_back_buffer( buf, len + 1 );
      req = ( event_forward_operation_request * )( dst - request_head_offset );
      request_head_offset += len + 1;
      strcpy( dst, e->data );
      dst[ len ] = '\0';
      ++n_services;
    }
  }

  req->n_services = htonl( n_services );
  return buf;
}


buffer*
create_event_forward_operation_reply( enum efi_event_type type, enum efi_result result, list_element *service_list ) {
  buffer *buf = alloc_buffer_with_length( sizeof( event_forward_operation_reply ) + MESSENGER_SERVICE_NAME_LENGTH );
  event_forward_operation_reply *rep = append_back_buffer( buf, sizeof( event_forward_operation_reply ) );
  memset( rep, 0, sizeof( event_forward_operation_reply ) );

  rep->type = ( uint8_t ) type;
  rep->result = ( uint8_t ) result;

  size_t reply_head_offset = sizeof( event_forward_operation_reply );
  uint32_t n_services = 0;
  if ( service_list != NULL ) {
    for ( list_element *e = service_list; e != NULL; e = e->next ) {
      const size_t len = strlen( e->data );
      char *dst = append_back_buffer( buf, len + 1 );
      rep = ( event_forward_operation_reply * )( dst - reply_head_offset );
      reply_head_offset += len + 1;
      strcpy( dst, e->data );
      dst[ len ] = '\0';
      ++n_services;
    }
  }

  rep->n_services = htonl( n_services );
  return buf;
}


static void
handle_event_forward_operation_reply( management_application_reply *reply, size_t length, struct event_forward_operation_request_param *param ) {
  event_forward_operation_result result;
  result.result = EFI_OPERATION_FAILED;
  result.n_services = 0;
  result.services = NULL;

  event_forward_operation_reply *efi_reply = ( event_forward_operation_reply * ) reply->data;
  efi_reply->n_services = ntohl( efi_reply->n_services );

  if ( reply->header.status == MANAGEMENT_REQUEST_FAILED ) {
    warn( "Management request failed." );
    result.result = EFI_OPERATION_FAILED;
    if ( param->callback != NULL ) {
      param->callback( result, param->user_data );
    }
    return;
  }

  switch ( efi_reply->result ) {
    case EFI_OPERATION_SUCCEEDED:
      result.result = EFI_OPERATION_SUCCEEDED;
      break;
    case EFI_OPERATION_FAILED:
      result.result = EFI_OPERATION_FAILED;
      break;
    default:
      warn( "Unknown result type %#x. Translating as FAILED.", efi_reply->result );
      result.result = EFI_OPERATION_FAILED;
      break;
  }

  if ( result.result == EFI_OPERATION_FAILED ) {
    if ( param->callback != NULL ) {
      param->callback( result, param->user_data );
    }
    return;
  }

  const size_t service_list_len = length
      - offsetof( management_application_reply, data )
      - offsetof( event_forward_operation_reply, service_list );

  // null terminate input to avoid overrun.
  efi_reply->service_list[ service_list_len ] = '\0';

  const uint32_t n_services_expected = efi_reply->n_services;
  result.services = xcalloc( n_services_expected, sizeof( char * ) );

  result.n_services = 0;
  const char *next_name = efi_reply->service_list;
  while ( next_name < &efi_reply->service_list[ service_list_len ] ) {
    const size_t name_len = strlen( next_name );
    if ( name_len > 0 ) {
      if ( result.n_services < n_services_expected ) {
        result.services[ result.n_services++ ] = next_name;
      }
      else {
        warn( "Expected %d name(s), but found more service name. Ignoring '%s'.", n_services_expected, next_name );
      }
    }
    else {
      warn( "Encountered empty service name." );
    }
    next_name += name_len + 1;
  }

  if ( param->callback != NULL ) {
    param->callback( result, param->user_data );
  }
  xfree( result.services );
}


static void
handle_get_switch_list_reply( management_application_reply *reply, size_t length, struct sw_list_request_param *param ) {

  if ( reply->header.status == MANAGEMENT_REQUEST_FAILED ) {
    warn( "Management request failed." );
    if ( param->callback != NULL ) {
      param->callback( NULL, 0, param->user_data );
    }
    return;
  }

  uint64_t *dpids = ( uint64_t * ) reply->data;
  const size_t data_bytes = length - sizeof( management_application_reply );
  if ( data_bytes % sizeof( uint64_t ) != 0 ) {
    warn( "data length was not multiple of uint64_t size %zu", data_bytes );
  }
  const size_t n_dpids = data_bytes / sizeof( uint64_t );

  for ( size_t i = 0 ; i < n_dpids ; ++i ) {
    dpids[ i ] = ntohll( dpids[ i ] );
  }

  if ( param->callback != NULL ) {
    param->callback( dpids, n_dpids, param->user_data );
  }
}


static void
handle_efi_reply( uint16_t tag, void *data, size_t length, void *user_data ) {

  debug( "handle_efi_reply: tag=%#x, data=%p, length=%zu, user_data=%p", tag, data, length, user_data );

  if ( tag != MESSENGER_MANAGEMENT_REPLY ) {
    error( "Unexpected message received tag=%#x", tag );
    return;
  }
  if ( data == NULL ) {
    error( "Received reply had no data" );
    return;
  }
  if ( length < sizeof( management_application_reply ) ) {
    error( "Data length too short %zu. expecting >= %zu", length, sizeof( management_application_reply ) );
    return;
  }

  management_application_reply *reply = data;
  if ( ntohs( reply->header.type ) != MANAGEMENT_APPLICATION_REPLY ) {
    error( "Invalid reply type %#x, expecting MANAGEMENT_APPLICATION_REPLY:%#x", ntohs( reply->header.type ), MANAGEMENT_APPLICATION_REPLY );
    return;
  }
  const uint32_t command = ntohl( reply->application_id );

  // check command validity
  switch ( command ) {
    case EVENT_FORWARD_ENTRY_ADD:
    case EVENT_FORWARD_ENTRY_DELETE:
    case EVENT_FORWARD_ENTRY_DUMP:
    case EVENT_FORWARD_ENTRY_SET:
    {
      struct event_forward_operation_request_param *param = user_data;
      handle_event_forward_operation_reply( reply, length, param );
      xfree( param );
      return;
    }
    break;

    case EFI_GET_SWLIST:
    {
      struct sw_list_request_param *param = user_data;
      handle_get_switch_list_reply( reply, length, param );
      xfree( param );
      return;
    }
    break;

    default:
      warn( "Invalid command/application_id: %#x", command );
      return;
      break;
  }
}


bool
init_event_forward_interface( void ) {
  if ( efi_queue_name != NULL ) {
    warn( "already initialized." );
    return false;
  }

  efi_queue_name = xcalloc( 1, MESSENGER_SERVICE_NAME_LENGTH );

  int chWrite = snprintf( efi_queue_name, MESSENGER_SERVICE_NAME_LENGTH,
                          "%s-efic-%d", get_trema_name(), trema_getpid() );
  if ( chWrite >= MESSENGER_SERVICE_NAME_LENGTH ) {
    snprintf( efi_queue_name, MESSENGER_SERVICE_NAME_LENGTH,
              "efic-%d", trema_getpid() );
  }

  // management reply handler
  add_message_replied_callback( efi_queue_name, handle_efi_reply );

  efi_tx_table = create_hash( compare_uint32, hash_uint32 );
  return true;
}


static void
maybe_init_event_forward_interface( void ) {
  if ( efi_queue_name == NULL ) {
    info( "Initializing at maybe_init_event_forward_interface()" );
    init_event_forward_interface();
  }
}


void
_cleanup_tx_table() {
  hash_iterator it;
  init_hash_iterator( efi_tx_table, &it );
  hash_entry *e = NULL;
  while ( ( e = iterate_hash_next( &it ) ) != NULL ) {
    all_sw_tx *tx = e->value;
    warn( "txid:%#x was left behind.", tx->txid );

    delete_hash_entry( efi_tx_table, &tx->txid );
    xfree_all_sw_tx( tx );
  }
}


bool
finalize_event_forward_interface( void ) {
  if ( efi_queue_name == NULL ) {
    warn( "already finalized." );
    return false;
  }
  delete_message_replied_callback( efi_queue_name, handle_efi_reply );

  xfree( efi_queue_name );
  efi_queue_name = NULL;

  _cleanup_tx_table();
  delete_hash( efi_tx_table );
  efi_tx_table = NULL;

  return true;
}


static bool
send_efi_event_config_request( const char *service_name, enum switch_management_command command, enum efi_event_type type, list_element *service_list, event_forward_entry_operation_callback callback, void *user_data ) {
  maybe_init_event_forward_interface();
  if ( service_name == NULL ) {
    return false;
  }
  switch( command ) {
  case EVENT_FORWARD_ENTRY_ADD:
  case EVENT_FORWARD_ENTRY_DELETE:
  case EVENT_FORWARD_ENTRY_DUMP:
  case EVENT_FORWARD_ENTRY_SET:
    // do nothing;
    break;
  default:
    return false;
  }

  buffer *all_req = alloc_buffer_with_length( sizeof( management_application_request ) + sizeof( event_forward_operation_request ) + MESSENGER_SERVICE_NAME_LENGTH );

  management_application_request *apreq = append_back_buffer( all_req, sizeof( management_application_request ) );
  apreq->header.type = htons( MANAGEMENT_APPLICATION_REQUEST );
  apreq->application_id = htonl( command );
  create_event_forward_operation_request( all_req, type, service_list );
  apreq = all_req->data;
  apreq->header.length = htonl( ( uint32_t ) all_req->length );

  struct event_forward_operation_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_request_param ) );
  param->callback = callback;
  param->user_data = user_data;

  bool sent = send_request_message( service_name, efi_queue_name, MESSENGER_MANAGEMENT_REQUEST,
                                   all_req->data, all_req->length, param );
  if ( !sent ) {
    xfree( param );
  }
  free_buffer( all_req );

  return sent;
}


// Low level API for switch manager
bool
set_switch_manager_event_forward_entries( enum efi_event_type type, list_element *service_list, event_forward_entry_operation_callback callback, void *user_data ) {
  const char switch_manager[] = "switch_manager.m";
  return send_efi_event_config_request( switch_manager, EVENT_FORWARD_ENTRY_SET, type, service_list, callback, user_data );
}


bool
add_switch_manager_event_forward_entry( enum efi_event_type type, const char *service_name, event_forward_entry_operation_callback callback, void *user_data ) {
  const char switch_manager[] = "switch_manager.m";
  list_element service_list;
  service_list.next = NULL;
  // Copying only to avoid const_cast warnings. Will not modify in callee.
  service_list.data = xstrdup( service_name );
  bool sent_ok = send_efi_event_config_request( switch_manager, EVENT_FORWARD_ENTRY_ADD, type, &service_list, callback, user_data );
  xfree( service_list.data );
  return sent_ok;
}


bool
delete_switch_manager_event_forward_entry( enum efi_event_type type, const char *service_name, event_forward_entry_operation_callback callback, void *user_data ) {
  const char switch_manager[] = "switch_manager.m";
  list_element service_list;
  service_list.next = NULL;
  // Copying only to avoid const_cast warnings. Will not modify in callee.
  service_list.data = xstrdup( service_name );
  bool sent_ok = send_efi_event_config_request( switch_manager, EVENT_FORWARD_ENTRY_DELETE, type, &service_list, callback, user_data );
  xfree( service_list.data );
  return sent_ok;
}


bool
dump_switch_manager_event_forward_entries( enum efi_event_type type, event_forward_entry_operation_callback callback, void *user_data ) {
  const char switch_manager[] = "switch_manager.m";
  return send_efi_event_config_request( switch_manager, EVENT_FORWARD_ENTRY_DUMP, type, NULL, callback, user_data );
}


bool
set_switch_event_forward_entries( uint64_t dpid, enum efi_event_type type, list_element *service_list, event_forward_entry_operation_callback callback, void *user_data ) {
  char switch_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  snprintf( switch_name, MESSENGER_SERVICE_NAME_LENGTH, "switch.%#" PRIx64 ".m", dpid  );
  return send_efi_event_config_request( switch_name, EVENT_FORWARD_ENTRY_SET, type, service_list, callback, user_data );
}


bool
add_switch_event_forward_entry( uint64_t dpid, enum efi_event_type type, const char *service_name, event_forward_entry_operation_callback callback, void *user_data ) {
  char switch_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  snprintf( switch_name, MESSENGER_SERVICE_NAME_LENGTH, "switch.%#" PRIx64 ".m", dpid  );
  list_element service_list;
  service_list.next = NULL;
  // Copying only to avoid const_cast warnings. Will not modify in callee.
  service_list.data = xstrdup( service_name );
  bool sent_ok = send_efi_event_config_request( switch_name, EVENT_FORWARD_ENTRY_ADD, type, &service_list, callback, user_data );
  xfree( service_list.data );
  return sent_ok;
}


bool
delete_switch_event_forward_entry( uint64_t dpid, enum efi_event_type type, const char *service_name, event_forward_entry_operation_callback callback, void *user_data ) {
  char switch_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  snprintf( switch_name, MESSENGER_SERVICE_NAME_LENGTH, "switch.%#" PRIx64 ".m", dpid  );
  list_element service_list;
  service_list.next = NULL;
  // Copying only to avoid const_cast warnings. Will not modify in callee.
  service_list.data = xstrdup( service_name );
  bool sent_ok = send_efi_event_config_request( switch_name, EVENT_FORWARD_ENTRY_DELETE, type, &service_list, callback, user_data );
  xfree( service_list.data );
  return sent_ok;
}


bool
dump_switch_event_forward_entries( uint64_t dpid, enum efi_event_type type, event_forward_entry_operation_callback callback, void *user_data ) {
  char switch_name[ MESSENGER_SERVICE_NAME_LENGTH ];
  snprintf( switch_name, MESSENGER_SERVICE_NAME_LENGTH, "switch.%#" PRIx64 ".m", dpid  );
  return send_efi_event_config_request( switch_name, EVENT_FORWARD_ENTRY_DUMP, type, NULL, callback, user_data );
}


bool
send_efi_switch_list_request( switch_list_request_callback callback, void *user_data ) {
  maybe_init_event_forward_interface();
  if ( callback == NULL ) {
    return false;
  }

  management_application_request req;
  req.header.type = htons( MANAGEMENT_APPLICATION_REQUEST );
  req.header.length = htonl( ( uint32_t ) sizeof( management_application_request ) );
  req.application_id = htonl( EFI_GET_SWLIST );

  struct sw_list_request_param *param = xcalloc( 1, sizeof( struct sw_list_request_param ) );
  param->callback = callback;
  param->user_data = user_data;

  bool sent_ok = send_request_message( "switch_manager.m", efi_queue_name, MESSENGER_MANAGEMENT_REQUEST,
                                   &req, sizeof( management_application_request ), param );
  if ( !sent_ok ) {
    xfree( param );
  }

  return sent_ok;
}


static uint32_t
get_txid() {
  static uint32_t txid = 0;
  return ++txid;
}


static bool
hash_table_is_empty( hash_table *tbl ) {
  hash_iterator it;
  init_hash_iterator( tbl, &it );
  return ( iterate_hash_next( &it ) == NULL );
}


static bool
check_tx_result_and_respond( all_sw_tx *tx ) {
  if ( tx->tx_result == EFI_OPERATION_FAILED ) {
    struct event_forward_operation_to_all_request_param *all_param = tx->request_param;
    if ( all_param->callback != NULL ) {
      all_param->callback( tx->tx_result, all_param->user_data );
    }

    info( "txid %#x completed with failure.", tx->txid );
    // remove and cleanup tx
    delete_hash_entry( efi_tx_table, &tx->txid );
    xfree_all_sw_tx( tx );
    return true;
  }
  return false;
}


void
_switch_response_timeout( void *user_data ) {
  struct txinfo *txinfo = user_data;
  const uint64_t dpid = txinfo->dpid;
  const uint32_t txid = txinfo->txid;
  //  xfree( txinfo ); // cannot free since _switch_response_handler may be called later.
  txinfo = NULL;
  debug( "txid %#x switch %#" PRIx64 " time out. called", txid, dpid );

  all_sw_tx *tx = lookup_hash_entry( efi_tx_table, &txid );
  if ( tx == NULL ) {
    // transaction already failed. Do nothing.
    return;
  }

  uint64_t *dpid_in_list = lookup_hash_entry( tx->waiting_dpid, &dpid );
  if ( dpid_in_list != NULL ) {
    info( "txid %#x switch %#" PRIx64 " timed out.", txid, dpid );
    delete_hash_entry( tx->waiting_dpid, &dpid );
    xfree( dpid_in_list );
    tx->tx_result = EFI_OPERATION_FAILED;
  }

  check_tx_result_and_respond( tx );
}


void
_switch_response_handler( event_forward_operation_result result, void *user_data ) {
  struct txinfo *txinfo = user_data;
  const uint64_t dpid = txinfo->dpid;
  const uint32_t txid = txinfo->txid;
  delete_timer_event( _switch_response_timeout, txinfo );
  xfree( txinfo );
  txinfo = NULL;
  debug( "txid %#x switch %#" PRIx64 " response.", txid, dpid );

  all_sw_tx *tx = lookup_hash_entry( efi_tx_table, &txid );
  if ( tx == NULL ) {
    // transaction already failed. Do nothing.
    return;
  }

  uint64_t *dpid_in_list = lookup_hash_entry( tx->waiting_dpid, &dpid );
  if ( dpid_in_list == NULL ) {
    // probably timed out. Do nothing.
    return;
  }
  delete_hash_entry( tx->waiting_dpid, &dpid );
  xfree( dpid_in_list );
  dpid_in_list = NULL;

  if ( result.result == EFI_OPERATION_FAILED ) {
    warn( "txid %#x Setting event forwarding entry to switch %#" PRIx64 " failed.", txid, dpid );
    tx->tx_result = EFI_OPERATION_FAILED;
  }

  bool tx_ended = check_tx_result_and_respond( tx );
  if ( tx_ended ) return;

  struct event_forward_operation_to_all_request_param *all_param = tx->request_param;
  if ( hash_table_is_empty( tx->waiting_dpid ) ) {
    // was last element in tx. callback result.
    if ( all_param->callback != NULL ) {
      all_param->callback( tx->tx_result, all_param->user_data );
    }

    info( "txid %#x complete.", tx->txid );
    // remove and cleanup tx
    delete_hash_entry( efi_tx_table, &tx->txid );
    xfree_all_sw_tx( tx );
    return;
  }
}


all_sw_tx*
_insert_tx( size_t n_dpids, struct event_forward_operation_to_all_request_param *param ) {
  all_sw_tx *tx = xcalloc( 1, sizeof(all_sw_tx) );
  tx->txid = get_txid();
  info( "txid %#x Start dispatching to switches", tx->txid );
  tx->request_param = param;
  tx->tx_result = EFI_OPERATION_SUCCEEDED;
  tx->waiting_dpid = create_hash_with_size( compare_datapath_id,
                                            hash_datapath_id,
                                            ( unsigned ) n_dpids );
  void *dupe_tx = insert_hash_entry( efi_tx_table, &tx->txid, tx );
  assert( dupe_tx == NULL );
  return tx;
}


void
_dispatch_to_all_switch( uint64_t *dpids, size_t n_dpids, void *user_data ) {
  struct event_forward_operation_to_all_request_param *param = user_data;

  all_sw_tx *tx = _insert_tx( n_dpids, param );

  // copy dpid hash to transaction.
  for ( size_t i = 0 ; i < n_dpids ; ++i ) {
    uint64_t *dpid = xmalloc( sizeof( uint64_t ) );
    *dpid = dpids[i];
    uint64_t *dupe_dpid = insert_hash_entry( tx->waiting_dpid, dpid, dpid );
    if ( dupe_dpid == NULL ) {
      struct txinfo *txinfo = xcalloc( 1, sizeof( struct txinfo ) );
      txinfo->dpid = *dpid;
      txinfo->txid = tx->txid;
      bool send_ok;

      if ( param->add ) {
        send_ok = add_switch_event_forward_entry( *dpid, param->type, param->service_name, _switch_response_handler, txinfo );
      }
      else {
        send_ok = delete_switch_event_forward_entry( *dpid, param->type, param->service_name, _switch_response_handler, txinfo );
      }

      if ( !send_ok ) {
        tx->tx_result = EFI_OPERATION_FAILED;
        warn( "txid %#x Failed to send request to switch %#" PRIx64 ".", tx->txid, *dpid );
        xfree( delete_hash_entry( tx->waiting_dpid, dpid ) );
        dpid = NULL;
        xfree( txinfo );
        continue;
      }

      struct itimerspec interval;
      interval.it_interval.tv_sec = 0;
      interval.it_interval.tv_nsec = 0;
      interval.it_value.tv_sec = 5; // FIXME make this configurable?
      interval.it_value.tv_nsec = 0;
      bool set_ok = add_timer_event_callback( &interval, _switch_response_timeout, txinfo );
      if ( !set_ok ) {
        tx->tx_result = EFI_OPERATION_FAILED;
        warn( "txid %#x Failed to set timeout timer for switch %#" PRIx64 ".", tx->txid, *dpid );
        xfree( delete_hash_entry( tx->waiting_dpid, dpid ) );
        dpid = NULL;
        // txinfo will be freed by _switch_response_handler
        continue;
      }
    }
    else {
      warn( "Duplicate dpid returned %#." PRIx64, *dupe_dpid );
      xfree( dupe_dpid );
    }
  }

  if ( tx->tx_result == EFI_OPERATION_FAILED ) {
    if ( param->callback != NULL ) {
      param->callback( tx->tx_result, param->user_data );
    }
    info( "txid %#x completed with failure.", tx->txid );
    // remove and cleanup tx
    delete_hash_entry( efi_tx_table, &tx->txid );
    xfree_all_sw_tx( tx );
  }
}


void
_get_switch_list_after_swm_succ( event_forward_operation_result result, void *user_data ) {
  struct event_forward_operation_to_all_request_param *param = user_data;
  bool success = true;
  if ( result.result == EFI_OPERATION_SUCCEEDED ) {
    // get switch list
    success = send_efi_switch_list_request( _dispatch_to_all_switch, param );
  }
  else {
    success = false;
  }

  if ( !success ) {
    warn( "Failed to set event forwarding entry to switch manager." );
    if ( param->callback != NULL ) {
      param->callback( EFI_OPERATION_FAILED, param->user_data );
    }
    xfree_event_forward_operation_to_all_request_param( param );
  }
}


bool
add_event_forward_entry_to_all_switches( enum efi_event_type type, const char *service_name, event_forward_entry_to_all_callback callback, void *user_data ) {
  struct event_forward_operation_to_all_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_to_all_request_param ) );
  param->add = true;
  param->type = type;
  param->service_name = xstrdup( service_name );
  param->callback = callback;
  param->user_data = user_data;
  bool sent_ok = add_switch_manager_event_forward_entry( type, service_name, _get_switch_list_after_swm_succ, param );
  if ( !sent_ok ) {
    xfree_event_forward_operation_to_all_request_param( param );
  }
  return sent_ok;
}


bool
delete_event_forward_entry_to_all_switches( enum efi_event_type type, const char *service_name, event_forward_entry_to_all_callback callback, void *user_data ) {
  struct event_forward_operation_to_all_request_param *param = xcalloc( 1, sizeof( struct event_forward_operation_to_all_request_param ) );
  param->add = false;
  param->type = type;
  param->service_name = xstrdup( service_name );
  param->callback = callback;
  param->user_data = user_data;
  bool sent_ok = delete_switch_manager_event_forward_entry( type, service_name, _get_switch_list_after_swm_succ, param );
  if ( !sent_ok ) {
    xfree_event_forward_operation_to_all_request_param( param );
  }
  return sent_ok;
}


