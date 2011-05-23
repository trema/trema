#
# The controller class of host.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


require "trema/cli"
require "trema/phost"


module Trema
  class Stats
    attr_reader :n_pkts

    
    def initialize ip_dst, tp_dst, ip_src, tp_src, n_pkts, n_octets
      @ip_dst = ip_dst
      @tp_dst = tp_dst
      @ip_src = ip_src
      @tp_src = tp_src
      @n_pkts = n_pkts.to_i
      @n_octets = n_octets.to_i
    end
  end

  
  class Host
    @@list = {}


    def self.all
      @@list
    end


    def self.[] name
      @@list[ name ]
    end


    def self.add host
      @@list[ host.name ] = host
    end
    
    
    attr_accessor :interface


    def initialize stanza
      @stanza = stanza
      @phost = Phost.new( self )
      @cli = Cli.new( self )
      self.class.add self
    end


    # Define host attribute accessors.
    # e.g., host.name is delegated to @stanza[ :name ]
    def method_missing message, *args
      @stanza.__send__ :[], message
    end


    def run
      @phost.run
      @cli.set_ip_and_mac_address
      @cli.enable_promisc if @stanza[ :promisc ]
    end


    def shutdown!
      @phost.shutdown!
    end


    def add_arp_entry hosts
      hosts.each do | each |
        @cli.add_arp_entry each
      end
    end


    def send_packet dest, options = {}
      @cli.send_packets dest, options
    end


    def tx_stats
      stat = @cli.tx_stats.split( "\n" )[ 1 ]
      if stat
        Stats.new *stat.split( "," )        
      else
        Stats.new 0, 0, 0, 0, 0, 0
      end
    end


    def rx_stats
      stat = @cli.rx_stats.split( "\n" )[ 1 ]
      if stat
        Stats.new *stat.split( "," )        
      else
        Stats.new 0, 0, 0, 0, 0, 0
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
