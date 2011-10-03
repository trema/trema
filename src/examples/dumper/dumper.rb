#
# Openflow message event dumper.
#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


class Dumper < Controller
  def packet_in datapath_id, message
    info "[packet_in]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "buffer_id: %#x" % message.buffer_id
    info "total_len: %u" % message.total_len
    info "in_port: %u" % message.in_port
    info "reason: %#x" % message.reason
    info "data: #{ message.data.unpack( "H*" ) }"
    @datapath_id, @match, @in_port = datapath_id, Match.from( message ), message.in_port
  end


  def flow_removed datapath_id, message
    info "[flow removed]"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"

    info "match:"
    info "  wildcards: %#x" % message.match.wildcards
    info "  in_port: %u" % message.match.in_port
    info "  dl_src: #{message.match.dl_src.to_s}"
    info "  dl_dst: #{message.match.dl_dst.to_s}"
    info "  dl_vlan: %u" % message.match.dl_vlan
    info "  dl_vlan_pcp: %u" % message.match.dl_vlan_pcp
    info "  dl_type: %#x" % message.match.dl_type
    info "  nw_tos: %u" % message.match.nw_tos
    info "  nw_proto: %#x" % message.match.nw_proto
    info "  nw_src: %#x" % message.match.nw_src
    info "  nw_dst: %#x" % message.match.nw_dst
    info "  tp_src: %u" % message.match.tp_src
    info "  tp_dst: %u" % message.match.tp_dst

    info "cookie: %#x" % message.cookie
    info "priority: %u" % message.priority
    info "reason: %#x" % message.reason
    info "duration_sec: %u" % message.duration_sec
    info "duration_nsec: %u" % message.duration_nsec
    info "idle_timeout: %u" % message.idle_timeout
    info "packet_count: %#x" % message.packet_count
    info "byte_count: %#x" % message.byte_count
    run_next_event
  end


  def get_config_reply message
    info "[get_config_reply]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "flags: %#x" % message.flags
    info "miss_send_len: %u" % message.miss_send_len
    run_next_event
  end


  def switch_disconnected datapath_id
    info "[switch_disconnected]"
    info "datapath_id: #{ datapath_id.to_hex }"
  end


  def port_status message
    info "[port status]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "reason: %#x" % message.reason
    dump_phy_port message.phy_port
  end


  def stats_reply message
    info "[stats_reply]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "type: %#x" % message.type
    info "flags: %#x" % message.flags
    message.stats.each { | each | info each.to_s }
    run_next_event
  end


  def openflow_error message
    info "[error]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "type: %#x" % message.type
    info "code: %#x" % message.code
    info "data: %s", message.data.unpack( "H*" )
    run_next_event
  end


  def queue_get_config_reply message
    info "[queue get_config_reply]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "port: %u" % message.port
    info( "queues:" );
    dump_packet_queue(message.queues)
  end


  def barrier_reply message
    info "[barrier_reply]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    run_next_event
  end


  def switch_ready datapath_id
    info "[switch ready]"
    info "datapath_id: #{ datapath_id.to_hex }"
    send_message datapath_id, FeaturesRequest.new
    set_events
    run_next_event
  end


  def features_reply message
    info "[features_reply]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "n_buffers: %u" % message.n_buffers
    info "n_tables: %u" % message.n_tables
    info "capabilities: %#x" % message.capabilities
    info "actions: %#x" % message.actions
    message.ports.each { | each | dump_phy_port each }
  end


  def vendor message
    info "[vendor]"
    info "datapath_id: #{ message.datapath_id.to_hex }"
    info "transaction_id: #{ transaction_id.to_hex }"
    info "data:"
    info message.buffer.unpack( "H*" )
  end


  def list_switches_reply dpids
    info "[list_switches_reply]"
    info "switches = %s" % dpids.collect { | each | sprintf( "0x%x", each ) }.join( ", " )
    run_next_event
  end


  ##############################################################################
  private
  ##############################################################################


  def dump_phy_port port
    # for testing port-status record the mac address if port.number == 2.
    @hw_addr = port.hw_addr if port.number == 2 
    info "port_no: %u" % port.number
    info "  hw_addr: #{port.hw_addr}"
    info "  name: #{port.name}"
    info "  config: %#x" % port.config
    info "  state: %#x"  % port.state
    info "  curr: %#x" % port.curr
    info "  advertised: %#x" % port.advertised
    info "  supported: %#x" % port.supported
    info "  peer: %#x" % port.peer
  end


  def dump_packet_queue queues
    queues.each do | packet_queue |
      info "queue_id: %#x" % packet_queue.queue_id
      info "  len: %u" % packet_queue.len
      info "  properties:"
      packet_queue.properties.each do | prop |
        info "    property: %#x" % prop.property
        info "    len: %#x" % prop.len
        info "      rate: %u" % prop.rate if prop.property == PacketQueue::OFPQT_MIN_RATE
      end
    end
  end


  def test_queue_reply
    pqs = [ PacketQueue.new( :queue_id => 1, :len => 64 ), PacketQueue.new( :queue_id => 2, :len => 640 ) ]
    idx = 2
    idx.times do | i |
      MinRateQueue.new( PacketQueue::OFPQT_MIN_RATE, ( i + 1 ) * 64, ( i + 1 ) * 1024, pqs[ 0 ] )
    end
    MinRateQueue.new( PacketQueue::OFPQT_MIN_RATE, ( idx + 1 ) * 64, ( idx + 1 ) * 1024, pqs[ 1 ] )
    attr = {
      :datapath_id => 0xabc,
      :transaction_id => 123,
      :port => 1,
      :queues => Queue.queues
    }
    qc = QueueGetConfigReply.new( attr )
    queue_get_config_reply qc
    send_flow_mod_add(
      @datapath_id,
      :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ),
      :actions => ActionOutput.new( OFPP_FLOOD ) 
    )
    run_next_event
  end
  
  
  def test_flow_removed
    send_flow_mod_add( @datapath_id,
      :idle_timeout => 10,
      :hard_timeout => 10,
      :send_flow_rem => true,
      :actions => ActionOutput.new( @in_port ),
      :match => @match
   )
  end

  
  def test_openflow_error
    send_message @datapath_id, PortMod.new( 2,
      Mac.new( "11:22:33:44:55:66" ),
      1,
      1,
      0 )
  end


  def test_barrier_reply
    send_message @datapath_id, BarrierRequest.new( 123 )
  end


  def test_stats_reply
    send_message @datapath_id, 
      FlowStatsRequest.new( :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ) ).to_packet.buffer
  end


  def test_get_config_reply
    send_message @datapath_id, GetConfigRequest.new( 123 )
  end


  def test_list_switches_reply
    send_list_switches_request 
  end


  def test_port_status
    send_message @datapath_id, PortMod.new( 2,
      @hw_addr,
      1,
      1,
      0 )
  end


  def set_events
    @events = [
      :test_port_status,
      :test_list_switches_reply,
      :test_get_config_reply,
      :test_flow_removed,
      :test_stats_reply,
      :test_barrier_reply,
      :test_openflow_error,
      :test_queue_reply
    ]
    @time_interval = 0
  end


  def run_next_event
    oneshot_timer_event @events.pop, next_interval unless @events.empty?
  end


  def next_interval
    @time_interval = 10
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
