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


describe ActionSetNwDst, ".new(value)" do
  subject { ActionSetNwDst.new( value ) }

  context %{when "192.168.1.1"} do
    let( :value ) { "192.168.1.1" }
    its( :value ) { should == IPAddr.new( "192.168.1.1" ) }
  end
end


describe ActionSetNwDst, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_nw_dst" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionSetNwDst.new( "192.168.1.1" )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_nw_dst" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionSetNwDst.new( "192.168.1.1" ) )
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_nw_dst:192.168.1.1/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
