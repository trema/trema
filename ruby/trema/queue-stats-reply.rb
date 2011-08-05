require "trema/stats-helper"

module Trema
  #
  # QueueStatsReply class
  # attributes mapped to ofp_queue_stats
  #
  class QueueStatsReply < StatsHelper
    FIELDS = %w(port_no queue_id tx_bytes tx_packets tx_errors)

    FIELDS.each { |field| attr_reader field.intern }


    def initialize options
      super FIELDS, options
    end
  end
end
