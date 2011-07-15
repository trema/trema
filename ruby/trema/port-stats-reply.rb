require "trema/stats"
module Trema
	class PortStatsReply < Stats
		FIELDS = %w(port_no rx_packets tx_packets rx_bytes) + 
			%w(tx_bytes rx_dropped tx_dropped rx_errors tx_errors) +
			%w(rx_frame_err rx_over_err rx_crc_err collisions )

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
