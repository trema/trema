# $Id: interface_types.rb,v 1.1.1.1 2005/07/02 19:10:57 hobe Exp $
#

require 'ifconfig/common/interface_types'

# base class for network adapters
class NetworkAdapter
  # Parse activity on interface
  # 
  def parse_activity
    #imaptest1# netstat -in
    #Name  Mtu  Net/Dest      Address        Ipkts  Ierrs Opkts  Oerrs Collis Queue 
    #lo0   8232 0.0.0.0       0.0.0.4        267824 0     267824 0     0      0     
    #bge0  1500 10.0.0.0      10.32.4.138    10935939 0     7741167 0     0      0 
    cmd = "netstat -in | grep #{@name}"
    line = IO.popen(cmd).gets
    return if line.nil?
    name,@mtu,dfgw,addr,@rx['packets'],@rx['errors'],
    @tx['packets'],@tx['errors'],collisions,queue = line.split
    @mtu = @mtu.to_i
  end
 
  # iterate line by line and dispatch to helper functions
  # for lines that match a pattern
  # 
  def parse_ifconfig
    @ifconfig.split("\n").each { |line|
      case line
        when /^\s+#{@protos}/
          add_network(line)
        when /flags\=/i
          parse_flags(line)
      end
    }
    parse_activity
  end
  
  # parses the "UP LOOPBACK RUNNING  MTU:3924  Metric:1" line
  #
  def parse_flags(line)
    flags = Regexp.compile(/\<(\S+)\>/i).match(line)[1]
    flags = flags.strip.split(',')
    @flags = flags
    @status = true if @flags.include?('UP')
  end

  # Parses networks on an interface based on the first token in the line.
  # eg: inet or inet6
  #
  def add_network(line)
    case line
      when /^\s+inet\s/
        addr,mask = Regexp.compile(/\s+([\d\d?\d?\.]{4,})\s+netmask\s+(\S+)/i).match(line)[1..2]
        bcast = Regexp.compile(/\s+broadcast\s([\d\d?\d?\.]{4,})/i).match(line)
        bcast = bcast[1] unless bcast.nil?
        @networks['inet'] = Ipv4Network.new(addr, mask, bcast)
      when /^\s+inet6\s/
        # there can be multiple inet6 entries
        addr,scope = 
        Regexp.compile(/inet6\s+(\S+)/i).match(line)[1]
        @networks["inet6:#{addr}"] = Ipv6Network.new(addr)
      else
        puts "unknown network type: #{line}"
    end
  end
end


class EthernetAdapter
  def set_mac
    begin
      @mac = Regexp.compile(/\s+ether\s+([a-f\d]{1,2}(?:\:[a-f\d]{1,2}){5})/im).match(@ifconfig)[1]
    rescue NoMethodError
      puts "Couldn't Parse MAC Address for: "+@name
    end
  end
end
