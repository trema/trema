require "trema/stats-helper"

module Trema
  class VendorStatsReply < StatsHelper
    FIELDS = %w(vendor_id)

    FIELDS.each { |field| attr_reader field.intern }

    
    # Vendor statistics reply.
    # A user would not implicitly instantiate a {VendorStatsReply} object but would
    # be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_VENDOR)+
    # openflow message.
    #
    # @overload initialize(otions={})
    #
    #   @example 
    #     VendorStatsReply.new(
    #       :vendor_id => 123
    #     )
    #
    #   @param [Hash] options the options hash.
    #
    #   @option options [Symbol] :vendor_id
    #     the specific vendor identifier.
    #
    # @return [VendorStatsReply] 
    #   an object that encapsulates the OFPST_STATS_REPLY(OPPST_VENDOR) openFlow message. 
    #
    def initialize options
      super FIELDS, options
    end
  end
end
