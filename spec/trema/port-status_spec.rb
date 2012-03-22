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


describe Trema::PortStatus do
  context "when created" do
    subject do
      Trema::PortStatus.new(
        :datapath_id => datapath_id,
        :transaction_id => transaction_id,
        :reason => reason,
        :phy_port => phy_port
      )
    end

    let( :datapath_id ) { 0xabc }
    let( :transaction_id ) { 123 }
    let( :reason ) { 2 }
    let( :phy_port ) { "PHY_PORT" }


    context "with datapath_id (0xabc)" do
      its( :datapath_id ) { should == 0xabc }
    end

    context "without datapath_id" do
      let( :datapath_id ) { nil }
      it { expect { subject }.to raise_error( ArgumentError, ":datapath_id is a mandatory option" ) }
    end


    context "with transaction_id" do
      its( :transaction_id ) { should == 123 }
    end

    context "without transaction_id" do
      let( :transaction_id ) { nil }
      it { expect { subject }.to raise_error( ArgumentError, ":transaction_id is a mandatory option" ) }
    end


    context "with reason" do
      its( :reason ) { should == 2 }
    end

    context "without reason" do
      let( :reason ) { nil }
      it { expect { subject }.to raise_error( ArgumentError, ":reason is a mandatory option" ) }
    end


    context "with phy_port" do
      let( :phy_port ) { "PHY_PORT" }
      its( :phy_port ) { should == "PHY_PORT" }
    end

    context "without phy_port" do
      let( :phy_port ) { nil }
      it { expect { subject }.to raise_error( ArgumentError, ":phy_port is a mandatory option" ) }
    end
  end
end


class PortStatusController < Controller
  def features_reply message
    ports = message.ports.select do | each |
      each.config == 0
    end.sort

    if ports.length > 0
      port_mod = PortMod.new(
        :port_no => ports[ 0 ].number,
        :hw_addr => ports[ 0 ].hw_addr,
        :config => 1, #config port down
        :mask => 1, #mask
        :advertise => 0
      )
      send_message message.datapath_id, port_mod
    end
  end
end


describe PortStatusController do
  context "when port_mod is sent to switch" do
    it "should receive #port_status" do
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


  context "when port_mod(port#1, down) is sent switch" do
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
