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


describe SendOutPort, :type => "actions" do
  context "#new( 1 )" do
    subject { SendOutPort.new( 1 ) }

    its( :port_number ) { should == 1 }
    its( :max_len ) { should == 2 ** 16 - 1 }
    its( :to_s ) { should == "SendOutPort: port=1, max_len=65535" }
  end

  context "#new( :port_number => number )" do
    subject { SendOutPort.new( :port_number => number ) }

    context "with port number (1)" do
      let( :number ) { 1 }
      its( :port_number ) { should == 1 }
      its( :to_s ) { should == "SendOutPort: port=1, max_len=65535" }
    end

    it_validates "option range", :number, 0..( 2 ** 16 - 1 )
  end

  context "#new( :port_number => 1, :max_len => number )" do
    subject { SendOutPort.new( :port_number => 1, :max_len => max_len ) }

    context "with :max_len == 256" do
      let( :max_len ) { 256 }
      its( :max_len ) { should == 256 }
      its( :to_s ) { should == "SendOutPort: port=1, max_len=256" }
    end

    it_validates "option range", :max_len, 0..( 2 ** 16 - 1 )
  end

  context "when sending a Flow Mod with SendOutPort action" do
    it "should insert a new flow entry with action (output:1)" do
      class TestController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( TestController ) {
        controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => SendOutPort.new( 1 ) )
        sleep 2
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[ 0 ].actions.should == "output:1"
      }
    end
  end

  context "when sending a Flow Mod with multiple SendOutPort actions" do
    it "should insert a new flow entry with actions (output:1\/output:2)" do
      class TestController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( TestController ) {
        controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => [ SendOutPort.new( 1 ), SendOutPort.new( 2 ) ] )
        sleep 2
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[ 0 ].actions.should match( /output:1\/output:2/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
