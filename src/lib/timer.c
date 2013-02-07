/*
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
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
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


typedef struct timer_callback_info {
  timer_callback function;
  struct timespec expires_at;
  struct timespec interval;
  void *user_data;
} timer_callback_info;


static dlist_element *timer_callbacks = NULL;


bool
_init_timer() {
  if ( timer_callbacks != NULL ) {
    error( "Called init_timer twice." );
    return false;
  }

  timer_callbacks = create_dlist();

  debug( "Initializing timer callbacks ( timer_callbacks = %p ).", timer_callbacks );
  return true;
}
bool ( *init_timer )( void ) = _init_timer;


bool
_finalize_timer() {
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
bool ( *finalize_timer )( void ) = _finalize_timer;


#define VALID_TIMESPEC( _a )                                  \
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

#define SUB_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec - ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec - ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec < 0 ) {                         \
      ( _return )->tv_sec--;                                  \
      ( _return )->tv_nsec += 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )

#define TIMESPEC_LESS_THEN( _a, _b )                          \
  ( ( ( _a )->tv_sec == ( _b )->tv_sec ) ?                    \
    ( ( _a )->tv_nsec < ( _b )->tv_nsec ) :                   \
    ( ( _a )->tv_sec < ( _b )->tv_sec ) )

#define TIMESPEC_TO_MICROSECONDS( _a, _b )                    \
  do {                                                        \
    struct timespec round = { 0, 999 };                       \
    ADD_TIMESPEC( _a, &round, &round );                       \
    ( *( _b ) = ( int ) ( round.tv_sec * 1000000 + round.tv_nsec / 1000 ) ); \
  }                                                           \
  while ( 0 )


static void
on_timer( timer_callback_info *callback, struct timespec *now ) {
  assert( callback != NULL );
  assert( callback->function != NULL );

  debug( "Executing a timer event ( function = %p, expires_at = %" PRIu64 ".%09lu, interval = %" PRIu64 ".%09lu, user_data = %p ).",
         callback->function, ( int64_t ) callback->expires_at.tv_sec, callback->expires_at.tv_nsec,
         ( int64_t ) callback->interval.tv_sec, callback->interval.tv_nsec, callback->user_data );

  if ( VALID_TIMESPEC( &callback->expires_at ) ) {
    callback->function( callback->user_data );
    if ( VALID_TIMESPEC( &callback->interval ) ) {
      ADD_TIMESPEC( &callback->expires_at, &callback->interval, &callback->expires_at );
      if ( TIMESPEC_LESS_THEN( &callback->expires_at, now ) ) {
        callback->expires_at.tv_sec = now->tv_sec;
        callback->expires_at.tv_nsec = now->tv_nsec;
      }
    }
    else {
      callback->expires_at.tv_sec = 0;
      callback->expires_at.tv_nsec = 0;
      callback->function = NULL;
    }
    debug( "Set expires_at value to %" PRIu64 ".%09lu.", ( int64_t ) callback->expires_at.tv_sec, callback->expires_at.tv_nsec );
  }
  else {
    error( "Invalid expires_at value." );
  }
}


static void
insert_timer_callback( timer_callback_info *new_cb ) {
  assert( timer_callbacks != NULL );
  dlist_element *element, *last = timer_callbacks;
  for ( element = timer_callbacks->next; element != NULL; element = element->next ) {
    timer_callback_info *cb = element->data;
    if ( TIMESPEC_LESS_THEN( &new_cb->expires_at, &cb->expires_at ) ) {
      insert_before_dlist( element, new_cb );
      return;
    }
    last = element;
  }
  insert_after_dlist( last, new_cb );
}


void
_execute_timer_events( int *next_timeout_usec ) {
  assert( next_timeout_usec != NULL );
  struct timespec now;
  timer_callback_info *callback;
  dlist_element *element, *element_next;

  debug( "Executing timer events ( timer_callbacks = %p ).", timer_callbacks );

  assert( clock_gettime( CLOCK_MONOTONIC, &now ) == 0 );
  assert( timer_callbacks != NULL );

  for ( element = timer_callbacks->next; element; element = element_next ) {
    element_next = element->next;
    callback = element->data;
    if ( callback->function != NULL ) {
      if ( TIMESPEC_LESS_THEN( &now, &callback->expires_at ) ) {
        break;
      }
      on_timer( callback, &now );
    }
    delete_dlist_element( element );
    if ( callback->function == NULL ) {
      xfree( callback );
    }
    else {
      insert_timer_callback( callback );
    }
  }

  struct timespec max_timeout = { ( INT_MAX / 1000000 ), 0 };
  struct timespec min_timeout = { 0, 0 };
  if ( timer_callbacks->next == NULL ) {
    TIMESPEC_TO_MICROSECONDS( &max_timeout, next_timeout_usec );
  }
  else {
    callback = timer_callbacks->next->data;
    if ( TIMESPEC_LESS_THEN( &callback->expires_at, &now ) ) {
      TIMESPEC_TO_MICROSECONDS( &min_timeout, next_timeout_usec );
    }
    else {
      struct timespec timeout;
      SUB_TIMESPEC( &callback->expires_at, &now, &timeout );
      if ( TIMESPEC_LESS_THEN( &timeout, &max_timeout ) ) {
        TIMESPEC_TO_MICROSECONDS( &timeout, next_timeout_usec );
      }
      else {
        TIMESPEC_TO_MICROSECONDS( &max_timeout, next_timeout_usec );
      }
    }
  }
}
void ( *execute_timer_events )( int * ) = _execute_timer_events;


bool
_add_timer_event_callback( struct itimerspec *interval, timer_callback callback, void *user_data ) {
  assert( interval != NULL );
  assert( callback != NULL );

  debug( "Adding a timer event callback ( interval = %" PRIu64 ".%09lu, initial expiration = %" PRIu64 ".%09lu, callback = %p, user_data = %p ).",
         ( int64_t ) interval->it_interval.tv_sec, interval->it_interval.tv_nsec,
         ( int64_t ) interval->it_value.tv_sec, interval->it_value.tv_nsec, callback, user_data );

  timer_callback_info *cb;
  struct timespec now;

  cb = xmalloc( sizeof( timer_callback_info ) );
  memset( cb, 0, sizeof( timer_callback_info ) );
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

  debug( "Set an initial expiration time to %" PRIu64 ".%09lu.", ( int64_t ) now.tv_sec, now.tv_nsec );

  assert( timer_callbacks != NULL );
  insert_timer_callback( cb );

  return true;
}
bool ( *add_timer_event_callback )( struct itimerspec *interval, timer_callback callback, void *user_data ) = _add_timer_event_callback;


bool
_add_periodic_event_callback( const time_t seconds, timer_callback callback, void *user_data ) {
  assert( callback != NULL );

  debug( "Adding a periodic event callback ( interval = %" PRIu64 ", callback = %p, user_data = %p ).",
         ( const int64_t ) seconds, callback, user_data );

  struct itimerspec interval;

  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  interval.it_interval.tv_sec = seconds;
  interval.it_interval.tv_nsec = 0;

  return add_timer_event_callback( &interval, callback, user_data );
}
bool ( *add_periodic_event_callback )( const time_t seconds, timer_callback callback, void *user_data ) = _add_periodic_event_callback;


bool
_delete_timer_event( timer_callback callback, void *user_data ) {
  assert( callback != NULL );

  debug( "Deleting a timer event ( callback = %p, user_data = %p ).", callback, user_data );

  dlist_element *e;

  if ( timer_callbacks == NULL ) {
    debug( "All timer callbacks are already deleted or not created yet." );
    return false;
  }

  for ( e = timer_callbacks->next; e; e = e->next ) {
    timer_callback_info *cb = e->data;
    if ( cb->function == callback && cb->user_data == user_data ) {
      debug( "Deleting a callback ( callback = %p, user_data = %p ).", callback, user_data );
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
bool ( *delete_timer_event )( timer_callback callback, void *user_data ) = _delete_timer_event;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
