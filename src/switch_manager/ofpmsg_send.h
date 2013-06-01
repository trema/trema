/*
 * OpenFlow Switch Manager
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


#ifndef OFPMSG_SEND_H
#define OFPMSG_SEND_H


#include "trema.h"
#include "switch.h"


#define OFP_ERROR_MSG_MAX_DATA 64


int ofpmsg_send_hello( struct switch_info *sw_info );
int ofpmsg_send_echorequest( struct switch_info *sw_info, uint32_t xid, buffer *body );
int ofpmsg_send_echoreply( struct switch_info *sw_info, uint32_t xid, buffer *body );
int ofpmsg_send_featuresrequest( struct switch_info *sw_info );
int ofpmsg_send_setconfig( struct switch_info *sw_info );
int ofpmsg_send_error_msg( struct switch_info *sw_info, uint16_t type, uint16_t code, buffer *data );
int ofpmsg_send( struct switch_info *sw_info, buffer *buf, char *service_name );
int ofpmsg_send_delete_all_flows( struct switch_info *sw_info );
int ofpmsg_send_deny_all( struct switch_info *sw_info );


#endif // OFPMSG_SEND_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
