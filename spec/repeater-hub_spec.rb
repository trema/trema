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


require File.join( File.dirname( __FILE__ ), "spec_helper" )
require "trema"


include Trema


class RepeaterHub < Controller
  def packet_in message
    send_flow_mod_add(
      message.datapath_id,
      :match => Match.from( message ),
      :buffer_id => message.buffer_id,
      :actions => ActionOutput.new( OFPP_FLOOD )
    )
    if not message.buffered?
      send_packet_out(
        message.datapath_id,
        message.buffer_id,
        message.in_port,
        ActionOutput.new( OFPP_FLOOD ),
        message.data
      )
    end
  end
end


describe RepeaterHub do
  before :each do
    trema_conf do
      vswitch("repeater_hub") {
        datapath_id "0xabc"
      }

      vhost("host1") { promisc "On" }
      vhost("host2") { promisc "On" }
      vhost("host3") { promisc "On" }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
    end
  end


  it "should send a flow_mod message" do
    trema_session( RepeaterHub ) do
      switch( "repeater_hub" ).should_receive( :flow_mod_add ).once

      send_packets "host1", "host2"
    end    
  end
  
  
  it "should add a flow entry with actions = FLOOD" do
    trema_session( RepeaterHub ) do
      send_packets "host1", "host2"

      switch( "repeater_hub" ).flows.size.should == 1
      switch( "repeater_hub" ).flows[ 0 ].actions.should == "FLOOD"
    end
  end


  it "should repeat packets to host2 and host3" do
    trema_session( RepeaterHub ) do
      send_packets "host1", "host2", :pps => 100
      
      host( "host2" ).rx_stats.n_pkts.should == 100
      host( "host3" ).rx_stats.n_pkts.should == 100
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
