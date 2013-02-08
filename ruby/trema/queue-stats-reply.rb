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
  class QueueStatsReply < StatsHelper
    FIELDS = %w(port_no queue_id tx_bytes tx_packets tx_errors)
    FIELDS.each { |field| attr_reader field.intern }


    # Queue statistics for a port.
    # A user would not explicitly instantiate a {QueueStatsReply} object but
    # would be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_QUEUE)+
    # message.
    #
    # @overload initialize(options={})
    #
    #   @example
    #     QueueStatsReply.new(
    #       :port_no => 1,
    #       :queue_id => 123,
    #       :tx_bytes => 128
    #       :tx_packets => 2
    #       :tx_errors => 0
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [Number] :port_no
    #     a specific port or all port if +OFPT_ALL+.
    #
    #   @option options [Number] :queue_id
    #     a specific queue identifier or all queues if +OFPQ_ALL+.
    #
    #   @option options [Number] :tx_bytes
    #     a counter of transmitted bytes.
    #
    #   @option options [Number] :tx_packets
    #     a counter of transmitted packets.
    #
    #   @option options [Number] :tx_errors
    #     a counter of transmitted errors.
    #
    #   @return [QueueStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_QUEUE) OpenFlow message.
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
