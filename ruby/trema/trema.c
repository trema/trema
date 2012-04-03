/*
 * Ruby wrapper around libtrema.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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
#include "packet-in.h"
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
#include "vendor.h"
#include "queue-get-config-request.h"
#include "queue-get-config-reply.h"
#include "port-mod.h"
#include "match.h"
#include "ruby.h"
#include "switch.h"


VALUE mTrema;


void
Init_trema() {
  mTrema = rb_define_module( "Trema" );

  rb_define_const( mTrema, "OFPC_FLOW_STATS", INT2NUM( OFPC_FLOW_STATS ) );
  rb_define_const( mTrema, "OFPC_TABLE_STATS", INT2NUM( OFPC_TABLE_STATS ) );
  rb_define_const( mTrema, "OFPC_PORT_STATS", INT2NUM( OFPC_PORT_STATS ) );
  rb_define_const( mTrema, "OFPC_STP", INT2NUM( OFPC_STP) );
  rb_define_const( mTrema, "OFPC_RESERVED", INT2NUM( OFPC_RESERVED ) );
  rb_define_const( mTrema, "OFPC_IP_REASM", INT2NUM( OFPC_IP_REASM ) );
  rb_define_const( mTrema, "OFPC_QUEUE_STATS", INT2NUM( OFPC_QUEUE_STATS ) );
  rb_define_const( mTrema, "OFPC_ARP_MATCH_IP", INT2NUM( OFPC_ARP_MATCH_IP ) );

  rb_define_const( mTrema, "OFPAT_OUTPUT", INT2NUM( OFPAT_OUTPUT ) );
  rb_define_const( mTrema, "OFPAT_SET_VLAN_VID", INT2NUM( OFPAT_SET_VLAN_VID ) );
  rb_define_const( mTrema, "OFPAT_SET_VLAN_PCP", INT2NUM( OFPAT_SET_VLAN_PCP ) );
  rb_define_const( mTrema, "OFPAT_STRIP_VLAN", INT2NUM( OFPAT_STRIP_VLAN ) );
  rb_define_const( mTrema, "OFPAT_SET_DL_SRC", INT2NUM( OFPAT_SET_DL_SRC) );
  rb_define_const( mTrema, "OFPAT_SET_DL_DST", INT2NUM( OFPAT_SET_DL_DST) );
  rb_define_const( mTrema, "OFPAT_SET_NW_SRC", INT2NUM( OFPAT_SET_NW_SRC ) );
  rb_define_const( mTrema, "OFPAT_SET_NW_DST", INT2NUM( OFPAT_SET_NW_DST ) );
  rb_define_const( mTrema, "OFPAT_SET_NW_TOS", INT2NUM( OFPAT_SET_NW_TOS ) );
  rb_define_const( mTrema, "OFPAT_SET_TP_SRC", INT2NUM( OFPAT_SET_TP_SRC ) );
  rb_define_const( mTrema, "OFPAT_SET_TP_DST", INT2NUM( OFPAT_SET_TP_DST ) );
  rb_define_const( mTrema, "OFPAT_ENQUEUE", INT2NUM( OFPAT_ENQUEUE ) );
  rb_define_const( mTrema, "OFPAT_VENDOR", INT2NUM( OFPAT_VENDOR ) );

  rb_require( "trema/host" );
  rb_require( "trema/openflow-switch" );
  rb_require( "trema/path" );

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
  Init_vendor();
  Init_port_mod();
  Init_switch();

  rb_require( "trema/exact-match" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
