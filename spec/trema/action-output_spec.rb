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


describe ActionOutput, ".new" do
  it { expect { subject }.to raise_error( ArgumentError ) }
end


describe ActionOutput, ".new( 1 )" do
  subject { ActionOutput.new( 1 ) }
  its( :port ) { should == 1 }
  its( :max_len ) { should == 2 ** 16 - 1 }
end


describe ActionOutput, ".new( :port => number )" do
  subject { ActionOutput.new :port => port }
  it_validates "option range", :port, 0..( 2 ** 16 - 1 )

  context "when :port == 1" do
    let( :port ) { 1 }
    its( :port ) { should == 1 }
  end
end


describe ActionOutput, ".new( :port => 1, :max_len => number )" do
  subject { ActionOutput.new :port => 1, :max_len => max_len }
  it_validates "option range", :max_len, 0..( 2 ** 16 - 1 )

  context "when :max_len == 256" do
    let( :max_len ) { 256 }
    its( :max_len ) { should == 256 }
  end
end


describe ActionOutput, ".new( VALID OPTIONS )" do
  context "when an action output is set to #flow_mod(add) " do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionOutput.new(:port => 1 )
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
          :actions => ActionOutput.new( :port => 1 ) )
        sleep 2 # FIXME: wait to send_flow_mod_add
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /output:1/ )
      }
    end
  end


  context "when multiple output actions assigned to #flow_mod(add)" do
    it "should have its actions set to output:1\/output:2" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => [ ActionOutput.new( :port => 1 ), ActionOutput.new( :port => 2 ) ] )
        sleep 2 # FIXME: wait to send_flow_mod_add
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /output:1\/output:2/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
