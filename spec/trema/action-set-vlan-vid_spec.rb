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


describe ActionSetVlanVid, ".new( VALID OPTION )" do
  subject { ActionSetVlanVid.new( :vlan_vid => 1024 ) }
  its( :vlan_vid ) { should == 1024 }
  it "should inspect its attributes" do
    subject.inspect.should == "#<Trema::ActionSetVlanVid vlan_vid=1024>"
  end
end


describe ActionSetVlanVid, ".new( MANDATORY OPTION MISSING )" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError )
  end
end


describe ActionSetVlanVid, ".new( INVALID OPTION )" do
  context "when value outside allowed range" do
    subject { ActionSetVlanVid.new( :vlan_vid => 0 ) }
    it "should raise RangeError" do
      expect { subject }.to raise_error( RangeError )
    end
  end


  context "when argument type Array instead of Hash" do
    subject { ActionSetVlanVid.new( [ 1024 ] ) }
    it "should raise TypeError" do
      expect { subject }.to raise_error( TypeError )
    end
  end
end


describe ActionSetVlanVid, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_vlan_vid" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionSetVlanVid.new( :vlan_vid => 1024 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_vlan_vid" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionSetVlanVid.new( :vlan_vid => 1024 ) )
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_vlan_vid:1024/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
