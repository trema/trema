/*
 * Author: <addme>
 *
 * Copyright (C) <addme>
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

#include "event_handler.h"

#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "timer.h"


typedef struct event_fd {
  int fd;
  event_fd_callback read_callback;
  event_fd_callback write_callback;
  void* read_data;
  void* write_data;
} event_fd;

event_fd event_list[FD_SETSIZE];
event_fd *event_last;

event_fd *event_fd_set[FD_SETSIZE];

fd_set event_read_set;
fd_set event_write_set;
fd_set current_read_set;
fd_set current_write_set;


void
init_event_handler() {
  event_last = event_list;

  memset(event_list, 0, sizeof(struct event_fd) * FD_SETSIZE);
  memset(event_fd_set, 0, sizeof(struct event_fd*) * FD_SETSIZE);

  FD_ZERO( &event_read_set );
  FD_ZERO( &event_write_set );
}


void
finalize_event_handler() {
  if ( event_last != event_list ) {
    warn( "Event Handler finalized with %i fd event handlers still active. (%i, ...)",
          (event_last - event_list), (event_last > event_list ? event_list->fd : -1) );
    return;
  }
}


void
set_event_handler_fd_set( fd_set* read_set, fd_set* write_set ) {
  memcpy( read_set, &event_read_set, sizeof( fd_set ) );
  memcpy( write_set, &event_write_set, sizeof( fd_set ) );
}


bool
run_event_handler_once() {
  struct timeval timeout = { 0, 100 * 1000 };

  set_event_handler_fd_set( &current_read_set, &current_write_set );

  // Don't us FD_SETSIZE here...
  int set_count = select( FD_SETSIZE, &current_read_set, &current_write_set, NULL, &timeout );

  if ( set_count == -1 ) {
    if ( errno == EINTR ) {
      return true;
    }
    error( "Failed to select ( errno = %s [%d] ).", strerror( errno ), errno );
    return false;

  } else if ( set_count == 0 ) {
    // timed out
    return true;
  }

  event_fd *event_itr = event_list;

  while ( event_itr < event_last ) {
    event_fd current_event = *event_itr;

    if ( FD_ISSET( current_event.fd, &current_read_set ) ) {
      (*current_event.read_callback)( current_event.fd, current_event.read_data );
    }

    if ( FD_ISSET( current_event.fd, &current_write_set ) ) {
      (*current_event.write_callback)( current_event.fd, current_event.write_data );
    }

    // In the rare cases the current fd is closed, a new one is opened
    // with the same fd and is put in the same location we can just
    // wait for the next select call.
    event_itr = event_itr + ( current_event.fd == event_itr->fd );
  }

  return true;
}


void
add_fd_event( int fd,
              event_fd_callback read_callback, void* read_data,
              event_fd_callback write_callback, void* write_data ) {
  info( "Adding event handler for fd %i, %p, %p.", fd, read_callback, write_callback );
  
  // Currently just issue critical warnings instead of killing the
  // program."
  if ( event_fd_set[fd] != NULL ) {
    critical( "Tried to add an already active fd event handler." );
    return;
  }

  if ( fd < 0 || fd >= FD_SETSIZE) {
    critical( "Tried to add an invalid fd." );
    return;
  }

  if ( event_last >= event_list + FD_SETSIZE ) {
    critical( "Event handler list in invalid state." );
    return;
  }

  event_last->fd = fd;
  event_last->read_callback = read_callback;
  event_last->write_callback = write_callback;
  event_last->read_data = read_data;
  event_last->write_data = write_data;
  
  event_fd_set[fd] = event_last++;
}

void
delete_fd_event( int fd ) {
  info( "Deleting event handler for fd %i.", fd );
  
  event_fd* event = event_list;

  while (event != event_last && event->fd != fd) {
    event++;
  }

  if ( event >= event_last || event_fd_set[fd] == NULL ) {
    critical( "Tried to delete an inactive fd event handler." );
    return;
  }

  if ( FD_ISSET(fd, &event_read_set) ) {
    critical( "Tried to delete an fd event handler with active read notification." );
    //    return;
    FD_CLR( fd, &event_read_set );
  }

  if ( FD_ISSET(fd, &event_write_set) ) {
    critical( "Tried to delete an fd event handler with active write notification." );
    //    return;
    FD_CLR( fd, &event_write_set );
  }

  FD_CLR( fd, &current_read_set );
  FD_CLR( fd, &current_write_set );

  event_fd_set[fd] = NULL;

  if ( event != --event_last ) {
    memcpy( event, event_last, sizeof(struct event_fd) );
    event_fd_set[event->fd] = event;
  }

  memset( event_last, 0, sizeof(struct event_fd) );
  event_last->fd = -1;
}


void
notify_readable_event( int fd, bool state ) {
  if ( event_fd_set[fd] == NULL || event_fd_set[fd]->read_callback == NULL ) {
    critical( "Invalid fd to notify_readable_event call; %i, %p.", fd, event_fd_set[fd]->read_callback );
    return;
  }

  if ( state ) {
    FD_SET( fd, &event_read_set );
  } else {
    FD_CLR( fd, &event_read_set );
    FD_CLR( fd, &current_read_set );
  }
}


void
notify_writable_event( int fd, bool state ) {
  if ( event_fd_set[fd] == NULL || event_fd_set[fd]->write_callback == NULL ) {
    critical( "Invalid fd to notify_writeable_event call; %i, %p.", fd, event_fd_set[fd]->write_callback );
    return;
  }

  if ( state ) {
    FD_SET( fd, &event_write_set );
  } else {
    FD_CLR( fd, &event_write_set );
    FD_CLR( fd, &current_write_set );
  }
}


bool
is_notifying_readable_event( int fd ) {
  return FD_ISSET( fd, &event_read_set );
}


bool
is_notifying_writable_event( int fd ) {
  return FD_ISSET( fd, &event_write_set );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
