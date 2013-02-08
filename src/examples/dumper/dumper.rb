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


#
# Openflow message event dumper.
#
class Dumper < Controller
  def switch_ready datapath_id
    info "[switch_ready]"
    info "  datapath_id: #{ datapath_id.to_hex }"
  end


  def switch_disconnected datapath_id
    info "[switch_disconnected]"
    info "  datapath_id: #{ datapath_id.to_hex }"
  end


  def openflow_error datapath_id, message
    info "[error]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "type: #{ message.type.to_hex }"
    info "code: #{ message.code.to_hex }"
    info "data: #{ message.data.unpack "H*" }"
  end


  def vendor datapath_id, message
    info "[vendor]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "data:"
    info "#{ message.buffer.unpack "H*" }"
  end


  def features_reply datapath_id, message
    info "[features_reply]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "n_buffers: #{ message.n_buffers }"
    info "n_tables: #{ message.n_tables }"
    info "capabilities: #{ message.capabilities.to_hex }"
    info "actions: #{ message.actions.to_hex }"
    message.ports.each do | each |
      dump_phy_port each
    end
  end


  def get_config_reply datapath_id, message
    info "[get_config_reply]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "flags: #{ message.flags.to_hex }"
    info "miss_send_len: #{ message.miss_send_len }"
  end


  def packet_in datapath_id, message
    info "[packet_in]"
    info "  datapath_id: #{ datapath_id.to_hex }"
    info "  transaction_id: #{ message.transaction_id.to_hex }"
    info "  buffer_id: #{ message.buffer_id.to_hex }"
    info "  total_len: #{ message.total_len }"
    info "  in_port: #{ message.in_port }"
    info "  reason: #{ message.reason.to_hex }"
    info "  data: #{ message.data.unpack "H*" }"
  end


  def flow_removed datapath_id, message
    info "[flow_removed]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"

    info "match:"
    info "  wildcards: #{ message.match.wildcards.to_hex }"
    info "  in_port: #{ message.match.in_port }"
    info "  dl_src: #{ message.match.dl_src }"
    info "  dl_dst: #{ message.match.dl_dst }"
    info "  dl_vlan: #{ message.match.dl_vlan }"
    info "  dl_vlan_pcp: #{ message.match.dl_vlan_pcp }"
    info "  dl_type: #{ message.match.dl_type.to_hex }"
    info "  nw_tos: #{ message.match.nw_tos }"
    info "  nw_proto: #{ message.match.nw_proto.to_hex }"
    info "  nw_src: #{ message.match.nw_src.to_hex }"
    info "  nw_dst: #{ message.match.nw_dst.to_hex }"
    info "  tp_src: #{ message.match.tp_src }"
    info "  tp_dst: #{ message.match.tp_dst }"

    info "cookie: #{ message.cookie.to_hex }"
    info "priority: #{ message.priority }"
    info "reason: #{ message.reason.to_hex }"
    info "duration_sec: #{ message.duration_sec }"
    info "duration_nsec: #{ message.duration_nsec }"
    info "idle_timeout: #{ message.idle_timeout }"
    info "packet_count: #{ message.packet_count.to_hex }"
    info "byte_count: #{ message.byte_count.to_hex }"
  end


  def port_status datapath_id, message
    info "[port_status]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "reason: #{ message.reason.to_hex }"
    dump_phy_port message.phy_port
  end


  def stats_reply datapath_id, message
    info "[stats_reply]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "type: #{ message.type.to_hex }"
    info "flags: #{ message.flags.to_hex }"
    message.stats.each { | each | info each.to_s }
  end


  def barrier_reply datapath_id, message
    info "[barrier_reply]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
  end


  def queue_get_config_reply datapath_id, message
    info "[queue_get_config_reply]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "port: #{ message.port }"
    info( "queues:" );
    dump_packet_queue message.queues
  end


  ##############################################################################
  private
  ##############################################################################


  def dump_phy_port port
    # for testing port-status record the mac address if port.number == 2.
    @hw_addr = port.hw_addr if port.number == 2
    info "port_no: #{ port.number }"
    info "  hw_addr: #{ port.hw_addr }"
    info "  name: #{ port.name }"
    info "  config: #{ port.config.to_hex }"
    info "  state: #{ port.state.to_hex }"
    info "  curr: #{ port.curr.to_hex }"
    info "  advertised: #{ port.advertised.to_hex }"
    info "  supported: #{ port.supported.to_hex }"
    info "  peer: #{ port.peer.to_hex }"
  end


  def dump_packet_queue queues
    queues.each do | packet_queue |
      info "queue_id: #{ packet_queue.queue_id.to_hex }"
      info "  len: #{ packet_queue.len }"
      info "  properties:"
      packet_queue.properties.each do | prop |
        info "    property: #{ prop.property.to_hex }"
        info "    len: #{ prop.len.to_hex }"
        info "      rate: %u" % prop.rate if prop.property == PacketQueue::OFPQT_MIN_RATE
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
