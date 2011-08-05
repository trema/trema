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
require "trema/ip"


describe Trema::ActionSetNwDst do
  context "when an instance is created" do
    it "should have its nw_dst attribute specified as an Trema::IP object" do
      action_set_nw_dst = Trema::ActionSetNwDst.new( IP.new( "192.168.1.1" ) )
      action_set_nw_dst.nw_dst.should be_an_instance_of Trema::IP 
    end
  end
  
  
  it "should raise an argument error if its nw_dst attribute is not specified" do
    expect {
      action_set_nw_dst = Trema::ActionSetNwDst.new( )
    }.to raise_error ArgumentError
  end
  
  
  it "should raise an error if its nw_dst attribute is not an IP object" do
    expect {
      action_set_nw_dst = Trema::ActionSetNwDst.new( 1234 )
    }.to raise_error ArgumentError, /nw dst address should be an IP object/
  end
  
  
  it "should respond to #to_s and return a string" do
    action_set_nw_dst = Trema::ActionSetNwDst.new( IP.new( "192.168.1.1" ) )
    action_set_nw_dst.should respond_to :to_s 
    action_set_nw_dst.to_s.should == "#<Trema::ActionSetNwDst> nw_dst = 192.168.1.1"
  end 
  
  
  it "should respond to #to_i and return an Integer" do
    action_set_nw_dst = Trema::ActionSetNwDst.new( IP.new( "192.168.1.1" ) )
    action_set_nw_dst.should respond_to :to_i
    action_set_nw_dst.to_i.should == 3232235777
  end
  
  
  it "should append its nw_dst attribute to a list of actions" do
    action_set_nw_dst = Trema::ActionSetNwDst.new( Trema::IP.new( "192.168.1.1" ) )
    openflow_actions = double( )
    action_set_nw_dst.should_receive( :append ).with( openflow_actions )
    action_set_nw_dst.append( openflow_actions )
  end
  
  
  context "when sending #flow_mod(add) with action set to nw dst" do
    it "should have a flow with action set to mod_nw_dst" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => ActionSetNwDst.new( IP.new( "192.168.1.1" ) ) )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /mod_nw_dst:192.168.1.1/ ) 
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
