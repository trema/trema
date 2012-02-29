#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


describe Trema::PortStatus, ".new( VALID OPTIONS )" do
  class PortStatusController < Controller
    def features_reply message
      ports = message.ports.select{ |each| each.config == 0 }.sort
      if ports.length > 0 
        port_mod = PortMod.new( 
          :port_no => ports[0].number,
          :hw_addr => ports[0].hw_addr, 
          :config => 1, #config port down
          :mask => 1, #mask
          :advertise => 0
        )
        send_message message.datapath_id, port_mod
      end
    end
  end


  it "should have datapath_id" do
    PortStatus.new( :datapath_id => 0xabc ).datapath_id.should == 0xabc
  end


  it "should have transaction_id" do
    PortStatus.new( :transaction_id => 123 ).transaction_id.should == 123
  end


  it "should have reason" do
    PortStatus.new( :reason => 2 ).reason.should == 2
  end


  it "should have phy_port" do
    port = mock( "port" )
    PortStatus.new( :phy_port => port ).phy_port.should == port
  end


  context "when #port_mod is sent" do
    it "should #port_status" do
      network {
        vswitch("port-status") { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "port-status"
        link "host2", "port-status"
      }.run( PortStatusController ) {
        controller( "PortStatusController" ).should_receive( :port_status )
        controller( "PortStatusController" ).send_message( 0xabc, FeaturesRequest.new )
        sleep 2 # FIXME: wait to receive port_status
      }
    end
  end


  context "when #port_mod(port#1,down) is sent" do
    it "should #port_status(port#1,down)"  do
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
