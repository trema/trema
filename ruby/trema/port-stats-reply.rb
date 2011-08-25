require "trema/stats-helper"

module Trema
  class PortStatsReply < StatsHelper
    FIELDS = %w(port_no rx_packets tx_packets rx_bytes) + 
      %w(tx_bytes rx_dropped tx_dropped rx_errors tx_errors) +
      %w(rx_frame_err rx_over_err rx_crc_err collisions )

    FIELDS.each { |field| attr_reader field.intern }


    # Port counters or errors for one or more physical ports.
    # A user would not implicitly instantiate a {PortStatsReply} object but would
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
    #   @param [Hash] options the options hash.
    #   
    #   @option options [Symbol] :port_no
    #     the port_no statistics are reported for.
    #     
    #   @option options [Symbol] :rx_packets
    #     a counter of received packets.
    #
    #   @option options [Symbol] :tx_packets
    #     a counter of transmitted packets.
    #
    #   @option options [Symbol] :rx_bytes
    #     a counter of received bytes.
    #
    #   @option options [Symbol] :tx_bytes
    #     a counter of transmitted bytes.
    #
    #   @option options [Symbol] :rx_dropped
    #     a counter of received dropped frames.
    #   
    #   @option options [Symbol] :tx_dropped
    #     a counter of transmitted dropped frames.
    #         
    #   @option options [Symbol] :rx_errors
    #     a counter of received errors.
    #
    #   @option options [Symbol] :tx_errors
    #     a counter of transmitted errors.
    #
    #   @option options [Symbol] :rx_frame_err
    #     a counter of received frame errors.
    #
    #   @option options [Symbol] :rx_over_err
    #     a counter of received overrun errors.
    #
    #   @option options [Symbol] :rx_crc_err
    #     a counter of crc errors.
    #
    #   @option options [Symbol] :collisions
    #     a counter of detected collisions.
    # 
    # @return [PortStatsReply] 
    #   an object that encapsulates the OFPST_STATS_REPLY(OFPST_PORT) openFlow message.
    #
    def initialize options 
      super FIELDS, options
    end
  end
end
