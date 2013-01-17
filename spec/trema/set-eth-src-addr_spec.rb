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


describe SetEthSrcAddr, ".new( mac_address )", :type => "actions" do
  subject { SetEthSrcAddr.new( mac_address ) }

  context "with mac_address (52:54:00:a8:ad:8c)" do
    let( :mac_address ) { "52:54:00:a8:ad:8c" }
    its( "mac_address.to_s" ) { should == "52:54:00:a8:ad:8c" }
    its( :to_s ) { should == "SetEthSrcAddr: mac_address=52:54:00:a8:ad:8c" }
  end

  context %q{with mac_address (Mac.new("52:54:00:a8:ad:8c"))} do
    let( :mac_address ) { Mac.new("52:54:00:a8:ad:8c") }
    its( "mac_address.to_s" ) { should == "52:54:00:a8:ad:8c" }
    its( :to_s ) { should == "SetEthSrcAddr: mac_address=52:54:00:a8:ad:8c" }
  end

  context "with mac_address (0x525400a8ad8c)" do
    let( :mac_address ) { 0x525400a8ad8c }
    its( "mac_address.to_s" ) { should == "52:54:00:a8:ad:8c" }
    its( :to_s ) { should == "SetEthSrcAddr: mac_address=52:54:00:a8:ad:8c" }
  end

  context %q{with invalid mac_address ("INVALID MAC STRING")} do
    let( :mac_address ) { "INVALID MAC STRING" }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end

  context "with invalid mac_address ([1, 2, 3])" do
    let( :mac_address ) { [ 1, 2, 3 ] }
    it { expect { subject }.to raise_error( TypeError ) }
  end

  it_validates "option range", :mac_address, 0..0xffffffffffff

  context "when sending a Flow Mod with action set to SetEthSrcAddr" do
    let( :mac_address ) { "52:54:00:a8:ad:8c" }

    it "should insert a new flow with action set to mod_dl_src" do
      class TestController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( TestController ) {
        controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => subject )
        sleep 2
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[ 0 ].actions.should == "mod_dl_src:52:54:00:a8:ad:8c"
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
