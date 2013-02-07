/*
 * Private functions that are only called from [trema]/src/lib or [trema]/unittests.
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


#ifndef TREMA_PRIVATE_H
#define TREMA_PRIVATE_H


#include <sqlite3.h>
#include "bool.h"
#include "management_service_interface.h"


void set_trema_home( void );
const char *get_trema_home( void );
void unset_trema_home( void );

void set_trema_tmp( void );
const char *get_trema_tmp( void );
void unset_trema_tmp( void );


const char *_get_trema_home( void );
const char *_get_trema_tmp( void );
void _free_trema_name( void );

const char *_get_db_file( void );
const sqlite3 *_get_db_handle( void );

bool _get_backend_initialized( void );

size_t _get_max_key_length( void );
size_t _get_max_value_length( void );

bool *_get_management_interface_initialized( void );

void _set_management_application_request_handler( management_application_request_handler callback, void *user_data );


#endif // TREMA_PRIVATE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
