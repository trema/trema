#
# A test example program to send a OFPT_FEATURES_REQUEST message and print
# the reply.
#
# Copyright (C) 2008-2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


class FeaturesRequestController < Controller
  def switch_ready datapath_id
    send_message datapath_id, FeaturesRequest.new
  end


  def features_reply datapath_id, message
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "n_buffers: #{ message.n_buffers }"
    info "n_tables: #{ message.n_tables }"
    print_capabilities message.capabilities
    print_actions message.actions
    print_ports message.ports
  end


  ##############################################################################
  private
  ##############################################################################


  def print_capabilities capabilities
    info "capabilities:"
    info "  OFPC_FLOW_STATS" if capabilities & OFPC_FLOW_STATS != 0
    info "  OFPC_TABLE_STATS" if capabilities & OFPC_TABLE_STATS != 0
    info "  OFPC_PORT_STATS" if capabilities & OFPC_PORT_STATS != 0
    info "  OFPC_STP" if capabilities & OFPC_STP != 0
    info "  OFPC_RESERVED" if capabilities & OFPC_RESERVED != 0
    info "  OFPC_IP_REASM" if capabilities & OFPC_IP_REASM != 0
    info "  OFPC_QUEUE_STATS" if capabilities & OFPC_QUEUE_STATS != 0
    info "  OFPC_ARP_MATCH_IP" if capabilities & OFPC_ARP_MATCH_IP != 0
  end


  def print_actions actions
    info "actions:"
    info "  OFPAT_OUTPUT" if actions & ( 1 << OFPAT_OUTPUT ) != 0
    info "  OFPAT_SET_VLAN_VID" if actions & ( 1 << OFPAT_SET_VLAN_VID ) != 0
    info "  OFPAT_SET_VLAN_PCP" if actions & ( 1 << OFPAT_SET_VLAN_PCP ) != 0
    info "  OFPAT_STRIP_VLAN" if actions & ( 1 << OFPAT_STRIP_VLAN ) != 0
    info "  OFPAT_SET_DL_SRC" if actions & ( 1 << OFPAT_SET_DL_SRC ) != 0
    info "  OFPAT_SET_DL_DST" if actions & ( 1 << OFPAT_SET_DL_DST ) != 0
    info "  OFPAT_SET_NW_SRC" if actions & ( 1 << OFPAT_SET_NW_SRC ) != 0
    info "  OFPAT_SET_NW_DST" if actions & ( 1 << OFPAT_SET_NW_DST ) != 0
    info "  OFPAT_SET_NW_TOS" if actions & ( 1 << OFPAT_SET_NW_TOS ) != 0
    info "  OFPAT_SET_TP_SRC" if actions & ( 1 << OFPAT_SET_TP_SRC ) != 0
    info "  OFPAT_SET_TP_DST" if actions & ( 1 << OFPAT_SET_TP_DST ) != 0
    info "  OFPAT_ENQUEUE" if actions & ( 1 << OFPAT_ENQUEUE ) != 0
    info "  OFPAT_VENDOR" if actions & OFPAT_VENDOR != 0
  end


  def print_ports ports
    info "ports:"
    ports.each do | each |
      info "  port_no: %u" % each.number
      info "    hw_addr = #{ each.hw_addr.to_s }"
      info "    name = #{ each.name }"
      info "    config = #{ each.config.to_hex }"
      info "    state = #{ each.state.to_hex }"
      info "    curr = #{ each.curr.to_hex }"
      info "    advertised = #{ each.advertised.to_hex }"
      info "    supported = #{ each.supported.to_hex }"
      info "    peer = #{ each.peer.to_hex }"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
