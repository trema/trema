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


#include <pthread.h>
#include "checks.h"


int
mock_pthread_mutex_init( pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr ) {
  UNUSED( mutex );
  UNUSED( mutexattr );
  return 0;
}


int
mock_pthread_mutexattr_init( pthread_mutexattr_t *attr ) {
  UNUSED( attr );
  return 0;
}


int
mock_pthread_mutexattr_settype( pthread_mutexattr_t *attr, int kind ) {
  UNUSED( attr );
  UNUSED( kind );
  return 0;
}


int
mock_pthread_mutex_lock( pthread_mutex_t *mutex ) {
  UNUSED( mutex );
  return 0;
}


int
mock_pthread_mutex_unlock( pthread_mutex_t *mutex ) {
  UNUSED( mutex );
  return 0;
}


int
mock_pthread_mutex_destroy( pthread_mutex_t *mutex ) {
  UNUSED( mutex );
  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
