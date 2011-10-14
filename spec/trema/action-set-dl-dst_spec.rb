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


describe ActionSetDlDst do
  describe ActionSetDlDst, ".new( mac )" do
    subject { ActionSetDlDst.new( Mac.new( "52:54:00:a8:ad:8c" ) ) }
    its( :dl_dst ) { should be_an_instance_of( Mac ) }
    it "should print its attributes" do
      subject.inspect.should == "#<Trema::ActionSetDlDst dl_dst=52:54:00:a8:ad:8c>"
    end
  end


  describe ActionSetDlDst, ".new" do
    it_should_behave_like "any incorrect signature constructor"
  end


  describe ActionSetDlDst, ".new( mac ) - mac not a Trema::Mac object" do
    subject { ActionSetDlDst.new 1234 }
    it "should raise" do
      expect { subject }.to raise_error( ArgumentError, /dl dst address should be a Mac object/ )
    end
  end


  context "when sending #flow_mod(add) with action set to mod_dl_dst" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionSetDlDst.new( Mac.new( "52:54:00:a8:ad:8c" ) )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_dl_dst" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions =>  ActionSetDlDst.new( Mac.new( "52:54:00:a8:ad:8c" ) ) )
        sleep 2 # FIXME: wait to send_flow_mod
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /mod_dl_dst:52:54:00:a8:ad:8c/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
