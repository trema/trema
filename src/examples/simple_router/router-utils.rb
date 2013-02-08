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


require 'ipaddr'


def get_checksum csum, val
  sum = ( ~csum & 0xffff ) + val
  while sum > 0xffff
    sum = ( sum & 0xffff ) + ( sum >> 16 )
  end
  ~sum & 0xffff
end


class IPAddr
  def to_a
    self.to_s.split( "." ).collect do | each |
      each.to_i
    end
  end
end


class EthernetHeader
  attr_accessor :macda, :macsa, :eth_type


  def initialize macda, macsa, eth_type
    @macda = macda
    @macsa = macsa
    @eth_type = eth_type
  end


  def pack
    ( @macda.to_a + @macsa.to_a + [ eth_type ] ).pack( "C12n" )
  end
end


class ARPPacket
  attr_accessor :type, :tha, :sha, :tpa, :spa


  def initialize type, tha, sha, tpa, spa
    @type = type
    @tha = tha
    @sha = sha
    @tpa = tpa
    @spa = spa
  end


  def pack
    eth_header = EthernetHeader.new( @tha, @sha, 0x0806 )

    # arp
    arp = [ 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, @type ]
    arp += @sha.to_a + @spa.to_a + @tha.to_a + @tpa.to_a

    while arp.length < 46 do
      arp += [ 0x00 ]
    end

    eth_header.pack + arp.pack( "C*" )
  end
end


class ARPRequest < ARPPacket
  def initialize sha, tpa, spa
    tha = [ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff ]
    super( 1, tha, sha, tpa, spa )
  end
end


class ARPReply < ARPPacket
  def initialize tha, sha, tpa, spa
    super( 2, tha, sha, tpa, spa )
  end
end


class IPPacket
  attr_accessor :id, :protocol, :daddr, :saddr, :payload


  def initialize options
    @id = options[ :id ]
    @protocol = options[ :protocol ]
    @daddr = options[ :daddr ]
    @saddr = options[ :saddr ]
    @payload = options[ :payload ]
    @tot_len = 20 + payload.length
  end


  def pack
    csum = get_checksum( 0, 0x4500 )
    header = [ 0x45, 0x00 ] # Version, IHL, ToS

    csum = get_checksum( csum, @tot_len )
    header += [ @tot_len >> 8, @tot_len & 0xff ] # len

    csum = get_checksum( csum, @id )
    header += [ @id >> 8, @id & 0xff ] # ID

    csum = get_checksum( csum, 0x4000 )
    header += [ 0x40, 0x00 ] # Flags, Frag offset

    csum = get_checksum( csum, 0x40 * 0x100 + @protocol )
    header += [ 0x40, @protocol ] # ttl, protocol

    csum = get_checksum( csum, @saddr.to_i >> 16 )
    csum = get_checksum( csum, @saddr.to_i & 0xffff )
    csum = get_checksum( csum, @daddr.to_i >> 16 )
    csum = get_checksum( csum, @daddr.to_i & 0xffff )
    header += [ csum >> 8, csum & 0xff ] # checksum
    header += @saddr.to_a + @daddr.to_a

    header.pack( "C*" ) + @payload.pack
  end
end


class ICMPPacket
  attr_reader :payload, :length


  def initialize type, code, payload
    @type = type
    @code = code
    @payload = payload
    @length = 4 + payload.length
  end


  def pack
    @checksum = get_checksum( 0, @type * 0x100 + @code )

    words = @payload.pack( "C*" ).unpack( "n*" )
    words.each do | each |
      @checksum = get_checksum( @checksum, each )
    end

    [ @type, @code, @checksum ].pack( "C2n" ) + @payload.pack( "C*" )
  end
end


class ICMPEchoReply < ICMPPacket
  def initialize payload
    super( 0x00, 0x00, payload )
  end
end


module RouterUtils
  def create_arp_request_from interface, addr
    arp = ARPRequest.new( interface.hwaddr, addr, interface.ipaddr )
    arp.pack
  end


  def create_arp_reply_from message, replyaddr
    arp = ARPReply.new( message.macsa, replyaddr, message.arp_spa, message.arp_tpa )
    arp.pack
  end


  def create_icmpv4_reply entry, interface, message
    offset = 14 + 20 + 4
    payload = message.data.unpack( "C*" )[ offset .. message.data.length - 1 ]
    icmp = ICMPEchoReply.new( payload )
    ip_packet = IPPacket.new( :id => message.ipv4_id,
                              :protocol => message.ipv4_protocol,
                              :ttl => message.ipv4_ttl,
                              :daddr => message.ipv4_saddr,
                              :saddr => message.ipv4_daddr,
                              :payload => icmp )
    eth_header = EthernetHeader.new( entry.hwaddr, interface.hwaddr, 0x0800 )

    eth_header.pack + ip_packet.pack
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
