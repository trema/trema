#
# Copyright (C) 2008-2013 NEC Corporation
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


describe VendorAction, "new(vendor_id)", :type => "actions" do
  subject { VendorAction.new vendor_id }

  context "with vendor_id (0x00004cff)" do
    let( :vendor_id ) { 0x00004cff }
    its( :vendor_id ) { should == 0x00004cff }
  end

  it_validates "option is within range", :vendor_id, 0..( 2 ** 32 - 1 )

  context %{with vendor_id ("0x00004cff")} do
    let( :vendor_id ) { "0x00004cff" }
    it { expect { subject }.to raise_error( TypeError ) }
  end
end


describe VendorAction, ".new(0x00002320, body)", :type => "actions" do
  subject { VendorAction.new 0x00002320, body }

  context %{with body ("deadbeef")} do
    let( :body ) { "deadbeef" }
    it { expect { subject }.to raise_error( TypeError ) }
  end

  context "with body 9 octets long" do
    let( :body ) { [ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 ] }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end

  context "when sending a Flow Mod with VendorAction" do
    let( :body ) { [ 0x00, 0x08, 0x54, 0x72, 0x65, 0x6d, 0x61, 0x00 ] }

    it "should insert a new flow entry with action (note:54.72.65.6d.61.00)" do
      class TestController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( TestController ) {
        controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => subject )
        sleep 2
        expect( vswitch( "0xabc" ) ).to have( 1 ).flows
        expect( vswitch( "0xabc" ).flows[ 0 ].actions ).to eq( "note:54.72.65.6d.61.00" )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
