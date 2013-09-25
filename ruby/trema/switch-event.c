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
#include "ruby.h"
#include "switch-event.h"
#include "compat.h"

#include "trema.h"

extern VALUE mTrema;
VALUE mSwitchEvent;

static ID idVendor;
static ID idPacketIn;
static ID idPortStatus;
static ID idStateNotify;


typedef struct callback_info {
  VALUE self;
  VALUE block;
} callback_info;


static bool
event_type_symbol_to_enum( VALUE rType, enum efi_event_type *cType ) {
  assert( cType != NULL );
  const ID idType = rb_to_id( rType );
  if ( false ) {
  }
  else if ( idType == idVendor ) {
    *cType = EVENT_FORWARD_TYPE_VENDOR;
  }
  else if ( idType == idPacketIn ) {
    *cType = EVENT_FORWARD_TYPE_PACKET_IN;
  }
  else if ( idType == idPortStatus ) {
    *cType = EVENT_FORWARD_TYPE_PORT_STATUS;
  }
  else if ( idType == idStateNotify ) {
    *cType = EVENT_FORWARD_TYPE_STATE_NOTIFY;
  }
  else {
    return false;
  }
  return true;
}


static void
handle_event_forward_entry_to_all_callback( enum efi_result result,
                                            void *user_data ) {
  debug( "%s", __func__ );
  callback_info *cb = user_data;
  if ( cb->block != Qnil ) {
    if ( result == EFI_OPERATION_SUCCEEDED ) {
      rb_funcall( cb->block, rb_intern( "call" ), 1, Qtrue );
    }
    else {
      rb_funcall( cb->block, rb_intern( "call" ), 1, Qfalse );
    }
  }
  xfree( cb );

}

/*
 * @!group Operation for all existing switches and switch manager
 *
 * @overload add_forward_entry_to_all_switches type, service_name, {|result| ... }
 * Add forwarding entry to all existing switches and switch manager.
 *
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [String] service_name Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful on all switches and switch manager
 */
static VALUE
add_forward_entry_to_all_switches( VALUE self, VALUE type, VALUE service_name ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const char *c_service_name = RSTRING_PTR( service_name );
  if ( strlen( c_service_name ) == 0 ) {
    warn( "service_name cannot be empty" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = add_event_forward_entry_to_all_switches(
                                    c_type, c_service_name,
                                    handle_event_forward_entry_to_all_callback,
                                    cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for all existing switches and switch manager
 *
 * @overload delete_forward_entry_from_all_switches type, service_name, {|result| ... }
 * Delete forwarding entry to all existing switches and switch manager.
 *
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [String] service_name Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful on all switches and switch manager
 */
static VALUE
delete_forward_entry_from_all_switches( VALUE self, VALUE type,
                                        VALUE service_name ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const char *c_service_name = RSTRING_PTR( service_name );
  if ( strlen( c_service_name ) == 0 ) {
    warn( "service_name cannot be empty" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = delete_event_forward_entry_to_all_switches(
                                    c_type, c_service_name,
                                    handle_event_forward_entry_to_all_callback,
                                    cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


static void
handle_event_forward_entry_operation_callback(
    event_forward_operation_result result, void *user_data ) {
  debug( "%s", __func__ );
  callback_info *cb = user_data;

  if ( cb->block != Qnil ) {
    if ( result.result == EFI_OPERATION_SUCCEEDED ) {
      VALUE aryDpid = rb_ary_new2( ( long ) result.n_services );
      for ( uint32_t i = 0 ; i < result.n_services ; ++i ) {
        VALUE service_name = rb_str_new2( result.services[i] );
        rb_ary_push( aryDpid, service_name );
      }
      rb_funcall( cb->block, rb_intern( "call" ), 2, Qtrue, aryDpid );
    }
    else {
      VALUE aryDpid = rb_ary_new();
      rb_funcall( cb->block, rb_intern( "call" ), 2, Qfalse, aryDpid );
    }
  }
  xfree( cb );
}


/*
 * @!group Operation for existing switch
 *
 * @overload add_forward_entry_to_switch datapath_id, type, service_name, {|result, services| ... }
 * Add forwarding entry of a switch specified.
 *
 * @param [Integer] dpid Switch datapath_id
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [String] service_name Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful.
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
add_forward_entry_to_switch( VALUE self, VALUE dpid, VALUE type,
                             VALUE service_name ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const uint64_t c_dpid = NUM2ULL( dpid );
  const char *c_service_name = RSTRING_PTR( service_name );
  if ( strlen( c_service_name ) == 0 ) {
    warn( "service_name cannot be empty" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = add_switch_event_forward_entry(
                                c_dpid, c_type, c_service_name,
                                handle_event_forward_entry_operation_callback,
                                cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for existing switch
 *
 * @overload delete_forward_entry_from_switch datapath_id, type, service_name, {|result, services| ... }
 * Delete forwarding entry of a switch specified.
 *
 * @param [Integer] dpid Switch datapath_id
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [String] service_name Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful.
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
delete_forward_entry_from_switch( VALUE self, VALUE dpid, VALUE type,
                                  VALUE service_name ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const uint64_t c_dpid = NUM2ULL( dpid );
  const char *c_service_name = RSTRING_PTR( service_name );
  if ( strlen( c_service_name ) == 0 ) {
    warn( "service_name cannot be empty" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = delete_switch_event_forward_entry(
                                c_dpid, c_type, c_service_name,
                                handle_event_forward_entry_operation_callback,
                                cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for existing switch
 *
 * @overload set_forward_entries_to_switch datapath_id, type, service_name, {|result, services| ... }
 * Set forwarding entries of a switch specified.
 *
 * @param [Integer] dpid Switch datapath_id
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [Array<String>] service_names Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful.
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
set_forward_entries_to_switch( VALUE self, VALUE dpid, VALUE type,
                               VALUE service_names ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const uint64_t c_dpid = NUM2ULL( dpid );
  list_element *service_list = NULL;
  create_list( &service_list );
  for ( long i = 0 ; i < RARRAY_LEN( service_names ) ; ++i ) {
    VALUE ruby_service_name = rb_ary_entry( service_names, i );
    char *c_service_name = RSTRING_PTR( ruby_service_name );
    if ( strlen( c_service_name ) == 0 ) {
      warn( "Ignoring empty service_name" );
      continue;
    }
    append_to_tail( &service_list, c_service_name );
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = set_switch_event_forward_entries(
                                c_dpid, c_type, service_list,
                                handle_event_forward_entry_operation_callback,
                                cb );

  delete_list( service_list );

  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for existing switch
 *
 * @overload dump_forward_entries_from_switch datapath_id, type, {|result, services| ... }
 * Dump forwarding entry of a switch specified.
 *
 * @param [Integer] dpid Switch datapath_id
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful on all switches and switch manager
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
dump_forward_entries_from_switch( VALUE self, VALUE dpid, VALUE type ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const uint64_t c_dpid = NUM2ULL( dpid );

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = dump_switch_event_forward_entries(
                                c_dpid, c_type,
                                handle_event_forward_entry_operation_callback,
                                cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for switch manager
 *
 * @overload add_forward_entry_to_switch_manager type, service_name, {|result, services| ... }
 * Add forwarding entry to a switch manager.
 *
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [String] service_name Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful.
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
add_forward_entry_to_switch_manager( VALUE self, VALUE type,
                                     VALUE service_name ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const char *c_service_name = RSTRING_PTR( service_name );
  if ( strlen( c_service_name ) == 0 ) {
    warn( "service_name cannot be empty" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = add_switch_manager_event_forward_entry(
                                c_type, c_service_name,
                                handle_event_forward_entry_operation_callback,
                                cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for switch manager
 *
 * @overload delete_forward_entry_from_switch_manager type, service_name, {|result, services| ... }
 * Delete forwarding entry of a switch manager.
 *
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [String] service_name Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful.
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
delete_forward_entry_from_switch_manager( VALUE self, VALUE type,
                                          VALUE service_name ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  const char *c_service_name = RSTRING_PTR( service_name );
  if ( strlen( c_service_name ) == 0 ) {
    warn( "service_name cannot be empty" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = delete_switch_manager_event_forward_entry(
                                c_type, c_service_name,
                                handle_event_forward_entry_operation_callback,
                                cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for switch manager
 *
 * @overload set_forward_entries_to_switch_manager type, service_names, {|result, services| ... }
 * Set forwarding entries of a switch manager.
 *
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @param [Array<String>] service_names Name of controller to forward event.
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful.
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
set_forward_entries_to_switch_manager( VALUE self, VALUE type,
                                       VALUE service_names ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  list_element *service_list = NULL;
  create_list( &service_list );
  for ( long i = 0 ; i < RARRAY_LEN( service_names ) ; ++i ) {
    VALUE ruby_service_name = rb_ary_entry( service_names, i );
    char *c_service_name = RSTRING_PTR( ruby_service_name );
    if ( strlen( c_service_name ) == 0 ) {
      warn( "Ignoring empty service_name" );
      continue;
    }
    append_to_tail( &service_list, c_service_name );
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = set_switch_manager_event_forward_entries(
                                c_type, service_list,
                                handle_event_forward_entry_operation_callback,
                                cb );

  delete_list( service_list );

  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * @!group Operation for switch manager
 *
 * @overload dump_forward_entries_from_switch type, {|result, services| ... }
 * Dump forwarding entry of a switch manager.
 *
 * @param [Symbol] type Switch event type. it must be one of :vendor, :packet_in, :port_status, :state_notify
 * @return [Boolean] true if request was sent successfully.
 *
 * @yield Callback to notify the result of operation.
 * @yieldparam result [Boolean] true if result successful on all switches and switch manager
 * @yieldparam services [Array<String>] Service Name list on forwarding entry after operation.
 */
static VALUE
dump_forward_entries_from_switch_manager( VALUE self, VALUE type ) {
  debug( "%s", __func__ );
  enum efi_event_type c_type;
  if ( !event_type_symbol_to_enum( type, &c_type ) ) {
    warn( "Invalid event type was specified" );
    return Qfalse;
  }

  callback_info *cb = xcalloc( 1, sizeof( callback_info ) );
  cb->self = self;
  cb->block = Qnil;
  if ( rb_block_given_p() == Qtrue ) {
    cb->block = rb_block_proc();
  }
  bool succ = dump_switch_manager_event_forward_entries(
                                c_type,
                                handle_event_forward_entry_operation_callback,
                                cb );
  if ( succ ) {
    return Qtrue;
  }
  else {
    xfree( cb );
    return Qfalse;
  }
}


/*
 * Document-module: Trema::SwitchEvent
 */
void
Init_switch_event( void ) {
  mSwitchEvent = rb_define_module_under( mTrema, "SwitchEvent" );

  idVendor = rb_intern( "vendor" );
  idPacketIn = rb_intern( "packet_in" );
  idPortStatus = rb_intern( "port_status" );
  idStateNotify = rb_intern( "state_notify" );

  rb_define_method( mSwitchEvent, "add_forward_entry_to_all_switches",
                    add_forward_entry_to_all_switches, 2 );
  rb_define_method( mSwitchEvent, "delete_forward_entry_from_all_switches",
                    delete_forward_entry_from_all_switches, 2 );

  rb_define_method( mSwitchEvent, "add_forward_entry_to_switch",
                    add_forward_entry_to_switch, 3 );
  rb_define_method( mSwitchEvent, "delete_forward_entry_from_switch",
                    delete_forward_entry_from_switch, 3 );
  rb_define_method( mSwitchEvent, "set_forward_entries_to_switch",
                    set_forward_entries_to_switch, 3 );
  rb_define_method( mSwitchEvent, "dump_forward_entries_from_switch",
                    dump_forward_entries_from_switch, 2 );

  rb_define_method( mSwitchEvent, "add_forward_entry_to_switch_manager",
                    add_forward_entry_to_switch_manager, 2 );
  rb_define_method( mSwitchEvent, "delete_forward_entry_from_switch_manager",
                    delete_forward_entry_from_switch_manager, 2 );
  rb_define_method( mSwitchEvent, "set_forward_entries_to_switch_manager",
                    set_forward_entries_to_switch_manager, 2 );
  rb_define_method( mSwitchEvent, "dump_forward_entries_from_switch_manager",
                    dump_forward_entries_from_switch_manager, 1 );
}
