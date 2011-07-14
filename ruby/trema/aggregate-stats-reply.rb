require "trema/stats"

module Trema
	class AggregateStatsReply < Stats
		FIELDS = %w(packet_count byte_count flow_count)

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
