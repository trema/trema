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


#include "action_output.h"
#include "controller.h"
#include "features_reply.h"
#include "features_request.h"
#include "hello.h"
#include "logger.h"
#include "packet_in.h"
#include "port.h"
#include "rbuffer.h"
#include "action_set_dl_dst.h"
#include "action_set_dl_src.h"
#include "action_enqueue.h"
#include "action_set_nw_src.h"
#include "action_set_nw_dst.h"
#include "action_set_tp_src.h"
#include "action_set_tp_dst.h"
#include "action_set_nw_tos.h"
#include "action_set_vlan_vid.h"
#include "action_set_vlan_pcp.h"
#include "action_strip_vlan.h"
#include "stats_request.h"
#include "match.h"
#include "flow_removed.h"
#include "port_status.h"
#include "stats-reply.h"
#include "openflow_error.h"
#include "ruby.h"


VALUE mTrema;


void
Init_trema() {
  mTrema = rb_define_module( "Trema" );
  init_log( NULL, "/tmp", false );

  rb_require( "trema/host" );
  rb_require( "trema/path" );
  rb_require( "trema/sub-commands" );
  rb_require( "trema/switch" );

  Init_action_output();
  Init_action_set_dl_dst();
  Init_action_set_dl_src();
  Init_action_enqueue();
  Init_action_set_nw_src();
  Init_action_set_nw_dst();
  Init_action_set_tp_src();
  Init_action_set_tp_dst();
  Init_action_set_nw_tos();
  Init_action_set_vlan_vid();
  Init_action_set_vlan_pcp();
  Init_action_strip_vlan();
  Init_buffer();
  Init_logger();
  Init_controller();
  Init_features_reply();
  Init_features_request();
  Init_stats_request();
  Init_hello();
  Init_match();
  Init_packet_in();
  Init_port();
  Init_flow_removed();
  Init_port_status();
  Init_stats_reply();
  Init_openflow_error();
  rb_require( "trema/exact-match" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

