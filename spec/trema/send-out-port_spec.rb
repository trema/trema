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


describe SendOutPort, :type => "actions" do
  describe "#new(port_number)" do
    subject( :send_out_port ) { SendOutPort.new( port_number ) }


    context "with port_number (1)" do
      let( :port_number ) { 1 }

      its( :port_number ) { should eq( 1 ) }
      its( :max_len ) { should eq( 2 ** 16 - 1 ) }
      its( :to_s ) { should eq( "Trema::SendOutPort: port_number=1, max_len=65535" ) }
    end


    context "with port_number (10)" do
      let( :port_number ) { 10 }

      context "when set as FlowMod's action", :sudo => true do
        it "should insert a new flow entry with action (output:10)" do
          class TestController < Controller; end
          network {
            vswitch { datapath_id 0xabc }
          }.run( TestController ) {
            controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => send_out_port )
            sleep 2
            expect( vswitch( "0xabc" ) ).to have( 1 ).flows
            expect( vswitch( "0xabc" ).flows[ 0 ].actions ).to eq( "output:10" )
            pending( "Test actions as an object using Trema::Switch" ) do
              expect( vswitch( "0xabc" ) ).to have( 1 ).flows
              expect( vswitch( "0xabc" ).flows[ 0 ] ).to have( 1 ).actions
              expect( vswitch( "0xabc" ).flows[ 0 ].actions[ 0 ] ).to be_a( SendOutPort )
              expect( vswitch( "0xabc" ).flows[ 0 ].actions[ 0 ].port_number ).to eq( 10 )
            end
          }
        end
      end
    end


    it_validates "option is within range", :port_number, 0..( 2 ** 16 - 1 )
  end


  describe "#new(:port_number => value)" do
    subject { SendOutPort.new( :port_number => port_number ) }


    context "with option (:port_number => 1)" do
      let( :port_number ) { 1 }

      its( :port_number ) { should eq( 1 ) }
      its( :max_len ) { should eq( 2 ** 16 - 1 ) }
      its( :to_s ) { should eq( "Trema::SendOutPort: port_number=1, max_len=65535" ) }
    end
  end


  describe "#new(:port_number => value1, :max_len => value2)" do
    subject { SendOutPort.new( options ) }


    context "with options (:port_number => 1, :max_len => 256)" do
      let( :options ) { { :port_number => 1, :max_len => 256 } }

      its( :port_number ) { should eq( 1 ) }
      its( :max_len ) { should eq( 256 ) }
      its( :to_s ) { should eq( "Trema::SendOutPort: port_number=1, max_len=256" ) }
    end


    context "with options (:port_number => 1, :max_len => max_len)" do
      let( :options ) { { :port_number => 1, :max_len => max_len } }

      it_validates "option is within range", :max_len, 0..( 2 ** 16 - 1 )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
