require "trema/stats-helper"

module Trema
  #
  # TableStasReply class
  # attributes mapped to ofp_table_stats
  #
  class TableStatsReply < StatsHelper
    FIELDS = %w(table_id name wildcards max_entries ) +
      %w(active_count lookup_count matched_count)

    FIELDS.each { |field| attr_reader field.intern }


    def initialize options
      super FIELDS, options
    end
  end
end
