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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )


describe Trema::Shell, ".vswitch" do
  around do | example |
    Trema::OpenflowSwitch.clear
    example.run
    Trema::OpenflowSwitch.clear
  end


  context "executed without a shell" do
    before { $config = nil }


    it "should raise" do
      expect {
        Trema::Shell.vswitch { dpid "0xabc" }
      }.to raise_error( "Not in Trema shell" )
    end
  end


  context "executed within a shell" do
    before {
      $config = mock( "config", :port => 6653 )
      $context = mock( "context", :dump => true )
    }
    after { Trema::OpenflowSwitch[ "0xabc" ].shutdown! if Trema::OpenflowSwitch[ "0xabc" ] }


    it "should create a new vswitch if name given" do
      Trema::Shell.vswitch { dpid "0xabc" }
      expect( Trema::OpenflowSwitch ).to have( 1 ).switch
      expect( Trema::OpenflowSwitch[ "0xabc" ].name ).to eq( "0xabc" )
      expect( Trema::OpenflowSwitch[ "0xabc" ].dpid_short ).to eq( "0xabc" )
      expect( Trema::OpenflowSwitch[ "0xabc" ].dpid_long ).to eq( "0000000000000abc" )
    end


    it "should create a new vswitch if dpid given" do
      Trema::Shell.vswitch "0xabc"

      expect( Trema::OpenflowSwitch ).to have( 1 ).switch
      expect( Trema::OpenflowSwitch[ "0xabc" ].name ).to eq( "0xabc" )
      expect( Trema::OpenflowSwitch[ "0xabc" ].dpid_short ).to eq( "0xabc" )
      expect( Trema::OpenflowSwitch[ "0xabc" ].dpid_long ).to eq( "0000000000000abc" )
    end


    it "should raise if dpid not given" do
      expect {
        Trema::Shell.vswitch
      }.to raise_error( "No dpid given" )
    end


    it "should raise if the name is invalid and block not given" do
      expect { Trema::Shell.vswitch "INVALID_DPID" }.to raise_error( "Invalid dpid: INVALID_DPID" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
