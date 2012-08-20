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


describe SetTransportSrcPort, "new( number )" do
  subject { SetTransportSrcPort.new number }

  context "when number == 5555" do
    let( :number ) { 5555 }
    its( :port_number ) { should == 5555 }
  end

  it_validates "option range", :number, 0..( 2 ** 16 - 1 )
end


describe SetTransportSrcPort, ".new( string )" do
  it { expect { SetTransportSrcPort.new( "5555" ) }.to raise_error( TypeError ) }
end


describe SetTransportSrcPort, ".new( array )" do
  it { expect { SetTransportSrcPort.new( [ 1, 2, 3 ] ) }.to raise_error( TypeError ) }
end


describe SetTransportSrcPort, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_tp_src" do
    it "should have a flow with action set to mod_tp_src" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => SetTransportSrcPort.new( 5555 ) )
	sleep 2 # FIXME: wait to send_flow_mod_add
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_tp_src:5555/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
