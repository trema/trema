#
# Simple layer-2 switch with traffic monitoring.
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


require "counter"
require "fdb"


class TrafficMonitor < Controller
  periodic_timer_event :show_counter, 10


  def start
    @counter = Counter.new
    @fdb = FDB.new
  end


  def packet_in datapath_id, message
    macsa = message.macsa
    macda = message.macda

    @fdb.learn macsa, message.in_port
    @counter.add macsa, 1, message.total_len
    out_port = @fdb.lookup( macda )
    if out_port
      packet_out datapath_id, message, out_port
      flow_mod datapath_id, macsa, macda, out_port
    else
      flood datapath_id, message
    end
  end


  def flow_removed datapath_id, message
    @counter.add message.match.dl_src, message.packet_count, message.byte_count
  end


  ##############################################################################
  private
  ##############################################################################


  def show_counter
    puts Time.now
    @counter.each_pair do | mac, counter |
      puts "#{ mac } #{ counter[ :packet_count ] } packets (#{ counter[ :byte_count ] } bytes)"
    end
  end


  def flow_mod datapath_id, macsa, macda, out_port
    send_flow_mod_add(
      datapath_id,
      :hard_timeout => 10,
      :match => Match.new( :dl_src => macsa, :dl_dst => macda ),
      :actions => ActionOutput.new( out_port )
    )
  end


  def packet_out datapath_id, message, out_port
    send_packet_out(
      datapath_id,
      :packet_in => message,
      :actions => ActionOutput.new( out_port )
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
