require "trema/stats-helper"

module Trema
  class AggregateStatsReply < StatsHelper
    FIELDS = %w(packet_count byte_count flow_count)

    FIELDS.each { |field| attr_reader field.intern }


    #
    # Create AggregateStatsReply from options hash.
    #
    # @example AggregateStatsReply.new( options = {} )
    #   options = { :packet_count => 1, :byte_count => 64, :flow_count => 1 }
    #
    # @return [AggregateStatsReply] an object that encapsulates the OFPST_STATS_REPLY(OFPST_AGGREGATE) OpenFlow message.
    #
    def initialize options
      super FIELDS, options
    end
  end
end
