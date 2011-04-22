/*
 * Author: Yasunobu Chiba <y-chiba@bq.jp.nec.com>
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


#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <net/route.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include "redirector.h"


#define PKT_BUF_SIZE 1500
#define TUN_DEV "/dev/net/tun"
#define TUN_DEV_TXQ_LEN 100000
#define HOST_DB_ENTRY_TIMEOUT 600
#define HOST_DB_AGING_INTERVAL 60


static int fd = -1;
static const uint8_t redirector_mac[ ETH_ADDRLEN ] = { 0x00, 0x00, 0x00, 0x01, 0x01, 0x01 };
static char TUN_INTERFACE[ IFNAMSIZ ] = "of0";

typedef struct host_db_entry {
  uint8_t mac[ ETH_ADDRLEN ];
  uint32_t ip;
  uint64_t dpid;
  uint16_t port;
  time_t updated_at;
} host_entry;

static hash_table *host_db = NULL;


static bool
compare_ip_address( const void *x, const void *y ) {
  if ( memcmp( x, y, IPV4_ADDRLEN ) != 0 ) {
    return false;
  }
  return true;
}


static unsigned int
hash_ip_address( const void *ip ) {
  unsigned int value = 0;

  memcpy( &value, ip, IPV4_ADDRLEN );

  return value;
}


static bool
update_host_route( const int operation, const uint32_t ip ) {
  int nfd;
  struct sockaddr_in addr;
  struct rtentry rt;

  memset( &rt, 0, sizeof( rt ) );
  memset( &addr, 0, sizeof( addr ) );

  rt.rt_dev = TUN_INTERFACE;
  rt.rt_metric = 0;
  rt.rt_flags = RTF_UP;

  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = ip;

  memcpy( &rt.rt_dst, &addr, sizeof( struct sockaddr_in ) );

  inet_aton( "255.255.255.255", &addr.sin_addr );
  memcpy( &rt.rt_genmask, &addr, sizeof( struct sockaddr_in ) );

  switch ( operation ) {
  case SIOCADDRT:
  case SIOCDELRT:
    break;
  default:
    error( "Undefined operation type (operation = %d).", operation );
    return false;
  }

  nfd = socket( AF_INET, SOCK_DGRAM, 0 );
  
  if( ioctl( nfd, ( long unsigned int ) operation, ( void * ) &rt ) < 0 ) {
    error( "Cannot add/delete a routing table entry (%s).", strerror( errno ) );
    close( nfd );
    return false;
  }

  close( nfd );

  return true;
}

static bool
add_host_route( const uint32_t ip ) {
  return update_host_route( SIOCADDRT, ip );
}


static bool
delete_host_route( const uint32_t ip ) {
  return update_host_route( SIOCDELRT, ip );
}


static host_entry*
lookup_host( const uint32_t ip ) {
  host_entry *entry;
  struct in_addr addr;

  addr.s_addr = ip;

  debug( "Looking up a host entry (ip = %s).", inet_ntoa( addr ) );

  entry = lookup_hash_entry( host_db, &ip );

  if ( entry == NULL ) {
    debug( "Entry not found." );
    return NULL;
  }

  addr.s_addr = entry->ip;
  uint8_t *mac = entry->mac;
  uint64_t dpid = entry->dpid;
  uint16_t port = entry->port;
  time_t updated_at = entry->updated_at;

  debug( "A host entry found (mac = %02x:%02x:%02x:%02x:%02x:%02x, "
         "ip = %s, dpid = %#" PRIx64 ", port = %u, updated_at = %u).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ],
         inet_ntoa( addr ), dpid, port, updated_at );

  return entry;
}


static void
add_host( const uint8_t *mac, const uint32_t ip, const uint64_t dpid, const uint16_t port ) {
  host_entry *entry;
  struct in_addr addr;

  addr.s_addr = ip;

  entry = lookup_host( ip );

  if ( entry != NULL ) {
    debug( "Updating a host entry (mac = %02x:%02x:%02x:%02x:%02x:%02x, "
           "ip = %s, dpid = %#" PRIx64 ", port = %u).",
           mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ],
           inet_ntoa( addr ), dpid, port );

    memcpy( entry->mac, mac, ETH_ADDRLEN );
    entry->dpid = dpid;
    entry->port = port;
    entry->updated_at = time( NULL );

    return;
  }

  debug( "Adding a host entry (mac = %02x:%02x:%02x:%02x:%02x:%02x, "
         "ip = %s, dpid = %#" PRIx64 ", port = %u).",
         mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ],
         inet_ntoa( addr ), dpid, port );

  entry = xmalloc( sizeof( host_entry ) );

  memcpy( entry->mac, mac, ETH_ADDRLEN );
  entry->ip = ip;
  entry->dpid = dpid;
  entry->port = port;
  entry->updated_at = time( NULL );

  insert_hash_entry( host_db, &entry->ip, entry );

  add_host_route( entry->ip );
}


static void
age_host_db_entry( void *key, void *value, void *user_data ) {
  UNUSED( user_data );

  host_entry *entry = value;

  if ( entry->updated_at + HOST_DB_ENTRY_TIMEOUT < time( NULL ) ) {
    struct in_addr addr;
    addr.s_addr = entry->ip;
    debug( "Host DB: age out (ip = %s).", inet_ntoa( addr ) );
    delete_host_route( entry->ip );
    void *deleted = delete_hash_entry( host_db, key );
    xfree( deleted );
  }
}


static void
age_host_db( void *user_data ) {
  UNUSED( user_data );

  debug( "Host DB: start aging." );

  foreach_hash( host_db, age_host_db_entry, NULL );
}


static bool
init_tun( const char *name ) {
  int flags;
  int nfd;
  struct ifreq ifr;

  memset( &ifr, 0, sizeof( ifr ) );

  if ( fd >= 0 ) {
    error( "A tun device is already created." );
    return false;
  }

  fd = open( TUN_DEV, O_RDWR );
  if ( fd < 0 ) {
    error( "Cannot open %s.", TUN_DEV );
    return false;
  }

  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  strncpy( ifr.ifr_name, name, IFNAMSIZ );
  if( ioctl( fd, TUNSETIFF, ( void * ) &ifr ) < 0 ) {
    error( "Cannot set device name to %s.", ifr.ifr_name );
    close( fd );
    return false;
  }

  flags = fcntl( fd, F_GETFL );
  if( fcntl( fd, F_SETFL, O_NONBLOCK | flags ) < 0 ){
    error( "Cannot enable non-blocking mode (fd = %d).", fd );
    close( fd );
    return false;
  }

  nfd = socket( AF_INET, SOCK_DGRAM, 0 );
  
  ifr.ifr_qlen = TUN_DEV_TXQ_LEN;
  if ( ioctl( nfd, SIOCSIFTXQLEN, ( void * ) &ifr ) < 0 ) {
    error( "Cannot set txqueuelen to %d.", ifr.ifr_qlen );
    close( nfd );
    return false;
  }

  ifr.ifr_flags = 0;
  if ( ioctl( nfd, SIOCGIFFLAGS, ( void * ) &ifr ) < 0 ) {
    error( "Cannot get interface flags from %s.", ifr.ifr_name );
    close( nfd );
    return false;
  }
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  if( ioctl( nfd, SIOCSIFFLAGS, ( void * ) &ifr ) < 0 ) {
    error( "Cannot set interface flags to %#lx.", ifr.ifr_flags );
    close( nfd );
    return false;
  }

  close( nfd );

  return true;
}


static bool
finalize_tun() {
  if ( fd < 0 ) {
    error( "tun device is already closed." );
    return false;
  }

  if ( close( fd ) < 0 ) {
    error( "Cannot close tun device (fd = %d).", fd );
    return false;
  }

  return true;
}


static void
recv_packet_from_tun() {
  char data[ PKT_BUF_SIZE ];
  ssize_t ret;

  ret = read( fd, data, PKT_BUF_SIZE );

  if ( ret <= 0 ) {
    return;
  }

  debug( "%d bytes packet received from tun interface (fd = %d).", ret, fd );

  // FIXME: we need to parse the ipv4 packet.

  ipv4_header_t *ip_header = ( ipv4_header_t * ) data;
  host_entry *entry = lookup_host( ip_header->daddr );
  struct in_addr addr;
  addr.s_addr = ip_header->daddr;

  if ( entry == NULL ) {
    error( "Failed to resolve host location (ip = %s).",
           inet_ntoa( addr ) );
    return;
  }

  // create an Ethernet frame and send a packet-out
  void *p;
  uint16_t *type;
  buffer *frame;
  buffer *pout;
  openflow_actions *actions;
  size_t frame_length = ETH_ADDRLEN * 2 + 2 + ( size_t ) ret;

  frame = alloc_buffer_with_length( frame_length );

  p = append_back_buffer( frame, ETH_ADDRLEN );
  memcpy( p, entry->mac, ETH_ADDRLEN );
  p = append_back_buffer( frame, ETH_ADDRLEN );
  memcpy( p, redirector_mac, ETH_ADDRLEN );

  type = append_back_buffer( frame, 2 );
  *type = htons( ETH_ETHTYPE_IPV4 );

  p = append_back_buffer( frame, ( size_t ) ret );
  memcpy( p, data, ( size_t ) ret );

  debug( "Sending a packet-out to a switch (ip = %s, dpid = %#" PRIx64 ", port = %u).",
         inet_ntoa( addr ), entry->dpid, entry->port );

  actions = create_actions();
  append_action_output( actions, entry->port, UINT16_MAX );

  pout = create_packet_out( get_transaction_id(), UINT32_MAX, OFPP_NONE,
                            actions, frame );

  send_openflow_message( entry->dpid, pout );

  free_buffer( frame );
  free_buffer( pout );
  delete_actions( actions );
}


static void
send_packet_to_tun( const void *data, size_t length ) {
  ssize_t ret;

  debug( "Sending an IP packet to a tun interface (fd = %d, length = %u).",
         fd, length );

  ret = write( fd, data, length );
  if ( ret <= 0 ) {
    if ( ret == EAGAIN ) {
      warn( "EAGAIN" );
    }
    error( "Failed to send a packet to a tun interface (fd = %d).", fd );
    return;
  }
  
  if ( ret != ( ssize_t ) length ) {
    warn( "Only a part of packet is sent (pkt_len = %d, sent_len = %u).", length, ret );
  }

  debug( "A packet is sent to a tun interface (fd = %d, sent_len = %u).", fd, length );
}


static void
set_tun_fd( fd_set *read_set, fd_set *write_set ) {
  UNUSED( write_set );

  if ( fd < 0 ) {
    error( "Invalid file descripter (fd = %d).", fd );
    return;
  }

#undef __FDMASK
#define __FDMASK( d ) ( ( __fd_mask ) 1 << ( ( d ) % __NFDBITS ) )
  FD_SET( fd, read_set );
}


static void
read_tun_fd( fd_set *read_set, fd_set *write_set ) {
  UNUSED( write_set );

  if ( fd < 0 ) {
    error( "Invalid file descripter (fd = %d).", fd );
    return;
  }

  if ( FD_ISSET( fd, read_set ) ) {
    recv_packet_from_tun();
  }
}


bool
init_redirector() {
  if ( !init_tun( TUN_INTERFACE ) ) {
    error( "Cannot create a tun interface." );
    return false;
  }

  if ( host_db != NULL ) {
    error( "Host database is already created." );
    return false;
  }

  host_db = create_hash( compare_ip_address, hash_ip_address );

  set_fd_set_callback( set_tun_fd );
  set_check_fd_isset_callback( read_tun_fd );

  add_periodic_event_callback( HOST_DB_AGING_INTERVAL, age_host_db, NULL );

  return true;
}


bool
finalize_redirector() {
  hash_iterator iter;
  hash_entry *entry;

  init_hash_iterator( host_db, &iter );
  while ( ( entry = iterate_hash_next( &iter ) ) != NULL ) {
      xfree( entry->value );
  }
  delete_hash( host_db );
  
  return finalize_tun();
}


void
redirect( uint64_t datapath_id, uint16_t in_port, const buffer *data ) {
  debug( "A message received (dpid = %#" PRIx64 ", in_port = %u, length = %u).",
         datapath_id, in_port, data->length );

  uint8_t *mac = packet_info( data )->l2_data.eth->macsa;
  uint32_t ip = packet_info( data )->l3_data.ipv4->saddr;

  add_host( mac, ip, datapath_id, in_port );

  debug( "Redirecting an IP packet to tun interface." );
  // redirect an IP packet to a tun interface
  send_packet_to_tun( packet_info( data )->l3_data.l3,
                      ntohs( packet_info( data )->l3_data.ipv4->tot_len ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
