#
# ARP processing routines
#
# Copyright (C) 2012 NEC Corporation
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


class ARPEntry
  include Trema::Logger


  attr_reader :port
  attr_reader :hwaddr
  attr_writer :age_max


  def initialize port, hwaddr, age_max
    @port = port
    @hwaddr = hwaddr
    @age_max = age_max
    @last_updated = Time.now
    info "New entry: MAC addr = #{ @hwaddr.to_s }, port = #{ @port }"
  end


  def update port, hwaddr
    @port = port
    @hwaddr = hwaddr
    @last_updated = Time.now
    info "Update entry: MAC addr = #{ @hwaddr.to_s }, port = #{ @port }"
  end


  def aged_out?
    aged_out = Time.now - @last_updated > @age_max
    info "Age out: An ARP entry (MAC address = #{ @hwaddr.to_s }, port number = #{ @port }) has been aged-out" if aged_out
    aged_out
  end
end


class ARPTable
  DEFAULT_AGE_MAX = 300


  def initialize
    @db = {}
  end


  def update port, ipaddr, hwaddr
    entry = @db[ ipaddr.to_i ]
    if entry
      entry.update( port, hwaddr )
    else
      new_entry = ARPEntry.new( port, hwaddr, DEFAULT_AGE_MAX )
      @db[ ipaddr.to_i ] = new_entry
    end
  end


  def lookup ipaddr
    @db[ ipaddr.to_i ]
  end


  def age
    @db.delete_if do | ipaddr, entry |
      entry.aged_out?
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
