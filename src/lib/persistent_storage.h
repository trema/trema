/*
 * A persistent data storage library
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


#ifndef PERSISTENT_STORAGE_H
#define PERSISTENT_STORAGE_H


#include "bool.h"


bool init_persistent_storage();
bool finalize_persistent_storage();
bool clear_persistent_storage();
bool set_value( const char *key, const char *value );
bool get_value( const char *key, char *value, const size_t length );
bool delete_key_value( const char *key );


#endif // PERSISTENT_STORAGE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
