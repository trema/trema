#
# A router implementation in Trema
#
# Copyright (C) 2013 NEC Corporation
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


require "arp-table"
require "routing-table"


class Interface
  attr_reader :hwaddr
  attr_reader :ipaddr
  attr_reader :masklen
  attr_reader :port


  def initialize options
    @port = options[ :port ]
    @hwaddr = Mac.new( options[ :hwaddr ] )
    @ipaddr = IPAddr.new( options[ :ipaddr ] )
    @masklen = options[ :masklen ]
  end


  def has? mac
    mac == hwaddr
  end
end


class Interfaces
  def initialize interfaces = []
    @list = []
    interfaces.each do | each |
      @list << Interface.new( each )
    end
  end


  def find_by_port port
    @list.find do | each |
      each.port == port
    end
  end


  def find_by_ipaddr ipaddr
    @list.find do | each |
      each.ipaddr == ipaddr
    end
  end


  def find_by_prefix ipaddr
    @list.find do | each |
      masklen = each.masklen
      each.ipaddr.mask( masklen ) == ipaddr.mask( masklen )
    end
  end


  def find_by_port_and_ipaddr port, ipaddr
    @list.find do | each |
      each.port == port and each.ipaddr == ipaddr
    end
  end


  def ours? port, macda
    return true if macda.broadcast?

    interface = find_by_port( port )
    if not interface.nil? and interface.has?( macda )
      return true
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
