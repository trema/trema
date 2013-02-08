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


#ifndef OPENFLOW_ERROR_H
#define OPENFLOW_ERROR_H


#include "ruby.h"
#include "trema.h"


extern VALUE cOpenflowError;


void Init_openflow_error( void );


void handle_openflow_error(
        uint64_t datapath_id,
        uint32_t transaction_id,
        uint16_t type,
        uint16_t code,
        const buffer *body,
        void *user_data
        );


#endif // OPENFLOW_ERROR_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
