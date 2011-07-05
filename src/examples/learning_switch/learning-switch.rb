#
# Simple learning switch application in Ruby
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


class ForwardingEntry
  include Trema::Logger


  attr_reader :mac
  attr_reader :port_no
  attr_writer :age_max


  def initialize mac, port_no, age_max
    @mac = mac
    @port_no = port_no
    @age_max = age_max
    @last_update = Time.now
    debug "New entry: MAC address = #{ @mac.to_s }, port number = #{ @port_no }"
  end


  def update port_no
    debug "Update: The port number of #{ @mac.to_s } has been changed #{ @port_no } => #{ port_no }"
    @port_no = port_no
    @last_update = Time.now
  end


  def aged_out?
    aged_out = Time.now - @last_update > @age_max
    debug "Age out: An entry (MAC address = #{ @mac.to_s }, port number = #{ @port_no }) has been aged-out" if aged_out
    aged_out
  end
end


#
# A database that keep pairs of MAC address and port number
#
class ForwardingDB
  AGE_MAX = 300


  def initialize
    @db = {}
  end


  def find mac
    @db[ mac ]
  end


  def learn mac, port_no
    entry = find( mac )
    if entry
      entry.update port_no
    else
      new_entry = ForwardingEntry.new( mac, port_no, AGE_MAX )
      @db[ new_entry.mac ] = new_entry
    end
  end


  def age
    @db.delete_if do | mac, entry |
      entry.aged_out?
    end
  end
end


#
# A OpenFlow controller class that emulates a layer-2 switch.
#
class LearningSwitch < Trema::Controller
  timer_event :age_db, 5


  def packet_in message
    learn message
    if dest_port_of( message )
      flow_mod message
      packet_out message
    else
      flood message
    end
  end


  def age_db
    @fdb.each_value do | each |
      each.age
    end
  end


  ##############################################################################
  private
  ##############################################################################


  def learn message
    fdb( message.datapath_id ).learn message.macsa, message.in_port
  end


  def fdb datapath_id
    @fdb ||= {}
    @fdb[ datapath_id ] ||= ForwardingDB.new
    @fdb[ datapath_id ]
  end


  def dest_port_of message
    dest = fdb( message.datapath_id ).find( message.macda )
    if dest
      dest.port_no
    else
      nil
    end
  end


  def flow_mod message
    send_flow_mod_add(
      message.datapath_id,
      :match => Match.from( message ),
      :actions => Trema::ActionOutput.new( dest_port_of( message ) )
    )
  end


  def packet_out message
    send_packet_out message, Trema::ActionOutput.new( dest_port_of( message ) )
  end


  def flood message
    send_packet_out message, Trema::ActionOutput.new( OFPP_FLOOD )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
