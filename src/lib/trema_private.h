/*
 * Private functions that are only called from [trema]/src/lib or [trema]/unittests.
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


#ifndef TREMA_PRIVATE_H
#define TREMA_PRIVATE_H


/**
 * Functions called within [trema]/src/lib.
 */

void set_trema_home( void );
const char *get_trema_home( void );
void unset_trema_home( void );

void set_trema_tmp( void );
const char *get_trema_tmp( void );
void unset_trema_tmp( void );


/**
 * Functions defined only for unit tests.
 */

const char *_get_trema_home( void );
const char *_get_trema_tmp( void );


#endif // TREMA_PRIVATE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
