require "trema/stats-helper"


module Trema
  class FlowStatsReply < StatsHelper
    FIELDS = %w(length table_id match duration_sec duration_nsec) + 
      %w(priority idle_timeout hard_timeout cookie packet_count byte_count actions)

    FIELDS.each { |field| attr_reader field.intern }

    
    #
    # Create FlowStatsReply from options hash.
    #
    # @example FlowStatsReply.new( options = {} )
    #   options { :length => 86, :table_id => 1, :match => Match.new
    #   :duration_sec => 10, :duration_nsec => 555, :priority => 0, :idle_timeout => 0,
    #   :hard_timeout => 0, :cookie => 0xabcd, :packet_count => 1, :byte_count => 1
    #   :actions => [ ActionOutput.new ] }
    #
    # @return [FlowStatsReply] an object that encapsulates the OFPST_STATS_REPLY(OPPST_FLOW) OpenFlow message. 
    #
    def initialize options 
      super FIELDS, options
    end
  end
end
