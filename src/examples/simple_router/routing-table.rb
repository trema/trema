#
# Routing Table
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


require "ipaddr"


class RoutingTable
  ADDR_LEN = 32


  def initialize route = []
    @db = Array.new( ADDR_LEN + 1 ) { Hash.new }
    route.each do | each |
      add( each )
    end
  end


  def add options
    dest = IPAddr.new( options[ :destination ] )
    masklen = options[ :masklen ]
    prefix = dest.mask( masklen )
    @db[ masklen ][ prefix.to_i ] = IPAddr.new( options[ :nexthop ] )
  end


  def delete options
    dest = IPAddr.new( options[ :destination ] )
    masklen = options[ :masklen ]
    prefix = dest.mask( masklen )
    @db[ masklen ].delete( prefix.to_i )
  end


  def lookup dest
    ( 0..ADDR_LEN ).reverse_each do | masklen |
      prefix = dest.mask( masklen )
      entry = @db[ masklen ][ prefix.to_i ]
      return entry if entry
    end
    nil
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
