#
# Simple layer-2 switch with traffic monitoring.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


require "fdb"


class TrafficStat
  attr_accessor :packet_count
  attr_accessor :byte_count
  
  
  def initialize
    @packet_count = 0
    @byte_count = 0
  end
end


class TrafficMonitor < Trema::Controller
  periodic_timer_event :show_stats, 10


  def start
    @stats = {}
    @fdb = FDB.new
  end


  def packet_in datapath_id, message
    @fdb.learn message.macsa, message.in_port
    out_port = @fdb.port_number_of( message.macda )
    if out_port
      flow_mod datapath_id, message, out_port
      packet_out datapath_id, message, out_port
    else
      flood datapath_id, message
    end
  end


  def flow_removed datapath_id, message
    src_mac = message.match.dl_src
    @stats[ src_mac ] ||= TrafficStat.new
    @stats[ src_mac ].packet_count += message.packet_count
    @stats[ src_mac ].byte_count += message.byte_count
  end


  ##############################################################################
  private
  ##############################################################################


  def show_stats
    @stats.each_pair do | mac, stats |
      puts "#{ mac.to_s } #{ stats.packet_count } packets, #{ stats.byte_count } bytes"
    end
  end


  def flow_mod datapath_id, message, out_port
    send_flow_mod_add(
      datapath_id,
      :send_flow_rem => true,                      
      :hard_timeout => 10,
      :match => Match.new( :dl_src => message.macsa.to_s, :dl_dst => message.macda.to_s ),
      :actions => Trema::ActionOutput.new( out_port )
    )
  end


  def packet_out datapath_id, message, out_port
    send_packet_out(
      datapath_id,
      :packet_in => message,
      :actions => Trema::ActionOutput.new( out_port )
    )
  end


  def flood datapath_id, message
    packet_out datapath_id, message, OFPP_FLOOD
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
