require 'pio'

module RouterUtils
  def create_arp_request_from( interface, addr )
    arp_request = Pio::Arp::Request.new( source_mac: interface.hwaddr,
                                        sender_protocol_address: interface.ipaddr,
                                        target_protocol_address: addr
                                       )
    arp_request.to_binary
  end

  def create_arp_reply_from( message, replyaddr )
    arp_reply = Pio::Arp::Reply.new( destination_mac: message.data.source_mac,
                                    source_mac: replyaddr,
                                    sender_protocol_address: message.data.target_protocol_address,
                                    target_protocol_address: message.data.sender_protocol_address
                                   )
    arp_reply.to_binary
  end

  def create_icmpv4_reply( entry, interface, message )
    icmp_request = Pio::Icmp.read( message.raw_data )
    icmp_reply = Pio::Icmp::Reply.new( identifier: icmp_request.icmp_identifier,
                                      source_mac: message.destination_mac,
                                      destination_mac: message.source_mac,
                                      ip_destination_address: message.ip_source_address,
                                      ip_source_address: message.ip_destination_address,
                                      sequence_number: icmp_request.icmp_sequence_number,
                                      echo_data: icmp_request.echo_data
                                     )
    icmp_reply.to_binary
  end
end
