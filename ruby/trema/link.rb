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
    # Returns the name of link interface
    #
    # @example
    #   link.name => "trema3-0"
    #
    # @return [String]
    #
    # @api public
    #
    attr_reader :name


    #
    # Returns the name of link peer interface
    #
    # @example
    #   link.name => "trema3-1"
    #
    # @return [String]
    #
    # @api public
    #
    attr_reader :name_peer
    

    #
    # Returns the configuration names of link peers
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
      @link_id = Link.instances.size
      @name = "trema#{ @link_id }-0"
      @name_peer = "trema#{ @link_id }-1"
      @peers = stanza.peers
      Link.add self
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
      sh "sudo ip link add name #{ @name } type veth peer name #{ @name_peer }"
      sh "sudo /sbin/ifconfig #{ @name } up"
      sh "sudo /sbin/ifconfig #{ @name_peer } up"
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
      sh "sudo ip link delete #{ @name } 2>/dev/null" rescue nil
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
