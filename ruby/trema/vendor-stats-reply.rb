require "trema/stats-helper"

module Trema
  #
  # The vendor stats reply class
  #
  class VendorStatsReply < StatsHelper
    FIELDS = %w(vendor_id)

    FIELDS.each { |field| attr_reader field.intern }

    
   #
   # Sets the attribute vendor_id to a passed in argument
   # 
   # @example VendorStatsReply.new 0x1234
   #
   # @return [VendorStatsReply]
   # 
   # @api public
   #
    def initialize options
      super FIELDS, options
    end
  end
end
