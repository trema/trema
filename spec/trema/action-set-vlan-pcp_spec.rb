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


describe Trema::ActionSetVlanPcp do
  context "when an instance is created" do
    it "should have a valid VLAN priority attribute" do
      action_set_vlan_pcp = Trema::ActionSetVlanPcp.new( 7 )
      action_set_vlan_pcp.vlan_pcp.should == 7
    end
  end
  
  it "should respond to #to_s and return a string" do
    action_set_vlan_pcp = Trema::ActionSetVlanPcp.new( 7 )
    action_set_vlan_pcp.should respond_to :to_s 
    action_set_vlan_pcp.to_s.should == "#<Trema::ActionSetVlanPcp> vlan_pcp = 7"
  end 
  
  
  it "should append its VLAN priority attribute to a list of actions" do
    action_set_vlan_pcp = Trema::ActionSetVlanPcp.new( 7 )
    openflow_actions = double( )
    action_set_vlan_pcp.should_receive( :append ).with( openflow_actions )
    action_set_vlan_pcp.append( openflow_actions )
  end
  
  
  context "when sending #flow_mod(add) message with action set to VLAN priority" do
    it "should have a flow with action set to mod_vlan_pcp" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => ActionSetVlanPcp.new( 7 ) )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /mod_vlan_pcp:7/ ) 
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
