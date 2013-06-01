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


require "trema/stats-helper"


module Trema
  class FlowStatsReply < StatsHelper
    FIELDS = %w(length table_id match duration_sec duration_nsec) +
      %w(priority idle_timeout hard_timeout cookie packet_count byte_count actions)
    FIELDS.each { |field| attr_reader field.intern }


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
    #       :byte_count => 1,
    #       :actions => [ ActionOutput.new ]
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [Number] :length
    #     the length of this packet.
    #
    #   @option options [Number] :table_id
    #     set to zero.
    #
    #   @option options [Match] :match
    #     Match object describing flow fields.
    #
    #   @option options [Number] :duration_sec
    #     the time in seconds the flow been active.
    #
    #   @option options [Number] :duration_nsec
    #     the time in nanosecs the flow been active.
    #
    #   @option options [Number] :priority
    #     the priority of the flow.
    #
    #   @option options [Number] :idle_timeout
    #     an inactivity time in seconds before the flow is deleted. Zero means
    #     no deletion.
    #
    #   @option options [Number] :hard_timeout
    #     a fixed time interval before the flow is deleted. Zero means no
    #     deletion.
    #
    #   @option options [Number] :cookie
    #     an opaque identifier used as a unique key to match flow entries.
    #
    #   @option options [Number] :packet_count
    #     count of the number of packets matched the flow.
    #
    #   @option options [Number] :byte_count
    #     count of the number of bytes matched the flow.
    #
    #   @option options [Array] :actions
    #     an array of action objects for the flow.
    #
    #   @return [FlowStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_FLOW) OpenFlow message.
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
