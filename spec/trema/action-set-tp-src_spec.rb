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


describe ActionSetTpSrc, ".new( value )" do
  subject { ActionSetTpSrc.new( value ) }

  context "when 5555" do
    let( :value ) { 5555 }
    its( :value ) { should == ActionSetTpSrc.new( 5555 ).value }
  end
end


describe ActionSetTpSrc, ".new( invalid_value )" do
  subject { ActionSetTpSrc.new( invalid_value ) }

  context "when 1048576" do
    let( :invalid_value ) { 1048576 }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end

  context "when -256" do
    let( :invalid_value ) { -256 }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end

  context "when 256.0" do
    let( :invalid_value ) { 256.0 }
    it { expect { subject }.to raise_error( TypeError ) }
  end

  context %{ when "5555" } do
    let( :invalid_value ) { "5555" }
    it { expect { subject }.to raise_error( TypeError ) }
  end

  context "when [ 5555 ]" do
    let( :invalid_value ) { [ 5555 ] }
    it { expect { subject }.to raise_error( TypeError ) }
  end
end


describe ActionSetTpSrc, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_tp_src" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionSetTpSrc.new( 5555 )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_tp_src" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => ActionSetTpSrc.new( 5555 ) )
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
