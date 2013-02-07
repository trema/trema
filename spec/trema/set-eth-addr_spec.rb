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


[ SetEthSrcAddr, SetEthDstAddr ].each do | klass |
  describe klass, "#new(mac_address)", :type => "actions" do
    subject { klass.new( mac_address ) }


    context "with mac_address (52:54:00:a8:ad:8c)" do
      let( :mac_address ) { "52:54:00:a8:ad:8c" }

      its( "mac_address.to_s" ) { should eq( "52:54:00:a8:ad:8c" ) }
      its( :to_s ) { should eq( "#{ klass.to_s }: mac_address=52:54:00:a8:ad:8c" ) }
    end


    context "with mac_address (11:22:33:44:55:66)" do
      let( :mac_address ) { "11:22:33:44:55:66" }

      context "when set as FlowMod's action", :sudo => true do
        it "should insert a new flow with action (mod_dl_{src,dst}:11:22:33:44:55:66)" do
          class TestController < Controller; end
          network {
            vswitch { datapath_id 0xabc }
          }.run( TestController ) {
            controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => subject )
            sleep 2
            expect( vswitch( "0xabc" ) ).to have( 1 ).flows
            expect( vswitch( "0xabc" ).flows[ 0 ].actions ).to match( /mod_dl_(src|dst):11:22:33:44:55:66/ )
            pending( "Test actions as an object using Trema::Switch" ) do
              expect( vswitch( "0xabc" ) ).to have( 1 ).flows
              expect( vswitch( "0xabc" ).flows[ 0 ] ).to have( 1 ).actions
              expect( vswitch( "0xabc" ).flows[ 0 ].actions[ 0 ] ).to be_a( klass )
              expect( vswitch( "0xabc" ).flows[ 0 ].actions[ 0 ].mac_address.to_s ).to eq( "11:22:33:44:55:66" )
            end
          }
        end
      end
    end


    context %q{with mac_address (Mac.new("52:54:00:a8:ad:8c"))} do
      let( :mac_address ) { Mac.new("52:54:00:a8:ad:8c") }

      its( "mac_address.to_s" ) { should eq( "52:54:00:a8:ad:8c" ) }
      its( :to_s ) { should eq( "#{ klass.to_s }: mac_address=52:54:00:a8:ad:8c" ) }
    end


    context "with mac_address (0x525400a8ad8c)" do
      let( :mac_address ) { 0x525400a8ad8c }

      its( "mac_address.to_s" ) { should eq( "52:54:00:a8:ad:8c" ) }
      its( :to_s ) { should eq( "#{ klass.to_s }: mac_address=52:54:00:a8:ad:8c" ) }
    end


    context %q{with invalid mac_address ("INVALID MAC STRING")} do
      let( :mac_address ) { "INVALID MAC STRING" }

      it { expect { subject }.to raise_error( ArgumentError ) }
    end


    context "with invalid mac_address ([1, 2, 3])" do
      let( :mac_address ) { [ 1, 2, 3 ] }

      it { expect { subject }.to raise_error( TypeError ) }
    end


    it_validates "option is within range", :mac_address, 0..0xffffffffffff
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
