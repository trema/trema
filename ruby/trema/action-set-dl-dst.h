/*
 * Prototype for setting the destination data link address flow action.
 *
 * Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


#ifndef ACTION_SET_DL_DST_H
#define ACTION_SET_DL_DST_H


#include "ruby.h"


extern VALUE cActionSetDlDst;


void Init_action_set_dl_dst( void );


#endif // ACTION_SET_DL_DST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
