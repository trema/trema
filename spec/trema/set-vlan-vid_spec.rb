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


describe SetVlanVid, ".new( number )" do
  subject { SetVlanVid.new( vlan_id ) }

  context "when vlan_id == 1024" do
    let( :vlan_id ) { 1024 }
    its( :vlan_id ) { should == 1024 }
  end

  it_validates "option range", :vlan_id, 1..4095
end


describe SetVlanVid, %{.new( "1024" )} do
  it { expect { SetVlanVid.new( "1024" ) }.to raise_error( TypeError ) }
end


describe SetVlanVid, ".new( [ 1024 ] )" do
  it { expect { SetVlanVid.new( [ 1024 ] ) }.to raise_error( TypeError ) }
end


describe SetVlanVid, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_vlan_vid" do
    it "should have a flow with action set to mod_vlan_vid" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => SetVlanVid.new( 1024 ) )
	sleep 2 # FIXME: wait to send_flow_mod_add
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_vlan_vid:1024/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
