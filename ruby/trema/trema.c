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


#include "action-output.h"
#include "controller.h"
#include "features-reply.h"
#include "features-request.h"
#include "set-config.h"
#include "hello.h"
#include "logger.h"
#include "packet_in.h"
#include "port.h"
#include "action-set-dl-dst.h"
#include "action-set-dl-src.h"
#include "action-enqueue.h"
#include "action-set-nw-src.h"
#include "action-set-nw-dst.h"
#include "action-set-tp-src.h"
#include "action-set-tp-dst.h"
#include "action-set-nw-tos.h"
#include "action-set-vlan-vid.h"
#include "action-set-vlan-pcp.h"
#include "action-strip-vlan.h"
#include "action-vendor.h"
#include "echo-request.h"
#include "echo-reply.h"
#include "error.h"
#include "stats-request.h"
#include "flow-removed.h"
#include "port-status.h"
#include "stats-reply.h"
#include "openflow-error.h"
#include "get-config-reply.h"
#include "get-config-request.h"
#include "barrier-reply.h"
#include "barrier-request.h"
#include "vendor-request.h"
#include "queue-get-config-request.h"
#include "queue-get-config-reply.h"
#include "port-mod.h"
#include "match.h"
#include "ruby.h"


VALUE mTrema;


void
Init_trema() {
  mTrema = rb_define_module( "Trema" );

  rb_require( "trema/host" );
  rb_require( "trema/path" );
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
  Init_action_vendor();
  Init_echo_reply();
  Init_echo_request();
  Init_error();
  Init_logger();
  Init_controller();
  Init_features_reply();
  Init_features_request();
  Init_set_config();
  Init_stats_request();
  Init_hello();
  Init_match();
  Init_packet_in();
  Init_port();
  Init_flow_removed();
  Init_port_status();
  Init_stats_reply();
  Init_openflow_error();
  Init_get_config_request();
  Init_get_config_reply();
  Init_barrier_request();
  Init_barrier_reply();
  Init_queue_get_config_request();
  Init_queue_get_config_reply();
  Init_vendor_request();
  Init_port_mod();
  rb_require( "trema/exact-match" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
