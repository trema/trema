require "trema/stats-helper"

module Trema
  class AggregateStatsReply < StatsHelper
    FIELDS = %w(packet_count byte_count flow_count)

    FIELDS.each { |field| attr_reader field.intern }


    # Aggregate counters for multiple flows.
    # A user would not implicitly instantiate a {AggregateStatsReply} object but
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
    #   @param [Hash] options the options hash.
    #   
    #   @option options [Symbol] :packet_count
    #     the total packet counter.
    #  
    #   @option options [Symbol] :byte_count
    #     the total byte counter.
    #
    #   @option options [Symbol] :flow_count
    #     the total flow counter.
    #     
    # @return [AggregateStatsReply] 
    #   an object that encapsulates the OFPST_STATS_REPLY(OFPST_AGGREGATE) openFlow message.
    #
    def initialize options
      super FIELDS, options
    end
  end
end
