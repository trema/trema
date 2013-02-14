#
# Copyright (C) 2008-2013 NEC Corporation
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

describe Hop, "test" do
  before do
    datapath_id = 0x1;
    in_port = 1;
    out_port = 2;
    @hop = Hop.new(datapath_id, in_port, out_port)

    datapath_id2 = 0x2;
    in_port2 = 2;
    out_port2 = 1;
    Array actions2 = Array.new([SendOutPort.new(:port_number => 1, :max_len => 256), 
                      SetEthSrcAddr.new("11:22:33:44:55:66"),
                      SetEthDstAddr.new("11:22:33:44:55:66"),
                      SetIpSrcAddr.new("192.168.1.1"),
                      SetIpDstAddr.new("192.168.1.1"),
                      SetIpTos.new(32),
                      SetTransportSrcPort.new( 5555 ),
                      SetTransportDstPort.new( 5555 ),
                      ActionSetVlanVid.new( 1 ),
                      SetVlanPriority.new( 7 ),
                      StripVlanHeader.new,
                      VendorAction.new( 0x00004cff, Array["test", "test2"] )])
    @hop2 = Hop.new(datapath_id2, in_port2, out_port2, actions2 )

    datapath_id3 = 0x3;
    in_port3 = 2;
    out_port3 = 1;
    action = VendorAction.new( 0x00004cff)
    @hop3 = Hop.new(datapath_id3, in_port3, out_port3, action)
  end

  it "new with no argument" do
    expect {Hop.new()}.to raise_error()
  end 

  it "new with 3 arguments" do
    tempHop = Hop.new(0x1, 1, 2)
    expect( tempHop.instance_of?(Hop) ).to be_true
  end

  it "new with 4 arugments with single actions" do
      action = SendOutPort.new(:port_number => 1, :max_len => 256)
      tempHop = Hop.new(0x1, 1, 2, action)
      expect( tempHop.instance_of?(Hop) ).to be_true
  end

  it "new with 4 arugments with multiple actions" do
      Array actions = Array.new([SendOutPort.new(:port_number => 1, :max_len => 256), 
                SetEthSrcAddr.new("11:22:33:44:55:66"),
                SetEthDstAddr.new("11:22:33:44:55:66"),
                SetIpSrcAddr.new("192.168.1.1"),
                SetIpDstAddr.new("192.168.1.1"),
                SetIpTos.new(32),
                SetTransportSrcPort.new( 5555 ),
                SetTransportDstPort.new( 5555 ),
                ActionSetVlanVid.new( 1 ),
                SetVlanPriority.new( 7 ),
                StripVlanHeader.new,
                VendorAction.new( 0x00004cff, Array["test", "test2"] )])
      tempHop = Hop.new(0x1, 1, 2, actions)
      expect( tempHop.instance_of?(Hop) ).to be_true
  end

  it "new with 4 arguments with strange action" do
      action = Match.new()
      expect {Hop.new(0x1, 1, 2, action)}.to raise_error() 
  end

  it "new with 4 arguments with a bad action array" do
        datapath_id = 0x4;
        in_port = 2;
        out_port = 1;
        actions = Array.new([SendOutPort.new(:port_number => 1, :max_len => 256),
                              VendorAction.new( 0x00004cff),
                             Match.new() ] )
        expect {Hop.new(datapath_id, in_port, out_port, actions)}.to raise_error("actions argument must be an Array of Action objects")
  end

  it "new with 4 arguments with a differenct object" do
        datapath_id = 0x4;
        in_port = 2;
        out_port = 1;
        actions = Match.new()
        expect {Hop.new(datapath_id, in_port, out_port, actions)}.to raise_error("actions argument must be an Array or an Action object")
  end


  it "new with 3 arguments with zeros" do
        hop = Hop.new(0x0, 0, 0)

        expect( hop.in_port ).to be( 0 )
         expect( hop.datapath_id ).to be( 0 )
        expect( hop.out_port ).to be( 0 )
  end

  it "new with 3 arguments with big value" do
        expect {hop = Hop.new(1000000000000000000000000000000000000000000000000000, 1, 1)}.to raise_error()
  end

  it "new with 3 arguments with minus datapath_id" do
        expect {hop = Hop.new(-1, 1, 1)}.to raise_error()
  end

  it "new with 3 arguments with minus in_port value" do
        expect {hop = Hop.new(1, -1, 1)}.to raise_error()
  end

  it "new with 3 arguments with minus out_port value" do
        expect {hop = Hop.new(1, 1, -1)}.to raise_error()
  end

  it ".datapath_id" do
      expect( @hop.datapath_id ).to be( 0x1 )
      expect( @hop2.datapath_id ).to be( 0x2 )
  end
  
  it ".in_port" do
      expect( @hop.in_port ).to be( 1 )
      expect( @hop2.in_port ).to be( 2 )
  end
  
  it ".out_port" do
      expect( @hop.out_port ).to be( 2 )
      expect( @hop2.out_port ).to be( 1 )
  end
  
  it ".actions" do
    	Array actions = @hop.actions
      p actions
      expect( actions ).to be_false

      Array actions2 = @hop2.actions
      p actions2

      expect( actions2[0].instance_of?(SendOutPort) ).to be_true
      expect( actions2[0].port_number ).to eq( 1 )
      expect( actions2[0].max_len ).to eq( 256 )
      expect( actions2[1].instance_of?(SetEthSrcAddr) ).to be_true
      expect( actions2[1].mac_address.to_s ).to eq( "11:22:33:44:55:66" )
      expect( actions2[2].instance_of?(SetEthDstAddr) ).to be_true
      expect( actions2[2].mac_address.to_s ).to eq( "11:22:33:44:55:66" )
      expect( actions2[3].instance_of?(SetIpSrcAddr) ).to be_true
      expect( actions2[3].ip_address.to_s ).to eq( "192.168.1.1" )
      expect( actions2[4].instance_of?(SetIpDstAddr) ).to be_true
      expect( actions2[4].ip_address.to_s ).to eq( "192.168.1.1" )
      expect( actions2[5].type_of_service ).to eq( 32 )
      expect( actions2[6].instance_of?(SetTransportSrcPort) ).to be_true
      expect( actions2[6].port_number ).to eq( 5555 )
      expect( actions2[7].instance_of?(SetTransportDstPort) ).to be_true
      expect( actions2[7].port_number ).to eq( 5555 )
      expect( actions2[8].instance_of?(ActionSetVlanVid) ).to be_true
      expect( actions2[8].vlan_id ).to eq( 1 )
      expect( actions2[9].instance_of?(SetVlanPriority) ).to be_true
      expect( actions2[9].vlan_priority ).to eq( 7 )
      expect( actions2[10].instance_of?(StripVlanHeader) ).to be_true
      expect( actions2[11].instance_of?(VendorAction) ).to be_true
      expect( actions2[11].vendor_id ).to eq( 0x00004cff )

      Array body = actions2[11].body
      expect( body[0] ).to eq( "test" )
      expect( body[1] ).to eq( "test2" )

      action = @hop3.actions
      expect( action.instance_of?(VendorAction) ).to be_true
  end
	
  after do
  	@hop = nil
    @hop2 = nil
    @hop3 = nil
  end
end
