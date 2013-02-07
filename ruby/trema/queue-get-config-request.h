/*
 * Ruby wrapper class for OFPT_QUEUE_GET_CONFIG_REPLY message.
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


#ifndef QUEUE_GET_CONFIG_REQUEST_H
#define QUEUE_GET_CONFIG_REQUEST_H


#include "ruby.h"


extern VALUE cQueueGetConfigRequest;


void Init_queue_get_config_request( void );


#endif // QUEUE_GET_CONFIG_REQUEST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
