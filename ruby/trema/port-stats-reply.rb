require "trema/stats-helper"

module Trema
  #
  # PortStats Reply class
  # attributes mapped to ofp_port_stats
  #
  class PortStatsReply < StatsHelper
    FIELDS = %w(port_no rx_packets tx_packets rx_bytes) + 
      %w(tx_bytes rx_dropped tx_dropped rx_errors tx_errors) +
      %w(rx_frame_err rx_over_err rx_crc_err collisions )

    FIELDS.each { |field| attr_reader field.intern }


    def initialize options 
      super FIELDS, options
    end
  end
end
