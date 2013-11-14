#
# A router implementation in Trema
#
# Copyright (C) 2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

require 'rubygems'
require 'ipaddr'
require 'pio'

class IPAddr
  def to_a
    to_s.split('.').map do |each|
      each.to_i
    end
  end
end

module RouterUtils
  def create_arp_request_from(interface, addr)
    Pio::Arp::Request.new(:source_mac => interface.hwaddr,
                          :sender_protocol_address => interface.ipaddr,
                          :target_protocol_address => addr).to_binary
  end

  def create_arp_reply_from(message, replyaddr)
    Pio::Arp::Reply(:source_mac => message.macsa,
                    :destination_mac => replyaddr,
                    :sender_protocol_address => message.arp_spa,
                    :target_protocol_address => message.arp_tpa).to_binary
  end

  def create_icmpv4_reply(entry, interface, message)
    request = Pio::Icmp::Request.read(message.data)
    Pio::Icmp::Reply.new(:destination_mac => entry.hwaddr,
                         :source_mac => interface.hwaddr,
                         :ip_source_address => message.ipv4_saddr,
                         :ip_destination_address => message.ipv4_daddr,
                         :icmp_identifier => request.icmp_identifier,
                         :icmp_sequence_number => request.icmp_sequence_number,
                         :echo_data => request.echo_data).to_binary
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
