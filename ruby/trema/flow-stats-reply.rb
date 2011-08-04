require "trema/stats-helper"


module Trema
  #
  # FlowStatsReply class
  #
  class FlowStatsReply < StatsHelper
    FIELDS = %w(length table_id match duration_sec duration_nsec) + 
      %w(priority idle_timeout hard_timeout cookie packet_count byte_count actions)

    FIELDS.each { |field| attr_reader field.intern }

    
    #
    # Initialize the fields(attributes) provided by the options hash
    #
    # @example FlowStatsReply.new( :length => 86, :table_id => 1, :match => Match.new
    # :duration_sec => 10, :duration_nsec => 555, :priority => 0, :idle_timeout => 0,
    # :hard_timeout => 0, :cookie => 0xabcd, :packet_count => 1, :byte_count => 1
    # :actions => [ ActionOutput.new ] )
    # The match option is an instance of a Match object
    # The actions option is specified as an array of actions objects
    #
    # @returns a FlowStatsReply instance
    #
    # @api public
    #
    def initialize options 
      super FIELDS, options
    end
  end
end
