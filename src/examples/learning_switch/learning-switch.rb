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


require "forwardable"


class ForwardingEntry
  attr_reader :mac
  attr_reader :port_no
  attr_writer :age_max


  def initialize mac, port_no, age_max
    @mac = mac
    @age_max = age_max
    update port_no
  end


  def update port_no
    @port_no = port_no
    @last_update = Time.now
  end


  def aged_out?
    Time.now - @last_update > @age_max
  end
end


class ForwardingDB
  extend Forwardable


  AGE_MAX = 300


  def_delegators :@db, :[]
  def_delegators :@db, :delete_if


  def initialize
    @db = {}
  end


  def learn mac, port_no
    entry = @db[ mac ]
    if entry
      entry.update port_no
    else
      new_entry = ForwardingEntry.new( mac, port_no, AGE_MAX )
      @db[ new_entry.mac ] = new_entry
    end
  end


  def age_max= new_value
    @db.each do | mac, entry |
      entry.age_max = new_value
    end
  end
end


class LearningSwitch < Trema::Controller
  timer_event :age_db, 5


  def packet_in message
    fdb( message.datapath_id ).learn message.macsa, message.in_port
    dest = fdb( message.datapath_id )[ message.macda ]
    if dest
      flow_mod message, dest.port_no
      packet_out message, dest.port_no
    else
      flood message
    end
  end


  def age_db
    @fdb.each do | datapath_id, db |
      db.delete_if do | mac, entry |
        entry.aged_out?
      end
    end
  end


  ##############################################################################
  private
  ##############################################################################


  def fdb datapath_id
    @fdb ||= {}
    @fdb[ datapath_id ] ||= ForwardingDB.new
    @fdb[ datapath_id ]
  end


  def flow_mod message, port_no
    send_flow_mod_add(
      message.datapath_id,
      :match => Match.from( message ),
      :buffer_id => message.buffer_id,
      :actions => Trema::ActionOutput.new( port_no )
    )
  end


  def packet_out message, port_no
    return if message.buffered?
    send_packet_out(
      message.datapath_id,
      message.buffer_id,
      message.in_port,
      Trema::ActionOutput.new( port_no ),
      message.data
    )
  end


  def flood message
    send_packet_out(
      message.datapath_id,
      message.buffer_id,
      message.in_port,
      Trema::ActionOutput.new( OFPP_FLOOD ),
      message.buffered? ? nil : message.data
    )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
