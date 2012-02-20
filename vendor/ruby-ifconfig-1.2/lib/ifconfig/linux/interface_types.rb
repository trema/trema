# $Id: interface_types.rb,v 1.1.1.1 2005/07/02 19:10:57 hobe Exp $
#

require 'ifconfig/common/interface_types'

class NetworkAdapter
  # Parse activity on interface
  #
  def parse_activity(line)
    atr = %w(packets,errors,dropped,overruns)
    case line.strip!

    when /^RX packets/
      @rx = array_to_hash_elem(
        line.match(/^RX\s+(\w+)\:(\d+)\s+(\w+)\:(\d+)\s+(\w+)\:(\d+)\s+(\w+)\:(\d)/i)[1..8])
      match = line.match(/\s+frame\:(\d+)/i)
      @rx['frame'] = match[1] unless match.nil?

    when /^TX packets/
      @tx = array_to_hash_elem(
        line.match(/^TX\s+(\w+)\:(\d+)\s+(\w+)\:(\d+)\s+(\w+)\:(\d+)\s+(\w+)\:(\d+)/i)[1..8])
      match = line.match(/\s+carrier\:(\d+)/i)
      @tx['carrier'] = match[1] unless match.nil?

    when /^RX bytes/
      match = line.match(/RX\s+(\w+)\:(\d+).*TX\s+(\w+)\:(\d+)/i)[1..4]
      @rx.merge!(array_to_hash_elem(match[0..1]))
      @tx.merge!(array_to_hash_elem(match[2..3]))
    end
  end

  # iterate line by line and dispatch to helper functions
  # for lines that match a pattern
  #
  def parse_ifconfig
    @ifconfig.split("\n").each { |line|
      case line
        when /^\s+#{@protos}/
          add_network(line)
        when /MTU\:\d+\s+Metric\:/
          parse_flags(line)
        when /^\s+RX|TX/
          parse_activity(line)
      end
    }
  end

  # parses the "UP LOOPBACK RUNNING  MTU:3924  Metric:1" line
  #
  def parse_flags(line)
    flags = line.strip.split
    @metric =  flags.pop.split(':')[1].to_i
    @mtu =  flags.pop.split(':')[1].to_i
    @flags = flags
    @status = true if @flags.include?('UP')
  end

  # Parses networks on an interface based on the first token in the line.
  # eg: inet or inet6
  #
  def add_network(line)
    case line
      when /^\s+inet\s/
        addr = line.match(/\s+addr\:([\d\d?\d?\.]{4,})/i)[1]
        mask = line.match(/\s+mask\:([\d\d?\d?\.]{4,})/i)[1]
        bcast = line.match(/\s+bcast\:([\d\d?\d?\.]{4,})/i)
        bcast = bcast[1] unless bcast.nil?
        ptp = line.match(/\s+P-t-P\:([\d\d?\d?\.]{4,})/i)
        ptp = ptp[1] unless ptp.nil?
        @networks['inet'] = Ipv4Network.new(addr, mask, bcast, ptp)

      when /^\s+inet6\s/
        # there can be multiple inet6 entries
        addr,scope = line.match(/\s+addr\:\s+(\S+)\s+scope\:\s*(\w+)/i)[1..2]
        @networks["inet6:#{addr}"] = Ipv6Network.new(addr,scope)

      when /^\s+#{Regexp.escape('IPX/Ethernet II')}\s/
        addr = line.match(/\s+addr\:\s*(\S+)/)[1]
        @networks["IPX/Ethernet II"] = IPX_EthernetII.new(addr)

      when /^\s+#{Regexp.escape('IPX/Ethernet 802.2')}\s/
        addr = line.match(/\s+addr\:\s*(\S+)/)[1]
        @networks["IPX/Ethernet 802.2"] = IPX_Ethernet802_2.new(addr)

      when /^\s+#{Regexp.escape('IPX/Ethernet 802.3')}\s/
        addr = line.match(/\s+addr\:\s*(\S+)/)[1]
        @networks["IPX/Ethernet 802.3"] = IPX_Ethernet802_3.new(addr)

      when /^\s+#{Regexp.escape('EtherTalk Phase 2')}\s/
        addr = line.match(/\s+addr\:\s*(\S+)/)[1]
        @networks["EtherTalk Phase 2"] = EtherTalkPhase2.new(addr)

      when /^\s+#{Regexp.escape('IPX/Ethernet SNAP')}\s/
        addr = line.match(/\s+addr\:\s*(\S+)/)[1]
        @networks["IPX/Ethernet SNAP"] = IPX_EthernetSNAP.new(addr)

      else
        puts "unknown network type: #{line}"
    end
  end
end


class EthernetAdapter
  def set_mac
    begin
      return @ifconfig.match(/\s+hwaddr\s+([a-f\d]{1,2}(?:\:[a-f\d]{1,2}){5})/im)[1]
    rescue NoMethodError
      puts "Couldn't Parse MAC Address for: #{@name}: #{$!}"
    end
  end
end
