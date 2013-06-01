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
  class PortStatsReply < StatsHelper
    FIELDS = %w(port_no rx_packets tx_packets rx_bytes) +
      %w(tx_bytes rx_dropped tx_dropped rx_errors tx_errors) +
      %w(rx_frame_err rx_over_err rx_crc_err collisions )
    FIELDS.each { |field| attr_reader field.intern }


    # Port counters or errors for one or more physical ports.
    # A user would not explicitly instantiate a {PortStatsReply} object but would
    # be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_PORT)+ message.
    #
    # @overload initialize(options={})
    #
    #   @example
    #     PortStatsReply.new(
    #       :port_no => 1,
    #       :rx_packets => 7,
    #       :tx_packets => 10,
    #       :rx_bytes => 1454,
    #       :tx_bytes => 2314
    #       :rx_dropped => 0,
    #       :tx_dropped => 0,
    #       :rx_errors => 0,
    #       :tx_errors => 0,
    #       :rx_frame_err => 0,
    #       :rx_over_err => 0,
    #       :rx_crc_err => 0,
    #       :collisions => 0
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [Number] :port_no
    #     the port_no statistics are reported for.
    #
    #   @option options [Number] :rx_packets
    #     a counter of received packets.
    #
    #   @option options [Number] :tx_packets
    #     a counter of transmitted packets.
    #
    #   @option options [Number] :rx_bytes
    #     a counter of received bytes.
    #
    #   @option options [Number] :tx_bytes
    #     a counter of transmitted bytes.
    #
    #   @option options [Number] :rx_dropped
    #     a counter of received dropped frames.
    #
    #   @option options [Number] :tx_dropped
    #     a counter of transmitted dropped frames.
    #
    #   @option options [Number] :rx_errors
    #     a counter of received errors.
    #
    #   @option options [Number] :tx_errors
    #     a counter of transmitted errors.
    #
    #   @option options [Number] :rx_frame_err
    #     a counter of received frame errors.
    #
    #   @option options [Number] :rx_over_err
    #     a counter of received overrun errors.
    #
    #   @option options [Number] :rx_crc_err
    #     a counter of crc errors.
    #
    #   @option options [Number] :collisions
    #     a counter of detected collisions.
    #
    #   @return [PortStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_PORT) OpenFlow message.
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
