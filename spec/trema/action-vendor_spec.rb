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


shared_examples_for "any OpenFlow message with vendor option" do
  it_should_behave_like "any OpenFlow message", :option => :vendor, :name => "Vendor id", :size => 32
end


describe ActionVendor, ".new( VALID OPTION )" do
  subject  { ActionVendor.new( :vendor => vendor ) }
  let( :vendor ) { 1 }
  its( :vendor ) { should == 1 }
  it "should inspect its attributes" do
    subject.inspect.should == "#<Trema::ActionVendor vendor=1>"
  end
  it_should_behave_like "any OpenFlow message with vendor option"
end


describe ActionVendor, ".new( MANDATORY OPTION MISSING )" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError )
  end
end


describe ActionVendor, ".new( INVALID OPTION ) - argument type Array instead of Hash" do
  subject { ActionVendor.new( [ 1 ] ) }
  it "should raise TypeError" do
    expect { subject }.to raise_error( TypeError )
  end
end


describe ActionVendor, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_vendor" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionVendor.new( :vendor => 1 )
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
