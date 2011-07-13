module Trema
	class PhyPort
		PORT_CONFIG = { 
			1 << 0 => :OFPPC_PORT_DOWN,
			1 << 1 => :OFPPC_NO_STP,
			1 << 2 => :OFPPC_NO_RECV,
			1 << 3 => :OFPPC_NO_RECV_STP,
			1 << 4 => :OFPPC_NO_FLOOD,
			1 << 5 => :OFPPC_NO_FWD,
			1 << 5 => :OFPPC_NO_PACKET_IN
		}
			
		def to_s
			puts "config #{PORT_CONFIG[self.config].to_s}"
		end
	end
end
