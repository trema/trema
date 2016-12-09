require "pio"

require "arp-table"
require "interface"
require "routing-table"
require "router-utils"

class SimpleRouter < Trema::Controller
  include RouterUtils

  def start( _args )
    load "simple_router.conf"
    @interfaces = Interfaces.new( $interface )
    @arp_table = ARPTable.new
    @routing_table = RoutingTable.new( $route )
  end

  def switch_ready( datapath_id )
    logger.info "#{datapath_id.to_hex} is connected"
    @switche = datapath_id
    init_flows( datapath_id )
  end

  def packet_in( datapath_id, message )
    return if not to_me?( message )

    case message.data
    when Pio::Arp::Request
      handle_arp_request( datapath_id, message )
    when Pio::Arp::Reply
      handle_arp_reply( message )
    when Pio::Parser::IPv4Packet
      handle_ipv4( datapath_id, message )
    else
      fail 'Failed to handle packet_in data type'
    end
  end

  private

  def init_flows( datapath_id )
    send_flow_mod_delete( datapath_id, match: Match.new() )
  end

  def to_me?( message )
    return true if message.destination_mac.broadcast?

    interface = @interfaces.find_by_port( message.in_port )
    if interface and interface.has?( message.destination_mac )
      return true
    end
  end

  def handle_arp_request( datapath_id, message )
    port = message.in_port
    destination_mac = message.data.target_protocol_address
    interface = @interfaces.find_by_port_and_ipaddr( port, destination_mac )
    if interface
      arp_reply = create_arp_reply_from( message, interface.hwaddr )
      packet_out_raw( datapath_id, arp_reply, SendOutPort.new( interface.port ) )
    end
  end

  def handle_arp_reply( message )
    @arp_table.update( message.in_port,
                      message.sender_protocol_address,
                      message.source_mac
                     )
  end

  def handle_ipv4( datapath_id, message )
    if should_forward?( message )
      forward( datapath_id, message )
    elsif message.data.ip_protocol == 1
      icmp = Pio::Icmp.read( message.raw_data )
      if icmp.icmp_type == 8
        handle_icmpv4_echo_request( datapath_id, message )
      else
        fail "Faild to handle icmp type"
      end
    else
      fail "Faild to handle ipv4 packet"
    end
  end

  def should_forward?( message )
    not @interfaces.find_by_ipaddr( message.data.ip_destination_address )
  end

  def handle_icmpv4_echo_request( datapath_id, message )
    interface = @interfaces.find_by_port( message.in_port )
    ip_source_address = message.data.ip_source_address
    arp_entry = @arp_table.lookup( ip_source_address )
    if arp_entry
      icmpv4_reply = create_icmpv4_reply( arp_entry, interface, message )
      packet_out_raw( datapath_id, icmpv4_reply, SendOutPort.new( interface.port ) )
    else
      handle_unresolved_packet( datapath_id, message, interface, ip_source_address )
    end
  end

  def forward( datapath_id, message )
    next_hop = resolve_next_hop( message.data.ip_destination_address )

    interface = @interfaces.find_by_prefix( next_hop )
    if not interface or interface.port == message.in_port
      return
    end

    arp_entry = @arp_table.lookup( next_hop )
    if arp_entry
      macsa = interface.hwaddr
      macda = arp_entry.hwaddr
      action = create_action_from( macsa, macda, interface.port )
      flow_mod( datapath_id, message, action )
      packet_out( datapath_id, message, action )
    else
      handle_unresolved_packet( datapath_id, message, interface, next_hop )
    end
  end

  def resolve_next_hop( ip_destination_address )
    interface = @interfaces.find_by_prefix( ip_destination_address )
    if interface
      ip_destination_address
    else
      @routing_table.lookup( ip_destination_address )
    end
  end

  def flow_mod( datapath_id, message, action )
    send_flow_mod_add(
      datapath_id,
      match: ExactMatch.new( message ),
      actions: action
    )
  end

  def packet_out( datapath_id, packet, action )
    send_packet_out(
      datapath_id,
      packet_in: packet,
      actions: action
    )
  end

  def packet_out_raw( datapath_id, raw_data, action )
    send_packet_out(
      datapath_id,
      raw_data: raw_data,
      actions: action
    )
  end

  def handle_unresolved_packet( datapath_id, message, interface, ipaddr )
    arp_request = create_arp_request_from( interface, ipaddr )
    packet_out_raw( datapath_id, arp_request, SendOutPort.new( interface.port ) )
  end

  def create_action_from( source_mac, destination_mac, port )
    [
      SetEtherSourceAddress.new( source_mac ),
      SetEtherDestinationAddress.new( destination_mac ),
      SendOutPort.new( port )
    ]
  end
end
