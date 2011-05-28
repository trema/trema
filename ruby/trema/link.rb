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


require "trema/network-component"


module Trema
  #
  # Network link between hosts and switches.
  #
  class Link < NetworkComponent
    #
    # Returns a pair of link peers
    #
    # @example
    #   link.peers => [ "host 0", "switch 1" ]
    #
    # @return [Array]
    #
    # @api public
    #
    attr_reader :peers


    #
    # Creates a new Trema link from {DSL::Link}
    #
    # @example
    #   link = Trema::Link.new( stanza )
    #
    # @return [Link]
    #
    # @api public
    #
    def initialize stanza
      @link_id = self.class.instances.size
      @peers = stanza.peers
      self.class.add self
    end


    #
    # Returns the name of link
    #
    # @example
    #   link.name => "trema1"
    #
    # @return [String]
    #
    # @api public
    #
    def name
      "trema#{ @link_id }"
    end


    #
    # Returns a pair of network interface name
    #
    # @example
    #   link.interfaces => [ "trema0-0", "trema0-1" ]
    #
    # @return [Array]
    #
    # @api public
    #
    def interfaces
      [ "trema#{ @link_id }-0", "trema#{ @link_id }-1" ]
    end


    #
    # Enables network interfaces
    #
    # @example
    #   link.up!
    #
    # @return [undefined]
    #
    # @api public
    #
    def up!
      sh "sudo ip link add name #{ interfaces[ 0 ] } type veth peer name #{ interfaces[ 1 ] }"
      sh "sudo /sbin/ifconfig #{ interfaces[ 0 ] } up"
      sh "sudo /sbin/ifconfig #{ interfaces[ 1 ] } up"
    end


    #
    # Disables network interfaces
    #
    # @example
    #   link.down!
    #
    # @return [undefined]
    #
    # @api public
    #
    def down!
      sh "sudo /sbin/ifconfig #{ interfaces[ 0 ] } down 2>/dev/null" rescue nil
      sh "sudo /sbin/ifconfig #{ interfaces[ 1 ] } down 2>/dev/null" rescue nil
      sh "sudo ip link delete #{ interfaces[ 0 ] } 2>/dev/null" rescue nil
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
