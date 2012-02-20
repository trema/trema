# $Id: network_types.rb,v 1.1.1.1 2005/07/02 19:10:57 hobe Exp $
#

require 'ifconfig/common/network_types'

#
# Base class for IPX and Appletalk classes
# Shouldn't be used directly
#
class MiscEthernet < BasicNetworkType
  def initialize(addr)
    super()
    @addr = addr
  end
  attr_reader :addr
  def to_s
    " #{@nettype} Address: #{@addr}"
  end
end

class IPX_EthernetII < MiscEthernet
  def initialize(addr)
    super(addr)
    @nettype = 'IPX/Ethernet II'
  end
end

class IPX_Ethernet802_2 < MiscEthernet
    def initialize(addr)
    super(addr)
    @nettype = 'IPX/Ethernet 802.2'
  end
end

class IPX_Ethernet802_3 < MiscEthernet
    def initialize(addr)
    super(addr)
    @nettype = 'IPX/Ethernet 802.3'
  end
end

class EtherTalkPhase2 < MiscEthernet
    def initialize(addr)
    super(addr)
    @nettype = 'EtherTalk Phase 2'
  end
end

class IPX_EthernetSNAP < MiscEthernet
  def initialize(addr)
    super(addr)
    @nettype = 'IPX/Ethernet SNAP'
  end
end

