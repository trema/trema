# $Id: ifconfig.rb,v 1.1.1.1 2005/07/02 19:10:57 hobe Exp $
#

require 'ifconfig/common/ifconfig'

class Ifconfig
  #
  # Can manually specify the platform (should be output of the 'uname' command)
  # and the ifconfig input
  # 
  def initialize(input=nil,verbose=nil)
    if input.nil?
      cmd = IO.popen('which ifconfig'){ |f| f.readlines[0] }
      exit unless !cmd.nil?
      @ifconfig = IO.popen("/sbin/ifconfig -a"){ |f| f.readlines.join }
    else
      @ifconfig = input
    end
    @verbose = verbose
    
    require 'ifconfig/sunos/network_types'
    require 'ifconfig/sunos/interface_types'
    
    @ifaces = {}
    
    split_interfaces(@ifconfig).each do |iface|
      iface_name = get_iface_name(iface)
      case iface
        when /^lo\d\:/im
          @ifaces[iface_name] = LoopbackInterface.new(iface_name,iface)
        when /\s+ether\s+/im
          @ifaces[iface_name] = EthernetAdapter.new(iface_name,iface)
        else
          puts "Unknown Adapter Type: #{iface}" if @verbose
      end
    end
  end
end
