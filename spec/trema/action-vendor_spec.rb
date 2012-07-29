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


describe ActionVendor, "new( vendor_id )" do
  subject { ActionVendor.new vendor_id }

  it_validates "option range", :vendor_id, 0..( 2 ** 32 - 1 )

  context "when vendor_id == 0x00004cff" do
    let( :vendor_id ) { 0x00004cff }
    its( :vendor_id ) { should == 0x00004cff }

    context "when sending Flow Mod Add with action set to ActionVendor" do
      it "should have a flow with action set to ActionVendor" do
        class FlowModAddController < Controller; end
        pending "ActionVendor not yet implemented"
        network {
          vswitch { datapath_id 0xabc }
        }.run( FlowModAddController ) {
          controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionVendor.new( 0x00004cff, "deadbeef".unpack( "C*" ) ) )
          vswitch( "0xabc" ).should have( 1 ).flows
          vswitch( "0xabc" ).flows[ 0 ].actions.should match( /mod_vendor/ )
        }
      end
    end
  end
end


describe ActionVendor, ".new( string )" do
  it { expect { ActionVendor.new( "0x00004cff" ) }.to raise_error( TypeError ) }
end


describe ActionVendor, ".new( array )" do
  it { expect { ActionVendor.new( [ 1, 2, 3 ] ) }.to raise_error( TypeError ) }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
