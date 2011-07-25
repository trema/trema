/*
 * Author: Shuji Ishii
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


#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "trema.h"
#include "probe_timer_table.h"
#include "lldp.h"


#ifdef UNIT_TESTING

#ifdef create_link
#undef create_link
#endif
#define create_link mock_create_link
struct link *mock_create_link( uint64_t from_dpid, uint16_t from_port_no, uint64_t to_dpid, uint16_t to_port_no, uint32_t link_speed );

#ifdef die
#undef die
#endif
#define die mock_die
void mock_die( const char *err, ... );

#ifdef info
#undef info
#endif
#define info mock_info
void mock_info( const char *format, ... );

#ifdef warn
#undef warn
#endif
#define warn mock_warn
void mock_warn( const char *format, ... );

#ifdef notice
#undef notice
#endif
#define notice mock_notice
void mock_notice( const char *format, ... );

#ifdef create_actions
#undef create_actions
#endif
#define create_actions mock_create_actions
openflow_actions *mock_create_actions( void );

#ifdef append_action_output
#undef append_action_output
#endif
#define append_action_output mock_append_action_output
bool mock_append_action_output( openflow_actions *actions, const uint16_t port, const uint16_t max_len );

#ifdef get_transaction_id
#undef get_transaction_id
#endif
#define get_transaction_id mock_get_transaction_id
uint32_t mock_get_transaction_id( void );

#ifdef create_packet_out
#undef create_packet_out
#endif
#define create_packet_out mock_create_packet_out
buffer * create_packet_out( const uint32_t transaction_id,
                            const uint32_t buffer_id,
                            const uint16_t in_port,
                            const openflow_actions *actions,
                            const buffer *data );

#ifdef send_openflow_message
#undef send_openflow_message
#endif
#define send_openflow_message mock_send_openflow_message
bool mock_send_openflow_message( const uint64_t datapath_id, buffer *message );

#ifdef lookup_mac
#undef lookup_mac
#endif
#define lookup_mac mock_lookup_mac
struct port *mock_lookup_mac( uint8_t *mac );

#ifdef lookup_port
#undef lookup_port
#endif
#define lookup_port mock_lookup_port
struct port *mock_lookup_port( uint64_t dpid, uint16_t port_no );

#ifdef create_link
#undef create_link
#endif
#define create_link mock_create_link
struct link *mock_create_link( uint64_t from_dpid, uint16_t from_port_no, uint64_t to_dpid, uint16_t to_port_no, uint32_t link_speed );

#ifdef set_packet_in_handler
#undef set_packet_in_handler
#endif
#define set_packet_in_handler mock_set_packet_in_handler
bool mock_set_packet_in_handler( packet_in_handler callback, void *user_data );

#endif // UNIT_TESTING

static int recv_lldp( uint64_t *dpid, uint16_t *port, const buffer *buf );
static buffer *create_lldp_frame( const uint8_t *mac, uint64_t dpid,
                                  uint16_t port_no );
static int parse_lldp_ull( void *str, uint64_t *value, uint32_t len );
static int parse_lldp_us( void *str, uint16_t *value, uint32_t len );

static const uint16_t ethtype = ETH_ETHTYPE_LLDP;


bool
send_lldp( probe_timer_entry *port ) {
  buffer *lldp;

  lldp = create_lldp_frame( port->mac, port->datapath_id, port->port_no );

  openflow_actions *actions = create_actions();
  if ( !append_action_output( actions, port->port_no, UINT16_MAX ) ) {
    free_buffer( lldp );
    error( "Failed to sent LLDP frame(%#" PRIx64 ", %u)", port->datapath_id, port->port_no );
    return false;
  }

  uint32_t transaction_id = get_transaction_id();

  fill_ether_padding( lldp );

  buffer *packetout = create_packet_out( transaction_id, UINT32_MAX,
                                         OFPP_NONE, actions, lldp );
  if ( !send_openflow_message( port->datapath_id, packetout ) ) {
    free_buffer( lldp );
    free_buffer( packetout );
    die( "send_openflow_message" );
  }

  free_buffer( lldp );
  free_buffer( packetout );

  debug( "Sent LLDP frame(%#" PRIx64 ", %u)", port->datapath_id, port->port_no );
  return true;
}


static int
recv_lldp( uint64_t *dpid, uint16_t *port_no, const buffer *buf ) {
  int ret;
  ether_header_t *lldp_frame;
  size_t remain_len = 0;
  uint16_t *lldp_tlv;
  uint8_t *lldp_du;
  uint32_t type = 0;
  uint32_t len = 0;
  uint8_t subtype = 0;

  lldp_frame = packet_info( buf )->l2_data.eth;
  remain_len = buf->length;

  if ( packet_info( buf )->vtag ) {
    die( "no vlan tag is supported" );
  }

  remain_len -= ( sizeof( ether_header_t ) - ETH_PREPADLEN );
  lldp_tlv = packet_info( buf )->l3_data.l3;

  do {
    if ( remain_len < LLDP_TLV_HEAD_LEN ) {
      info( "Missing size of LLDP tlv header." );
      return -1;
    }

    type = LLDP_TYPE( *lldp_tlv );
    len = ( uint32_t ) LLDP_LEN( *lldp_tlv );
    assert( len <= LLDP_TLV_INFO_MAX_LEN );

    remain_len -= LLDP_TLV_HEAD_LEN;
    if ( remain_len < len ) {
      info( "Missing size of LLDP data unit." );
      return -1;
    }

    lldp_du = ( uint8_t * ) lldp_tlv + LLDP_TLV_HEAD_LEN;

    switch ( type ) {
    case LLDP_TYPE_END:
      break;

    case LLDP_TYPE_CHASSIS_ID:
      subtype = *( ( uint8_t * ) lldp_du );
      lldp_du += LLDP_SUBTYPE_LEN;
      switch ( subtype ) {
      case CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED:
        ret = parse_lldp_ull( lldp_du, dpid,
                              ( len - LLDP_SUBTYPE_LEN ) );
        if ( ret < 0 ) {
          info( "Failed to parse dpid." );
          return -1;
        }
        break;

      default:
        break;
      }
      break;

    case LLDP_TYPE_PORT_ID:
      subtype = *( ( uint8_t * ) lldp_du );
      lldp_du += LLDP_SUBTYPE_LEN;
      switch ( subtype ) {
      case PORT_ID_SUBTYPE_LOCALLY_ASSINED:
        ret = parse_lldp_us( lldp_du, port_no,
                             ( len - LLDP_SUBTYPE_LEN ) );
        if ( ret < 0 ) {
          info( "Failed to parse port_no." );
          return -1;
        }
        break;

      default:
        break;
      }
      break;

    case LLDP_TYPE_TTL:
      break;

    default:
      info( "Unknown LLDP type (%#x).", type );
      return -1;
    }

    remain_len -= len;
    lldp_tlv = ( uint16_t * ) ( ( uint8_t * ) lldp_tlv + LLDP_TLV_HEAD_LEN + len );
  } while ( type != LLDP_TYPE_END );

  return 0;
}


static buffer *
create_lldp_frame( const uint8_t *mac, uint64_t dpid, uint16_t port_no ) {
  buffer *lldp_buf;
  ether_header_t *ether;

  struct tlv *chassis_id_tlv;
  uint32_t chassis_id_tlv_len = 0;
  uint32_t chassis_id_strlen = 0;
  struct tlv *port_id_tlv;
  uint32_t port_id_tlv_len = 0;
  uint32_t port_id_strlen = 0;
  struct tlv *ttl_tlv;
  struct tlv *end_tlv;
  char *info;
  char dpid_str[ LLDP_TLV_INFO_MAX_LEN ];
  char port_no_str[ LLDP_TLV_INFO_MAX_LEN ];
  uint16_t *ttl_val;

  const uint8_t lldp_multicast[ ETH_ADDRLEN ] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e };

  debug( "Create LLDP Frame." );

  // Convert Chassis ID into string
  chassis_id_strlen =
    ( uint32_t )snprintf( dpid_str, ( LLDP_TLV_INFO_MAX_LEN - 1 ),
                          "%#" PRIx64, dpid );

  assert ( chassis_id_strlen <= ( LLDP_TLV_INFO_MAX_LEN - 1 ) );

  chassis_id_tlv_len = LLDP_TLV_HEAD_LEN + LLDP_SUBTYPE_LEN + chassis_id_strlen;

  // Convert Port ID into string
  port_id_strlen =
    ( uint32_t )snprintf( port_no_str, ( LLDP_TLV_INFO_MAX_LEN - 1 ),
                          "%d", port_no );
  assert ( port_id_strlen <= ( LLDP_TLV_INFO_MAX_LEN - 1 ) );
  port_id_tlv_len = LLDP_TLV_HEAD_LEN + LLDP_SUBTYPE_LEN + port_id_strlen;

  lldp_buf = alloc_buffer_with_length(
    sizeof( struct ofp_packet_out )
    + sizeof( struct ofp_action_output ) * 64
    + sizeof( ether_header_t ) +
    + chassis_id_tlv_len
    + port_id_tlv_len
    + LLDP_TTL_LEN
    + LLDP_END_PDU_LEN );

  // Create ether frame header
  ether = append_back_buffer( lldp_buf, sizeof( ether_header_t ) );
  remove_front_buffer( lldp_buf, sizeof( ether->prepad ) );
  memcpy( ether->macda, lldp_multicast, ETH_ADDRLEN );
  memcpy( ether->macsa, mac, ETH_ADDRLEN );
  ether->type = htons( ethtype );

  // Create Chassis ID TLV 
  chassis_id_tlv = append_back_buffer( lldp_buf, chassis_id_tlv_len );
  chassis_id_tlv->type_len = LLDP_TL( LLDP_TYPE_CHASSIS_ID, chassis_id_tlv_len );
  info = chassis_id_tlv->val;
  *info = CHASSIS_ID_SUBTYPE_LOCALLY_ASSINED;
  info += LLDP_SUBTYPE_LEN;
  strncpy( info, dpid_str, chassis_id_strlen );

  // Create Port ID TLV
  port_id_tlv = append_back_buffer( lldp_buf, port_id_tlv_len );
  port_id_tlv->type_len = LLDP_TL( LLDP_TYPE_PORT_ID, port_id_tlv_len );
  info = port_id_tlv->val;
  *info = PORT_ID_SUBTYPE_LOCALLY_ASSINED;
  info += LLDP_SUBTYPE_LEN;
  strncpy( info, port_no_str, port_id_strlen );

  // Create TTL TLV
  ttl_tlv = append_back_buffer( lldp_buf, LLDP_TTL_LEN );
  ttl_tlv->type_len = LLDP_TL( LLDP_TYPE_TTL, LLDP_TTL_LEN );
  ttl_val = ( uint16_t * ) ttl_tlv->val;
  *ttl_val = htons( LLDP_DEFAULT_TTL );

  // Create END OF LLDPDU TLV
  end_tlv = append_back_buffer( lldp_buf, LLDP_END_PDU_LEN );
  end_tlv->type_len = LLDP_TL( LLDP_TYPE_END, LLDP_TLV_HEAD_LEN );

  void *appended_buffer = append_back_buffer( lldp_buf, 128 );
  memset( appended_buffer, 0, 128 );

  return lldp_buf;
}


#ifdef UNIT_TESTING
buffer *
_create_lldp_frame( const uint8_t *mac, uint64_t dpid, uint16_t port_no ) {
  return create_lldp_frame( mac, dpid, port_no );
}
#endif // UNIT_TESTING


static int
parse_lldp_ull( void *str, uint64_t *value, uint32_t len ) {
  unsigned int i;
  char *end_p;
  char buf[ LLDP_TLV_INFO_MAX_LEN ];
  char *p = ( char * ) str;

  for ( i = 0; i < len; i++ ) {
    buf[ i ] = *( ( char * ) p );
    p++;
  }
  buf[ len ] = '\0';

  *value = strtoull( buf, &end_p, 0 );
  if ( *end_p != '\0' ) {
    error( "Failed to strtoull." );
    return -1;
  }

  return 0;
}


static int
parse_lldp_us( void *str, uint16_t *value, uint32_t len ) {
  unsigned int i;
  char *end_p;
  char buf[ LLDP_TLV_INFO_MAX_LEN ];
  char *p = ( char * ) str;

  for ( i = 0; i < len; i++ ) {
    buf[ i ] = *( ( char * ) p );
    p++;
  }
  buf[ len ] = '\0';

  *value = ( uint16_t ) strtoul( buf, &end_p, 0 );
  if ( *end_p != '\0' ) {
    error( "Failed to strtoull." );
    return -1;
  }

  return 0;
}


static void
handle_packet_in( uint64_t dst_datapath_id,
                  uint32_t transaction_id __attribute__((unused)),
                  uint32_t buffer_id __attribute__((unused)),
                  uint16_t total_len __attribute__((unused)),
                  uint16_t dst_port_no,
                  uint8_t reason __attribute__((unused)),
                  const buffer *m,
                  void *user_data __attribute__((unused)) ) {
  uint16_t type = ntohs( packet_info( m )->l2_data.eth->type );

  // check if LLDP or not
  if ( type != ETH_ETHTYPE_LLDP ) {
    notice( "not LLDP: %x", type );
    return;
  }

  uint64_t src_datapath_id;
  uint16_t src_port_no;
  if ( recv_lldp( &src_datapath_id, &src_port_no, m ) < 0 ) {
    return;
  }

  debug( "Receive LLDP Frame (%#" PRIx64 ", %u) from (%#" PRIx64 ", %u).",
         dst_datapath_id, dst_port_no, src_datapath_id, src_port_no );
  probe_timer_entry *entry = delete_probe_timer_entry( &src_datapath_id,
                                                       src_port_no );
  if ( entry == NULL ) {
    debug( "Not found dst datapath_id (%#" PRIx64 ", %u).", src_datapath_id,
            src_port_no );
    return;
  }

  probe_request( entry, PROBE_TIMER_EVENT_RECV_LLDP, &dst_datapath_id, dst_port_no );
}


#ifdef UNIT_TESTING
void
_handle_packet_in( uint64_t datapath_id, uint32_t transaction_id,
                   uint32_t buffer_id, uint16_t total_len,
                   uint16_t in_port, uint8_t reason,
                   const buffer *m, void *user_data ) {
  handle_packet_in( datapath_id, transaction_id,
                    buffer_id, total_len, in_port, reason, m, user_data );
}
#endif // UNIT_TESTING


bool
init_lldp() {
#ifdef UNIT_TESTING
#define handle_packet_in _handle_packet_in
#endif

  // set packet-in handler
  init_openflow_application_interface( get_trema_name() );
  debug( "lldp service name: %s", get_trema_name() );
  set_packet_in_handler( handle_packet_in, NULL );

  return true;

#ifdef UNIT_TESTING
#undef handle_packet_in
#endif
}


bool
finalize_lldp() {
  // do something here
  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
