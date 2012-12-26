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


describe VendorAction, "new( vendor_id )" do
  subject { VendorAction.new vendor_id }

  it_validates "option range", :vendor_id, 0..( 2 ** 32 - 1 )

  context "when vendor_id == 0x00004cff" do
    let( :vendor_id ) { 0x00004cff }
    its( :vendor_id ) { should == 0x00004cff }
  end
end


describe VendorAction, ".new( string )" do
  it { expect { VendorAction.new "0x00004cff" }.to raise_error( TypeError ) }
end


describe VendorAction, ".new( array )" do
  it { expect { VendorAction.new [ 1, 2, 3 ] }.to raise_error( TypeError ) }
end


describe VendorAction, ".new( vendor_id, body )" do
  subject { VendorAction.new vendor_id, body }
  let( :vendor_id ) { 0x00002320 }

  context %{when body == [ 0x00, 0x08, 0x54, 0x72, 0x65, 0x6d, 0x61, 0x00 ]} do
    let( :body ) { [ 0x00, 0x08, 0x54, 0x72, 0x65, 0x6d, 0x61, 0x00 ] }

    its( :vendor_id ) { should == 0x00002320 }
    its( :body ) { should == [ 0, 8, 84, 114, 101, 109, 97, 0 ] }

    context "when sending Flow Mod Add with action set to VendorAction" do
      it "should have a flow with action set to VendorAction" do
        class FlowModAddController < Controller; end
        network {
          vswitch { datapath_id 0xabc }
        }.run( FlowModAddController ) {
	  body = [ 0x00, 0x08, 0x54, 0x72, 0x65, 0x6d, 0x61, 0x00 ]
          controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => VendorAction.new( 0x00002320, body ) )
	  sleep 2 # FIXME: wait to send_flow_mod_add
          vswitch( "0xabc" ).should have( 1 ).flows
          vswitch( "0xabc" ).flows[ 0 ].actions.should match( /note:54.72.65.6d.61.00/ )
        }
      end
    end
  end
end


describe VendorAction, "new( vendor_id, string )" do
  it { expect { VendorAction.new 0x00004cff, "deadbeef" }.to raise_error( TypeError ) }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
