#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


describe ActionVendor, ".new( value )" do
  subject  { ActionVendor.new( value ) }

  context "when 19711" do
    let( :value ) { 19711 }
    its( :value ) { should == ActionVendor.new( 19711 ).value }
  end

  context "when 1" do
    let( :value ) { 1 }
    its( :value ) { should == ActionVendor.new( 1 ).value }
  end
end


describe ActionVendor, ".new( invalid_value )" do
  subject  { ActionVendor.new( invalid_value ) }

  context "when -1" do
    let( :invalid_value ) { -1 }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end

  context %{ when "0x00004cff" } do
    let( :invalid_value ) { "0x00004cff" }
    it { expect { subject }.to raise_error( TypeError ) }
  end
end


describe ActionVendor, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_vendor" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionVendor.new( 1 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_vendor" do
      class FlowModAddController < Controller; end
      pending "ActionVendor not yet implemented"
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc,
          :actions => ActionVendor.new( 123 ) )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /mod_vendor/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
