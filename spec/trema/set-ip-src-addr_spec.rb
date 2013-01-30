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


describe SetIpSrcAddr, ".new(ip_address)", :type => "actions" do
  subject { SetIpSrcAddr.new( ip_address ) }

  context %{with ip_address ("192.168.1.1")} do
    let( :ip_address ) { "192.168.1.1" }
    its( "ip_address.to_s" ) { should == "192.168.1.1" }
  end

  context %{with invalid ip_address ("192.168.1")} do
    let( :ip_address ) { "192.168.1" }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end

  context "with invalid ip_address ([1, 2, 3])" do
    let( :ip_address ) { [ 1, 2, 3 ] }
    it { expect { subject }.to raise_error( TypeError ) }
  end

  context "when sending a Flow Mod with SetIpSrcAddr" do
    let( :ip_address ) { "192.168.1.1" }

    it "should insert a flow entry with action (mod_nw_src:192.168.1.1)" do
      class TestController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( TestController ) {
        controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => subject )
        sleep 2
        expect( vswitch( "0xabc" ) ).to have( 1 ).flows
        expect( vswitch( "0xabc" ).flows[ 0 ].actions ).to eq( "mod_nw_src:192.168.1.1" )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
