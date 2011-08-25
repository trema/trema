require "trema/stats-helper"

module Trema
  class QueueStatsReply < StatsHelper
    FIELDS = %w(port_no queue_id tx_bytes tx_packets tx_errors)

    FIELDS.each { |field| attr_reader field.intern }


    # Queue statistics for a port.
    # A user would not implicitly instantiate a {QueueStatsReply} object but 
    # would be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_QUEUE)+
    # message.
    # 
    # @overload initialize(options={})
    # 
    #   @example 
    #     QueueStatsReply.new(
    #       :port_no => 1,
    #       :queue_id => 123,
    #       :tx_bytes => 128
    #       :tx_packets => 2
    #       :tx_errors => 0
    #     )
    #   
    #   @param [Hash] options the options hash.
    #   
    #   @option options [Symbol] :port_no
    #     a specific port or all port if +OFPT_ALL+.
    #   
    #   @option options [Symbol] :queue_id
    #     a specific queue identifier or all queues if +OFPQ_ALL+.
    #   
    #   @option options [Symbol] :tx_bytes
    #     a counter of transmitted bytes.
    #   
    #   @option options [Symbol] :tx_packets
    #     a counter of transmitted packets.
    #   
    #   @option options [Symbol] :tx_errors
    #     a counter of transmitted errors.
    #     
    # @return [QueueStatsReply] 
    #   an object that encapsulates the OFPST_STATS_REPLY(OPPST_QUEUE) openFlow message. 
    #
    def initialize options
      super FIELDS, options
    end
  end
end
