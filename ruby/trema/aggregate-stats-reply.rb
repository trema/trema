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
  class AggregateStatsReply < StatsHelper
    FIELDS = %w(packet_count byte_count flow_count)
    FIELDS.each { |field| attr_reader field.intern }


    # Aggregate counters for flows.
    # A user would not explicitly instantiate a {AggregateStatsReply} object but
    # would be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_AGGREGATE)
    # message.
    #
    # @overload initialize(options={})
    #
    #   @example
    #     AggregateStatsReply.new(
    #       :packet_count => 10,
    #       :byte_count => 640,
    #       :flow_count => 2
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [Number] :packet_count
    #     the total number of packets that matched.
    #
    #   @option options [Number] :byte_count
    #     the total number of bytes from packets that matched.
    #
    #   @option options [Number] :flow_count
    #     the total number of flows that matched.
    #
    #   @return [AggregateStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_AGGREGATE) OpenFlow message.
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
