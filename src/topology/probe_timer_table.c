/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include "trema.h"
#include "probe_timer_table.h"

#include "discovery_management.h"
#include "service_management.h"


dlist_element *probe_timer_table;
dlist_element *probe_timer_last;


static inline void
get_current_time( struct timespec *now ) {
  clock_gettime( CLOCK_MONOTONIC, now );
}


static inline void
set_expires( long millisecond, probe_timer_entry *entry ) {
  const long MSEC_IN_NSEC = 1000000;
  const long SEC_IN_NSEC = 1000000000;
  get_current_time( &( entry->expires ) );
  entry->expires.tv_sec += millisecond / 1000;
  entry->expires.tv_nsec += ( millisecond * MSEC_IN_NSEC ) % SEC_IN_NSEC;
  if ( entry->expires.tv_nsec >= SEC_IN_NSEC ) {
    entry->expires.tv_sec++;
    entry->expires.tv_nsec -= SEC_IN_NSEC;
  }
  debug( "set expires: %ld", millisecond );
}


static inline void
set_random_expires( long min, long max, probe_timer_entry *entry ) {
  long millisecond = min + ( max - min ) * random() / RAND_MAX;
  set_expires( millisecond, entry );
}


static inline bool
timespec_le( const struct timespec *a, const struct timespec *b ) {
  // "a" is less than or equal to "b"
  if ( a->tv_sec < b->tv_sec ) {
    return true;
  }
  if ( a->tv_sec > b->tv_sec ) {
    return false;
  }
  if ( a->tv_nsec < b->tv_nsec ) {
    return true;
  }
  if ( a->tv_nsec > b->tv_nsec ) {
    return false;
  }
  return true;
}


static inline void
mark_peer_dirty_if_link_up( probe_timer_entry *entry ) {
  probe_timer_entry *peer = lookup_probe_timer_entry( &entry->to_datapath_id, entry->to_port_no );
  if ( peer == NULL ) {
    return;
  }
  if ( peer->link_up ) {
    peer->dirty = true;
  }
}


static inline void
mark_peer_dirty_if_link_mismatch( probe_timer_entry *entry ) {
  probe_timer_entry *peer = lookup_probe_timer_entry( &entry->to_datapath_id, entry->to_port_no );
  if ( peer == NULL ) {
    return;
  }
  if ( entry->link_up != peer->link_up ) {
    peer->dirty = true;
  }
}


static void
set_inactive_state( probe_timer_entry *entry ) {
  if ( entry->state != PROBE_TIMER_STATE_INACTIVE ) {
    entry->state = PROBE_TIMER_STATE_INACTIVE;
    entry->retry_count = 1;
  }
  set_expires( 0, entry );
}


static void
set_send_delay_state( probe_timer_entry *entry ) {
  if ( entry->state != PROBE_TIMER_STATE_SEND_DELAY ) {
    entry->state = PROBE_TIMER_STATE_SEND_DELAY;
    entry->retry_count = 1;
  }
  set_random_expires( 500, 2000, entry );
}


static void
set_wait_state( probe_timer_entry *entry ) {
  if ( entry->state != PROBE_TIMER_STATE_WAIT ) {
    entry->state = PROBE_TIMER_STATE_WAIT;
    entry->retry_count = 2;
  }
  if ( entry->retry_count > 1 ) {
    set_random_expires( 2000, 4000, entry );
  }
  else {
    set_expires( 8000, entry );
  }
}


static void
reset_wait_state( probe_timer_entry *entry ) {
  entry->retry_count++;
  if ( entry->retry_count > 1 ) {
    set_random_expires( 4000, 8000, entry );
  }
  else {
    // unreachable in current transition rule.
    set_expires( 16000, entry );
  }
}


static void
set_confirmed_state( probe_timer_entry *entry ) {
  if ( entry->state != PROBE_TIMER_STATE_CONFIRMED ) {
    entry->state = PROBE_TIMER_STATE_CONFIRMED;
    entry->retry_count = 12;
  }
  set_expires( 5000, entry );
}


static void
reset_confirmed_state( probe_timer_entry *entry ) {
  entry->retry_count = 24;
}


void
probe_request( probe_timer_entry *entry, int event, uint64_t *dpid, uint16_t port_no ) {
  int old_state = entry->state;
  switch ( entry->state ) {
    case PROBE_TIMER_STATE_INACTIVE:
      switch( event ) {
        case PROBE_TIMER_EVENT_UP:
          set_send_delay_state( entry );
          break;
        default:
          break;
      }
      break;
    case PROBE_TIMER_STATE_SEND_DELAY:
      switch( event ) {
        case PROBE_TIMER_EVENT_DOWN:
          set_inactive_state( entry );
          break;
        case PROBE_TIMER_EVENT_TIMEOUT:
          set_wait_state( entry );
          bool ret = send_probe( entry->mac, entry->datapath_id, entry->port_no );
          if ( !ret ) {
            reset_wait_state( entry );
          }
          else {
            entry->dirty = false;
          }
          break;
        default:
          break;
      }
      break;
    case PROBE_TIMER_STATE_WAIT:
      switch( event ) {
        case PROBE_TIMER_EVENT_DOWN:
          set_inactive_state( entry );
          break;
        case PROBE_TIMER_EVENT_RECV_LLDP:
          set_confirmed_state( entry );

          topology_update_link_status link_status;
          link_status.from_dpid = entry->datapath_id;
          link_status.from_portno = entry->port_no;
          link_status.to_dpid = *dpid;
          link_status.to_portno = port_no;
          link_status.status = TD_LINK_UP;
          entry->to_datapath_id = *dpid;
          entry->to_port_no = port_no;
          entry->link_up = true;
          debug( "Link up (%#" PRIx64 ":%u)->(%#" PRIx64 ":%u)", link_status.from_dpid, link_status.from_portno, link_status.to_dpid, link_status.to_portno );
          uint8_t result = set_discovered_link_status( &link_status );
          if ( result == TD_RESPONSE_OK ) {
            mark_peer_dirty_if_link_mismatch( entry );
          }
          else {
            reset_confirmed_state( entry );
          }
          break;
        case PROBE_TIMER_EVENT_TIMEOUT:
          if ( --entry->retry_count > 0 ) {
            set_wait_state( entry );
            bool ret = send_probe( entry->mac, entry->datapath_id, entry->port_no );
            if ( !ret ) {
              reset_wait_state( entry );
            }
            else {
              entry->dirty = false;
            }
          } else {
            set_confirmed_state( entry );

            topology_update_link_status link_status;
            link_status.from_dpid = entry->datapath_id;
            link_status.from_portno = entry->port_no;
            link_status.to_dpid = entry->to_datapath_id;
            link_status.to_portno = entry->to_port_no;
            if ( entry->link_up ) {
              link_status.status = TD_LINK_UNSTABLE;
            }
            else {
              link_status.status = TD_LINK_DOWN;
            }
            entry->to_datapath_id = 0;
            entry->to_port_no = 0;
            entry->link_up = false;
            debug( "Link down (%#" PRIx64 ":%u)->(%#" PRIx64 ":%u)", link_status.from_dpid, link_status.from_portno, link_status.to_dpid, link_status.to_portno );
            uint8_t result = set_discovered_link_status( &link_status );
            if ( result == TD_RESPONSE_OK ) {
              mark_peer_dirty_if_link_mismatch( entry );
            }
            else {
              reset_confirmed_state( entry );
            }
          }
          break;
        default:
          break;
      }
      break;
    case PROBE_TIMER_STATE_CONFIRMED:
      switch( event ) {
        case PROBE_TIMER_EVENT_DOWN:
          set_inactive_state( entry );
          break;
        case PROBE_TIMER_EVENT_TIMEOUT:
          --(entry->retry_count);
          if ( entry->retry_count > 0
            && !entry->dirty ) {
            set_confirmed_state( entry );
          } else {
            set_send_delay_state( entry );
          }
          break;
        case PROBE_TIMER_EVENT_RECV_LLDP:
          if ( !entry->link_up ) {
            // unstable link
            set_confirmed_state( entry );
            entry->dirty = true;

            topology_update_link_status link_status;
            link_status.from_dpid = entry->datapath_id;
            link_status.from_portno = entry->port_no;
            link_status.to_dpid = *dpid;
            link_status.to_portno = port_no;
            link_status.status = TD_LINK_UNSTABLE;
            debug( "Link unstable (%#" PRIx64 ":%u)->(%#" PRIx64 ":%u)", link_status.from_dpid, link_status.from_portno, link_status.to_dpid, link_status.to_portno );
            uint8_t result = set_discovered_link_status( &link_status );
            if ( result != TD_RESPONSE_OK ) {
              warn( "Failed to set (%#" PRIx64 ",%d)->(%#" PRIx64 ",%d) status to TD_LINK_UNSTABLE.", link_status.from_dpid, link_status.from_portno, link_status.to_dpid, link_status.to_portno );
            }
          }
          break;
        default:
          break;
      }
      break;
    default:
      UNREACHABLE();
      break;
  }

  if ( entry->state != old_state ) {
    debug( "Update probe state: %d <= %d by event %d. dpid %#" PRIx64 " %u.",
           entry->state, old_state, event,
           entry->datapath_id, entry->port_no );
  }

  if ( entry->state == PROBE_TIMER_STATE_INACTIVE ) {
    mark_peer_dirty_if_link_up( entry );
  }
  else {
    insert_probe_timer_entry( entry );
  }
}


static void interval_timer_event( void *user_data );


static void
set_interval_timer( void ) {
  struct itimerspec interval;

  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 500000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;

  add_timer_event_callback( &interval, interval_timer_event, NULL );
  debug( "set interval timer" );
}


static void
remove_interval_timer( void ) {
  delete_timer_event( interval_timer_event, NULL );
  debug( "remove interval timer" );
}


static void
interval_timer_event( void *user_data ) {
  UNUSED( user_data );

  struct timespec now;
  dlist_element *dlist, *next;
  probe_timer_entry *entry;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = next ) {
    next = dlist->next;
    entry = dlist->data;
    get_current_time( &now );
    if ( timespec_le( &( entry->expires ), &now ) ) {
      if ( dlist == probe_timer_last ) {
        probe_timer_last = dlist->prev;
      }
      delete_dlist_element( dlist );
      probe_request( entry, PROBE_TIMER_EVENT_TIMEOUT, 0, 0 );
    } else {
      return;
    }
  }
}


void
init_probe_timer_table( void ) {
  probe_timer_table = create_dlist();
  probe_timer_last = probe_timer_table;

  srandom( ( unsigned int ) time( NULL ) );

  set_interval_timer();
}


void
finalize_probe_timer_table( void ) {
  remove_interval_timer();

  dlist_element *dlist;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = dlist->next ) {
    xfree( dlist->data );
  }
  delete_dlist( probe_timer_table );
  probe_timer_table = NULL;
  probe_timer_last = NULL;
}


probe_timer_entry *
allocate_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no,
                            const uint8_t *mac ) {
  probe_timer_entry *new_entry;

  new_entry = xmalloc( sizeof( probe_timer_entry ) );
  new_entry->datapath_id = *datapath_id;
  new_entry->port_no = port_no;
  memcpy( new_entry->mac, mac, ETH_ADDRLEN );
  new_entry->state = PROBE_TIMER_STATE_INACTIVE;
  new_entry->retry_count = 0;
  new_entry->link_up = false;
  new_entry->to_datapath_id = 0;
  new_entry->to_port_no = 0;
  new_entry->dirty = false;

  return new_entry;
}


void
free_probe_timer_entry( probe_timer_entry *free_entry ) {
  xfree( free_entry );
}


void
insert_probe_timer_entry( probe_timer_entry *new_entry ) {
  if ( probe_timer_table->next == NULL ) { // empty
    probe_timer_last = insert_after_dlist( probe_timer_table, new_entry );
    return;
  }

  dlist_element *dlist;
  probe_timer_entry *entry;
  for ( dlist = probe_timer_last; dlist->prev != NULL; dlist = dlist->prev ) {
    entry = dlist->data;
    if ( timespec_le( &(entry->expires ), &( new_entry->expires ) ) ) { // entry <= new_entry
      break;
    }
  }
  dlist_element *new_dlist = insert_after_dlist( dlist, new_entry );
  if ( dlist == probe_timer_last ) {
    probe_timer_last = new_dlist;
  }
}


probe_timer_entry *
delete_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no ) {
  dlist_element *dlist;
  probe_timer_entry *entry;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = dlist->next ) {
    entry = dlist->data;
    if ( entry->datapath_id == *datapath_id && entry->port_no == port_no ) {
      if ( dlist == probe_timer_last ) {
        probe_timer_last = dlist->prev;
      }
      delete_dlist_element( dlist );
      return entry;
    }
  }

  return NULL;
}


probe_timer_entry *
lookup_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no ) {
  dlist_element *dlist;
  probe_timer_entry *entry;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = dlist->next ) {
    entry = dlist->data;
    if ( entry->datapath_id == *datapath_id && entry->port_no == port_no ) {
      return entry;
    }
  }
  return NULL;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

