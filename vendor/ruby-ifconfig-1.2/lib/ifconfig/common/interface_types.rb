# $Id: interface_types.rb,v 1.1.1.1 2005/07/02 19:10:58 hobe Exp $
#
class NetworkAdapter
  def initialize(name, ifacetxt)
    @name = name
    @ifconfig = ifacetxt
    @status = false
    @protos = ['inet','inet6','IPX/Ethernet II',
               'IPX/Ethernet SNAP',
               'IPX/Ethernet 802.2',
               'IPX/Ethernet 802.3',
               'EtherTalk Phase 2'].join("|")
    @networks = {}
    @flags = []
    @mtu = nil
    @metric = nil
    @rx = @tx = {}
    parse_ifconfig
  end
  attr_reader :status, :name, :flags, :mtu, :networks
  attr_accessor :tx, :rx

  # take array and turn each two entries into
  # hash key and value, also converts each value to an integer
  #
  # [1,2,3,4] => { 1 => 2, 3 => 4}
  #
  # Internal utility function used to populate rx and tx hashes
  def array_to_hash_elem(array)
    h = {}
    if array.length.modulo(2) != 0
      puts "Array mus have even number of elements to turn into a hash"
      return nil
    end
    while array.length > 0
      h[array.shift] = array.shift.to_i
    end
    return h
  end

  # Return all addresses bound to this interface or
  # optionally only the specified type of network address
  #
  def addresses(type=nil)
    a = []
    @networks.each_value { |network|
      a << (network.addr) if network.nettype == type or type.nil?
    }
    return a
  end

  def ifacetype
    return self.class
  end

  def up?
    return status
  end

  # returns array of arrays
  # [ [address , type ] ]
  #
  def addrs_with_type
    addrs = []
    @networks.each_value { |network|
      addrs.push([network.addr,network.nettype])
    }
    return addrs
  end

  def addr_types
    types = []
    @networks.each_value { |network|
      types.push(network.nettype) unless types.include?(network.nettype)
    }
    return types
  end

  def has_addr?(addr)
    return self.addresses.include?(addr)
  end

  def to_s
    s = @name+":"+self.ifacetype.to_s+"\n"
    @networks.keys.sort.each { |network|
      s += @networks[network].to_s+"\n"
    }
    if self.rx['bytes'] && self.tx['bytes']
      s += " RX bytes: #{self.rx['bytes']}, TX bytes: #{self.tx['bytes']}\n"
    elsif self.rx['packets'] && self.tx['packets']
      s += " RX packets: #{self.rx['packets']}, TX packets: #{self.tx['packets']}\n"
    end

    s += " MTU: #{@mtu}\n"
    s += " Metric: #{@metric}\n"
    s += " Flags: #{@flags.join(',')}\n"
    s += " Status: UP" if self.status
    return s
  end
end

# each platform defines it's own set_mac
# function to get the mac address
#
class EthernetAdapter < NetworkAdapter
  def initialize(name,ifconfigtxt)
    super(name,ifconfigtxt)
    @mac = set_mac
  end

  attr_reader :mac, :interrupt, :rxbytes, :txbytes, :rxpackets,
              :txpackets

  def to_s
    super + "\n MAC: #{@mac}"
  end

end

class PPP < NetworkAdapter
end

class LoopbackInterface < NetworkAdapter
end

class IPv6_in_IPv4 < NetworkAdapter
end

class SerialLineIP < NetworkAdapter
end
