class Dumper < Controller

  def packet_in message
		puts "packet in received"
		match = Match.from( message )
    send_flow_mod_add(
      message.datapath_id,
      :match => match,
      :actions => [ ActionOutput.new( OFPP_FLOOD ) ]
    )
    send_packet_out message, ActionOutput.new( OFPP_FLOOD )

		options = { }
		send_message VendorStatsRequest.new(options).to_packet.buffer, message.datapath_id
  end

	def flow_removed message
		info "[flow removed]"
		info "datapath_id 0x#{message.datapath_id.to_s(16)}"
  	info "transaction_id: 0x#{message.transaction_id.to_s(16)}"
		info "match"
  	info "  wildcards: #{message.match.wildcards.to_s(16)}"
  	info "  in_port: #{message.match.in_port}"
  	info "  dl_src: #{message.match.dl_src.to_s}"
  	info "  dl_src: #{message.match.dl_dst.to_s}"
  	info "  dl_vlan: #{message.match.dl_vlan}"
  	info "  dl_vlan_pcp: #{message.match.dl_vlan_pcp}"
  	info "  dl_type: 0x#{message.match.dl_type.to_s(16)}"
  	info "  nw_tos: #{message.match.nw_tos}"
  	info "  nw_proto: 0x#{message.match.nw_proto.to_s(16)}"
  	info "  nw_src: 0x#{message.match.nw_src.to_s(16)}"
  	info "  nw_src: 0x#{message.match.nw_dst.to_s(16)}"
  	info "  tp_src: #{message.match.tp_src}"
  	info "  tp_dst: #{message.match.tp_dst}"
		info "#{message.match.to_s}"
	end

	def switch_disconnected datapath_id
		info "switch disconnected"
		info "datapath_id 0x#{datapath_id.to_s(16)}"
	end

	def port_status message
		info "[port status]"
		info "datapath_id 0x#{message.datapath_id.to_s(16)}"
  	info "transaction_id: 0x#{message.transaction_id.to_s(16)}"
  	info "reason: 0x#{message.reason.to_s(16)}"
		info "phy_port" 
		phy_port = message.phy_port;
  	info "  port_no: #{phy_port[:port_no]}"
  	info "  hw_addr: #{phy_port[:hw_addr].to_s}"
  	info "  name: #{phy_port[:name]}"
  	info "  config: 0x#{phy_port[:config].to_s(16)}"
  	info "  state: 0x#{phy_port[:state].to_s(16)}"
  	info "  curr: 0x#{phy_port[:curr].to_s(16)}"
  	info "  advertised: 0x#{phy_port[:advertised].to_s(16)}"
  	info "  supported: 0x#{phy_port[:supported].to_s(16)}"
  	info "  peer: 0x#{phy_port[:peer].to_s(16)}"
	end

	def stats_reply message
		info "[stats_reply]"
		info "datapath_id 0x#{message.datapath_id.to_s(16)}"
		info "type: 0x#{message.type.to_s(16)}"
		arr = message.stats
		arr.each do |each| 
			puts each.to_s
		end
	end

	def openflow_error message
		info "[error]"
		info "datapath_id 0x#{message.datapath_id.to_s(16)}"
		info "type: 0x#{message.type.to_s(16)}"
		info "code: 0x#{message.code.to_s(16)}"
		info "data: "
		message.data.each do |d|
			puts (d.gsub(/../) { |byte| byte.hex.chr}).unpack("H*")
		end
	end
end
