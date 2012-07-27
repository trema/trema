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


describe ActionEnqueue, ".new" do
  it { expect { subject }.to raise_error( ArgumentError ) }
end


describe ActionEnqueue, ".new( :port => 1 )" do
  it { expect { ActionEnqueue.new( :port => 1 ) }.to raise_error( ArgumentError ) }
end


describe ActionEnqueue, ".new( :queue_id => 1 )" do
  it { expect { ActionEnqueue.new( :queue_id => 1 ) }.to raise_error( ArgumentError ) }
end


describe ActionEnqueue, ".new( :port => number, :queue_id => 1 )" do
  subject { ActionEnqueue.new :port => port, :queue_id => 1 }
  it_validates "option range", :port, 0..( 2 ** 16 - 1 )

  context "when :port == 1" do
    let( :port ) { 1 }
    its( :port ) { should == 1 }
  end
end


describe ActionEnqueue, ".new( :port => 1, :queue_id => number )" do
  subject { ActionEnqueue.new :port => 1, :queue_id => queue_id }
  it_validates "option range", :queue_id, 0..( 2 ** 32 - 1 )

  context "when :queue_id == 256" do
    let( :queue_id ) { 256 }
    its( :queue_id ) { should == 256 }
  end
end


describe ActionEnqueue, ".new( VALID OPTIONS )" do
  context "when sending #flow_mod(add) with action set to enqueue" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionEnqueue.new( :port => 1, :queue_id => 123 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
      }
    end


    it "should have a flow with action set to enqueue" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionEnqueue.new( :port => 1, :queue_id => 123 ) )
        sleep 2 # FIXME: wait to send_flow_mod
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /enqueue:1q123/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
