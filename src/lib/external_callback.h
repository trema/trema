/*
 * External callback implementation
 *
 * Copyright (C) 2013 NEC Corporation
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


#ifndef EXTERNAL_CALLBACK_H
#define EXTERNAL_CALLBACK_H


#include <sys/types.h>
#include "bool.h"

typedef void ( *external_callback_t )( void );

extern void ( *init_external_callback )( void );
extern void ( *finalize_external_callback )( void );
extern bool ( *push_external_callback )( external_callback_t callback );
extern void ( *run_external_callback )( void );

#define set_external_callback( callback ) push_external_callback( callback )


#endif // EXTERNAL_CALLBACK_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
