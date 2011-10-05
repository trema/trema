#
# Forwarding database (FDB)
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


class FdbEntry
  attr_reader :mac
  attr_accessor :port_number


  def initialize mac, port_number
    @mac = mac
    @port_number = port_number
  end
end


#
# A database that keep pairs of MAC address and port number
#
class FDB
  def initialize
    @db = {}
  end


  def port_number_of mac
    return @db[ mac ].port_number if @db[ mac ]
    nil
  end


  def learn mac, port_number
    if @db[ mac ]
      @db[ mac ].port_number = port_number
    else
      @db[ mac ] = FdbEntry.new( mac, port_number )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
