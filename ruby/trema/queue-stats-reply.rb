require "trema/stats"
module Trema
	class QueueStatsReply < Stats
		FIELDS = %w(port_no queue_id tx_bytes tx_packets tx_errors)

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
