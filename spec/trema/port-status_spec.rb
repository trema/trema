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


describe Trema::PortStatus do
  class PortStatusController < Controller
    def features_reply message
      ports = message.ports.select{ |each| each.config == 0 }.sort
      if ports.length > 0 
        port_mod = PortMod.new( ports[0].number,
          ports[0].hw_addr, 
          1, #config port down
          1, #mask
          0)
        send_message message.datapath_id, port_mod
      end
    end
  end
  
  
  context "when #port_mod is sent" do
    it "should receive a #port_status" do
      network {
        vswitch("port-status") { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "port-status"
        link "host2", "port-status"
      }.run( PortStatusController ) {
        controller( "PortStatusController" ).send_message( 0xabc, FeaturesRequest.new )
        controller( "PortStatusController" ).should_receive( :port_status )
        sleep 1 # FIXME: wait to receive port_status
      }
    end
  end
  
  
  context "when #port_mod(port-down) is sent" do
    it "should receive #port_status with valid attributes"  do
      network {
        vswitch( "port-status" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "port-status"
        link "host2", "port-status"
      }.run( PortStatusController ) {
        controller( "PortStatusController" ).send_message( 0xabc, FeaturesRequest.new )
        controller( "PortStatusController" ).should_receive( :port_status ) do | message |
          message.datapath_id.should == 0xabc
          message.phy_port.number.should == 1
          message.phy_port.config.should == 1
          message.reason.should == 2
        end
        sleep 1 # FIXME: wait to receive port_status
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
