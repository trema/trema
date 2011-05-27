#
# Network link between hosts and switches.
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


require "trema/host"
require "trema/network-component"
require "trema/switch"


module Trema
  class Link < NetworkComponent
    attr_reader :peers


    def initialize stanza, link_id
      @link_id = link_id
      @peers = stanza.peers
      self.class.add self
    end


    def name
      "trema#{ @link_id }"
    end

    
    def interfaces
      [ "trema#{ @link_id }-0", "trema#{ @link_id }-1" ]
    end


    def up!
      sh "sudo ip link add name #{ interfaces[ 0 ] } type veth peer name #{ interfaces[ 1 ] }"
      sh "sudo /sbin/ifconfig #{ interfaces[ 0 ] } up"
      sh "sudo /sbin/ifconfig #{ interfaces[ 1 ] } up"
    end


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
