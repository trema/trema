require "trema/stats-helper"

module Trema
  class VendorStatsReply < StatsHelper
    FIELDS = %w(vendor_id)

    FIELDS.each { |field| attr_reader field.intern }

    NAME = self.name

    def initialize options
      super FIELDS, options
    end

    def to_s
      str="#{NAME}\n" + super.to_s
    end
  end
end
