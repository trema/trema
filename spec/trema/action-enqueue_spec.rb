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


describe ActionEnqueue do
  describe ActionEnqueue, ".new( port, queue_id )" do
    subject { ActionEnqueue.new 1, 123  }
    its ( :port ) { should == 1 }
    its ( :queue_id ) { should == 123 }
    it "should inspect its attributes" do
      subject.inspect.should == "#<Trema::ActionEnqueue port=1,queue_id=123>"
    end
  end


  describe ActionEnqueue, ".new( port )" do
    subject { ActionEnqueue.new 1 }
    it_should_behave_like "any incorrect signature constructor"
  end 


  describe ActionEnqueue, ".new" do
    it_should_behave_like "any incorrect signature constructor"
  end


  context "when sending #flow_mod(add) with action set to enqueue" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionEnqueue.new( 1, 123 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
      }
    end


    it "should have a flow with action set to enqueue" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionEnqueue.new( 1, 123 ) )
        sleep 2 # FIXME: wait to send_flow_mod
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /enqueue:1q123/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
