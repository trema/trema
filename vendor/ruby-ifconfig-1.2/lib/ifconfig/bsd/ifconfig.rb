# $Id: ifconfig.rb,v 1.1.1.1 2005/07/02 19:10:58 hobe Exp $
#
require 'ifconfig/common/ifconfig'
require 'ifconfig/bsd/network_types'
require 'ifconfig/bsd/interface_types'

class Ifconfig
  #
  # ifconfig = user provided ifconifg output
  # netstat = same, but for netstat -in
  #
  @@ifcfg_cmd = "/usr/bin/env ifconfig -a"
  @@netstat_cmd = "/usr/bin/env netstat -in"
  def initialize(ifconfig=nil,netstat=nil,verbose=nil)
    @ifconfig = ifconfig
    @ifconfig ||= IO.popen(@@ifcfg_cmd){ |f| f.readlines.join }

    @netstat = netstat
    @netstat ||= IO.popen(@@netstat_cmd){ |f| f.readlines.join }

    @verbose = verbose

    @ifaces = {}
    split_interfaces(@ifconfig).each do |iface|
      iface_name = get_iface_name(iface)
      case iface
        when /^lo\d\:/im
          @ifaces[iface_name] = LoopbackInterface.new(iface_name,iface)
          parse_activity(iface_name)
        when /\s+media\:\s+Ethernet\s+/im
          @ifaces[iface_name] = EthernetAdapter.new(iface_name,iface)
          parse_activity(iface_name)
        when /\s+supported\smedia\:\s+none\s+autoselect\s+/im
          # This clause is only matched on Darwin. This pattern will only be
          # matched on ethernet devices (won't match on fw0 or any other
          # interface I can see).
          @ifaces[iface_name] = EthernetAdapter.new(iface_name,iface)
          parse_activity(iface_name)
        else
          puts "Unknown Adapter Type: #{iface}" if @verbose
      end
    end
  end

  # Parse activity on interface
  #
  # Not doing it in each interface class so we can pass in
  # fake input
  #
  def parse_activity(iface)
    mtu = rxpackets = rxerrors = txpackets = txerrors = 0
    @netstat.split("\n").each { |line|
      line.strip!
      if line =~ /^#{iface}/
        next if line.split[2] =~ /\<Link\#\d\>/
        puts "matched line for "+iface
        toks = line.split
        mtu = toks[1]
        rxpackets += toks[4].to_i
        rxerrors += toks[5].to_i
        txpackets += toks[6].to_i
        txerrors += toks[7].to_i
        @ifaces[iface].mtu = mtu.to_i
        @ifaces[iface].rx = { 'packets' => rxpackets,
                              'errors' => rxerrors }
        @ifaces[iface].tx = { 'packets' => txpackets,
                              'errors' => txerrors }
      end
    }
  end

end
