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
require "trema/ip"


describe ActionSetNwSrc, ".new( VALID OPTIONS )" do
  subject { ActionSetNwSrc.new( :nw_src => IP.new( "192.168.1.1" ) ) }
  its( :nw_src ) { should be_an_instance_of Trema::IP  }
  it "should inspect its attributes" do
    subject.inspect.should == "#<Trema::ActionSetNwSrc nw_src=192.168.1.1>"
  end
  it { should respond_to( :to_i ) }
  it "should return an Integer" do
    subject.nw_src.to_i.should == 3232235777
  end
end


describe ActionSetNwSrc, ".new( MANDATORY OPTION MISSING )" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError )
  end
end


describe ActionSetNwSrc, ".new( INVALID OPTION )" do
  context "when argument type Array instead of Hash" do
    subject { ActionSetNwSrc.new( [ 1234 ] ) }
    it "should raise TypeError" do
      expect { subject }.to raise_error( TypeError )
    end
  end


  context "when nw src not a Trema::IP object" do
    subject { ActionSetNwSrc.new( :nw_src => 1234 ) }
    it "should raise TypeError" do
      expect { subject }.to raise_error( TypeError, /nw src address should be an IP object/ )
    end
  end
end


describe ActionSetNwSrc, ".new( VALID OPTION )" do
  context "when sending #flow_mod(add) with action set to mod_nw_src" do
    it "should respond to #append" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        action = ActionSetNwSrc.new( :nw_src => IP.new( "192.168.1.1" ) )
        action.should_receive( :append )
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, :actions => action )
     }
    end


    it "should have a flow with action set to mod_nw_src" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc,
          :actions => ActionSetNwSrc.new( :nw_src => IP.new( "192.168.1.1" ) ) )
        vswitch( "0xabc" ).should have( 1 ).flows
        vswitch( "0xabc" ).flows[0].actions.should match( /mod_nw_src:192.168.1.1/ )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
