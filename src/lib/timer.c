/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
#include <errno.h>
#include "doubly_linked_list.h"
#include "log.h"
#include "timer.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#ifdef clock_gettime
#undef clock_gettime
#endif
#define clock_gettime mock_clock_gettime
extern int mock_clock_gettime( clockid_t clk_id, struct timespec *tp );

#ifdef error
#undef error
#endif
#define error mock_error
void mock_error( const char *format, ... );

#ifdef debug
#undef debug
#endif
#define debug mock_debug
void mock_debug( const char *format, ... );

#define static

#endif // UNIT_TESTING


typedef struct timer_callback {
  void ( *function )( void *user_data );
  struct timespec expires_at;
  struct timespec interval;
  void *user_data;
} timer_callback;


static dlist_element *timer_callbacks = NULL;


bool
init_timer() {
  timer_callbacks = create_dlist();
  return true;
}


bool
finalize_timer() {
  dlist_element *e;

  debug( "Deleting timer callbacks ( timer_callbacks = %p ).", timer_callbacks );

  if ( timer_callbacks != NULL ) {
    for ( e = timer_callbacks->next; e; e = e->next ) {
      xfree( e->data );
    }
    delete_dlist( timer_callbacks );
    timer_callbacks = NULL;
  }
  else {
    error( "All timer callbacks are already deleted or not created yet." );
  }
  return true;
}


#define VALID_TIMESPEC( _a )                                    \
  ( ( ( _a )->tv_sec > 0 || ( _a )->tv_nsec > 0 ) ? 1 : 0 )

#define ADD_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec + ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec + ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec >= 1000000000 ) {               \
      ( _return )->tv_sec++;                                  \
      ( _return )->tv_nsec -= 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )


static void
on_timer( timer_callback *callback ) {
  assert( callback != NULL );
  assert( callback->function != NULL );

  debug( "Executing a timer event ( function = %p, expires_at = %u.%09u, interval = %u.%09u, user_data = %p ).",
         callback->function, callback->expires_at.tv_sec, callback->expires_at.tv_nsec,
         callback->interval.tv_sec, callback->interval.tv_nsec, callback->user_data );

  if ( VALID_TIMESPEC( &callback->expires_at ) ) {
    callback->function( callback->user_data );
    if ( VALID_TIMESPEC( &callback->interval ) ) {
      ADD_TIMESPEC( &callback->expires_at, &callback->interval, &callback->expires_at );
    }
    else {
      callback->expires_at.tv_sec = 0;
      callback->expires_at.tv_nsec = 0;
      callback->function = NULL;
    }
    debug( "Set expires_at value to %u.%09u.", callback->expires_at.tv_sec, callback->expires_at.tv_nsec );
  }
  else {
    error( "Invalid expires_at value." );
  }
}


void
execute_timer_events() {
  struct timespec now;
  timer_callback *callback;
  dlist_element *element, *element_next;

  debug( "Executing timer events ( timer_callbacks = %p ).", timer_callbacks );

  assert( clock_gettime( CLOCK_MONOTONIC, &now ) == 0 );
  assert( timer_callbacks != NULL );

  // TODO: timer_callbacks should be a list which is sorted by expiry time
  for ( element = timer_callbacks->next; element; element = element_next ) {
    element_next = element->next;
    callback = element->data;
    if ( callback->function != NULL
         && ( ( callback->expires_at.tv_sec < now.tv_sec )
              || ( ( callback->expires_at.tv_sec == now.tv_sec )
                   && ( callback->expires_at.tv_nsec <= now.tv_nsec ) ) ) ) {
      on_timer( callback );
    }
    if ( callback->function == NULL ) {
      xfree( callback );
      delete_dlist_element( element );
    }
  }
}


bool
add_timer_event_callback( struct itimerspec *interval, void ( *callback )( void *user_data ), void *user_data ) {
  assert( interval != NULL );
  assert( callback != NULL );

  debug( "Adding a timer event callback ( interval = %u.%09u, initial expiration = %u.%09u, callback = %p, user_data = %p ).",
         interval->it_interval.tv_sec, interval->it_interval.tv_nsec,
         interval->it_value.tv_sec, interval->it_value.tv_nsec, callback, user_data );

  timer_callback *cb;
  struct timespec now;

  cb = xmalloc( sizeof( timer_callback ) );
  memset( cb, 0, sizeof( timer_callback ) );
  cb->function = callback;
  cb->user_data = user_data;

  if ( clock_gettime( CLOCK_MONOTONIC, &now ) != 0 ) {
    error( "Failed to retrieve monotonic time ( %s [%d] ).", strerror( errno ), errno );
    xfree( cb );
    return false;
  }

  cb->interval = interval->it_interval;

  if ( VALID_TIMESPEC( &interval->it_value ) ) {
    ADD_TIMESPEC( &now, &interval->it_value, &cb->expires_at );
  }
  else if ( VALID_TIMESPEC( &interval->it_interval ) ) {
    ADD_TIMESPEC( &now, &interval->it_interval, &cb->expires_at );
  }
  else {
    error( "Timer must not be zero when a timer event is added." );
    xfree( cb );
    return false;
  }

  debug( "Set an initial expiration time to %u.%09u.", now.tv_sec, now.tv_nsec );

  assert( timer_callbacks != NULL );
  insert_after_dlist( timer_callbacks, cb );

  return true;
}


bool
delete_timer_event_callback( void ( *callback )( void *user_data ) ) {
  assert( callback != NULL );

  debug( "Deleting a timer event callback ( callback = %p ).", callback );

  dlist_element *e;

  if ( timer_callbacks == NULL ) {
    debug( "All timer callbacks are already deleted or not created yet." );
    return false;
  }

  for ( e = timer_callbacks->next; e; e = e->next ) {
    timer_callback *cb = e->data;
    if ( cb->function == callback ) {
      debug( "Deleting a callback ( callback = %p ).", callback );
      //xfree( cb );
      //delete_dlist_element( e );
      cb->function = NULL;
      cb->user_data = NULL;
      cb->expires_at.tv_sec = 0;
      cb->expires_at.tv_nsec = 0;
      cb->interval.tv_sec = 0;
      cb->interval.tv_nsec = 0;
      return true;
    }
  }

  error( "No registered timer event callback found." );

  return false;
}


bool
add_periodic_event_callback( const time_t seconds, void ( *callback )( void *user_data ), void *user_data ) {
  assert( callback != NULL );

  debug( "Adding a periodic event callback ( interval = %u, callback = %p, user_data = %p ).",
         seconds, callback, user_data );

  struct itimerspec interval;

  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = seconds;
  interval.it_interval.tv_nsec = 0;

  return add_timer_event_callback( &interval, callback, user_data );
}


bool
delete_periodic_event_callback( void ( *callback )( void *user_data ) ) {
  assert( callback != NULL );

  debug( "Deleting a periodic event callback ( callback = %p ).", callback );

  return delete_timer_event_callback( callback );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
