/*
 * Timer events and callbacks.
 *
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


/**
 * @timer.h
 * Function declaration of timer functions
 */
#ifndef TIMER_H
#define TIMER_H


#include <stdbool.h>
#include <time.h>


bool init_timer( void );
bool finalize_timer( void );

bool add_timer_event_callback( struct itimerspec *interval, void ( *callback )( void *user_data ), void *user_data );
bool delete_timer_event_callback( void ( *callback )( void *user_data ) );

bool add_periodic_event_callback( const time_t seconds, void ( *callback )( void *user_data ), void *user_data );
bool delete_periodic_event_callback( void ( *callback )( void *user_data ) );

void execute_timer_events( void );


#endif // TIMER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
