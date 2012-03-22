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
  subject { Trema::PortStatus }
  its( :constants ) { should include "OFPPR_ADD" }
  its( :constants ) { should include "OFPPR_DELETE" }
  its( :constants ) { should include "OFPPR_MODIFY" }


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
    let( :reason ) { Trema::PortStatus::OFPPR_ADD }
    let( :phy_port ) { "PHY_PORT" }


    context "with datapath_id (0xabc)" do
      its( :datapath_id ) { should == 0xabc }
      its( :dpid ) { should == 0xabc }
    end

    context "without datapath_id" do
      let( :datapath_id ) { nil }
      it { expect { subject }.to raise_error( ArgumentError, ":datapath_id is a mandatory option" ) }
    end


    context "with transaction_id" do
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
    end

    context "without transaction_id" do
      let( :transaction_id ) { nil }
      it { expect { subject }.to raise_error( ArgumentError, ":transaction_id is a mandatory option" ) }
    end


    context "with reason" do
      its( :reason ) { should == Trema::PortStatus::OFPPR_ADD }
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


module Trema
  class PortStatusController < Controller
    def features_reply dpid, message
      message.ports.select( &:up? ).each do | each |
        port_mod = PortMod.new(
          :port_no => each.number,
          :hw_addr => each.hw_addr,
          :config => Port::OFPPC_PORT_DOWN,
          :mask => Port::OFPPC_PORT_DOWN,
          :advertise => 0
        )
        send_message dpid, port_mod
      end
    end
  end


  describe Controller do
    context "when one port goes down" do
      it "should receive port_status" do
        network {
          vswitch { datapath_id 0xabc }
          vhost "host"
          link "host", "0xabc"
        }.run( PortStatusController ) {
          controller( "PortStatusController" ).should_receive( :port_status ).with do | dpid, message |
            dpid.should == 0xabc
            message.reason.should == PortStatus::OFPPR_MODIFY
          end

          controller( "PortStatusController" ).send_message 0xabc, FeaturesRequest.new
          sleep 2  # FIXME: wait to receive port_status
        }
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
