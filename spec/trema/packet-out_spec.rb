#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema"


class PacketOutController < Controller 
  def packet_in datapath_id, message
    send_flow_mod_add(
      datapath_id,
      :match => Match.from( message ),
      :actions => Trema::ActionOutput.new( 2 )
    )
    send_packet_out(
      datapath_id,
      :packet_in => message,
      :actions => Trema::ActionOutput.new( 2 )
    )
  end
end


describe "packet out" do
  context "a controller instance" do
    it "should respond to #send_packet_out" do
      PacketOutController.new.should respond_to(:send_packet_out)
    end
  end
    
  
  context "when called with no datapath_id" do
    it "should raise an error" do
      lambda do
        PacketOutController.new.send_packet_out
      end.should raise_error("wrong number of arguments (0 for 1)")
    end
  end
  
  
  context "when #packet_in" do
    it "should #packet_out" do
      network {
        vswitch( "packet-out" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "packet-out"
        link "host2", "packet-out"
      }.run( PacketOutController ) {
        send_packets "host2", "host1"
        sleep 2
        host( "host1" ).rx_stats.n_pkts.should == 1
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
