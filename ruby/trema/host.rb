#
# Copyright (C) 2008-2013 NEC Corporation
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
require "trema/network-component"


module Trema
  #
  # The controller class of host
  #
  class Host < NetworkComponent
    #
    # Set a network interface
    #
    # @example
    #   host.interface #=> "trema0-1"
    #
    # @return [String]
    #
    # @api public
    #
    attr_accessor :interface


    #
    # Creates a new Trema host from {DSL::Vhost}
    #
    # @example
    #   host = Trema::Host.new( stanza )
    #
    # @return [Host]
    #
    # @api public
    #
    def initialize stanza
      @stanza = stanza
      @phost = Phost.new( self )
      @cli = Cli.new( self )
      @index = Host.instances.size
      Host.add self
    end


    #
    # Define host attribute accessors
    #
    # @example
    #   host.name  # delegated to @stanza[ :name ]
    #
    # @return an attribute value
    #
    # @api public
    #
    def method_missing message, *args
      @stanza.__send__ :[], message
    end


    #
    # Returns IP address
    #
    # @example
    #   host.ip #=> "192.168.0.1"
    #
    # @return [String]
    #
    # @api public
    #
    def ip
      stanza_ip = @stanza[ :ip ]
      if stanza_ip.nil?
        # FIXME: Find unused addresses
        "192.168.0.#{ @index + 1 }"
      else
        stanza_ip
      end
    end


    #
    # Returns MAC address
    #
    # @example
    #   host.mac #=> "00:00:00:00:00:01"
    #
    # @return [String]
    #
    # @api public
    #
    def mac
      stanza_mac = @stanza[ :mac ]
      if stanza_mac.nil?
        "00:00:00:00:00:#{ format "%02x", @index + 1 }"
      else
        stanza_mac
      end
    end


    #
    # Returns netmask
    #
    # @example
    #   host.netmask #=> "255.255.0.0"
    #
    # @return [String]
    #
    # @api public
    #
    def netmask
      stanza_netmask = @stanza[ :netmask ]
      if stanza_netmask.nil?
        "255.255.255.255"
      else
        stanza_netmask
      end
    end


    #
    # Runs a host process
    #
    # @example
    #   host.run! => self
    #
    # @return [Host]
    #
    # @api public
    #
    def run!
      @phost.run!
      @cli.set_ip_and_mac_address
      @cli.enable_promisc if @stanza[ :promisc ]
      self
    end


    #
    # Kills running host
    #
    # @example
    #   host.shutdown!
    #
    # @return [undefined]
    #
    # @api public
    #
    def shutdown!
      @phost.shutdown!
    end


    #
    # Add arp entries of <code>hosts</code>
    #
    # @example
    #   host.add_arp_entry [ host1, host2, host3 ]
    #
    # @return [undefined]
    #
    # @api public
    #
    def add_arp_entry hosts
      hosts.each do | each |
        @cli.add_arp_entry each
      end
    end


    #
    # Send packets to <code>dest</code>
    #
    # @example
    #   host.send_packet host1, :pps => 100
    #
    # @return [undefined]
    #
    # @api public
    #
    def send_packet dest, options = {}
      dest_host = if dest.is_a?( String )
                    vhost( dest )
                  else
                    dest
                  end
      @cli.send_packets dest_host, options
    end


    #
    # Check stats type and delegate processing.
    #
    # @raise [RuntimeError] if stats type is invalid.
    #
    # @return [Stat]
    #   the object that represents the results of a particular stats type.
    #
    def stats type
      case type
      when :tx
        @cli.tx_stats
      when :rx
        @cli.rx_stats
      else
        raise
      end
    end


    #
    # Returns tx stats
    #
    # @example
    #   host.tx_stats
    #
    # @return [Stat]
    #
    # @api public
    #
    def tx_stats
      @cli.tx_stats
    end


    #
    # Returns rx stats
    #
    # @example
    #   host.rx_stats
    #
    # @return [Stat]
    #
    # @api public
    #
    def rx_stats
      @cli.rx_stats
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
