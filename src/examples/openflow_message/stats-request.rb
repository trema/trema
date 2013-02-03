#
# A test example program to send OFPT_STATS_REQUEST messages and print
#
# Copyright (C) 2012-2013 Hiroyasu OHYAMA
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

class StatsRequestController < Controller
  def switch_ready datapath_id
    # This is for getting a reply of ofp_flow_stats
    send_flow_mod_add( datapath_id, :match => Match.new)

    send_message( datapath_id, DescStatsRequest.new )
    send_message( datapath_id, FlowStatsRequest.new( :match => Match.new ) )
    send_message( datapath_id, AggregateStatsRequest.new( :match => Match.new ) )
    send_message( datapath_id, TableStatsRequest.new )
    send_message( datapath_id, PortStatsRequest.new )
  end


  def desc_stats_reply datapath_id, message
    info "[ desc_stats_reply ] message: #{ message.class }"
  end


  def flow_stats_reply datapath_id, message
    info "[ flow_stats_reply ] message: #{ message.class }"
  end


  def aggregate_stats_reply datapath_id, message
    info "[ aggregate_stats_reply ] message: #{ message.class }"
  end


  def table_stats_reply datapath_id, message
    info "[ table_stats_reply ] message: #{ message.class }"
  end


  def port_stats_reply datapath_id, message
    info "[ port_stats_reply ] message: #{ message.class }"
  end
end
