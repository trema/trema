require "trema/stats"
module Trema
	class FlowStatsReply < Stats
		FIELDS = %w(length table_id match duration_sec duration_nsec) + 
			%w(priority idle_timeout hard_timeout cookie packet_count byte_count)

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
