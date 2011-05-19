/*
 * Ruby wrapper around libtrema.
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


#include "controller.h"
#include "features_reply.h"
#include "features_request.h"
#include "packet_in.h"
#include "hello.h"
#include "port.h"
#include "ruby.h"


VALUE mTrema;


void
Init_trema() {
  mTrema = rb_define_module( "Trema" );
  init_log( NULL, false );

  rb_require( "trema/host" );
  rb_require( "trema/path" );
  rb_require( "trema/sub-commands" );
  rb_require( "trema/switch" );

  Init_controller();
  Init_features_reply();
  Init_features_request();
  Init_packet_in();
  Init_hello();
  Init_port();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
