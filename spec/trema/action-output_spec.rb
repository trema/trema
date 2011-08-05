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


describe Trema::ActionOutput do
  context "when an instance is created" do
    before :all do 
      @action_output = Trema::ActionOutput.new( 1 )
    end
    
    it "should have a valid port attribute" do
      @action_output.port.should == 1
    end
    
    
    it "should have a valid default max_len attribute" do
      @action_output.max_len.should == 65535
    end
  end
  
  
  context "when an instance is created with all attributes specified" do
    before :all do 
      @action_output = Trema::ActionOutput.new( 1, 256 )
    end
    
    
    it "should have a valid max_len attribute" do
      @action_output.max_len.should == 256
    end
  
    
    it "should respond to #to_s and return a string" do
      @action_output.should respond_to :to_s 
      @action_output.to_s.should == "#<Trema::ActionOutput> port = 1, max_len = 256"
    end 
  end
    
  
  it "appends its attributes to a list of actions" do
    action_output = Trema::ActionOutput.new( 1 )
    openflow_actions = double( )
    action_output.should_receive( :append ).with( openflow_actions )
    action_output.append( openflow_actions )
  end
  
  
  context "when a single ActionOutput object is assigned to #flow_mod(add) " do
    it "should have its action set to output:1" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => ActionOutput.new( 1 ) )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /output:1/ ) 
      }
    end
  end
  
  
  context "when multiple ActionOutput objects assigned to #flow_mod(add)" do
    it "should have its actions set to output:1\/output:2" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => [ ActionOutput.new( 1 ), ActionOutput.new( 2 ) ] )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /output:1\/output:2/ ) 
      }
    end
  end
end



### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
