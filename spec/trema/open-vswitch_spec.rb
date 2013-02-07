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
require "trema/open-vswitch"


module Trema
  describe OpenflowSwitch do
    around do | example |
      begin
        OpenflowSwitch.clear
        example.run
      ensure
        OpenflowSwitch.clear
      end
    end

    it "should keep a list of vswitches" do
      OpenVswitch.new mock( "stanza 0", :name => "vswitch 0", :validate => true )
      OpenVswitch.new mock( "stanza 1", :name => "vswitch 1", :validate => true )
      OpenVswitch.new mock( "stanza 2", :name => "vswitch 2", :validate => true )

      expect( OpenflowSwitch ).to have( 3 ).vswitches
      expect( OpenflowSwitch[ "vswitch 0" ] ).not_to be_nil
      expect( OpenflowSwitch[ "vswitch 1" ] ).not_to be_nil
      expect( OpenflowSwitch[ "vswitch 2" ] ).not_to be_nil
    end
  end


  describe OpenVswitch, %[dpid = "0xabc"] do
    subject {
      stanza = { :dpid_short => "0xabc", :dpid_long => "0000000000000abc", :ip => "127.0.0.1" }
      stanza.stub!( :validate )
      stanza.stub!( :name ).and_return( name )
      OpenVswitch.new stanza
    }


    context "when its name is not set" do
      let( :name ) { "0xabc" }

      its( :name ) { should == "0xabc" }
      its( :dpid_short ) { should == "0xabc" }
      its( :dpid_long ) { should == "0000000000000abc" }
      its( :network_device ) { should == "vsw_0xabc" }
    end


    context "when its name is set" do
      let( :name ) { "Otosan Switch" }

      its( :name ) { should == "Otosan Switch" }
      its( :dpid_short ) { should == "0xabc" }
      its( :dpid_long ) { should == "0000000000000abc" }
      its( :network_device ) { should == "vsw_0xabc" }
    end


    context "when getting its flows" do
      let( :name ) { "0xabc" }

      it "should execute ofctl to get the flows" do
        ofctl = mock( "ofctl" )
        Ofctl.stub!( :new ).and_return( ofctl )

        ofctl.should_receive( :users_flows ).with( subject ).once

        subject.flows
      end
    end


    context "when running it" do
      let( :name ) { "0xabc" }

      it "should execute ovs openflowd" do
        subject.should_receive( :sh ).with do | command |
          expect( command ).to include( Executables.ovs_openflowd )
        end

        subject.run!
      end

      it "should be connected to virtual ports" do
        subject << "VirtualInterface0"
        subject << "VirtualInterface1"
        subject << "VirtualInterface2"

        subject.should_receive( :sh ).with do | command |
          expect( command ).to include( "--ports=VirtualInterface0,VirtualInterface1,VirtualInterface2" )
        end

        subject.run!
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
