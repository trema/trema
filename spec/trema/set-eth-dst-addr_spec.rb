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


describe SetEthDstAddr, %{.new( "52:54:00:a8:ad:8c" )} do
  subject { SetEthDstAddr.new( "52:54:00:a8:ad:8c" ) }
  its( :mac_address ) { should == Mac.new( "52:54:00:a8:ad:8c" ) }
end


describe SetEthDstAddr, %{.new( "INVALID MAC STRING" )} do
  it { expect { SetEthDstAddr.new( "INVALID MAC STRING" ) }.to raise_error( ArgumentError ) }
end


describe SetEthDstAddr, ".new( number )" do
  subject { SetEthDstAddr.new( mac_address ) }

  context "when mac_address == 0x525400a8ad8c" do
    let( :mac_address ) { 0x525400a8ad8c }
    its( :mac_address ) { should == Mac.new( "52:54:00:a8:ad:8c" ) }
  end

  it_validates "option range", :mac_address, 0..0xffffffffffff
end


describe SetEthDstAddr, ".new( [ 1, 2, 3 ] )" do
  it { expect { SetEthDstAddr.new( [ 1, 2, 3 ] ) }.to raise_error( TypeError ) }
end


describe SetEthDstAddr, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_dl_dst" do
    it "should have a flow with action set to mod_dl_dst" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add(
          0xabc,
          :actions => SetEthDstAddr.new( "52:54:00:a8:ad:8c" )
        )
        sleep 2 # FIXME: wait to send_flow_mod
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_dl_dst:52:54:00:a8:ad:8c/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
