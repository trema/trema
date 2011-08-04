require "trema/stats-helper"

module Trema
  #
  # AggregateStatsReply class
  #  attributes mapped to ofp_stats_aggregate_reply
  #
  class AggregateStatsReply < StatsHelper
    FIELDS = %w(packet_count byte_count flow_count)

    FIELDS.each { |field| attr_reader field.intern }


    #
    # Initialize the fields(attributes) from the options hash
    #
    # @example AggregateStatsReply.new :packet_count => 1, :byte_count => 64, 
    # :flow_count => 1
    #
    # @returns AggregateStatsReply instance
    #
    # @api public
    #
    def initialize options
      super FIELDS, options
    end
  end
end
