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


require "trema/network-component"


module Trema
  #
  # The controller class of network namespace
  #
  class Netns < NetworkComponent
    #
    # Set a network interface
    #
    # @example
    #   netns.interface #=> "trema0-1"
    #
    # @return [String]
    #
    # @api public
    #
    attr_accessor :interface


    #
    # Creates a new Trema netns from {DSL::Netns}
    #
    # @example
    #   netns = Trema::Netns.new( stanza )
    #
    # @return [Netns]
    #
    # @api public
    #
    def initialize stanza
      @stanza = stanza
      Netns.add self
    end


    #
    # Define netns attribute accessors
    #
    # @example
    #   netns.name  # delegated to @stanza[ :name ]
    #
    # @return an attribute value
    #
    # @api public
    #
    def method_missing message, *args
      @stanza.__send__ :[], message
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
      @stanza[ :netmask ] || "255.255.255.255"
    end


    #
    # Runs a netns process
    #
    # @example
    #   netns.run! => self
    #
    # @return [Netns]
    #
    # @api public
    #
    def run!
      sh "sudo ip netns add #{ name }"
      sh "sudo ip link set dev #{ interface } netns #{ name }"
      sh "sudo ip netns exec #{ name } ifconfig lo 127.0.0.1"
      sh "sudo ip netns exec #{ name } ifconfig #{ interface } #{ @stanza[ :ip ] } netmask #{ netmask }" if @stanza[ :ip ]
      sh "sudo ip netns exec #{ name } route add -net #{ @stanza[ :net ] } gw #{ @stanza[ :gw ] }" if @stanza[ :net ] and @stanza[ :gw ]
      self
    end


    #
    # Kills running netns
    #
    # @example
    #   netns.shutdown!
    #
    # @return [undefined]
    #
    # @api public
    #
    def shutdown!
      sh "sudo ip netns delete #{ name }"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
