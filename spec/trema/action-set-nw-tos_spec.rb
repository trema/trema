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


describe Trema::ActionSetNwTos do
  context "when an instance is created" do
    it "should have a valid nw_tos attribute" do
      action_set_nw_tos = Trema::ActionSetNwTos.new( 4 )
      action_set_nw_tos.nw_tos.should == 4
    end
  end
  
  
  it "should respond to #to_s and return a string" do
    action_set_nw_tos = Trema::ActionSetNwTos.new( 4 )
    action_set_nw_tos.should respond_to :to_s 
    action_set_nw_tos.to_s.should == "#<Trema::ActionSetNwTos> nw_tos = 4"
  end 
  
  
  it "should append its nw_tos attribute to a list of actions" do
    action_set_nw_tos = Trema::ActionSetNwTos.new( 5555 )
    openflow_actions = double( )
    action_set_nw_tos.should_receive( :append ).with( openflow_actions )
    action_set_nw_tos.append( openflow_actions )
  end
  
  
  context "when sending #flow_mod(add) with action set to nw_tos" do
    it "should have a flow with action set to mod_nw_tos" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => ActionSetNwTos.new( 4 ) )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /mod_nw_tos:4/ ) 
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
