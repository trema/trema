# $Id: network_types.rb,v 1.1.1.1 2005/07/02 19:10:58 hobe Exp $
#

# base type to hold information about the kinds of networks
# on to each adapter.  ipv4,6 ppp etc
class BasicNetworkType
  def initialize
    @nettype = nil
  end
  attr_reader :nettype
end

#
# Ipv4 Network type
# Optional Broadcast and Point to Point Arguments
#
class Ipv4Network < BasicNetworkType
  def initialize(addr,mask,bcast=nil,ptp=nil)
    super()
    @nettype = 'inet'
    @addr = IPAddr.new(addr)
    @bcast = bcast
    @mask = mask
    @ptp = ptp
  end
  attr_reader :addr, :bcast, :mask, :ptp

  def to_s
    a = [" #{@nettype} Address: #{@addr}","Mask: #{@mask}"]
    a.push "Broadcast: #{@bcast}" unless @bcast.nil?
    a.push "P-t-P: #{@ptp}" unless @ptp.nil?
    return a.join(', ')
  end

end

class Ipv6Network < BasicNetworkType
  def initialize(addr,scope=nil)
    super()
    @nettype = 'inet6'
    @addr = IPAddr.new(addr)
    @scope = scope
  end
  attr_reader :addr, :scope

  def to_s
    " #{@nettype} Address: #{@addr}"
  end
end
