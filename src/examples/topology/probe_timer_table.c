/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include "trema.h"
#include "libtopology.h"
#include "lldp.h"
#include "probe_timer_table.h"


#ifdef UNIT_TESTING

#define static

#endif // UNIT_TESTING


dlist_element *probe_timer_table;
dlist_element *probe_timer_last;


struct timespec port_down_time;


static inline void
get_current_time( struct timespec *now ) {
  clock_gettime( CLOCK_MONOTONIC, now );
}


static inline void
set_expires( long millisecond, probe_timer_entry *entry ) {
  get_current_time( &( entry->expires ) );
  entry->expires.tv_sec += millisecond / 1000;
  entry->expires.tv_nsec += ( millisecond * 1000000 ) % 1000000000;
  if ( entry->expires.tv_nsec >= 1000000000 ) {
    entry->expires.tv_sec++;
    entry->expires.tv_nsec -= 1000000000;
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


static inline bool
other_port_status_changed( probe_timer_entry *entry ) {
  if ( entry->link_up ) {
    probe_timer_entry *peer = lookup_probe_timer_entry( &entry->to_datapath_id,
                                                        entry->to_port_no );
    return peer == NULL;
  }
  return !timespec_le( &port_down_time, &( entry->link_down_time ) );
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
    entry->retry_count = 3;
  }
  set_random_expires( 500, 2000, entry );
}


static void
set_confirmed_state( probe_timer_entry *entry ) {
  if ( entry->state != PROBE_TIMER_STATE_CONFIRMED ) {
    entry->state = PROBE_TIMER_STATE_CONFIRMED;
    entry->retry_count = 6;
  }
  set_expires( 10000, entry );
}


void
probe_request( probe_timer_entry *entry, int event, uint64_t *dpid, uint16_t port_no ) {
  if ( event == PROBE_TIMER_EVENT_UP ) {
    get_current_time( &port_down_time );
  }

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
          send_lldp( entry );
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
          entry->link_up = true;
          entry->to_datapath_id = *dpid;
          entry->to_port_no = port_no;

          topology_update_link_status link_status;
          link_status.from_dpid = entry->datapath_id;
          link_status.from_portno = entry->port_no;
          link_status.to_dpid = *dpid;
          link_status.to_portno = port_no;
          link_status.status = TD_PORT_UP;
          set_link_status( &link_status, NULL, NULL );
          break;
        case PROBE_TIMER_EVENT_TIMEOUT:
          if ( --entry->retry_count > 0 ) {
            set_wait_state( entry );
            send_lldp( entry );
          } else {
            set_confirmed_state( entry );
            entry->link_up = false;
            entry->to_datapath_id = 0;
            entry->to_port_no = 0;
            get_current_time( &( entry->link_down_time ) );

            topology_update_link_status link_status;
            link_status.from_dpid = entry->datapath_id;
            link_status.from_portno = entry->port_no;
            link_status.to_dpid = 0;
            link_status.to_portno = 0;
            link_status.status = TD_PORT_DOWN;
            set_link_status( &link_status, NULL, NULL );
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
          if ( --entry->retry_count > 0
            && !other_port_status_changed( entry ) ) {
            set_confirmed_state( entry );
          } else {
            set_send_delay_state( entry );
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
    debug( "Update probe state: %d <= %d by event %d. dpid %" PRIx64 " %u.",
           entry->state, old_state, event,
           entry->datapath_id, entry->port_no );
  }

  if ( entry->state != PROBE_TIMER_STATE_INACTIVE ) {
    insert_probe_timer_entry( entry );
  }
}


static inline void
timespec_sub( const struct timespec *a, const struct timespec *b,
              struct timespec *result ) {
  result->tv_sec = a->tv_sec - b->tv_sec;
  result->tv_nsec = a->tv_nsec - b->tv_nsec;
  if ( result->tv_nsec < 0 ) {
    result->tv_sec--;
    result->tv_nsec += 1000000000;
  }
}


static void interval_timer_event( void *user_data );


#if 1
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
#else
static void
unset_interval_timer( void ) {
  delete_timer_event_callback( interval_timer_event );
  debug( "unset interval timer" );
}


static void
set_interval_timer( void ) {
  unset_interval_timer();

  if ( probe_timer_table->next == NULL ) {
    debug( "interval: nil" );
    return;
  }
  probe_timer_entry *entry = probe_timer_table->next->data;

  struct timespec now;
  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 0;
  get_current_time( &now );
  timespec_sub( &( entry->expires ), &now, &( interval.it_value ) );
  if ( interval.it_value.tv_sec < 0 || interval.it_value.tv_nsec < 0 ) {
    interval.it_value.tv_sec = 0;
    interval.it_value.tv_nsec = 1;
  }

  debug( "interval: %d.%09d", interval.it_value.tv_sec, interval.it_value.tv_nsec );

  add_timer_event_callback( &interval, interval_timer_event, NULL );
  debug( "set interval timer" );
}
#endif


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
#if 0
      set_interval_timer();
#endif
      return;
    }
  }
}


void
init_probe_timer_table( void ) {
  probe_timer_table = create_dlist();
  probe_timer_last = probe_timer_table;

  srandom( ( unsigned int ) time( NULL ) );

#if 1
  set_interval_timer();
#endif
}


void
finalize_probe_timer_table( void ) {
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

  return new_entry;
}


void
free_probe_timer_entry( probe_timer_entry *free_entry ) {
  xfree( free_entry );
}


void
insert_probe_timer_entry( probe_timer_entry *new_entry ) {
  probe_timer_entry *last_entry = probe_timer_last->data;
  if ( probe_timer_table->next == NULL
       || timespec_le( &(last_entry->expires ), &( new_entry->expires ) ) ) {
    probe_timer_last = insert_after_dlist( probe_timer_last, new_entry );

    goto set_interval_timer;
  }

  dlist_element *dlist;
  probe_timer_entry *entry;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = dlist->next ) {
    entry = dlist->data;
    if ( timespec_le( &(entry->expires ), &( new_entry->expires ) ) ) {
      continue;
    }
    insert_before_dlist( dlist, new_entry );
    goto set_interval_timer;
  }
  UNREACHABLE();

set_interval_timer:

  if ( probe_timer_table->next->data != new_entry ) {
    return;
  }

#if 0
  set_interval_timer();
#endif
}


probe_timer_entry *
delete_probe_timer_entry( const uint64_t *datapath_id, uint16_t port_no ) {
  dlist_element *dlist;
  probe_timer_entry *entry;
  bool top = true;
  for ( dlist = probe_timer_table->next; dlist != NULL; dlist = dlist->next ) {
    entry = dlist->data;
    if ( entry->datapath_id == *datapath_id && entry->port_no == port_no ) {
      if ( dlist == probe_timer_last ) {
        probe_timer_last = dlist->prev;
      }
      delete_dlist_element( dlist );

#if 0
      if ( probe_timer_table->next == NULL ) {
        unset_interval_timer();
      } else if ( top ) {
        set_interval_timer();
      }
#endif

      return entry;
    }
    top = false;
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

