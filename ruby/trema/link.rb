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
    attr_reader :name


    #
    # Returns the name of link peer interface
    #
    # @example
    #   link.name => "trema3-1"
    #
    # @return [String]
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
    attr_reader :peers


    #
    # Creates a new Trema link from {DSL::Link}
    #
    # @example
    #   link = Trema::Link.new( stanza )
    #
    # @return [Link]
    #
    def initialize stanza
      @link_id = Link.instances.size
      @name = "trema#{ @link_id }-0"
      @name_peer = "trema#{ @link_id }-1"
      @peers = stanza.peers
      Link.add self
    end


    #
    # Adds a virtual link
    #
    # @example
    #   link.add!
    #
    # @return [undefined]
    #
    def add!
      sh "sudo ip link add name #{ @name } type veth peer name #{ @name_peer }"
      sh "sudo sysctl -w net.ipv6.conf.#{ @name }.disable_ipv6=1 >/dev/null 2>&1"
      sh "sudo sysctl -w net.ipv6.conf.#{ @name_peer }.disable_ipv6=1 >/dev/null 2>&1"
    end


    #
    # Ups the peer interfaces of a virtual link
    #
    # @example
    #   link.up!
    #
    # @return [undefined]
    #
    def up!
      sh "sudo /sbin/ifconfig #{ @name } up"
      sh "sudo /sbin/ifconfig #{ @name_peer } up"
    end


    #
    # Creates and enables a virtual link
    #
    # @example
    #   link.enable!
    #
    # @return [undefined]
    #
    def enable!
      add!
      up!
    end


    #
    # Deletes a virtual link
    #
    # @example
    #   link.delete!
    #
    # @return [undefined]
    #
    def delete!
      # FIXME: do not rescue nil
      sh "sudo ip link delete #{ @name } 2>/dev/null" rescue nil
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
