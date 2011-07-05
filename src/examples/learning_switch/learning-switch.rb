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


$LOAD_PATH << "./src/examples/learning_switch/"


require "forwarding-db"


#
# A OpenFlow controller class that emulates a layer-2 switch.
#
class LearningSwitch < Trema::Controller
  timer_event :age_db, 5


  def start
    @fdb = ForwardingDB.new
  end


  def packet_in message
    @fdb.learn message.macsa, message.in_port
    port_no = @fdb.port_no_of( message.macda )
    if port_no
      flow_mod message, port_no
      packet_out message, port_no
    else
      flood message
    end
  end


  def age_db
    @fdb.age
  end


  ##############################################################################
  private
  ##############################################################################


  def flow_mod message, port_no
    send_flow_mod_add(
      message.datapath_id,
      :match => Match.from( message ),
      :actions => Trema::ActionOutput.new( port_no )
    )
  end


  def packet_out message, port_no
    send_packet_out message, Trema::ActionOutput.new( port_no )
  end


  def flood message
    packet_out message, OFPP_FLOOD
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
