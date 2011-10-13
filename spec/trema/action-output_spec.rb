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


describe ActionOutput do
  describe ActionOutput, ".new( port, max_len )" do
    subject { ActionOutput.new 1, 256  }
    its( :port ) { should == 1 }
    its( :max_len ) { should == 256 }
  end


  describe ActionOutput, ".new( port )" do
    subject { ActionOutput.new( 1 ) }
    its( :port ) { should  == 1 }
    its( :max_len ) { should == 65535 }
    it "should print its attributes" do
      subject.inspect.should == "#<Trema::ActionOutput port=1,max_len=65535>"
    end
  end


  describe ActionOutput, ".new" do
    it_should_behave_like "any incorrect signature constructor"
  end


  context "when an action output is set to #flow_mod(add) " do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionOutput.new( 1 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have its action set to output:1" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc,
          :actions => ActionOutput.new( 1 ) )
        sleep 2 # FIXME: wait to send_flow_mod_add
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /output:1/ )
      }
    end
  end
  
  
  context "when multiple output actions assigned to #flow_mod(add)" do
    it "should have its actions set to output:1\/output:2" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => [ ActionOutput.new( 1 ), ActionOutput.new( 2 ) ] )
        sleep 2 # FIXME: wait to send_flow_mod_add
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
