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


require "trema/stats-helper"


module Trema
  class FlowStatsReply < StatsHelper
    FIELDS = %w( length table_id match duration_sec duration_nsec priority idle_timeout hard_timeout cookie packet_count byte_count actions )
    FIELDS.each { | each | attr_reader each.intern }


    # Flow counters for one or more matched flows.
    # A user would not explicitly instantiate a {FlowStatsReply} object but 
    # would be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_FLOW)+ 
    # message.
    #
    # @overload initialize(options={})
    # 
    #   @example 
    #     FlowStatsReply.new(
    #       :length => 96, 
    #       :table_id => 0, 
    #       :match => Match.new
    #       :duration_sec => 10, 
    #       :duration_nsec => 106000000, 
    #       :priority => 0, 
    #       :idle_timeout => 0,
    #       :hard_timeout => 0, 
    #       :cookie => 0xabcd, 
    #       :packet_count => 1, 
    #       :byte_count => 1
    #       :actions => [ ActionOutput.new ]
    #     )
    #   
    #   @param [Hash] options the options hash.
    #   
    #   @option options [Symbol] :length
    #     the length of this packet.
    #   
    #   @option options [Symbol] :table_id
    #     set to zero.
    #     
    #   @option options [Symbol] :match
    #     Match object describing flow fields.
    #     
    #   @option options [Symbol] :duration_sec
    #     the time in seconds the flow been active.
    #   
    #   @option options [Symbol] :duration_nsec
    #     the time in nanosecs the flow been active.
    #   
    #   @option options [Symbol] :priority
    #     the priority of the flow.
    #   
    #   @option options [Symbol] :idle_timeout
    #     an inactivity time in seconds before the flow is deleted. Zero means
    #     no deletion.
    #     
    #   @option options [Symbol] :hard_timeout
    #     a fixed time interval before the flow is deleted. Zero means no 
    #     deletion.
    #   
    #   @option options [Symbol] :cookie
    #     an opaque identifier used as a unique key to match flow entries.
    #   
    #   @option options [Symbol] :packet_count
    #     count of the number of packets matched the flow.
    #   
    #   @option options [Symbol] :byte_count
    #     count of the number of bytes matched the flow.
    #   
    #   @option options [Symbol] :actions
    #     an array of action objects for the flow.
    #
    # @return [FlowStatsReply] an object that encapsulates the OFPST_STATS_REPLY (OPPST_FLOW) OpenFlow message.
    #
    def initialize options 
      super FIELDS, options
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
