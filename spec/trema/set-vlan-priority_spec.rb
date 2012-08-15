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


describe SetVlanPriority, ".new( number )" do
  subject { SetVlanPriority.new( vlan_priority ) }

  context "when vlan_priority == 4" do
    let( :vlan_priority ) { 4 }
    its( :vlan_priority ) { should == 4 }
  end

  it_validates "option range", :vlan_priority, 0..7
end


describe SetVlanPriority, %{.new( "4" )} do
  it { expect { SetVlanPriority.new( "4" ) }.to raise_error( TypeError ) }
end


describe SetVlanPriority, ".new( [ 4 ] )" do
  it { expect { SetVlanPriority.new( [ 4 ] ) }.to raise_error( TypeError ) }
end


describe SetVlanPriority, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_vlan_pcp" do
    it "should have a flow with action set to mod_vlan_pcp" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => SetVlanPriority.new( 7 ) )
	sleep 2 # FIXME: wait to send_flow_mod_add
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_vlan_pcp:7/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
