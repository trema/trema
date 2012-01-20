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


shared_examples_for "any OpenFlow message with tp_dst option" do
  it_should_behave_like "any OpenFlow message", :option => :tp_dst, :name => "Destination TCP or UDP port", :size => 16
end


describe ActionSetTpDst, ".new( VALID OPTION )" do
  subject { ActionSetTpDst.new( :tp_dst => tp_dst ) }
  let( :tp_dst ) { 5555 }
  its( :tp_dst ) { should == 5555 }
  it "should inspect its attributes" do
    subject.inspect.should == "#<Trema::ActionSetTpDst tp_port=5555>"
  end
  it_should_behave_like "any OpenFlow message with tp_dst option"
end


describe ActionSetTpDst, ".new( MANDATORY OPTION MISSING )" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError )
  end
end


describe ActionSetTpDst, ".new( INVALID OPTION ) - argument type Array instead of Hash" do
  subject { ActionSetTpDst.new( [ 5555 ] ) }
  it "should raise TypeError" do
    expect { subject }.to raise_error( TypeError )
  end
end


describe ActionSetTpDst, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_tp_dst" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionSetTpDst.new( :tp_dst => 5555 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_tp_dst" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionSetTpDst.new( :tp_dst => 5555 ) )
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_tp_dst:5555/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
